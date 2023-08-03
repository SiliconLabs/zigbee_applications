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
#include "battery_monitor.h"
#include "find-and-bind-initiator.h"

#define MILLI_TO_DECI_CONVERSION_FACTOR 100

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

void BatteryMonitorDataReadyCallback(uint16_t batteryVoltageMilliV)
{
  EmberAfStatus afStatus;
  uint8_t voltageDeciV;
  uint8_t i;
  uint8_t endpoint;
  uint8_t attribute_record[2];

  emberAfAppPrintln("New voltage reading: %d mV", batteryVoltageMilliV);

  // convert from mV to 100 mV, which are the units specified by zigbee spec for
  // the power configuration cluster's voltage attribute.
  voltageDeciV =
    ((uint8_t) (batteryVoltageMilliV / MILLI_TO_DECI_CONVERSION_FACTOR));
  attribute_record[0] = ZCL_BATTERY_VOLTAGE_ATTRIBUTE_ID;
  attribute_record[1] = voltageDeciV;
  // Cycle through all endpoints, check to see if the endpoint has a power
  // configuration server, and if so update the voltage attribute
  for (i = 0; i < emberAfEndpointCount(); i++) {
    endpoint = emberAfEndpointFromIndex(i);
    if (emberAfContainsServer(endpoint, ZCL_POWER_CONFIG_CLUSTER_ID)) {
      afStatus = emberAfWriteServerAttribute(endpoint,
                                             ZCL_POWER_CONFIG_CLUSTER_ID,
                                             ZCL_BATTERY_VOLTAGE_ATTRIBUTE_ID,
                                             &voltageDeciV,
                                             ZCL_INT8U_ATTRIBUTE_TYPE);

      // Fill a ZCL global report attributes command buffer
      emberAfFillCommandGlobalServerToClientReportAttributes(
        ZCL_POWER_CONFIG_CLUSTER_ID,
        attribute_record,
        sizeof(attribute_record) / sizeof(uint8_t));
      if (EMBER_ZCL_STATUS_SUCCESS != afStatus) {
        emberAfAppPrintln("Power Configuration Server: failed to write value "
                          "0x%x to cluster 0x%x attribute ID 0x%x: error 0x%x",
                          voltageDeciV,
                          ZCL_POWER_CONFIG_CLUSTER_ID,
                          ZCL_BATTERY_VOLTAGE_ATTRIBUTE_ID,
                          afStatus);
      }
    }
  }
}

static void finding_and_binding_event_handler(sl_zigbee_event_t *event)
{
  emberAfPluginFindAndBindInitiatorStart(1);
  emberAfAppPrintln("Start to find and binding initiator.");
}

// ----------------------
// Main init Callbacks
void emberAfMainInitCallback(void)
{
  emberAfSetDefaultSleepControlCallback(EMBER_AF_STAY_AWAKE);
  sl_zigbee_event_init(&finding_and_binding_event,
                       finding_and_binding_event_handler);
  emberAfAppPrintln("Main initialization.");
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
  if (status == EMBER_NETWORK_UP) {
    sl_zigbee_event_set_active(&finding_and_binding_event);
  }
}

/** @brief Find and Bind Complete
 *
 * This callback is fired by the initiator when the Find and Bind process is
 * complete.
 *
 * @param status Status code describing the completion of the find and bind
 * process Ver.: always
 */
void emberAfPluginFindAndBindInitiatorCompleteCallback(EmberStatus status)
{
  sl_zigbee_app_debug_print("Find and bind initiator %s: 0x%02X\n",
                            "complete",
                            status);
  emberAfPluginBatteryMonitorV2InitCallback();
}
