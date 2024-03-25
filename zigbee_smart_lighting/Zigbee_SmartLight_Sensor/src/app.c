/**
 * Zigbee Motion Sensor PIR application example project
 *
 * Hardware:
 * BRD4001A(WSTK)
 * BRD4162A(EFR32MG12P332F1024GL125 Radio board)
 * BRD8030A (Occupancy sensor EXP board)
 *
 *
 * How To Use:
 * 1. Build the project(s) and download to EFR32MG12P332F1024GL125
 * 2. D1 LED on the occupancy sensor EXP board
 * will turn on if motion is detected
 * 3. Press button PB0 for enable/disable motion sensor.
 * The built-in LCD will display the corresponding state.
 * If the state is disable, the D1 Led also is turned off
 * @copyright 2020 Silicon Laboratories Inc.
 */

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "sl_string.h"

#include "sl_board_control.h"
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"
#include "sl_simple_led_instances.h"

#include "glib.h"
#include "app/framework/include/af.h"
#include "pir_ira_s210st01.h"

#include "app/framework/plugin/network-steering/network-steering.h"
#include \
  "app/framework/plugin/find-and-bind-initiator/find-and-bind-initiator.h"

/****************************************************************************/
/*                      PRIVATE TYPES and DEFINITIONS                       */
/****************************************************************************/
#define LCD_MAX_CHARACTER_LEN        (16 + 1)
#define QUEUE_LENGTH                 12
#define SENSOR_ENDPOINT              (1)

#define TRANSITION_TIME_DS           20
#define FINDING_AND_BINDING_DELAY_MS 3000
#define LED_BLINK_PERIOD_MS          2000

#define led0_on()     sl_led_turn_on(&sl_led_led0);
#define led0_off()    sl_led_turn_off(&sl_led_led0);
#define led0_toggle() sl_led_toggle(&sl_led_led0);

#define led1_on()     sl_led_turn_on(&sl_led_led1);
#define led1_off()    sl_led_turn_off(&sl_led_led1);
#define led1_toggle() sl_led_toggle(&sl_led_led1);

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/
static bool commissioning = false;

static sl_zigbee_event_t commissioning_event;
static sl_zigbee_event_t led_event;
static sl_zigbee_event_t finding_and_binding_event;
static sl_zigbee_event_t adc_pir_event;

static GLIB_Context_t glibContext;
static bool pirStart = false;
static pir_sample_t pirQueue[QUEUE_LENGTH];

/******************************************************************************/
/*                              PUBLIC DATA                                   */
/******************************************************************************/

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
}

/**
 * @brief       Callback function.
 * Called after motion detection algorithm finishes
 * @param       status The state of motion sensor.
 * This will be set to true if the motion detected.
 * Otherwise, false means motion undetected.
 * @return      None
 */
void pirMotionDetectCallback(bool motionOn)
{
  static bool currentMotionOn = false;
  EmberStatus status;

  if (currentMotionOn == motionOn) {
    return;
  }

  currentMotionOn = motionOn;

  sl_zigbee_app_debug_print("Motion detected state: %s", motionOn?"ON":"OFF");

  if (motionOn) {
    led1_on();
    emberAfFillCommandOnOffClusterOn()
    sl_zigbee_app_debug_print("Command is zcl on-off ON");
  } else {
    led1_off();
    emberAfFillCommandOnOffClusterOff()
    sl_zigbee_app_debug_print("Command is zcl on-off OFF");
  }

  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    emberAfGetCommandApsFrame()->sourceEndpoint = SENSOR_ENDPOINT;
    status = emberAfSendCommandUnicastToBindings();
    sl_zigbee_app_debug_print("%p: 0x%X", "Send to bindings", status);
  }
}

/**
 * @brief   Callback function. Called in ADC interrupt service routine
 * @param   None
 * @return  None
 */
void pirADCIRQCallback(void)
{
  /* Notify adc_pir_event event to trigger detecting motion */
  sl_zigbee_event_set_active(&adc_pir_event);
}

/**
 * @brief   The function displays the current activated sensor state
 * @param   state	The state that is displayed. True means enable,
 * false for disable
 * @return  None
 */
void lcdDisplayState(bool state)
{
  char str[LCD_MAX_CHARACTER_LEN];

  GLIB_clear(&glibContext);
  GLIB_setFont(&glibContext, (GLIB_Font_t *)&GLIB_FontNormal8x8);
  snprintf(str, LCD_MAX_CHARACTER_LEN, "    ZIGBEE    ");
  GLIB_drawString(&glibContext, str, sl_strlen(str), 5, 10, 0);
  snprintf(str, LCD_MAX_CHARACTER_LEN, "OCCUPANCY SENSOR");
  GLIB_drawString(&glibContext, str, sl_strlen(str), 0, 25, 0);
  snprintf(str, LCD_MAX_CHARACTER_LEN, "     SENSOR   ");
  GLIB_drawString(&glibContext, str, sl_strlen(str), 0, 60, 0);
  snprintf(str,
           LCD_MAX_CHARACTER_LEN,
           state == true?"     ENABLE   ":"    DISABLE    ");
  GLIB_drawString(&glibContext, str, sl_strlen(str), 1, 75, 0);
  DMD_updateDisplay();
}

/**
 * @brief       Initializes LCD to display sensor's state
 * @param       None
 * @return      None
 */
void lcdInit(void)
{
  EMSTATUS status;

  /* Enable the memory lcd */
  status = sl_board_enable_display();
  EFM_ASSERT(status == SL_STATUS_OK);

  /* Initialize the DMD module for the DISPLAY device driver. */
  status = DMD_init(0);
  if (DMD_OK != status) {
    sl_zigbee_app_debug_print("DMD_init error");
    return;
  }

  /* Initialize the glib context */
  status = GLIB_contextInit(&glibContext);
  if (GLIB_OK != status) {
    sl_zigbee_app_debug_print("GLIB_contextInit error");
    return;
  }

  glibContext.backgroundColor = White;
  glibContext.foregroundColor = Black;

  lcdDisplayState(false);
}

/**
 * @brief       Handler function for starting motion detection algorithm.
 *              The event is triggered in pirADCIRQCallback
 * @param   None
 * @return  None
 */
void adc_pir_event_handler(void)
{
  /* Run motion detection algorithm */
  pir_detect_motion();
}

/**
 * @brief       Handler function for event led_event
 * @param       None
 * @return      None
 */
void led_event_handler(void)
{
  if (commissioning) {
    led0_toggle();
    sl_zigbee_event_set_delay_ms(&led_event, LED_BLINK_PERIOD_MS);
  } else if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    led0_on();
  }
}

/**
 * @brief       Handler function for event finding_and_binding_event
 * @param   None
 * @return  None
 */
void finding_and_binding_event_handler(void)
{
  EmberStatus status = emberAfPluginFindAndBindInitiatorStart(SENSOR_ENDPOINT);
  sl_zigbee_app_debug_print("Find and bind initiator %p: 0x%X", "start",
                            status);
}

/**
 * @brief       scheduleFindingAndBindingForInitiator() function.
 * Used to set finding_and_binding_event event for finding&binding process
 * @param   None
 * @return  None
 */
void scheduleFindingAndBindingForInitiator(void)
{
  sl_zigbee_event_set_delay_ms(&finding_and_binding_event,
                               FINDING_AND_BINDING_DELAY_MS);
}

/**
 * @brief       Handler function for event commissioning_event
 * @param       None
 * @return      None
 */
void commissioning_event_handler(void)
{
  EmberStatus status;

  if (emberAfNetworkState() != EMBER_JOINED_NETWORK) {
    // Clear binding table
    status = emberClearBindingTable();
    sl_zigbee_app_debug_print("%s 0x%x", "Clear binding table\n", status);

    // Leave network
    status = emberLeaveNetwork();
    sl_zigbee_app_debug_print("%s 0x%x", "leave\n", status);

    status = emberAfPluginNetworkSteeringStart();
    sl_zigbee_app_debug_print("%p network %p: 0x%X",
                              "Join",
                              "start",
                              status);
    sl_zigbee_event_set_active(&led_event);
    commissioning = true;
  } else {
    if (!commissioning) {
      scheduleFindingAndBindingForInitiator();
      sl_zigbee_app_debug_print("%p %p",
                                "FindAndBind",
                                "start");
      sl_zigbee_event_set_active(&led_event);
      commissioning = true;
    } else {
      sl_zigbee_app_debug_print("Already in commissioning mode");
    }
  }
}

/**
 * @brief       Override for emberAfMainInitCallback() function
 * @param       None
 * @return      None
 */
void emberAfMainInitCallback(void)
{
  sl_zigbee_event_init(&commissioning_event, commissioning_event_handler);
  sl_zigbee_event_init(&led_event, led_event_handler);
  sl_zigbee_event_init(&finding_and_binding_event,
                       finding_and_binding_event_handler);
  sl_zigbee_event_init(&adc_pir_event, adc_pir_event_handler);

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
    led0_off();
  } else if (status == EMBER_NETWORK_UP) {
    led0_on();
  }
  sl_zigbee_app_debug_print("Network status: %d\n", status);
  // This value is ignored by the framework.
  return false;
}

// -----------------------------------------------------------------------------
// Push button event handler
void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED) {
    if (&sl_button_btn0 == handle) {
      if (pirStart) {
        //      GPIO_PinOutSet(MOTION_B_PORT, MOTION_B_PIN);
        pirStart = false;
        sl_zigbee_app_debug_print("Disable sensor");
        pir_stop();
        lcdDisplayState(false);
      } else {
        pirStart = true;
        sl_zigbee_app_debug_print("Enable sensor");
        pir_start();
        lcdDisplayState(true);
      }
    } else if (&sl_button_btn1 == handle) {
      sl_zigbee_event_set_active(&commissioning_event);
    }
  }
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
  sl_zigbee_app_debug_print("%p network %p: 0x%X", "Join", "complete", status);

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
  sl_zigbee_app_debug_print("Find and bind initiator %p: 0x%X",
                            "complete",
                            status);

  commissioning = false;
}
