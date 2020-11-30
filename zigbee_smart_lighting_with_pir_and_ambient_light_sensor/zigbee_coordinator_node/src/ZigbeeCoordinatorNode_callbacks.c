/***************************************************************************//**
 * @file
 * @brief Callback implementation for ZigbeeMinimal sample application.
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

#include "app/framework/include/af.h"

#include EMBER_AF_API_NETWORK_CREATOR
#include EMBER_AF_API_NETWORK_CREATOR_SECURITY
#include EMBER_AF_API_NETWORK_STEERING
#include EMBER_AF_API_FIND_AND_BIND_TARGET

#define SENSOR_ENDPOINT (1)

static bool appStart = false;
EmberEventControl appStartEventControl;
EmberEventControl reopenNetworkEventControl;


/** @brief handler for appStartEventControl
 *
 */
void appStartEventHandler()
{
  emberEventControlSetInactive(appStartEventControl);
  emberAfCorePrintln("Coordinator start \n");

  EmberStatus status = emberAfPluginNetworkCreatorStart(true); // true: centralize //false: distribute
  emberAfCorePrintln("%p network %p: 0x%X", "Form", "start", status);
}


/** @brief handler for reopenNetworkEventControl
 *
 */
void reopenNetworkEventHandler()
{
  emberEventControlSetInactive(reopenNetworkEventControl);
  emberAfCorePrintln("Open network \n");

  EmberStatus status =  emberAfPluginNetworkCreatorSecurityOpenNetwork();
  emberAfCorePrintln("Open network: 0x%X \n", status);

  emberEventControlSetDelayMS(reopenNetworkEventControl, 200000); // reopen after 200s
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
  if (status == EMBER_NETWORK_UP) {
      if(appStart){
          //start find and bind target
          EmberStatus status =  emberAfPluginFindAndBindTargetStart(SENSOR_ENDPOINT);
          emberAfCorePrintln("find and bind target : 0x%X \n", status);

          //open network
          emberEventControlSetActive(reopenNetworkEventControl);
      }
  }
  // This value is ignored by the framework.
  return false;
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
void emberAfPluginNetworkCreatorCompleteCallback(const EmberNetworkParameters *network,
                                                 bool usedSecondaryChannels)
{
  emberAfCorePrintln("Form network success \n");
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
void emberAfHalButtonIsrCallback(uint8_t button, uint8_t state)
{
  if (state == BUTTON_RELEASED) {
      if(!appStart){
          appStart = true;
          emberEventControlSetActive(appStartEventControl);
      }
  }
}
