/***************************************************************************//**
 * @file app_rht.c
 * @brief RHT sensor Zigbee example - Callbacks
 * @version 1.0.0
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided \'as-is\', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 * EVALUATION QUALITY
 * This code has been minimally tested to ensure that it builds with
 * the specified dependency versions and is suitable as a demonstration for
 * evaluation purposes only.
 * This code will be maintained at the sole discretion of Silicon Labs.
 ******************************************************************************/

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.
// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "app/framework/include/af.h"
#include "em_gpio.h"

#include EMBER_AF_API_NETWORK_STEERING
#include EMBER_AF_API_FIND_AND_BIND_INITIATOR

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define RHT_MEASUREMENT_ENDPOINT    1  // Endpoint with the RH and Temp ZCL
#define FIND_AND_BIND_DELAY_MS      3000 // Delay for fidn and bind handler
                                         // execution

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static EmberStatus reportAttribute(EmberAfClusterId cluster,
                                   EmberAfAttributeId attributeID);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
EmberEventControl networkSteeringEventControl; // Custom event control
EmberEventControl findingAndBindingEventControl; // Custom event control
EmberEventControl leaveNetworkEventControl; // Custom event control
EmberEventControl attributeReportEventControl; // Custom event control

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
static bool commissioning = false; // Holds the commissioning status
static bool binding = false; // Holds the binding status

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
void networkSteeringEventHandler(void);
void findingAndBindingEventHandler(void);
void leaveNetworkEventHandler(void);
void attributeReportEventHandler(void);

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
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
  // This value is ignored by the framework.
  return false;
}

/** @brief Main Init
 *
 * This function is called from the application's main function. It gives the
 * application a chance to do any initialization required at system startup. Any
 * code that you would normally put into the top of the application's main()
 * routine should be put into this function. This is called before the clusters,
 * plugins, and the network are initialized so some functionality is not yet
 * available.
        Note: No callback in the Application Framework is
 * associated with resource cleanup. If you are implementing your application on
 * a Unix host where resource cleanup is a consideration, we expect that you
 * will use the standard Posix system calls, including the use of atexit() and
 * handlers for signals such as SIGTERM, SIGINT, SIGCHLD, SIGPIPE and so on. If
 * you use the signal() function to register your signal handler, please mind
 * the returned value which may be an Application Framework function. If the
 * return value is non-null, please make sure that you call the returned
 * function from your handler to avoid negating the resource cleanup of the
 * Application Framework itself.
 *
 */
void emberAfMainInitCallback(void)
{
  // Enable the Si70xx sensor in the WSTK
  GPIO_PinModeSet(BSP_I2CSENSOR_ENABLE_PORT,
                  BSP_I2CSENSOR_ENABLE_PIN,
                  gpioModePushPull,
                  1);

  emberAfCorePrintln("RHT SED Zigbee example");
}

/** @brief Hal Button Isr
 *
 * This callback is called by the framework whenever a button is pressed on the
 * device. This callback is called within ISR context.
 *
 * @param button The button which has changed state, either BUTTON0 or BUTTON1
 * as defined in the appropriate BOARD_HEADER.  Ver.: always
 * @param state The new state of the button referenced by the button parameter,
 * either ::BUTTON_PRESSED if the button has been pressed or ::BUTTON_RELEASED
 * if the button has been released.  Ver.: always
 */
void emberAfHalButtonIsrCallback(int8u button, int8u state)
{
  // Check the button that triggered the ISR
  if (button == BUTTON0) {
    // Button 0 was pressed and released, begin network steering
    if (state == BUTTON_RELEASED) {
      emberEventControlSetActive(networkSteeringEventControl);
    }
  } else {
    // Button 1 was pressed and released, leave network
    if (state == BUTTON_RELEASED) {
      emberEventControlSetActive(leaveNetworkEventControl);
    }
  }
}

/** @brief Leave Network Event Handler
 *
 * This event handler is called in response to it's respective control
 * activation. It handles the network leaving process.
 *
 */
void leaveNetworkEventHandler(void)
{
  EmberStatus status;

  emberEventControlSetInactive(leaveNetworkEventControl);

  // Clear binding table
  status = emberClearBindingTable();
  emberAfCorePrintln("%p 0x%x", "Clear binding table", status);

  // Leave network
  status = emberLeaveNetwork();
  emberAfCorePrintln("%p 0x%x", "leave", status);

  commissioning = false;
  binding = false;
}

/** @brief Network Steering Event Handler
 *
 * This event handler is called in response to it's respective control
 * activation. It handles the network steering process. If already in a network
 * it forces the device to report it's attributes: Temperature and relative
 * humidity
 *
 */
void networkSteeringEventHandler(void)
{
  EmberStatus status;

  emberEventControlSetInactive(networkSteeringEventControl);

  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    // Check if the device has successfully established bindings, if not do so
    if (!binding) {
      emberEventControlSetActive(findingAndBindingEventControl);
    } else {
      // If already in a network and bindings are valid, report attributes
      emberAfCorePrintln("Reporting attributes!");
      emberEventControlSetActive(attributeReportEventControl);
    }
  } else {
    // If not in a network, attempt to join one
    status = emberAfPluginNetworkSteeringStart();
    emberAfCorePrintln("%p network %p: 0x%X",
                       "Join",
                       "start",
                       status);
    commissioning = true;
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
  emberAfCorePrintln("%p network %p: 0x%X", "Join", "complete", status);

  if (status != EMBER_SUCCESS) {
    commissioning = false;
  } else {
    // On successful join, do find and bind after a short delay
    emberEventControlSetDelayMS(findingAndBindingEventControl,
                                FIND_AND_BIND_DELAY_MS);
  }
}

/** @brief Find and Bind Event Handler
 *
 * This event handler is called in response to it's respective control
 * activation. It handles the find and bind process as an initiator. It requires
 * a valid target. Upon a successful procedure, a series of binding will be
 * added to the binding table of the device for matching clusters found in the
 * target.
 *
 */
void findingAndBindingEventHandler(void)
{
  emberEventControlSetInactive(findingAndBindingEventControl);

  EmberStatus status = emberAfPluginFindAndBindInitiatorStart(RHT_MEASUREMENT_ENDPOINT);

  emberAfCorePrintln("Find and bind initiator %p: 0x%X", "start", status);

  binding = true;
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
  emberAfCorePrintln("Find and bind initiator %p: 0x%X", "complete", status);

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Ensure a valid binding target!");
    binding = false;
  }

}

/** @brief Attributes report Event Handler
 *
 * This event handler is called in response to its respective control
 * activation. It will report the MeasuredValue of the RH and Temperature
 * measurement server clusters. Data will be sent though the matching binding
 *
 */
void attributeReportEventHandler(void)
{
  emberEventControlSetInactive(attributeReportEventControl);

  EmberStatus status = EMBER_SUCCESS;

  // Report RH MeasuredValue
  status = reportAttribute(ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                           ZCL_RELATIVE_HUMIDITY_MEASURED_VALUE_ATTRIBUTE_ID);

  emberAfCorePrintln("%p reported: 0x%X", "RH - MeasuredValue", status);

  // Report Temperature MeasuredValue
  status = reportAttribute(ZCL_TEMP_MEASUREMENT_CLUSTER_ID,
                           ZCL_TEMP_MEASURED_VALUE_ATTRIBUTE_ID);

  emberAfCorePrintln("%p reported: 0x%X", "Temp - MeasuredValue", status);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/** @brief Report Attribute
 *
 * This function reports the specified attribute from the specified cluster
 * through a valid binding in the binding table. Therefore, a valid binding
 * entry with a  matching cluster must exist for the report to be successful.
 *
 */
static EmberStatus reportAttribute(EmberAfClusterId cluster,
                                   EmberAfAttributeId attributeID)
{
  EmberStatus status = EMBER_SUCCESS;
  uint8_t buff[2] = {0x00,0x00}; // Attribute buffer
  uint8_t attribute_type;

  // Retrieve the specified attribute
  status = emberAfReadServerAttribute(RHT_MEASUREMENT_ENDPOINT,
                                      cluster,
                                      attributeID,
                                      buff,
                                      2);
  // Fill attribute record - See af-structs.h for details of ReportAttributeRecord
  // contents.

  // Note: ReportAttributeRecord typedef is not used to avoid data padding issues
  //       of structures in C
  if (cluster == ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID) {
    attribute_type = ZCL_INT16U_ATTRIBUTE_TYPE;
  } else {
    attribute_type = ZCL_INT16S_ATTRIBUTE_TYPE;
  }

  uint8_t attribute_record[] = {
    LOW_BYTE(attributeID),   //uint16_t attributeId
    HIGH_BYTE(attributeID),
    attribute_type,          //uint8_t attributeType;
    buff[0], buff[1],        //uint8_t* attributeLocation;
  };

  //Fill a ZCL global report attributes command buffer
  emberAfFillCommandGlobalServerToClientReportAttributes(cluster,
                                      attribute_record,
                                      sizeof(attribute_record)/sizeof(uint8_t));

  //Specify endpoints for command sending
  emberAfSetCommandEndpoints(RHT_MEASUREMENT_ENDPOINT, 1);

  //Use binding table to send unicast command
  status = emberAfSendCommandUnicastToBindings();

  return status;
}
