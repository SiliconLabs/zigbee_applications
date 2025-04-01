/***************************************************************************//**
 * @file app.c
 * @brief Callbacks implementation and application specific code.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "app/framework/include/af.h"

// Sending-OnOff-Commands: Step 2
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

// Using-Events: Step 3
#include "sl_led.h"
#include "sl_simple_led_instances.h"

// NVM Storage: Step 4
#include "sl_token_manager.h"

// Sending-OnOff-Commands: Step 2
#define BUTTON0                      0
#define BUTTON1                      1
#define SWITCH_ENDPOINT              1
static uint8_t lastButton;
static sl_zigbee_af_event_t button_event;

// Using-Events: Step 3
static sl_zigbee_af_event_t ledBlinkingEventControl;

// NVM Storage: Step 4
lastButtonPressed lightStatus;
tokTypeMfgManufId mfgId;

void ledBlinkingEventHandler(void)
{
  sl_led_toggle(&sl_led_led0);

  // Reschedule the event after a delay of 2 seconds
  sl_zigbee_af_event_set_delay_ms(&ledBlinkingEventControl, 2000);
}

/** @brief Complete network steering.
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to SL_STATUS_OK to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt. Ver.: always
 *
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID. Ver.: always
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network. Ver.: always
 *
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete. Ver.: always
 */
void sl_zigbee_af_network_steering_complete_cb(sl_status_t status,
                                               uint8_t totalBeacons,
                                               uint8_t joinAttempts,
                                               uint8_t finalState)
{
  sl_zigbee_app_debug_println("%s network %s: 0x%02X",
                              "Join",
                              "complete",
                              status);
}

/** @brief
 *
 * Application framework equivalent of ::sl_zigbee_radio_needs_calibrating_handler
 */
void sl_zigbee_af_radio_needs_calibrating_cb(void)
{
  sl_mac_calibrate_current_channel();
}

// Sending-OnOff-Commands: Step 2
static void button_event_handler(sl_zigbee_af_event_t *event)
{
  sl_status_t status;

  if (sl_zigbee_af_network_state() == SL_ZIGBEE_JOINED_NETWORK) {
    sl_zigbee_af_get_command_aps_frame()->sourceEndpoint = SWITCH_ENDPOINT;
    if (lastButton == BUTTON0) {
      sl_zigbee_app_debug_println("Button 0 \r\n");
      sl_zigbee_af_fill_command_on_off_cluster_on();
    } else if (lastButton == BUTTON1) {
      sl_zigbee_app_debug_println("Button 1 \r\n");
      sl_zigbee_af_fill_command_on_off_cluster_off();
    }
    status = sl_zigbee_af_send_command_unicast_to_bindings();
    if (status == SL_STATUS_OK) {
      sl_zigbee_app_debug_println("Command is successfully sent");
    } else {
      sl_zigbee_app_debug_println("Failed to send");
      sl_zigbee_app_debug_println("Status code: 0x%x", status);
    }
  }
}

// Sending-OnOff-Commands: Step 2
void sl_zigbee_af_main_init_cb(void)
{
  // NVM Storage: Step 4
  sl_token_get_data(TOKEN_LAST_BUTTON_PRESSED, 1, &lightStatus,
                    sizeof(TOKEN_LAST_BUTTON_PRESSED));
  sl_zigbee_app_debug_println("The Last Button Pressed: %d", lightStatus);

  sl_token_get_manufacturing_data(TOKEN_MFG_MANUF_ID, 0, &mfgId, sizeof(mfgId));
  sl_zigbee_app_debug_println("MFG ID: 0x%x", mfgId);

  sl_zigbee_af_isr_event_init(&button_event, button_event_handler);
  sl_zigbee_af_event_init(&ledBlinkingEventControl, ledBlinkingEventHandler);
  sl_zigbee_af_event_set_delay_ms(&ledBlinkingEventControl, 5000);
}

// Sending-OnOff-Commands: Step 2
void sl_button_on_change(const sl_button_t *handle)
{
  if (SL_SIMPLE_BUTTON_INSTANCE(BUTTON0) == handle) {
    if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED) {
      lastButton = BUTTON0;

      // NVM Storage: Step 4
      sl_token_set_data(TOKEN_LAST_BUTTON_PRESSED, 1, &lastButton,
                        sizeof(TOKEN_LAST_BUTTON_PRESSED));
      sl_zigbee_app_debug_println("Button 0: %d", lastButton);

      sl_zigbee_af_event_set_active(&button_event);
    }
  } else if (SL_SIMPLE_BUTTON_INSTANCE(BUTTON1) == handle) {
    if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED) {
      lastButton = BUTTON1;

      // NVM Storage: Step 4
      sl_token_set_data(TOKEN_LAST_BUTTON_PRESSED, 1, &lastButton,
                        sizeof(TOKEN_LAST_BUTTON_PRESSED));
      sl_zigbee_app_debug_println("Button 1: %d", lastButton);

      sl_zigbee_af_event_set_active(&button_event);
    }
  }
}
