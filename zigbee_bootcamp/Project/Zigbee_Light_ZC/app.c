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

// Sending-OnOff-Commands: Step 1
void emberAfPostAttributeChangeCallback(uint8_t endpoint,
                                        EmberAfClusterId clusterId,
                                        EmberAfAttributeId attributeId,
                                        uint8_t mask,
                                        uint16_t manufacturerCode,
                                        uint8_t type,
                                        uint8_t size,
                                        uint8_t* value)
{
  if (clusterId == ZCL_ON_OFF_CLUSTER_ID
      && attributeId == ZCL_ON_OFF_ATTRIBUTE_ID
      && mask == CLUSTER_MASK_SERVER) {
    bool onOff;
    if (emberAfReadServerAttribute(endpoint,
                                   ZCL_ON_OFF_CLUSTER_ID,
                                   ZCL_ON_OFF_ATTRIBUTE_ID,
                                   (uint8_t *)&onOff,
                                   sizeof(onOff))
        == EMBER_ZCL_STATUS_SUCCESS) {
      if (onOff) {
        sl_led_turn_on(&sl_led_led0);
      } else {
        sl_led_turn_off(&sl_led_led0);
      }
    }
  }
}
