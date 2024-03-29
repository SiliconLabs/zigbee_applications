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
#include "networkJoin.h"
#include "networkRejoin.h"

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
  sl_zigbee_app_debug_print("%s network %s: 0x%02X\n",
                            "Join",
                            "complete",
                            status);
}

/** @brief
 *
 * Application framework equivalent of ::emberRadioNeedsCalibratingHandler
 */
void emberAfRadioNeedsCalibratingCallback(void)
{
  sl_mac_calibrate_current_channel();
}

/**
 * Callback when Network Key update complete
 * @param status
 */
void emberAfNetworkKeyUpdateCompleteCallback(EmberStatus status)
{
  EmberKeyStruct nwkKey;
  emberAfCorePrintln("Network Key Update Complete : ",
                     (status == EMBER_SUCCESS) ? "Success" : "Failed");
  if (status == EMBER_SUCCESS) {
    // Display new NWK Key
    emberGetKey(EMBER_CURRENT_NETWORK_KEY, &nwkKey);
    emberAfCorePrint("Current NWK Key : ");
    emberAfPrintKey(true, nwkKey.key.contents);
  }
}

/**
 * Main Callback called after initialization of the stack
 * WARNING : All the stack is not completely initialized
 *           You must wait set event after a little delay
 */
void emberAfMainInitCallback(void)
{
  // Initialize Events
  sl_zigbee_event_init(&eventRouterInfoCtrl, myEventRouterInfoHandler);

  // Initialize custom group event
  sl_cli_command_add_command_group(sl_cli_handles[0], &my_cli_command_group);
  sl_cli_command_add_command_group(sl_cli_handles[0], &my_rejoin_command_group);

  // Launch Events
  sl_zigbee_event_set_delay_ms(&eventRouterInfoCtrl, INFO_DELAY_MS);
}
