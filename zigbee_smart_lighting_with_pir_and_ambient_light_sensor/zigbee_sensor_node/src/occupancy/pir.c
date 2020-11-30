/***************************************************************************//**
 * @file
 * @brief PIR demonstration code for OCCUPANCY-EXP-EB
 * @version 1.0.2
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <stdbool.h>
#include <stdio.h>

#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_prs.h"

#include "em_cryotimer.h"

#include "em_opamp.h"
#include "em_adc.h"
#include "em_letimer.h"

#include "occupancy_config.h"
#include "pir.h"

#include "app/framework/include/af.h"


/**
 * @Local function
 *
*/
static void initial(PIR_Init_TypeDef *init, bool enter_em2);
static void init_adc(bool enter_em2);
static void init_non_inverting(void);
static void update_thresholds(int32_t enter_win_base, uint32_t enter_win_size);
static bool detect_motion();
static void motion_off();
static void motion_on();

/**
 * @ Local variable
 */
static PIR_Init_TypeDef config;    /* Configuration of the detection algorithm. */
static bool motion_detected = false; /* Output of PIR_DetectMotion. */
static uint32_t lock_out_counter = 0; /* The time, in samples, remaining on the lockout window after detection. */
static int32_t win_base = 0;         /* The low frequency mean of the PIR input signal. */

/* The adcSource variable should be set to adcSourceAdcDiff for PIR operation.
 * The single ended modes are additional modes for signal chain analysis with a voltage source. */
static AdcSource_Typedef_t adc_source = adcSourceAdcDiff;

/* This pointer use for call the callback when the sensor detected a motion
 */
static motion_detection_callback_t motion_detection_callback;

/* As a rule, this module operates with 16-bit values for sampled data. The only functions that should
 * shift data are functions that directly interact with the ADC registers and are required to transform
 * the values to a shifted variant due to oversampling or lack there of.
 */

/**
 * @brief init function for PIR occupancy sensing.
 *
 * @param[in] register_callback
 * register a callback from application
 */
void pir_init(motion_detection_callback_t callback_registration)
{
  /* Enable LDO to startup VPIR. */
  GPIO_PinOutSet(LDO_SHDN_B_PORT, LDO_SHDN_B_PIN);

  PIR_Init_TypeDef pirInit = PIR_INIT_DEFAULT;
  pirInit.opampMode = pirOpampModeExternal;

  initial(&pirInit, true);

  // set callback function for motion detection event
  motion_detection_callback = callback_registration;

  pir_start();
}

/**
 * @brief Initializes peripherals for PIR.
 */
void initial(PIR_Init_TypeDef *init, bool enter_em2)
{
  config.motionOnTime = init->motionOnTime;
  config.opampMode = init->opampMode;
  config.winSize = init->winSize;

  /* Analog initialization */
  if (init->opampMode == pirOpampModeInternal) {
      init_non_inverting();
  }
  init_adc(enter_em2);

  /* Initialize LED. */
  motion_off();
}

/**
 * @brief
 *   Begins running the motion detection sampling and algorithm.
 */
void pir_start(void)
{
  lock_out_counter = 0;
  ADC_Start(ADC0, adcStartSingle);
  ADC_IntEnable(ADC0, ADC_IF_SINGLE);         /* Wake up on FIFO reaching threshold. */
  ADC_IntEnable(ADC0, ADC_IF_SINGLECMP);      /* Wake up on ADC exceeding window threshold. */
  NVIC_EnableIRQ(ADC0_IRQn);
}

/**
 * @brief
 *   stop running the motion detection sampling and algorithm.
 */
void pir_stop(void)
{
  ADC_IntDisable(ADC0, ADC_IF_SINGLE);
  ADC_IntDisable(ADC0, ADC_IF_SINGLECMP);
  NVIC_DisableIRQ(ADC0_IRQn);
}

/**
 * @brief
 *  Initializes the non inverting amplifier for PIR signal chain.
 */
static void init_non_inverting(void)
{
  PIR_Opamp_Port_t port = pirOpampMainPorts;
  CMU_ClockEnable(cmuClock_VDAC0, true);

  /** Errata VDAC_E201 for PG/JG/MG/BG12 causes contention if APORT is used for output.
   *  Use of dedicated opamp output pin or careful mapping of APORT buses is recommended.
   */
  OPAMP_Init_TypeDef opaInit = OPA_INIT_NON_INVERTING;
  opaInit.resInMux = opaResInMuxDisable;
  opaInit.resSel = opaResSelDefault;

  if (port == pirOpampMainPorts) {
    opaInit.posSel = opaPosSelPosPad;
    opaInit.negSel = opaNegSelNegPad;
    opaInit.outMode = opaOutModeDisable;
    opaInit.outPen = VDAC_OPA_OUT_MAINOUTEN;
  } else {
    /* These APORT mappings match the main predefined analog paths. */
    opaInit.posSel = opaPosSelAPORT4XCH5;
    opaInit.negSel = opaNegSelAPORT3YCH7;
    opaInit.outMode = opaOutModeDisable;
    opaInit.outPen = VDAC_OPA_OUT_MAINOUTEN;  /* Workaround for VDAC_E201, use dedicate opamp output pin. */
  }

  const uint32_t opampCh = 1;
  OPAMP_Enable(VDAC0, OPA1, &opaInit);

  /* Set INCBW for 2.5x increase in GBW, only stable for G > 3. */
  VDAC0->OPA[opampCh].CTRL |= VDAC_OPA_CTRL_INCBW;

  /* Set DRIVESTRENGTH=0 to minimize current. */
  VDAC0->OPA[opampCh].CTRL &= ~_VDAC_OPA_CTRL_DRIVESTRENGTH_MASK;
  VDAC0->OPA[opampCh].CTRL |= (0 << _VDAC_OPA_CTRL_DRIVESTRENGTH_SHIFT);
}

/**
 * @brief
 *  Charges up the slow charging capacitor by bypassing the opamp feedback network.
 *
 *  Not used in the OCCUPANCY-EXP-EVB as the R_f * C_g settle time is short enough.
 *  May be used if R_f * C_g is increased further.
 *
 * @param[in] pirSettleTimeMs
 *  The time in milliseconds to short the feedback network with a unity gain buffer.
 */
void pir_settle_capacitor(uint32_t pir_settle_time_ms)
{
  CMU_ClockEnable(cmuClock_VDAC0, true);
  /* WARNING: See errata VDAC_E201 for PG/MG/BG12.
   * VDAC will drive output on all AY, BY, CY, DY APORTs. The ADC_P
   * should placed on an X port, and the output placed on a Y port. */
  OPAMP_Init_TypeDef opaInit = OPA_INIT_UNITY_GAIN;
  opaInit.posSel = ADC_P_OPA_APORT;
  opaInit.outMode = ADC_N_OPA_OUTMODE;
  opaInit.outPen = VDAC_OPA_OUT_APORTOUTEN;

  OPAMP_Enable(VDAC0, OPA0, &opaInit);

  /* convert Ms to Us */
  USTIMER_DelayIntSafe(pir_settle_time_ms*1000);

  /* Clear all APORT requests to avoid APORT contention with ADC. */
  VDAC0->OPA[0].OUT = 0;
  VDAC0->OPA[0].MUX = (VDAC_OPA_MUX_POSSEL_DISABLE << _VDAC_OPA_MUX_POSSEL_SHIFT)
                      | (VDAC_OPA_MUX_NEGSEL_DISABLE << _VDAC_OPA_MUX_NEGSEL_SHIFT);
  OPAMP_Disable(VDAC0, OPA0);
  CMU_ClockEnable(cmuClock_VDAC0, false);
}

/**
 * @brief
 *  Initializes the ADC, CRYOTIMER and PRS for PIR operation.
 *
 * @details
 *   PRS triggering of the ADC in EM2 is only possible on > Series 1 MCUs (PG/JG/BG/MG).
 *
 * @param[in] lowPowerMode
 *  Sets up ADC to run in EM2 when set to true.
 */
static void init_adc(bool enter_em2)
{
  /* Initialize the ADC. */
  CMU_ClockEnable(cmuClock_ADC0, true);

  ADC_Init_TypeDef adcInit = ADC_INIT_DEFAULT;
  adcInit.ovsRateSel = adcOvsRateSel2;

  if (enter_em2) {
    adcInit.em2ClockConfig = adcEm2ClockOnDemand;
    adcInit.prescale = ADC_PrescaleCalc(cmuAUXHFRCOFreq_38M0Hz, CMU_AUXHFRCOBandGet());
    adcInit.timebase = ADC_TimebaseCalc(CMU_AUXHFRCOBandGet());
  } else {
    adcInit.timebase = ADC_TimebaseCalc(0);
  }

  ADC_InitSingle_TypeDef adcInitSingle = ADC_INITSINGLE_DEFAULT;
  adcInitSingle.resolution = adcResOVS;
  adcInitSingle.reference = adcRef1V25;
  adcInitSingle.prsEnable = true;
  adcInitSingle.prsSel = adcPRSSELCh0;
  adcInitSingle.leftAdjust = true;
  adcInitSingle.acqTime = adcAcqTime64;


  switch (adc_source) {
    case adcSourceAdcP:
      adcInitSingle.posSel = ADC_P_APORT;
      adcInitSingle.negSel = adcNegSelVSS;
      adcInitSingle.diff = false;
      break;
    case adcSourceAdcN:
      /* The posSel of the ADC_N is not defined for the kit, only the negSel. */
      adcInitSingle.posSel = adcPosSelAPORT3XCH14;
      adcInitSingle.negSel = adcNegSelVSS;
      adcInitSingle.diff = false;
      break;
    case adcSourceAdcDiff:
      adcInitSingle.posSel = ADC_P_APORT;
      adcInitSingle.negSel = ADC_N_APORT;
      adcInitSingle.diff = true;
      break;
  }

  if (enter_em2) {
    CMU_ClockEnable(cmuClock_AUX, true);
    CMU_AUXHFRCOBandSet(cmuAUXHFRCOFreq_38M0Hz);
    CMU->ADCCTRL = CMU_ADCCTRL_ADC0CLKSEL_AUXHFRCO;
  }

  ADC_Init(ADC0, &adcInit);
  ADC_InitSingle(ADC0, &adcInitSingle);

  /* Set ADC FIFO level to max to minimize EM0 wakeups when the ADC sample remains within window. */
  static const uint32_t dataValidLevel = 3; /* ADC SINGLE IRQ is when DVL+1 single channels are available in the FIFO. */
  ADC0->SINGLECTRLX &= ~_ADC_SINGLECTRLX_DVL_MASK;
  ADC0->SINGLECTRLX |= (dataValidLevel << _ADC_SINGLECTRLX_DVL_SHIFT);

  update_thresholds(win_base, config.winSize);

  /****************************************************************************
  ************************ CRYOTIMER Initialization **************************
  ****************************************************************************/
  CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);
  CMU_ClockEnable(cmuClock_CRYOTIMER, true);

  /* Run CRYOTIMER on ULFRCO (1024 Hz) and trigger ADC to set the sampling period. */
  CRYOTIMER_Init_TypeDef cryoInit = CRYOTIMER_INIT_DEFAULT;
  cryoInit.osc = cryotimerOscULFRCO;
  cryoInit.presc = cryotimerPresc_1;
  cryoInit.period = cryotimerPeriod_32; // Sampling frequency is 1024 / 32 = 32 Hz.
  CRYOTIMER_Init(&cryoInit);

  /****************************************************************************
  *************************** PRS Initialization *****************************
  ****************************************************************************/
  /* PRS is the triggering connection between CRYOTIMER and ADC. */
  CMU_ClockEnable(cmuClock_PRS, true);
  PRS_SourceAsyncSignalSet(0, PRS_CH_CTRL_SOURCESEL_CRYOTIMER, PRS_CH_CTRL_SIGSEL_CRYOTIMERPERIOD);
}

/**
 * @brief
 *  Updates the ADC hardware thresholds for a motion event trigger.
 *
 *  Inputs must mast the conversion data representation. For oversampled
 *  conversions, the resolution is 16-bit, regardless if the OVS setting
 *  does not achieve 16-bit resolution.
 *
 * @param[in] win_base
 *  The midpoint of the window. Thresholds are the base +/- 0.5*winSize
 *
 * @param[in] winSize
 *  The total peak to peak width of the window.
 */
static void update_thresholds(int32_t enter_win_base, uint32_t enter_window_size)
{
  int32_t posThresh = enter_win_base + enter_window_size / 2;
  int32_t negThresh = enter_win_base - enter_window_size / 2;

  /* Ensure thresholds are within int16_t for ADC window comparator. */
  int32_t posThreshMax, negThreshMin;
  if (adc_source == adcSourceAdcDiff) {
    posThreshMax = 32767;
    negThreshMin = -32768;
  } else {
    posThreshMax = 65535;
    negThreshMin = 0;
  }

  if (posThresh > posThreshMax) {
    posThresh = posThreshMax;
  }
  if (negThresh < negThreshMin) {
    negThresh = negThreshMin;
  }

  ADC0->CMPTHR = ((posThresh << _ADC_CMPTHR_ADGT_SHIFT) & _ADC_CMPTHR_ADGT_MASK)
                 | ((negThresh << _ADC_CMPTHR_ADLT_SHIFT) & _ADC_CMPTHR_ADLT_MASK);
}

/**
 * @brief
 *   Runs the motion detection algorithm on the latest sample.
 *
 * @param[in] lowPowerMode
 *    Transmits the motion detection results over UART if lowPowerMode is false.
 *
 * @return
 *   Motion detected status. 0 = no motion, 1 = motion
 */
bool detect_motion()
{
  bool motion = false;
  uint32_t num_samples = 0;
  int32_t adcSample;

  bool dataValid = ADC0->SINGLEFIFOCOUNT > 0;
  int32_t adcThreshHigh, adcThreshLow;

  /* Read out each ADC sample, update the LPF mean, and check if it was within the window thresholds. */
  while (dataValid) {

    num_samples++;
    adcSample = ADC_DataSingleGet(ADC0) << 3; // adjust this with oversampling for OSR < 64. Need to left shift to get to 16b.
    /* Capture the ADC and win_base of the current sample. */
    if (adc_source == adcSourceAdcDiff) {
      adcThreshHigh = ((int16_t) ((ADC0->CMPTHR & _ADC_CMPTHR_ADGT_MASK) >> _ADC_CMPTHR_ADGT_SHIFT));
      adcThreshLow = ((int16_t) ((ADC0->CMPTHR & _ADC_CMPTHR_ADLT_MASK) >> _ADC_CMPTHR_ADLT_SHIFT));
    } else {
      adcThreshHigh = ((uint16_t) ((ADC0->CMPTHR & _ADC_CMPTHR_ADGT_MASK) >> _ADC_CMPTHR_ADGT_SHIFT));
      adcThreshLow = ((uint16_t) ((ADC0->CMPTHR & _ADC_CMPTHR_ADLT_MASK) >> _ADC_CMPTHR_ADLT_SHIFT));
    }

    /* If window was broken, move window thresholds to include the latest ADC reading.
     * Thresholds are recalculated in software because samples are batch processed. */
    if (adcSample > adcThreshHigh) {
      win_base = adcSample - config.winSize / 2;
      motion = true;
    } else if (adcSample < adcThreshLow) {
      win_base = adcSample + config.winSize / 2;
      motion = true;
    } else {
      /* Window was not broken, update win_base to follow the low frequency drift using a DT 1st order LPF.
       * Let a = 2^-a_shift
       * Equivalent continuous RC is given by Ts * (1-a) / a */
      uint32_t a_shift = 5;
      win_base = (adcSample >> a_shift) + (win_base - (win_base >> a_shift));
    }

    update_thresholds(win_base, config.winSize);
    dataValid = ADC0->SINGLEFIFOCOUNT > 0;
  } // while (dataValid)

  if (lock_out_counter > 0) {
    lock_out_counter -= (num_samples < lock_out_counter) ? num_samples : lock_out_counter;
    if (lock_out_counter == 0) {
        motion_off();
        (*motion_detection_callback)(false);
    }
  }

  /* Assert motion. */
  if (motion) {
    lock_out_counter = config.motionOnTime;
    motion_on();
    (*motion_detection_callback)(true);
  }

  return lock_out_counter > 0;
}

/**
 * @brief
 *  ADC interrupt handler to call PIR detection algorithm
 */
void ADC0_IRQHandler(void)
{
  uint32_t flags;
  flags = ADC_IntGetEnabled(ADC0);
  ADC_IntClear(ADC0, flags);
  NVIC_ClearPendingIRQ(ADC0_IRQn);
  motion_detected = detect_motion();
}

/**
 * @brief Turns on the LED to indicate motion.
 *
 */
void motion_on()
{
  GPIO_PinOutClear(MOTION_B_PORT, MOTION_B_PIN);
}

/**
 * @brief Turns off the LED to indicate end of motion.
 *
 */
void motion_off()
{
  GPIO_PinOutSet(MOTION_B_PORT, MOTION_B_PIN);
}

