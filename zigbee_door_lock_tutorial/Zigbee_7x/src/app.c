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
//#include "hal/hal.h"
#include "app/util/ezsp/ezsp-enum.h"
#include <string.h>
#include "sl_simple_led_instances.h"

#define DOOR_LOCK_ENDPOINT 1 // We will use endpoint 1 for this example
typedef enum {
  DOOR_NOT_FULLY_LOCKED = 0x00,
  DOOR_LOCKED = 0x01,
  DOOR_UNLOCKED = 0x02,
  DOOR_UNDEFINED=0xFF
} doorLockState_t;

doorLockState_t doorLockStatus;
// function to check the input pin is the correct one
boolean checkDoorLockPin(int8u* PIN)
{
  bool isPinCorrect = FALSE;
  bool isPinInUse = FALSE;
  uint8_t maxPinLength;
  uint8_t minPinLength;
  doorLockPin_t storedPin;

  halCommonGetToken(&isPinInUse, TOKEN_DOOR_LOCK_PIN_IN_USE);

  if (isPinInUse)
    emberAfCorePrintln("Lock pin in use \n");
  else
    emberAfCorePrintln("Lock pin Not in use \n");

  // Get the Max Pin Length to DOOR_LOCK_PIN_STRING_MAX_LENGTH
  emberAfReadServerAttribute(DOOR_LOCK_ENDPOINT,
                              ZCL_DOOR_LOCK_CLUSTER_ID,
                              ZCL_MAX_PIN_LENGTH_ATTRIBUTE_ID,
                              &maxPinLength,
                              ZCL_DATA8_ATTRIBUTE_TYPE);

  // Get the Min Pin Length to DOOR_LOCK_PIN_STRING_LENGTH
  emberAfReadServerAttribute(DOOR_LOCK_ENDPOINT,
                             ZCL_DOOR_LOCK_CLUSTER_ID,
                             ZCL_MIN_PIN_LENGTH_ATTRIBUTE_ID,
                             &minPinLength,
                             ZCL_DATA8_ATTRIBUTE_TYPE);

  // If no pin is initialized, we will return TRUE
  if (isPinInUse == TRUE)
  {
      // Get the Pin from the token
      halCommonGetToken(&storedPin, TOKEN_DOOR_LOCK_PIN);

      // First check that the PIN is within the min and Max range
      if ((storedPin[0] <= maxPinLength) && (storedPin[0] >= minPinLength))
      {
          // Check the input pin against the stored pin
          int8_t result = memcmp(storedPin, PIN, storedPin[0]);

          if (result == 0)
          {
              isPinCorrect = TRUE;
          }
      }

  }
  else
  {
      isPinCorrect = TRUE;
  }

  return isPinCorrect;
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
boolean emberAfStackStatusCallback(EmberStatus status)
{
  return false;
}
/** @brief
 *
 * Application framework equivalent of ::emberRadioNeedsCalibratingHandler
 */
void emberAfRadioNeedsCalibratingCallback(void)
{
  sl_mac_calibrate_current_channel();
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
  emberAfCorePrintln("Network Steering Completed: %p (0x%X)",
                     (status == EMBER_SUCCESS ? "Join Success" : "FAILED"),
                     status);
  emberAfCorePrintln("Finishing state: 0x%X", finalState);
  emberAfCorePrintln("Beacons heard: %d\nJoin Attempts: %d", totalBeacons, joinAttempts);
}

/** @brief Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 */
void emberAfClusterInitCallback(int8u endpoint,
                                EmberAfClusterId clusterId)
{
  if (clusterId == ZCL_DOOR_LOCK_CLUSTER_ID)
      {
        uint8_t maxPinLength = DOOR_LOCK_PIN_STRING_MAX_LENGTH;
        uint8_t minPinLength = DOOR_LOCK_PIN_STRING_MIN_LENGTH;


        // Set the Max Pin Length to DOOR_LOCK_PIN_STRING_MAX_LENGTH
        emberAfWriteServerAttribute(DOOR_LOCK_ENDPOINT,
                                             ZCL_DOOR_LOCK_CLUSTER_ID,
                                             ZCL_MAX_PIN_LENGTH_ATTRIBUTE_ID,
                                             &maxPinLength,
                                             ZCL_DATA8_ATTRIBUTE_TYPE);

        // Set the Min Pin Length to DOOR_LOCK_PIN_STRING_LENGTH
        emberAfWriteServerAttribute(DOOR_LOCK_ENDPOINT,
                                                   ZCL_DOOR_LOCK_CLUSTER_ID,
                                                   ZCL_MIN_PIN_LENGTH_ATTRIBUTE_ID,
                                                   &minPinLength,
                                                   ZCL_DATA8_ATTRIBUTE_TYPE);

        // Read the Door Lock State
        emberAfReadServerAttribute(DOOR_LOCK_ENDPOINT,
                                             ZCL_DOOR_LOCK_CLUSTER_ID,
                                             ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                             &doorLockStatus,
                                             sizeof(doorLockStatus));

        // Set the LED on for locked, off for unlock
        if (doorLockStatus == DOOR_LOCKED)
        {
            sl_led_turn_on(&sl_led_led0);
        }
        else
        {
            sl_led_turn_off(&sl_led_led0);
        }
      }
}

/** @brief Door Lock Cluster Clear Pin
 *
 *
 *
 * @param userId   Ver.: always
 */
boolean emberAfDoorLockClusterClearPinCallback(int16u userId)
{

    if (userId == 1)
    {
        doorLockPin_t newPin = DOOR_LOCK_DEFAULT_PIN;
        boolean isPinInUse = FALSE;

        // Reset the pin and tokens
        halCommonSetToken(TOKEN_DOOR_LOCK_PIN, &newPin);
        halCommonSetToken(TOKEN_DOOR_LOCK_PIN_IN_USE, &isPinInUse);

        // If the pin is incorrect, send a Clear Pin Response with status success
        emberAfFillCommandDoorLockClusterClearPinResponse(EMBER_ZCL_STATUS_SUCCESS);
        emberAfSendResponse();
    }
    else
    {
        // If the pin is incorrect, send a Clear Pin Response with status not authorized
        emberAfFillCommandDoorLockClusterClearPinResponse(EMBER_ZCL_STATUS_NOT_AUTHORIZED);
        emberAfSendResponse();
    }

    return TRUE;
}

/** @brief Door Lock Cluster Get Pin
 *
 *
 *
 * @param userId   Ver.: always
 */
boolean emberAfDoorLockClusterGetPinCallback(int16u userId)
{
    bool isPinInUse = FALSE;
    doorLockPin_t storedPin;

    halCommonGetToken(&isPinInUse, TOKEN_DOOR_LOCK_PIN_IN_USE);


    // Put the stored pin into a buffer to send
    halCommonGetToken(&storedPin, TOKEN_DOOR_LOCK_PIN);

    // Send the Get Pin response containing the pin
    emberAfFillCommandDoorLockClusterGetPinResponse(0,
                                                    0,
                                                    0,
                                                    storedPin);
    emberAfSendResponse();

    return TRUE;
}





/** @brief Door Lock Cluster Set Pin
 *
 *
 *
 */
boolean emberAfDoorLockClusterSetPinCallback(int8u* pin)
{
  uint8_t maxPinLength;
  uint8_t minPinLength;

  // Get the Max Pin Length to DOOR_LOCK_PIN_STRING_MAX_LENGTH
    emberAfReadServerAttribute(DOOR_LOCK_ENDPOINT,
                                ZCL_DOOR_LOCK_CLUSTER_ID,
                                ZCL_MAX_PIN_LENGTH_ATTRIBUTE_ID,
                                &maxPinLength,
                                ZCL_DATA8_ATTRIBUTE_TYPE);

    // Get the Min Pin Length to DOOR_LOCK_PIN_STRING_LENGTH
    emberAfReadServerAttribute(DOOR_LOCK_ENDPOINT,
                               ZCL_DOOR_LOCK_CLUSTER_ID,
                               ZCL_MIN_PIN_LENGTH_ATTRIBUTE_ID,
                               &minPinLength,
                               ZCL_DATA8_ATTRIBUTE_TYPE);

   if ((pin[0] <= maxPinLength) && (pin[0] >= minPinLength))
   {
       boolean isPinInUse = TRUE;

       // Set the pin tokens
       halCommonSetToken(TOKEN_DOOR_LOCK_PIN, pin);
       halCommonSetToken(TOKEN_DOOR_LOCK_PIN_IN_USE, &isPinInUse);

       // If the pin is incorrect, send a Set Pin Response with status success
       emberAfFillCommandDoorLockClusterSetPinResponse(EMBER_ZCL_STATUS_SUCCESS);
       emberAfSendResponse();
   }
   else
   {
       // If the pin is incorrect, send a Set Pin Response with status failure
       emberAfFillCommandDoorLockClusterSetPinResponse(EMBER_ZCL_STATUS_NOT_AUTHORIZED);
       emberAfSendResponse();
       return FALSE;
   }

   return TRUE;
}

/** @brief Door Lock command callback function
 *
 * This function lock the door if the door lock pin is correct.
 * It will return TRUE if the pin is correct or the pin token is set to
 * the default pin and FALSE if the pin is not correct.
 *
 * @param PIN
 */
boolean emberAfDoorLockClusterlockDoorCallback(int8u* PIN)
{


    // Check if the pin is correct
    boolean isPinCorrect = checkDoorLockPin(PIN);

    if (isPinCorrect == TRUE)
    {
        doorLockStatus = DOOR_LOCKED;
        //light on LED to simulate the lock
        sl_led_turn_on(&sl_led_led0);

        // We will set the Door Lock State attribute to "Locked"
        emberAfWriteServerAttribute(DOOR_LOCK_ENDPOINT,
                                             ZCL_DOOR_LOCK_CLUSTER_ID,
                                             ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                             &doorLockStatus,
                                             ZCL_ENUM8_ATTRIBUTE_TYPE);



        // Send a Door Lock Response with status success
        emberAfFillCommandDoorLockClusterLockDoorResponse(EMBER_ZCL_STATUS_SUCCESS);
        emberAfSendResponse();
    }
    else
    {
      // If the pin is incorrect, send a Door Lock Response with status failure
      emberAfFillCommandDoorLockClusterLockDoorResponse(EMBER_ZCL_STATUS_FAILURE);
      emberAfSendResponse();
      return FALSE;
    }

    return TRUE;
}
/** @brief Door unLock command callback function
 *
 * This function unlock the door if lock pin is correct. It will return TRUE if
 * the pin is correct or the pin token is set to the default pin and FALSE if
 * the pin is not correct.
 *
 * @param PIN
 */
boolean emberAfDoorLockClusterUnlockDoorCallback(int8u* PIN)
{

      doorLockState_t doorLockStatus;

      // Check if the pin is correct
      boolean isPinCorrect = checkDoorLockPin(PIN);

      if (isPinCorrect == TRUE)
      {
          doorLockStatus = DOOR_UNLOCKED;

          // LED off simulate door closed
          sl_led_turn_off(&sl_led_led0);
          // We will set the Door Lock State attribute to "unLocked"
          emberAfWriteServerAttribute(DOOR_LOCK_ENDPOINT,
                                               ZCL_DOOR_LOCK_CLUSTER_ID,
                                               ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                               &doorLockStatus,
                                               ZCL_ENUM8_ATTRIBUTE_TYPE);


          // Send a Door Lock Response with status success
          emberAfFillCommandDoorLockClusterLockDoorResponse(EMBER_ZCL_STATUS_SUCCESS);
          emberAfSendResponse();
      }
      else
      {
        // If the pin is incorrect, send a Door Lock Response with status failure
        emberAfFillCommandDoorLockClusterLockDoorResponse(EMBER_ZCL_STATUS_FAILURE);
        emberAfSendResponse();
        return FALSE;
      }


    return TRUE;
}

uint32_t emberAfLockClusterServerCommandParse(sl_service_opcode_t opcode,

                                               sl_service_function_context_t *context)

{

  (void)opcode;
   bool wasHandled = false;
   uint8_t* PIN;



  EmberAfClusterCommand *cmd = (EmberAfClusterCommand *)context->data;
  uint16_t payloadOffset = cmd->payloadStartIndex;


  if (!cmd->mfgSpecific) {

      switch (cmd->commandId) {
        case ZCL_LOCK_DOOR_COMMAND_ID:
        {
           sl_led_turn_on(&sl_led_led0);


        // PIN[0] = Length
           PIN = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);

           wasHandled = emberAfDoorLockClusterlockDoorCallback(PIN);
           break;
         }
         case ZCL_UNLOCK_DOOR_COMMAND_ID:
         {
             PIN = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);

             wasHandled = emberAfDoorLockClusterUnlockDoorCallback(PIN);
             break;
          }
         case ZCL_SET_PIN_COMMAND_ID:
         {
             PIN = emberAfGetString(cmd->buffer, payloadOffset+4, cmd->bufLen);

             wasHandled = emberAfDoorLockClusterSetPinCallback(PIN);
             break;
         }
         case ZCL_GET_PIN_COMMAND_ID:
         {
            wasHandled = emberAfDoorLockClusterGetPinCallback(1);
            break;
         }
         case ZCL_CLEAR_PIN_COMMAND_ID:
         {
            wasHandled = emberAfDoorLockClusterClearPinCallback(1);
            break;
         }
      }

  }
      return ((wasHandled)
                ? EMBER_ZCL_STATUS_SUCCESS
                : EMBER_ZCL_STATUS_UNSUP_COMMAND);
}





void emberAfMainInitCallback(void)

{
  doorLockPin_t initPin = DOOR_LOCK_DEFAULT_PIN;
  boolean isPinInUse = TRUE;

  emberAfClusterInitCallback(DOOR_LOCK_ENDPOINT,
                             ZCL_DOOR_LOCK_CLUSTER_ID);


  // Set the pin tokens
  halCommonSetToken(TOKEN_DOOR_LOCK_PIN, &initPin);
  halCommonSetToken(TOKEN_DOOR_LOCK_PIN_IN_USE, &isPinInUse);

  sl_led_init(&sl_led_led0);
  sl_zigbee_subscribe_to_zcl_commands(ZCL_DOOR_LOCK_CLUSTER_ID,

                                        0xFFFF,

                                        ZCL_DIRECTION_CLIENT_TO_SERVER,

                                        emberAfLockClusterServerCommandParse);

}

