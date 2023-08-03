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

sl_zigbee_event_t emberTimeSyncEventControl;
static void emberTimeSyncEventHandler();

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
  sl_zigbee_app_debug_println("%s network %s: 0x%02X",
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

static void emberTimeSyncEventHandler()
{
  sl_zigbee_event_set_inactive(&emberTimeSyncEventControl);

  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    // send read attribute
    uint8_t timeAttributeIds[] = {
      LOW_BYTE(ZCL_TIME_ATTRIBUTE_ID),
      HIGH_BYTE(ZCL_TIME_ATTRIBUTE_ID)
    };

    emberAfFillCommandGlobalClientToServerReadAttributes(ZCL_TIME_CLUSTER_ID,
                                                         timeAttributeIds,
                                                         sizeof(timeAttributeIds));
    emberAfGetCommandApsFrame()->sourceEndpoint = 1;
    emberAfGetCommandApsFrame()->destinationEndpoint = 1;
    EmberStatus status =
      emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, 0x0000);
    emberAfCorePrintln("Query time from the gateway status=0x%X", status);
  }
  sl_zigbee_event_set_delay_ms(&emberTimeSyncEventControl, 5000);
}

bool emberAfStackStatusCallback(EmberStatus status)
{
  if (status == EMBER_NETWORK_DOWN) {
    sl_zigbee_event_set_inactive(&emberTimeSyncEventControl);
  } else if (status == EMBER_NETWORK_UP) {
    sl_zigbee_event_set_delay_ms(&emberTimeSyncEventControl, 5000);
  }

  // This value is ignored by the framework.
  return false;
}

boolean emberAfReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                              int8u *buffer,
                                              int16u bufLen)
{
  if (ZCL_TIME_CLUSTER_ID != clusterId) {
    return false;
  }

  // attribute ID (2B) + status (1B) + date type (0B or 1B) + value (4B)
  if (bufLen < 7) {
    return false;
  }

  if ((emberAfGetInt16u(buffer, 0,
                        bufLen) == ZCL_TIME_ATTRIBUTE_ID)
      && (emberAfGetInt8u(buffer, 2, bufLen) == EMBER_ZCL_STATUS_SUCCESS)) {
    emberAfSetTime(emberAfGetInt32u(buffer, 4, bufLen));
    emberAfCorePrintln("time sync ok, time: %4x", emberAfGetCurrentTime());

    sl_zigbee_event_set_delay_ms(&emberTimeSyncEventControl,
                                 MILLISECOND_TICKS_PER_DAY);

    return true;
  }

  return false;
}

void emberAfMainInitCallback(void)
{
  sl_zigbee_event_init(&emberTimeSyncEventControl, emberTimeSyncEventHandler);
  sl_zigbee_event_set_active(&emberTimeSyncEventControl);
}
