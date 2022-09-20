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
#include "sl_led.h"
#include "sl_simple_led_instances.h"
#include "sl_token_manager.h"

ledOnOff led0OnOffStatus;

static sl_zigbee_event_t ledBlinkingEventControl;

/** @brief Complete network steering.
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to EMBER_SUCCESS to indicate a
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
void emberAfPluginNetworkSteeringCompleteCallback(EmberStatus status,
                                                  uint8_t totalBeacons,
                                                  uint8_t joinAttempts,
                                                  uint8_t finalState)
{
  sl_zigbee_app_debug_print("%s network %s: 0x%02X\n", "Join", "complete", status);
}

/** @brief
 *
 * Application framework equivalent of ::emberRadioNeedsCalibratingHandler
 */
void emberAfRadioNeedsCalibratingCallback(void)
{
  sl_mac_calibrate_current_channel();
}

#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"
#define BUTTON0 0
#define BUTTON1 1


// Sending-OnOff-Commands: Step 2
void sl_button_on_change(const sl_button_t *handle)
{
  EmberStatus status;

  if (SL_SIMPLE_BUTTON_INSTANCE(BUTTON0) == handle){
    if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED){
      emberAfFillCommandOnOffClusterOn();

      emberAfCorePrintln("Button0 is pressed");
      emberAfCorePrintln("Command is zcl on-off ON");

      emberAfSetCommandEndpoints(emberAfPrimaryEndpoint(),1);
      status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, 0x0000);

      if(status == EMBER_SUCCESS){
        emberAfCorePrintln("Command is successfully sent");
      }else{
        emberAfCorePrintln("Failed to send");
        emberAfCorePrintln("Status code: 0x%x",status);
      }
    }
  }

    if (SL_SIMPLE_BUTTON_INSTANCE(BUTTON1) == handle){
      if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED){
        emberAfFillCommandOnOffClusterOff();

        emberAfCorePrintln("Button1 is pressed");
        emberAfCorePrintln("Command is zcl on-off OFF");

        emberAfSetCommandEndpoints(emberAfPrimaryEndpoint(),1);
        status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, 0x0000);

       if(status == EMBER_SUCCESS){
          emberAfCorePrintln("Command is successfully sent");
        }else{
          emberAfCorePrintln("Failed to send");
          emberAfCorePrintln("Status code: 0x%x",status);
      }
    }
  }
}


// Using-event: Step 3
void ledBlinkingEventHandler(void)
{
  sl_zigbee_event_set_inactive(&ledBlinkingEventControl);

  // Retrieve the previous status of LED0
  sl_token_get_data(TOKEN_LED0_ON_OFF, 1, &led0OnOffStatus, sizeof(led0OnOffStatus));

  sl_led_toggle(&sl_led_led0);
  led0OnOffStatus = !led0OnOffStatus;

  // Store the current status of LED0
  sl_token_set_data(TOKEN_LED0_ON_OFF, 1, &led0OnOffStatus, sizeof(led0OnOffStatus));

  //Reschedule the event after a delay of 2 seconds
  sl_zigbee_event_set_delay_ms(&ledBlinkingEventControl, 2000);
}

void emberAfMainInitCallback(void)
{
  // Non-volatile Data Storage: Step 4
  tokTypeMfgString mfgString;

  sl_token_get_manufacturing_data (TOKEN_MFG_STRING, 1, &mfgString, sizeof(mfgString));

  emberAfAppPrintln("MFG String: %s", mfgString);

  sl_token_get_data(TOKEN_LED0_ON_OFF, 1, &led0OnOffStatus, sizeof(led0OnOffStatus));

  // Restore the LED0 status during initialization
  if(led0OnOffStatus){
      sl_led_turn_on(&sl_led_led0);
  }
  else{
      sl_led_turn_off(&sl_led_led0);
  }

  sl_zigbee_event_init(&ledBlinkingEventControl, ledBlinkingEventHandler);

  //add the following code the schedule the event with 5 seconds delay
  sl_zigbee_event_set_delay_ms(&ledBlinkingEventControl, 5000);
}
