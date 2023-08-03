/***************************************************************************//**
 * @file
 * @brief Callback implementation for Zigbee_SmartLight_Light sample
 *   application.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.
#include "sl_simple_led.h"
#include "sl_simple_led_instances.h"

#include "app/framework/include/af.h"
#include "app/framework/plugin/network-steering/network-steering.h"
#include "app/framework/plugin/network-creator/network-creator.h"
#include \
  "app/framework/plugin/network-creator-security/network-creator-security.h"
#include "app/framework/plugin/find-and-bind-target/find-and-bind-target.h"

#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

#define LIGHT_ENDPOINT               (1)
#define FINDING_AND_BINDING_DELAY_MS 3000
#define LED_BLINK_PERIOD_MS          2000

#define led0_on()     sl_led_turn_on(&sl_led_led0);
#define led0_off()    sl_led_turn_off(&sl_led_led0);
#define led0_toggle() sl_led_toggle(&sl_led_led0);

#define led1_on()     sl_led_turn_on(&sl_led_led1);
#define led1_off()    sl_led_turn_off(&sl_led_led1);
#define led1_toggle() sl_led_toggle(&sl_led_led1);

sl_zigbee_event_t led_event;
sl_zigbee_event_t finding_and_binding_event;

static void on_off_cluster_server_attribute_changed_callback(uint8_t endpoint);

static void led_event_handler(void)
{
  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    uint16_t identifyTime;
    emberAfReadServerAttribute(LIGHT_ENDPOINT,
                               ZCL_IDENTIFY_CLUSTER_ID,
                               ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                               (uint8_t *)&identifyTime,
                               sizeof(identifyTime));
    if (identifyTime > 0) {
      led0_toggle();
      sl_zigbee_event_set_delay_ms(&led_event,
                                   LED_BLINK_PERIOD_MS);
    } else {
      led0_on();
    }
  } else {
    EmberStatus status = emberAfPluginNetworkSteeringStart();
    emberAfCorePrintln("%p network %p: 0x%X", "Join", "start", status);
  }
}

static void finding_and_binding_event_handler()
{
  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    sl_zigbee_event_set_inactive(&finding_and_binding_event);
    emberAfCorePrintln("Find and bind target start: 0x%X",
                       emberAfPluginFindAndBindTargetStart(LIGHT_ENDPOINT));

    sl_zigbee_event_set_active(&led_event);
  }
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
  // Note, the ZLL state is automatically updated by the stack and the plugin.
  if (status == EMBER_NETWORK_DOWN) {
    led0_off();
  } else if (status == EMBER_NETWORK_UP) {
    led0_on();
    sl_zigbee_event_set_delay_ms(&finding_and_binding_event,
                                 (LED_BLINK_PERIOD_MS * 2));
  }

// This value is ignored by the framework.
  return false;
}

// ----------------------
// Implemented Callbacks
void emberAfMainInitCallback(void)
{
  sl_zigbee_event_init(&led_event, led_event_handler);
  sl_zigbee_event_init(&finding_and_binding_event,
                       finding_and_binding_event_handler);
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
    status = emberAfPluginNetworkCreatorStart(false); // distributed
    emberAfCorePrintln("%p network %p: 0x%X", "Form", "start", status);
  }
}

/** @brief Complete
 *
 * This callback notifies the user that the network creation process has
 * completed successfully.
 *
 * @param network The network that the network creator plugin successfully
 * formed. Ver.: always
 * @param usedSecondaryChannels Whether or not the network creator wants to
 * form a network on the secondary channels Ver.: always
 */
void emberAfPluginNetworkCreatorCompleteCallback(
  const EmberNetworkParameters *network,
  bool usedSecondaryChannels)
{
  emberAfCorePrintln("%p network %p: 0x%X",
                     "Form distributed",
                     "complete",
                     EMBER_SUCCESS);
}

/** @brief On/off Cluster Server Post Init
 *
 * Following resolution of the On/Off state at startup for this endpoint,
 *   perform any
 * additional initialization needed; e.g., synchronize hardware state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginOnOffClusterServerPostInitCallback(uint8_t endpoint)
{
  // At startup, trigger a read of the attribute and possibly a toggle of the
  // LED to make sure they are always in sync.
  on_off_cluster_server_attribute_changed_callback(endpoint);
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
  if ((clusterId == ZCL_ON_OFF_CLUSTER_ID)
      && (attributeId == ZCL_ON_OFF_ATTRIBUTE_ID)
      && (mask == CLUSTER_MASK_SERVER)) {
    on_off_cluster_server_attribute_changed_callback(endpoint);
  }
}

/** @brief Server Attribute Changed
 *
 * On/off cluster, Server Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
static void on_off_cluster_server_attribute_changed_callback(uint8_t endpoint)
{
  // When the on/off attribute changes, set the LED appropriately.  If an error
  // occurs, ignore it because there's really nothing we can do.
  uint8_t onOff;

  if (emberAfReadServerAttribute(endpoint,
                                 ZCL_ON_OFF_CLUSTER_ID,
                                 ZCL_ON_OFF_ATTRIBUTE_ID,
                                 (uint8_t *)&onOff,
                                 sizeof(onOff))
      == EMBER_ZCL_STATUS_SUCCESS) {
    if (onOff) {
      led1_on();
    } else {
      led1_off();
    }
  }
}

// -----------------------------------------------------------------------------
// Push button event handler
void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED) {
    if (&sl_button_btn0 == handle) {
      sl_zigbee_event_set_active(&finding_and_binding_event);
    }
  }
}
