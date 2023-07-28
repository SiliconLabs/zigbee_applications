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

#include EMBER_AF_API_NETWORK_STEERING
#include EMBER_AF_API_FIND_AND_BIND_INITIATOR

/****************************************************************************/
/*                      PRIVATE TYPES and DEFINITIONS                       */
/****************************************************************************/
#define LCD_MAX_CHARACTER_LEN	( 16 + 1 )
#define QUEUE_LENGTH       		12
#define SENSOR_ENDPOINT 		  (1)
/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/
static bool commissioning = false;

EmberEventControl commissioningEventControl;
EmberEventControl ledEventControl;
EmberEventControl findingAndBindingEventControl;

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

  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
      emberAfGetCommandApsFrame()->sourceEndpoint = SENSOR_ENDPOINT;
      status = emberAfSendCommandUnicastToBindings();
      emberAfCorePrintln("%p: 0x%X", "Send to bindings", status);
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
 * @brief 	Handler function for event ledEventControl
 * @param   None
 * @return  None
 */
void ledEventHandler(void)
{
  emberEventControlSetInactive(ledEventControl);

  if (commissioning) {
      halToggleLed(COMMISSIONING_STATUS_LED);
      emberEventControlSetDelayMS(ledEventControl, LED_BLINK_PERIOD_MS);
  } else if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
      halSetLed(COMMISSIONING_STATUS_LED);
  }
}

/**
 * @brief 	Handler function for event findingAndBindingEventControl
 * @param   None
 * @return  None
 */
void findingAndBindingEventHandler(void)
{
  emberEventControlSetInactive(findingAndBindingEventControl);
  EmberStatus status = emberAfPluginFindAndBindInitiatorStart(SENSOR_ENDPOINT);
  emberAfCorePrintln("Find and bind initiator %p: 0x%X", "start", status);
}

/**
 * @brief 	scheduleFindingAndBindingForInitiator() function. Used to set
 * 			findingAndBindingEventControl event for finding&binding process
 * @param   None
 * @return  None
 */
void scheduleFindingAndBindingForInitiator(void)
{
  emberEventControlSetDelayMS(findingAndBindingEventControl,
                              FINDING_AND_BINDING_DELAY_MS);
}

/**
 * @brief 	Handler function for event commissioningEventControl
 * @param   None
 * @return  None
 */
void commissioningEventHandler(void)
{
  EmberStatus status;

  emberEventControlSetInactive(commissioningEventControl);

  if (emberAfNetworkState() != EMBER_JOINED_NETWORK) {
      status = emberAfPluginNetworkSteeringStart();
      emberAfCorePrintln("%p network %p: 0x%X",
                         "Join",
                         "start",
                         status);
      emberEventControlSetActive(ledEventControl);
      commissioning = true;
  } else {
      if(!commissioning) {
          scheduleFindingAndBindingForInitiator();
          emberAfCorePrintln("%p %p",
                             "FindAndBind",
                             "start");
          emberEventControlSetActive(ledEventControl);
          commissioning = true;
      } else {
          emberAfCorePrintln("Already in commissioning mode");
      }
  }
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

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
bool emberAfStackStatusCallback(EmberStatus status)
{
  if (status == EMBER_NETWORK_DOWN) {
      halClearLed(COMMISSIONING_STATUS_LED);
  } else if (status == EMBER_NETWORK_UP) {
      halSetLed(COMMISSIONING_STATUS_LED);
  }

  // This value is ignored by the framework.
  return false;
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
  emberAfCorePrintln("Enter commissioning mode");
  emberEventControlSetActive(commissioningEventControl);
}

/** @brief Complete
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to EMBER_SUCCESS to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt. Ver.: always
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID. Ver.: always
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network. Ver.: always
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete. Ver.: always
 */
void emberAfPluginNetworkSteeringCompleteCallback(EmberStatus status,
                                                  uint8_t totalBeacons,
                                                  uint8_t joinAttempts,
                                                  uint8_t finalState)
{
  emberAfCorePrintln("%p network %p: 0x%X", "Join", "complete", status);

  if (status != EMBER_SUCCESS) {
      commissioning = false;
  } else {
      scheduleFindingAndBindingForInitiator();
  }
}

/** @brief Complete
 *
 * This callback is fired by the initiator when the Find and Bind process is
 * complete.
 *
 * @param status Status code describing the completion of the find and bind
 * process Ver.: always
 */
void emberAfPluginFindAndBindInitiatorCompleteCallback(EmberStatus status)
{
  emberAfCorePrintln("Find and bind initiator %p: 0x%X", "complete", status);

  commissioning = false;
}
