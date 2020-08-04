/**
 * Zigbee Motion Sensor PIR application example project
 *
 * Hardware: BRD4001A(WSTK) + BRD4162A(EFR32MG12P332F1024GL125 Radio board) + BRD8030A (
 * Occupancy sensor EXP board)
 *
 *
 * How To Use:
 * 1. Build the project(s) and download to EFR32MG12P332F1024GL125
 * 2. D1 LED on the occupancy sensor EXP board will turn on if motion is detected
 * 3. Press button PB0 for enable/disable motion sensor. The built-in LCD will
 * 	  display the corresponding state. If the state is disable, the D1 Led also is
 * 	  turned off
 * @copyright 2020 Silicon Laboratories Inc.
 */

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "em_device.h"
#include "em_chip.h"
#include "em_assert.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_pcnt.h"

#include "display.h"
#include "glib.h"
#include "app/framework/include/af.h"
#include "pir_config.h"
#include "pir.h"

/****************************************************************************/
/*                      PRIVATE TYPES and DEFINITIONS                       */
/****************************************************************************/
#define LCD_MAX_CHARACTER_LEN	( 16 + 1 )
#define QUEUE_LENGTH       		12

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/
static GLIB_Context_t glibContext;
static bool pirStart = false;
static pir_sample_t pirQueue[QUEUE_LENGTH];

/******************************************************************************/
/*                              PUBLIC DATA                                   */
/******************************************************************************/
EmberEventControl emberAfApplicationAdcPirEventControl;

/*******************************************************************************
 *************************   FUNCTION PROTOTYPES   *****************************
 ******************************************************************************/
static void pirMotionDetectCallback(bool motionOn);
static void pirADCIRQCallback();

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/
/**
 * @brief   Initialization function for OCCUPANCY-EXP-EB module
 * @param   None
 * @return  None
 */
void pirInit(void)
{
	pir_init_t pirInit = PIR_INIT_DEFAULT;
	pirInit.opamp_mode = pir_opamp_mode_external;
	pirInit.motion_detection_callback = pirMotionDetectCallback;
	pirInit.sample_queue_size = QUEUE_LENGTH;
	pirInit.sample_queue = pirQueue;
	pirInit.use_timestamp = false;
	pirInit.adc_irq_callback = pirADCIRQCallback;
	pir_init(&pirInit, true);

	GPIO_PinOutSet(MOTION_B_PORT, MOTION_B_PIN);
}

/**
 * @brief   Callback function. Called after motion detection algorithm finishes
 * @param 	status The state of motion sensor. This will be set to true if the
 * 			motion detected. Otherwise, false means motion undetected.
 * @return  None
 */


void pirMotionDetectCallback(bool motionOn)
{
	static bool currentMotionOn = false;
	EmberStatus status;

	if(currentMotionOn == motionOn) {
		return;
	}

	currentMotionOn = motionOn;

	emberAfCorePrintln("Motion detected state: %s",motionOn?"ON":"OFF");

	if(motionOn) {
		GPIO_PinOutClear(MOTION_B_PORT, MOTION_B_PIN);

		emberAfFillCommandOnOffClusterOn()
			emberAfCorePrintln("Command is zcl on-off ON");
	} else {
		GPIO_PinOutSet(MOTION_B_PORT, MOTION_B_PIN);

		emberAfFillCommandOnOffClusterOff()
			emberAfCorePrintln("Command is zcl on-off OFF");
	}

	emberAfSetCommandEndpoints(emberAfPrimaryEndpoint(),1);
	status=emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, 0x0000);

	if(status == EMBER_SUCCESS){
		emberAfCorePrintln("Command is successfully sent");
	}else{
		emberAfCorePrintln("Failed to send");
		emberAfCorePrintln("Status code: 0x%x",status);
	}
}

/**
 * @brief   Callback function. Called in ADC interrupt service routine
 * @param   None
 * @return  None
 */
void pirADCIRQCallback(void)
{
	/* Notify emberAfApplicationAdcPirEventControl event to trigger detecting motion */
	emberEventControlSetActive( emberAfApplicationAdcPirEventControl );
}


/**
 * @brief   The function displays the current activated sensor state
 * @param   state	The state that is displayed. True means enable, false for disable
 * @return  None
 */
void lcdDisplayState(bool state)
{
	char str[LCD_MAX_CHARACTER_LEN];

	GLIB_clear(&glibContext);
	GLIB_setFont(&glibContext, (GLIB_Font_t *)&GLIB_FontNormal8x8);
	snprintf(str, LCD_MAX_CHARACTER_LEN, "    ZIGBEE    ");
	GLIB_drawString(&glibContext, str, strlen(str), 5, 10, 0);
	snprintf(str, LCD_MAX_CHARACTER_LEN, "OCCUPANCY SENSOR");
	GLIB_drawString(&glibContext, str, strlen(str), 0, 25, 0);
	snprintf(str, LCD_MAX_CHARACTER_LEN, "     SENSOR   ");
	GLIB_drawString(&glibContext, str, strlen(str), 0, 60, 0);
	snprintf(str, LCD_MAX_CHARACTER_LEN, state==true?"     ENABLE   ":"    DISABLE    ");
	GLIB_drawString(&glibContext, str, strlen(str), 1, 75, 0);
	DMD_updateDisplay();
}

/**
 * @brief 	Initializes LCD to display sensor's state
 * @param   None
 * @return  None
 */
void lcdInit(void)
{
	EMSTATUS status;

	/* Initialize the display module. */
	status = DISPLAY_Init();
	if (DISPLAY_EMSTATUS_OK != status) {
		emberAfCorePrintln("DISPLAY_Init error");
		return ;
	}

	/* Initialize the DMD module for the DISPLAY device driver. */
	status = DMD_init(0);
	if (DMD_OK != status) {
		emberAfCorePrintln("DMD_init error");
		return ;
	}

	/* Initialize the glib context */
	status = GLIB_contextInit(&glibContext);
	if (GLIB_OK != status) {
		emberAfCorePrintln("GLIB_contextInit error");
		return;
	}

	glibContext.backgroundColor = White;
	glibContext.foregroundColor = Black;

	lcdDisplayState(false);
}

/**
 * @brief 	Override for emberAfMainInitCallback() function
 * @param   None
 * @return  None
 */
void emberAfMainInitCallback(void)
{
	/* Initialize lcd */
	lcdInit();

	/* Initialize Pir sensor*/
	pirInit();
}

/**
 * @brief 	Handler function for starting motion detection algorithm.
 *  	  	The event is triggered in pirADCIRQCallback
 * @param   None
 * @return  None
 */
void emberAfApplicationAdcPirEventHandler(void)
{
	/* Sets emberAfApplicationAdcPirEventControl as inactive */
	emberEventControlSetInactive( emberAfApplicationAdcPirEventControl );

	/* Run motion detection algorithm */
	pir_detect_motion();
}

/**
 * @brief 	Callback function for handling button pressed event
 * @param   timePressedMs  Indicates time button is pressed.
 * @return  None
 */
void emberAfPluginButtonInterfaceButton0PressedShortCallback(uint16_t timePressedMs)
{
	emberAfCorePrintln("Button0 is pressed for %d milliseconds",timePressedMs);

	if (pirStart) {
		GPIO_PinOutSet(MOTION_B_PORT, MOTION_B_PIN);

		pirStart = false;
		emberAfCorePrintln("Disable sensor");
		pir_stop();
		lcdDisplayState(false);
	} else {
		pirStart = true;
		emberAfCorePrintln("Enable sensor");
		pir_start();
		lcdDisplayState(true);
	}
}

/**
 * @brief Callback function for handling button pressed event
 * @param   timePressedMs  Indicates time button is pressed.
 */
void emberAfPluginButtonInterfaceButton1PressedShortCallback(uint16_t timePressedMs)
{
	emberAfCorePrintln("Button1 is pressed for %d milliseconds",timePressedMs);

}
