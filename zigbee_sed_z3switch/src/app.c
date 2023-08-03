/***************************************************************************//**
 * @file app.c
 * @brief Callbacks implementation and application specific code.
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "zigbee_sleep_config.h"
#include "network-steering.h"
#include "zll-commissioning.h"
#include "find-and-bind-initiator.h"

#define TRANSITION_TIME_DS           20
#define FINDING_AND_BINDING_DELAY_MS 3000
#define BUTTON0                      0
#define SWITCH_ENDPOINT              1

static bool commissioning = false;
static bool leaveNetwork = false;

static sl_zigbee_event_t commissioning_event;
static sl_zigbee_event_t finding_and_binding_event;

// ---------------
// Event handlers

static void commissioning_event_handler(sl_zigbee_event_t *event)
{
  EmberStatus status;
  uint16_t bytes;

  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    emberAfGetCommandApsFrame()->sourceEndpoint = SWITCH_ENDPOINT;
    bytes = emberAfFillCommandOnOffClusterToggle();
    if (bytes > 0) {
      emberAfSendCommandUnicastToBindings();
    } else {
      sl_zigbee_app_debug_print("Fill command failed!\n");
    }
  } else {
    if (!leaveNetwork) {
      status = emberLeaveNetwork();
      if ((status == EMBER_SUCCESS) || (status == EMBER_INVALID_CALL)) {
        leaveNetwork = true;
        status = emberAfPluginNetworkSteeringStart();
        if (status == EMBER_SUCCESS) {
        } else {
          sl_zigbee_app_debug_print("Initiate network failed: 0x%02X\n",
                                    status);
        }
      } else {
        sl_zigbee_app_debug_print("Leave network failed: 0x%02X\n", status);
      }
    }
  }
}

static void finding_and_binding_event_handler(sl_zigbee_event_t *event)
{
  emberAfPluginFindAndBindInitiatorStart(SWITCH_ENDPOINT);
}

// ----------------------
// Implemented Callbacks

void emberAfMainInitCallback(void)
{
  sl_zigbee_event_init(&commissioning_event, commissioning_event_handler);
  sl_zigbee_event_init(&finding_and_binding_event,
                       finding_and_binding_event_handler);
}

/** @brief Complete network steering.
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to EMBER_SUCCESS to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt.
 *
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID.
 *
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network.
 *
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete.
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

  if (status != EMBER_SUCCESS) {
    commissioning = false;
  } else {
    sl_zigbee_event_set_delay_ms(&finding_and_binding_event,
                                 FINDING_AND_BINDING_DELAY_MS);
  }
}

/** @brief Touch Link Complete
 *
 * This function is called by the ZLL Commissioning Common plugin when touch
 *   linking
 * completes.
 *
 * @param networkInfo The ZigBee and ZLL-specific information about the network
 * and target. Ver.: always
 * @param deviceInformationRecordCount The number of sub-device information
 * records for the target. Ver.: always
 * @param deviceInformationRecordList The list of sub-device information
 * records for the target. Ver.: always
 */
void emberAfPluginZllCommissioningCommonTouchLinkCompleteCallback(
  const EmberZllNetwork *networkInfo,
  uint8_t deviceInformationRecordCount,
  const EmberZllDeviceInfoRecord *deviceInformationRecordList)
{
  sl_zigbee_app_debug_print("%s network %s: 0x%02X\n",
                            "Touchlink",
                            "complete",
                            EMBER_SUCCESS);

  sl_zigbee_event_set_delay_ms(&finding_and_binding_event,
                               FINDING_AND_BINDING_DELAY_MS);
}

/** @brief Touch Link Failed
 *
 * This function is called by the ZLL Commissioning Client plugin if touch
 *   linking
 * fails.
 *
 * @param status The reason the touch link failed. Ver.: always
 */
void emberAfPluginZllCommissioningClientTouchLinkFailedCallback(
  EmberAfZllCommissioningStatus status)
{
  sl_zigbee_app_debug_print("%s network %s: 0x%02X\n",
                            "Touchlink",
                            "complete",
                            EMBER_ERR_FATAL);

  commissioning = false;
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
  commissioning = false;
}

/** @brief
 *
 * Application framework equivalent of ::emberRadioNeedsCalibratingHandler
 */
void emberAfRadioNeedsCalibratingCallback(void)
{
  sl_mac_calibrate_current_channel();
}

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT) \
  && (SL_ZIGBEE_APP_FRAMEWORK_USE_BUTTON_TO_STAY_AWAKE == 0)
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

/***************************************************************************//**
 * A callback called in interrupt context whenever a button changes its state.
 *
 * @remark Can be implemented by the application if required. This function
 * can contain the functionality to be executed in response to changes of state
 * in each of the buttons, or callbacks to appropriate functionality.
 *
 * @note The button state should not be updated in this function, it is updated
 * by specific button driver prior to arriving here
 *
 *    @param[out] handle             Pointer to button instance
 ******************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  if (SL_SIMPLE_BUTTON_INSTANCE(BUTTON0) == handle) {
    if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED) {
      sl_zigbee_event_set_active(&commissioning_event);
    }
  }
}

#endif \
// SL_CATALOG_SIMPLE_BUTTON_PRESENT &&
// SL_ZIGBEE_APP_FRAMEWORK_USE_BUTTON_TO_STAY_AWAKE == 0

// Internal testing stuff
#if defined(EMBER_TEST)
void emberAfHalButtonIsrCallback(uint8_t button,
                                 uint8_t state)
{
  if (state == BUTTON_RELEASED) {
    sl_zigbee_event_set_active(&commissioning_event);
  }
}

#endif // EMBER_TEST
