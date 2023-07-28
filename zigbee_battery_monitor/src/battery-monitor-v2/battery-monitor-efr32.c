 /***************************************************************************//**
* @file battery-monitor-efr32.c
* @brief An example about sampling the battery voltage through IADC(Series 2) or ADC(Series 1).
* @version v0.01
*******************************************************************************
* # License
* <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* SPDX-License-Identifier: Zlib
*
* The licensor of this software is Silicon Laboratories Inc.
*
* This software is provided \'as-is\', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
*******************************************************************************
* # Experimental Quality
* This code has not been formally tested and is provided as-is. It is not
* suitable for production environments. In addition, this code will not be
* maintained and there may be no bug maintenance planned for these resources.
* Silicon Labs may update projects from time to time.
******************************************************************************/

#include "app/framework/include/af.h"

#include EMBER_AF_API_BATTERY_MONITOR
#include EMBER_AF_API_GENERIC_INTERRUPT_CONTROL
#include "em_cmu.h"

#if defined(_SILICON_LABS_32B_SERIES_2)
#include "em_iadc.h"
#else
#include "em_adc.h"
#endif

#include "em_prs.h"

#define GPIO_PORT_A gpioPortA
#define GPIO_PORT_B gpioPortB
#define GPIO_PORT_C gpioPortC
#define GPIO_PORT_D gpioPortD

// Shorter macros for plugin options
#define FIFO_SIZE \
  EMBER_AF_PLUGIN_BATTERY_MONITOR_V2_SAMPLE_FIFO_SIZE
#define MS_BETWEEN_BATTERY_CHECK \
 (EMBER_AF_PLUGIN_BATTERY_MONITOR_V2_MONITOR_TIMEOUT_M * 60 * 1000)
#define BSP_BATTERYMON_TX_ACTIVE_CHANNEL \
    EMBER_AF_PLUGIN_BATTERY_MONITOR_V2_TX_ACTIVE_PRS_CH
#define BSP_BATTERYMON_TX_ACTIVE_PORT \
    EMBER_AF_PLUGIN_BATTERY_MONITOR_V2_TX_ACTIVE_PORT
#define BSP_BATTERYMON_TX_ACTIVE_PIN \
    EMBER_AF_PLUGIN_BATTERY_MONITOR_V2_TX_ACTIVE_PIN
#define BSP_BATTERYMON_TX_ACTIVE_LOC \
    EMBER_AF_PLUGIN_BATTERY_MONITOR_V2_TX_ACTIVE_LOC

#define MAX_INT_MINUS_DELTA              0xe0000000

#if defined(_SILICON_LABS_32B_SERIES_2)

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
#define PRS_SOURCE                       PRS_ASYNC_CH_CTRL_SOURCESEL_RAC
#define PRS_SIGNAL                       PRS_ASYNC_CH_CTRL_SIGSEL_RACTX
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2)
#define PRS_SOURCE                       PRS_ASYNC_CH_CTRL_SOURCESEL_RACL
#define PRS_SIGNAL                       PRS_ASYNC_CH_CTRL_SIGSEL_RACLTX
#else
error("please define the correct macros here!")
#endif

// Set CLK_ADC to 100kHz (this corresponds to a sample rate of 10ksps)
#define CLK_SRC_ADC_FREQ        1000000  // CLK_SRC_ADC
#define CLK_ADC_FREQ            100000   // CLK_ADC

/** Default config for IADC single input structure. */
#define IADC_SINGLEINPUT_BATTERY                             \
  {                                                           \
    iadcNegInputGnd,             /* Negative input GND */     \
    iadcPosInputAvdd,            /* Positive input iadcPosInputAvdd */     \
    0,                           /* Config 0 */               \
    false                        /* Do not compare results */ \
  }

#define IADC_REFERENCE_VOLTAGE_MILLIVOLTS 1210

#else //series 1

#define PRS_CH_CTRL_SOURCESEL_RAC_TX \
  (PRS_RAC_TX & _PRS_CH_CTRL_SOURCESEL_MASK)
#define PRS_CH_CTRL_SIGSEL_RAC_TX \
  (PRS_RAC_TX & _PRS_CH_CTRL_SIGSEL_MASK)

// Default settings to be used when configured the PRS to cause an external pin
// to emulate TX_ACTIVE functionality
#define PRS_SOURCE                       PRS_CH_CTRL_SOURCESEL_RAC_TX
#define PRS_SIGNAL                       PRS_CH_CTRL_SIGSEL_RAC_TX
#define PRS_EDGE                         prsEdgeOff
#define PRS_PIN_SHIFT                    (8                                   \
                                          * (BSP_BATTERYMON_TX_ACTIVE_CHANNEL \
                                             % 4))
#define PRS_PIN_MASK                     (0x1F << PRS_PIN_SHIFT)

#if BSP_BATTERYMON_TX_ACTIVE_CHANNEL < 4
#define PRS_ROUTE_LOC                    ROUTELOC0
#elif BSP_BATTERYMON_TX_ACTIVE_CHANNEL < 8
#define PRS_ROUTE_LOC                    ROUTELOC1
#else
#define PRS_ROUTE_LOC                    ROUTELOC2
#endif

// Default settings used to configured the ADC to read the battery voltage
#define ADC_INITSINGLE_BATTERY_VOLTAGE                                \
  {                                                                   \
    adcPRSSELCh0, /* PRS ch0 (if enabled). */                         \
    adcAcqTime16, /* 1 ADC_CLK cycle acquisition time. */             \
    adcRef5VDIFF, /* V internal reference. */                         \
    adcRes12Bit, /* 12 bit resolution. */                             \
    adcPosSelAVDD, /* Select Vdd as posSel */                         \
    adcNegSelVSS, /* Select Vss as negSel */                          \
    false,       /* Single ended input. */                            \
    false,       /* PRS disabled. */                                  \
    false,       /* Right adjust. */                                  \
    false,       /* Deactivate conversion after one scan sequence. */ \
    false,       /* No EM2 DMA wakeup from single FIFO DVL */         \
    false        /* Discard new data on full FIFO. */                 \
  }

#define ADC_REFERENCE_VOLTAGE_MILLIVOLTS 5000

#endif

// ------------------------------------------------------------------------------
// Forward Declaration
static uint16_t filterVoltageSample(uint16_t sample);
static uint32_t AdcToMilliV(uint32_t adcVal);

// ------------------------------------------------------------------------------
// Globals

EmberEventControl emberAfPluginBatteryMonitorV2ReadADCEventControl;

// structure used to store irq configuration from GIC plugin
static HalGenericInterruptControlIrqCfg *irqConfig;

// count used to track when the last measurement occurred
// Ticks start at 0.  We use this value to limit how frequently we make
// measurements in an effort to conserve battery power.  By setting this to an
// arbitrary value close to MAX_INT, we are going to make sure we make a
// battery measurement on the first transmission.
static uint32_t lastBatteryMeasureTick = MAX_INT_MINUS_DELTA;

// sample FIFO access variables
static uint8_t samplePtr = 0;
static uint16_t voltageFifo[FIFO_SIZE];
static bool fifoInitialized = false;

// Remember the last reported voltage value from callback, which will be the
// return value if anyone needs to manually poll for data
static uint16_t lastReportedVoltageMilliV;

// ------------------------------------------------------------------------------
// Implementation of public functions

void emberAfPluginBatteryMonitorV2InitCallback(void)
{
  halBatteryMonitorInitialize();
}

void halBatteryMonitorInitialize(void)
{
  uint32_t flags;

  #if defined(_SILICON_LABS_32B_SERIES_2)
  IADC_Init_t        init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t  initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitSingle_t  initSingle = IADC_INITSINGLE_DEFAULT;
  IADC_SingleInput_t initSingleInput = IADC_SINGLEINPUT_BATTERY;
  
  
  CMU_ClockEnable(cmuClock_IADC0, true);

  IADC_reset(IADC0);  

  CMU_ClockSelectSet(cmuClock_IADCCLK, cmuSelect_FSRCO); // 20MHz

  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0); 

  initAllConfigs.configs[0].reference = iadcCfgReferenceInt1V2;
  
  // Divides CLK_SRC_ADC to set the CLK_ADC frequency
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                                                     CLK_ADC_FREQ,
                                                                     0,
                                                                     iadcCfgModeNormal,
                                                                     init.srcClkPrescale);
  IADC_init(IADC0, &init, &initAllConfigs);

  initSingle.dataValidLevel = IADC_SCANFIFOCFG_DVL_VALID4;
  IADC_initSingle(IADC0, &initSingle, &initSingleInput);

  CMU_ClockEnable(cmuClock_PRS, true);

  // Initialize the PRS system to drive a GPIO high when the preamble is in the
  // air, effectively becoming a TX_ACT pin
  PRS_SourceAsyncSignalSet(BSP_BATTERYMON_TX_ACTIVE_CHANNEL,
                           PRS_SOURCE,
                           PRS_SIGNAL);
  PRS_PinOutput(BSP_BATTERYMON_TX_ACTIVE_CHANNEL, 
                prsTypeAsync, 
                BSP_BATTERYMON_TX_ACTIVE_PORT, 
                BSP_BATTERYMON_TX_ACTIVE_PIN);
  GPIO_PinModeSet(BSP_BATTERYMON_TX_ACTIVE_PORT,
                  BSP_BATTERYMON_TX_ACTIVE_PIN,
                  gpioModePushPull,
                  0);  

  #else //series 1
  
  ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
  ADC_InitSingle_TypeDef initAdc = ADC_INITSINGLE_BATTERY_VOLTAGE;

  // Enable ADC clock
  CMU_ClockEnable(cmuClock_ADC0, true);

  // Initialize the ADC peripheral
  ADC_Init(ADC0, &init);

  // Setup ADC for single conversions for reading AVDD with a 5V reference
  ADC_InitSingle(ADC0, &initAdc);

  flags = ADC_IntGet(ADC0);
  ADC_IntClear(ADC0, flags);
  ADC_Start(ADC0, adcStartSingle);

  CMU_ClockEnable(cmuClock_PRS, true);

  // Initialize the PRS system to drive a GPIO high when the preamble is in the
  // air, effectively becoming a TX_ACT pin
  PRS_SourceSignalSet(BSP_BATTERYMON_TX_ACTIVE_CHANNEL,
                      PRS_SOURCE,
                      PRS_SIGNAL,
                      PRS_EDGE);

  // Enable the PRS channel and set the pin routing per the settings in the
  // board configuration header
  BUS_RegMaskedSet(&PRS->ROUTEPEN, (1 << BSP_BATTERYMON_TX_ACTIVE_CHANNEL));
  PRS->PRS_ROUTE_LOC = PRS->PRS_ROUTE_LOC & ~PRS_PIN_MASK;
  PRS->PRS_ROUTE_LOC = PRS->PRS_ROUTE_LOC
                       | (BSP_BATTERYMON_TX_ACTIVE_LOC
                          << PRS_PIN_SHIFT);
  GPIO_PinModeSet(BSP_BATTERYMON_TX_ACTIVE_PORT,
                  BSP_BATTERYMON_TX_ACTIVE_PIN,
                  gpioModePushPull,
                  0);

  #endif

  // Set up the generic interrupt controller to activate the readADC event when
  // TX_ACTIVE goes hi
  irqConfig = halGenericInterruptControlIrqCfgInitialize(
    BSP_BATTERYMON_TX_ACTIVE_PIN,
    BSP_BATTERYMON_TX_ACTIVE_PORT,
    0);
  halGenericInterruptControlIrqEventRegister(
    irqConfig,
    &emberAfPluginBatteryMonitorV2ReadADCEventControl);
  halGenericInterruptControlIrqEdgeConfig(irqConfig,
                                          HAL_GIC_INT_CFG_LEVEL_POS);

  halGenericInterruptControlIrqEnable(irqConfig);
}

uint16_t halGetBatteryVoltageMilliV(void)
{
  return lastReportedVoltageMilliV;
}

static uint32_t halBatteryMonitorReadVoltage()
{
  uint32_t milliV = 0;
    
  #if defined(_SILICON_LABS_32B_SERIES_2)
  uint32_t vMax = 0xFFF; // 12 bit ADC maximum
  uint32_t referenceMilliV = IADC_REFERENCE_VOLTAGE_MILLIVOLTS;
  float milliVPerBit = (float)referenceMilliV / (float)vMax;

  IADC_InitSingle_t  initSingle = IADC_INITSINGLE_DEFAULT;
  IADC_SingleInput_t initSingleInput = IADC_SINGLEINPUT_BATTERY;

  initSingle.dataValidLevel = IADC_SCANFIFOCFG_DVL_VALID4;
  IADC_initSingle(IADC0, &initSingle, &initSingleInput);  
  
  // Start IADC conversion
  IADC_command(IADC0, iadcCmdStartSingle);

  // Wait for conversion to be complete
  while((IADC0->STATUS & (_IADC_STATUS_CONVERTING_MASK
              | _IADC_STATUS_SINGLEFIFODV_MASK)) != IADC_STATUS_SINGLEFIFODV); //while combined status bits 8 & 6 don't equal 1 and 0 respectively

  // Get IADC result
  IADC_Result_t sample = IADC_readSingleResult(IADC0);
  
  milliV = (uint32_t)(milliVPerBit * sample.data) * 4; //refer to RM 23.3.5.2 to understand the factor 4.
  emberAfAppPrintln("IADC sample: %d, milliV=%lu", sample.data, milliV);

  #else //series 1

  uint32_t flags;
  uint32_t vData;
  uint32_t vMax = 0xFFF; // 12 bit ADC maximum
  uint32_t referenceMilliV = ADC_REFERENCE_VOLTAGE_MILLIVOLTS;
  float milliVPerBit = (float)referenceMilliV / (float)vMax;
  ADC_InitSingle_TypeDef initAdc = ADC_INITSINGLE_BATTERY_VOLTAGE;
  
  // In case something else in the system was using the ADC, reconfigure it to
  // properly sample the battery voltage
  ADC_InitSingle(ADC0, &initAdc);  

  // The most common and shortest (other than the ACK) transmission is the
  // data poll.  It takes 512 uS for a data poll, which is plenty of time for
  // a 16 cycle conversion
  flags = ADC_IntGet(ADC0);
  ADC_IntClear(ADC0, flags);
  ADC_Start(ADC0, adcStartSingle);

  // wait for the ADC to finish sampling
  while ((ADC_IntGet(ADC0) & ADC_IF_SINGLE) != ADC_IF_SINGLE) {
  }
  vData = ADC_DataSingleGet(ADC0); 

  milliV = (uint32_t)(milliVPerBit * vData);
  emberAfAppPrintln("ADC sample: %d, milliV=%lu", vData, milliV);
  #endif    

  return milliV;
}

SL_WEAK void emberAfPluginBatteryMonitorDataReadyCallback(uint16_t batteryVoltageMilliV)
{

}

// This event will sample the ADC during a radio transmission and notify any
// interested parties of a new valid battery voltage level via the
// emberAfPluginBatteryMonitorDataReadyCallback
void emberAfPluginBatteryMonitorV2ReadADCEventHandler(void)
{
  uint32_t flags;
  uint32_t vData;
  uint16_t voltageMilliV;
  uint32_t currentMsTick = halCommonGetInt32uMillisecondTick();
  uint32_t timeSinceLastMeasureMS = currentMsTick - lastBatteryMeasureTick;

  emberEventControlSetInactive(emberAfPluginBatteryMonitorV2ReadADCEventControl);

  if (timeSinceLastMeasureMS >= MS_BETWEEN_BATTERY_CHECK) {
    voltageMilliV = halBatteryMonitorReadVoltage();

    // filter the voltage to prevent spikes from overly influencing data
    voltageMilliV = filterVoltageSample(voltageMilliV);

    emberAfPluginBatteryMonitorDataReadyCallback(voltageMilliV);
    lastReportedVoltageMilliV = voltageMilliV;
    lastBatteryMeasureTick = currentMsTick;
  }
}

// Provide smoothing of the voltage readings by reporting an average over the
// last few values
static uint16_t filterVoltageSample(uint16_t sample)
{
  uint32_t voltageSum;
  uint8_t i;

  if (fifoInitialized) {
    voltageFifo[samplePtr++] = sample;

    if (samplePtr >= FIFO_SIZE) {
      samplePtr = 0;
    }
    voltageSum = 0;
    for (i = 0; i < FIFO_SIZE; i++) {
      voltageSum += voltageFifo[i];
    }
    sample = voltageSum / FIFO_SIZE;
  } else {
    for (i = 0; i < FIFO_SIZE; i++) {
      voltageFifo[i] = sample;
    }
    fifoInitialized = true;
  }

  return sample;
}

void halSleepCallback(boolean enter, SleepModes sleepMode)
{
  if (sleepMode < SLEEPMODE_WAKETIMER) {
    return;
  }

  #if defined(_SILICON_LABS_32B_SERIES_2)
  if (enter) {
    IADC0->EN_CLR = IADC_EN_EN;
  } else {
      IADC0->EN_SET = IADC_EN_EN;
  }
  #endif
}

