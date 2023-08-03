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
#include "app_log.h"
#include "find-and-bind-target.h"

#define END_POINT_1  1
static sl_zigbee_event_t finding_and_binding_event;

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

/** @brief Post Attribute Change
 *
 * This function is called by the application framework after it changes an
 * attribute value. The value passed into this callback is the value to which
 * the attribute was set by the framework.
 */
void emberAfPostAttributeChangeCallback(uint8_t endpoint,
                                        EmberAfClusterId clusterId,
                                        EmberAfAttributeId attributeId,
                                        uint8_t mask,
                                        uint16_t manufacturerCode,
                                        uint8_t type,
                                        uint8_t size,
                                        uint8_t *value)
{
  if ((clusterId == ZCL_POWER_CONFIG_CLUSTER_ID)
      && (attributeId == ZCL_BATTERY_VOLTAGE_ATTRIBUTE_ID)
      && (mask == CLUSTER_MASK_CLIENT)) {
    uint8_t voltageDeciV;

    if (emberAfReadServerAttribute(endpoint,
                                   ZCL_POWER_CONFIG_CLUSTER_ID,
                                   ZCL_BATTERY_VOLTAGE_ATTRIBUTE_ID,
                                   &voltageDeciV,
                                   sizeof(voltageDeciV))
        == EMBER_ZCL_STATUS_SUCCESS) {
      emberAfAppPrintln("New voltage reading: %d mV", voltageDeciV);
    }
  }
}

static void finding_and_binding_event_handler(sl_zigbee_event_t *event)
{
  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    sl_zigbee_event_set_inactive(&finding_and_binding_event);

    sl_zigbee_app_debug_println("Find and bind target start: 0x%02X",
                                emberAfPluginFindAndBindTargetStart(END_POINT_1));
  }
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action. The framework
 * will always process the stack status after the callback returns.
 */
void emberAfStackStatusCallback(EmberStatus status)
{
  // Note, the ZLL state is automatically updated by the stack and the plugin.
  if (status == EMBER_NETWORK_UP) {
    sl_zigbee_event_set_active(&finding_and_binding_event);
  }
}

/** @brief Init
 * Application init function
 */
void emberAfMainInitCallback(void)
{
  sl_zigbee_event_init(&finding_and_binding_event,
                       finding_and_binding_event_handler);
}

/** @brief Report Attributes Callback
 *
 * This function is called by the application framework when a Report Attributes
 * command is received from an external device.  The application should return
 * true if the message was processed or false if it was not.
 * We only process the Relative humidity and temperature measurement cluster
 * attribute changes
 *
 * @param clusterId The cluster identifier of this command.  Ver.: always
 * @param buffer Buffer containing the list of attribute report records.  Ver.:
 * always
 * @param bufLen The length in bytes of the list.  Ver.: always
 */
bool emberAfReportAttributesCallback(EmberAfClusterId clusterId,
                                     int8u *buffer,
                                     int16u bufLen)
{
  int16_t attribute;
  uint16_t battery_voltage;

  emberAfAppPrintln("clusterId: %d", clusterId);

  // Custom processing done only for cluster 0x405 and 0x402 (RH and Temperature
  // measurement clusters)
  if ((clusterId == ZCL_POWER_CONFIG_CLUSTER_ID)
      || (clusterId == ZCL_BATTERY_VOLTAGE_ATTRIBUTE_ID)) {
    attribute = buffer[bufLen - 2];
    battery_voltage = buffer[bufLen - 1] * 100;
    emberAfAppPrintln("buffer: %d", battery_voltage);
    emberAfAppPrintln("attribute: %x", attribute);
  }
  return true;
}
