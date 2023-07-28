/***************************************************************************//**
 * @file
 * @brief PIR driver.
 * @version 1.0.2
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_prs.h"
#include "em_cryotimer.h"
#include "em_opamp.h"
#include "em_adc.h"
#include "em_letimer.h"

#include "pir.h"
#include "pir_config.h"
#include <stdbool.h>
#include <stdio.h>

#define ADC_SAMPLE_QUEUE_SIZE    4         // Must be at least 4

/// PIR sample queue structure
typedef struct {
	volatile uint16_t head;                  ///< Index of next byte to get.
	volatile uint16_t tail;                  ///< Index of where to enqueue next byte.
	volatile uint16_t used;                  ///< Number of bytes queued.
	uint16_t size;                           ///< Size of queue.
	pir_sample_t *sample;                    ///< Pointer to PIR sample buffer.
} sample_queue_t;

static pir_init_t pir_instance;      // An instance that holds the configuration
static uint32_t lockout_counter = 0; // The time, in samples, remaining on the lockout window after detection.
static int32_t win_base = 0;         // The low frequency mean of the PIR input signal.

/// The adcSource variable should be set to adcSourceAdcDiff for PIR operation.
/// The single ended modes are additional modes for signal chain analysis with a voltage source.
static pir_adc_source_t adc_source = pir_adc_source_diff;

static sample_queue_t app_queue;   // Buffer PIR samples for application layer debugging.
static sample_queue_t adc_queue;   // Buffer ADC samples for motion detection algorithm.
static pir_sample_t adc_sample_queue[ADC_SAMPLE_QUEUE_SIZE]; // ADC sample buffer.

/***************************************************************************//**
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
 * @param[in] win_size
 *  The total peak to peak width of the window.
 ******************************************************************************/
static void update_adc_thresholds(int32_t win_base, uint32_t win_size)
{
	int32_t pos_thresh = win_base + win_size / 2;
	int32_t neg_thresh = win_base - win_size / 2;

	// Ensure thresholds are within int16_t for ADC window comparator.
	int32_t pos_thresh_max, neg_thresh_min;
	if (adc_source == pir_adc_source_diff) {
		pos_thresh_max = 32767;
		neg_thresh_min = -32768;
	} else {
		pos_thresh_max = 65535;
		neg_thresh_min = 0;
	}

	if (pos_thresh > pos_thresh_max) {
		pos_thresh = pos_thresh_max;
	}
	if (neg_thresh < neg_thresh_min) {
		neg_thresh = neg_thresh_min;
	}

	ADC0->CMPTHR = ((pos_thresh << _ADC_CMPTHR_ADGT_SHIFT) & _ADC_CMPTHR_ADGT_MASK)
                		 | ((neg_thresh << _ADC_CMPTHR_ADLT_SHIFT) & _ADC_CMPTHR_ADLT_MASK);
}

/***************************************************************************//**
 * @brief
 *  Initializes the non inverting amplifier for PIR signal chain.
 ******************************************************************************/
static void init_opamp(void)
{
	pir_opamp_port_t port = pir_opamp_main_ports;
	CMU_ClockEnable(cmuClock_VDAC0, true);

	// Errata VDAC_E201 for PG/JG/MG/BG12 causes contention if APORT is used for output.
	// Use of dedicated opamp output pin or careful mapping of APORT buses is recommended.
	OPAMP_Init_TypeDef opaInit = OPA_INIT_NON_INVERTING;
	opaInit.resInMux = opaResInMuxDisable;
	opaInit.resSel = opaResSelDefault;

	if (port == pir_opamp_main_ports) {
		opaInit.posSel = opaPosSelPosPad;
		opaInit.negSel = opaNegSelNegPad;
		opaInit.outMode = opaOutModeDisable;
		opaInit.outPen = VDAC_OPA_OUT_MAINOUTEN;
	} else {
		// These APORT mappings match the main predefined analog paths.
		opaInit.posSel = opaPosSelAPORT4XCH5;
		opaInit.negSel = opaNegSelAPORT3YCH7;
		opaInit.outMode = opaOutModeDisable;
		opaInit.outPen = VDAC_OPA_OUT_MAINOUTEN;
	}

	const uint32_t opampCh = 1;
	OPAMP_Enable(VDAC0, OPA1, &opaInit);

	// Set INCBW for 2.5x increase in GBW, only stable for G > 3.
	VDAC0->OPA[opampCh].CTRL |= VDAC_OPA_CTRL_INCBW;

	// Set DRIVESTRENGTH=0 to minimize current.
	VDAC0->OPA[opampCh].CTRL &= ~_VDAC_OPA_CTRL_DRIVESTRENGTH_MASK;
	VDAC0->OPA[opampCh].CTRL |= (0 << _VDAC_OPA_CTRL_DRIVESTRENGTH_SHIFT);
}

/***************************************************************************//**
 * @brief
 *  Initializes the ADC, CRYOTIMER and PRS for PIR operation.
 *
 * @details
 *   PRS triggering of the ADC in EM2 is only possible on > Series 1 MCUs.
 *
 * @param[in] adc_enter_em2
 *  Sets up ADC to run in EM2 when set to true.
 ******************************************************************************/
static void init_adc(bool adc_enter_em2)
{
	// Initialize the ADC.
	CMU_ClockEnable(cmuClock_ADC0, true);

	ADC_Init_TypeDef adcInit = ADC_INIT_DEFAULT;
	adcInit.ovsRateSel = adcOvsRateSel2;

	if (adc_enter_em2) {
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
	case pir_adc_source_pos:
		adcInitSingle.posSel = ADC_P_APORT;
		adcInitSingle.negSel = adcNegSelVSS;
		adcInitSingle.diff = false;
		break;
	case pir_adc_source_neg:
		// The posSel of the ADC_N is not defined for the kit, only the negSel.
		adcInitSingle.posSel = adcPosSelDEFAULT;
		adcInitSingle.negSel = adcNegSelVSS;
		adcInitSingle.diff = false;
		break;
	case pir_adc_source_diff:
		adcInitSingle.posSel = ADC_P_APORT;
		adcInitSingle.negSel = ADC_N_APORT;
		adcInitSingle.diff = true;
		break;
	}

	if (adc_enter_em2) {
		CMU_ClockEnable(cmuClock_AUX, true);
		CMU_AUXHFRCOBandSet(cmuAUXHFRCOFreq_38M0Hz);
		CMU->ADCCTRL = CMU_ADCCTRL_ADC0CLKSEL_AUXHFRCO;
	}

	ADC_Init(ADC0, &adcInit);
	ADC_InitSingle(ADC0, &adcInitSingle);

	// Set ADC FIFO level to max to minimize EM0 wakeups when the ADC sample remains within window.
	static const uint32_t dataValidLevel = 3; // ADC SINGLE IRQ is when DVL+1 single channels are available in the FIFO.
	ADC0->SINGLECTRLX &= ~_ADC_SINGLECTRLX_DVL_MASK;
	ADC0->SINGLECTRLX |= (dataValidLevel << _ADC_SINGLECTRLX_DVL_SHIFT);

	update_adc_thresholds(win_base, pir_instance.win_size);

	// Initialize the CRYOTIMER.
	CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);
	CMU_ClockEnable(cmuClock_CRYOTIMER, true);

	// Run CRYOTIMER on ULFRCO (1024 Hz) and trigger ADC to set the sampling period.
	CRYOTIMER_Init_TypeDef cryoInit = CRYOTIMER_INIT_DEFAULT;
	cryoInit.osc = cryotimerOscULFRCO;
	cryoInit.presc = cryotimerPresc_1;
	cryoInit.period = cryotimerPeriod_32; // Sampling frequency is 1024 / 32 = 32 Hz.
	CRYOTIMER_Init(&cryoInit);

	// PRS is the triggering connection between CRYOTIMER and ADC.
	CMU_ClockEnable(cmuClock_PRS, true);
	PRS_SourceAsyncSignalSet(0, PRS_CH_CTRL_SOURCESEL_CRYOTIMER, PRS_CH_CTRL_SIGSEL_CRYOTIMERPERIOD);
}

/***************************************************************************//**
 * @brief
 *  Initialize GPIO.
 ******************************************************************************/
static void init_gpio(void)
{
	CMU_ClockEnable(cmuClock_GPIO, true);
	GPIO_PinModeSet(LDO_SHDN_B_PORT, LDO_SHDN_B_PIN, gpioModePushPull, 0);
	GPIO_PinModeSet(ADC_P_PORT, ADC_P_PIN, gpioModeDisabled, 0); // ADC_P
	GPIO_PinModeSet(ADC_N_PORT, ADC_N_PIN, gpioModeDisabled, 0); // ADC_N
	GPIO_PinModeSet(MOTION_B_PORT, MOTION_B_PIN, gpioModePushPull, 0);
}

/***************************************************************************//**
 * @brief
 *  Initialize timers for time stamping samples to the GUI.
 *
 * @details
 *  LETIMER0 will operate at 1024 Hz time stamp clock.
 ******************************************************************************/
static void init_timestamp_clock(void)
{
	// LETIMER Initialization
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);
	CMU_ClockEnable(cmuClock_HFLE, true);
	CMU_ClockEnable(cmuClock_LETIMER0, true);
	CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_32); // 32,768 Hz / 32 = 1024 Hz = 976.5625 us period

	LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;
	LETIMER_Init(LETIMER0, &letimerInit);
}

/***************************************************************************//**
 * @brief
 *   Enqueue the PIR sample.
 ******************************************************************************/
static void enqueue_sample(sample_queue_t *queue, pir_sample_t sample)
{
	queue->sample[queue->head] = sample;
	queue->head++;
	if (queue->head == queue->size) {
		queue->head = 0;
	}
	queue->used++;

	if (queue->head == queue->tail) {
		// If buffer is full, drop the oldest sample in the circular buffer and update the tail pointer.
		queue->tail++;
		if (queue->tail == queue->size) {
			queue->tail = 0;
		}
		queue->used = queue->size;
	}
}

/***************************************************************************//**
 * @brief
 *   Dequeue the PIR sample.
 ******************************************************************************/
static void dequeue_sample(sample_queue_t *queue, pir_sample_t *sample)
{
	if (queue->used < 1) {
		sample = NULL;
	}

	*sample = queue->sample[queue->tail];
	queue->tail++;
	if (queue->tail == queue->size) {
		queue->tail = 0;
	}
	queue->used -= 1;
}

/***************************************************************************//**
 * @brief
 *   Initializes peripherals for PIR
 *
 * @param[in] pir_init
 *   A pointer to PIR initialization structure
 *
 * @param[in] adc_enter_em2
 *   Sets up ADC to run in EM2 when set to true
 ******************************************************************************/
void pir_init(pir_init_t *pir_init, bool adc_enter_em2)
{
	init_gpio();
	// Enable LDO to startup VPIR.
	GPIO_PinOutSet(LDO_SHDN_B_PORT, LDO_SHDN_B_PIN);

	pir_instance.opamp_mode = pir_init->opamp_mode;
	pir_instance.motion_on_time = pir_init->motion_on_time;
	pir_instance.win_size = pir_init->win_size;
	pir_instance.motion_detection_callback = pir_init->motion_detection_callback;
	pir_instance.adc_irq_callback = pir_init->adc_irq_callback;
	pir_instance.use_timestamp = pir_init->use_timestamp;

	app_queue.size = pir_init->sample_queue_size;
	app_queue.sample = pir_init->sample_queue;

	adc_queue.size = ADC_SAMPLE_QUEUE_SIZE;
	adc_queue.sample = adc_sample_queue;

	// Peripheral initialization
	if (pir_init->opamp_mode == pir_opamp_mode_internal) {
		init_opamp();
	}
	if (pir_init->use_timestamp){
		init_timestamp_clock();
	}
	init_adc(adc_enter_em2);

	// Default to motion off state
	if (pir_instance.motion_detection_callback != NULL)
		pir_instance.motion_detection_callback(false);
}

/***************************************************************************//**
 * @brief
 *   Starts PIR sensor sampling.
 ******************************************************************************/
void pir_start(void)
{
	lockout_counter = 0;

	ADC_Start(ADC0, adcStartSingle);
	// Wake up on FIFO reaching threshold.
	ADC_IntEnable(ADC0, ADC_IF_SINGLE);
	// Wake up on ADC exceeding window threshold.
	ADC_IntEnable(ADC0, ADC_IF_SINGLECMP);
	NVIC_EnableIRQ(ADC0_IRQn);
}

/***************************************************************************//**
 * @brief
 *   Stops PIR sensor sampling.
 ******************************************************************************/
void pir_stop(void)
{
	ADC_IntDisable(ADC0, ADC_IF_SINGLE);
	ADC_IntDisable(ADC0, ADC_IF_SINGLECMP);
	NVIC_DisableIRQ(ADC0_IRQn);
}

/***************************************************************************//**
 * @brief
 *   Runs the motion detection algorithm
 *
 * @note
 *   This algorithms takes roughly 150us and must run on the latest ADC sample
 *   after receiving the ADC interrupt.
 ******************************************************************************/
void pir_detect_motion(void)
{
	bool motion = false;
	uint32_t num_of_samples = 0;
	pir_sample_t adc_sample;
	pir_sample_t pir_sample;
	int32_t adc_thresh_high, adc_thresh_low;

	// Read out each ADC sample, update the LPF mean, and check if it was within the window thresholds
	while (adc_queue.used > 0) {
		num_of_samples++;
		dequeue_sample(&adc_queue, &adc_sample);
		// Capture the ADC and winBase of the current sample.
		pir_sample.adc_sample = adc_sample.adc_sample;
		pir_sample.win_base = win_base;
		if (adc_source == pir_adc_source_diff) {
			adc_thresh_high = ((int16_t) ((ADC0->CMPTHR & _ADC_CMPTHR_ADGT_MASK) >> _ADC_CMPTHR_ADGT_SHIFT));
			adc_thresh_low = ((int16_t) ((ADC0->CMPTHR & _ADC_CMPTHR_ADLT_MASK) >> _ADC_CMPTHR_ADLT_SHIFT));
		} else {
			adc_thresh_high = ((uint16_t) ((ADC0->CMPTHR & _ADC_CMPTHR_ADGT_MASK) >> _ADC_CMPTHR_ADGT_SHIFT));
			adc_thresh_low = ((uint16_t) ((ADC0->CMPTHR & _ADC_CMPTHR_ADLT_MASK) >> _ADC_CMPTHR_ADLT_SHIFT));
		}

		// If window was broken, move window thresholds to include the latest ADC reading.
		// Thresholds are recalculated in software because samples are batch processed.
		if (adc_sample.adc_sample > adc_thresh_high) {
			win_base = adc_sample.adc_sample - pir_instance.win_size / 2;
			motion = true;
		} else if (adc_sample.adc_sample < adc_thresh_low) {
			win_base = adc_sample.adc_sample + pir_instance.win_size / 2;
			motion = true;
		} else {
			// Window was not broken, update winBase to follow the low frequency drift using a DT 1st order LPF.
			// Let a = 2^-a_shift
			// Equivalent continuous RC is given by Ts * (1-a) / a
			uint32_t a_shift = 5;
			win_base = (adc_sample.adc_sample >> a_shift) + (win_base - (win_base >> a_shift));
		}

		update_adc_thresholds(win_base, pir_instance.win_size);

		// Enqueue the PIR sample
		pir_sample.timestamp_ms = adc_sample.timestamp_ms;
		pir_sample.motion_status = (lockout_counter > 0 || motion);
		pir_sample.adc_upper_threshold = adc_thresh_high;
		pir_sample.adc_lower_threshold = adc_thresh_low;
		enqueue_sample(&app_queue, pir_sample);

	} // while (data_valid)

	if (lockout_counter > 0) {
		lockout_counter -= (num_of_samples < lockout_counter) ? num_of_samples : lockout_counter;
		if (lockout_counter == 0) {
			// Motion off
			if (pir_instance.motion_detection_callback != NULL)
				pir_instance.motion_detection_callback(false);
		}
	}

	// Motion On
	if (motion) {
		lockout_counter = pir_instance.motion_on_time * 32; // 32Hz
		if (pir_instance.motion_detection_callback != NULL)
			pir_instance.motion_detection_callback(true);
	}
}

/***************************************************************************//**
 * @brief
 *  ADC interrupt handler
 ******************************************************************************/
void ADC0_IRQHandler(void)
{
	uint32_t flags;
	pir_sample_t adc_sample;
	static int32_t last_timestamp = -1;

	flags = ADC_IntGetEnabled(ADC0);
	ADC_IntClear(ADC0, flags);
	NVIC_ClearPendingIRQ(ADC0_IRQn);

	// Enqueue the ADC sample
	while(ADC0->SINGLEFIFOCOUNT > 0)
	{
		if ((last_timestamp < 0) && pir_instance.use_timestamp) {
			last_timestamp = ~LETIMER_CounterGet(LETIMER0);
		}
		// adjust this with over-sampling for OSR < 64. Need to left shift to get to 16b.
		adc_sample.adc_sample = ADC_DataSingleGet(ADC0) << 3;
		adc_sample.timestamp_ms = last_timestamp;
		enqueue_sample(&adc_queue, adc_sample);
	}
	if (pir_instance.adc_irq_callback != NULL)
		pir_instance.adc_irq_callback();
}

/***************************************************************************//**
 * @brief
 *   Reads out a sample from the PIR sample queue.
 *
 * @param[out] pir_sample
 *   Pointer to the PIR sample
 ******************************************************************************/
void pir_read_queue(pir_sample_t *pir_sample)
{
	dequeue_sample(&app_queue, pir_sample);
}

/***************************************************************************//**
 * @brief
 *   Gets number of samples in the queue.
 *
 * @return
 *   The number of samples in the queue.
 ******************************************************************************/
uint16_t pir_get_queue_size(void)
{
	return app_queue.used;
}
