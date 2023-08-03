/***************************************************************************//**
 * @file
 * @brief Callback implementation for ZigbeeMinimal sample application.

********************************************************************************
 * DEPRECATION NOTICE
 * This code has been deprecated. It has been provided for historical reference
 * only and should not be used. This code will not be maintained.
 * This code is subject to the quality disclaimer at the point in time prior
 * to deprecation and superseded by this deprecation notice.
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

#include <string.h>
#include "app/framework/include/af.h"

#define DOOR_LOCK_ENDPOINT 1 // We will use endpoint 1 for this example
#define DOOR_LOCK_PIN_STRING_MIN_LENGTH 4 // Set min pin length to 4

// ZCL Attributes
#define DOOR_LOCK_TYPE_DEAD_BOLT 0x00 // Per ZCL, dead bolt is 0x00
#define DOOR_LOCK_ACTUATOR_DISABLED FALSE // Per ZCL, actuator disabled is FALSE


boolean checkDoorLockPin(int8u* PIN); // Helper function to check the pin

// Door Lock State enum definition from the ZCL
typedef enum {
  DOOR_NOT_FULLY_LOCKED = 0x00,
  DOOR_LOCKED = 0x01,
  DOOR_UNLOCKED = 0x02,
  DOOR_UNDEFINED=0xFF
} doorLockState_t;

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 *
 * @param status   Ver.: always7
 */
bool emberAfStackStatusCallback(EmberStatus status)
{
  // This value is ignored by the framework.
  return false;
}

/** @brief Door Lock Cluster Lock Door
 * This function is called by the application framework when the node
 * receives a Lock Door Command.
 *
 * In this callback, we will implement the logic to "lock the door"
 * by checking the PIN, setting the LED, and sending the appropriate
 * response command.
 * @param PIN   Ver.: since ha-1.2-05-3520-29
 */
boolean emberAfDoorLockClusterLockDoorCallback(int8u* PIN)
{
  doorLockState_t doorLockStatus;

  // Check if the pin is correct
  boolean isPinCorrect = checkDoorLockPin(PIN);

  if (isPinCorrect == TRUE)
  {
      doorLockStatus = DOOR_LOCKED;

      // We will set the Door Lock State attribute to "Locked"
      emberAfWriteServerAttribute(DOOR_LOCK_ENDPOINT,
                                           ZCL_DOOR_LOCK_CLUSTER_ID,
                                           ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                           &doorLockStatus,
                                           ZCL_ENUM8_ATTRIBUTE_TYPE);

      // Turn on the LED to simulate the door locking
      halSetLed(1);

      // Send a Door Lock Response with status success
      emberAfFillCommandDoorLockClusterLockDoorResponse(EMBER_ZCL_STATUS_SUCCESS);
      emberAfSendResponse();
  }
  else
  {
    // If the pin is incorrect, send a Door Lock Response with status failure
    emberAfFillCommandDoorLockClusterLockDoorResponse(EMBER_ZCL_STATUS_FAILURE);
    emberAfSendResponse();
  }

  return TRUE;
}


/** @brief Door Lock Cluster Unlock Door
 * This function is called by the application framework when the node
 * receives an Unlock Door Command.
 *
 * In this callback, we will implement the logic to "unlock the door"
 * by checking the PIN, clearing the LED, and sending the appropriate
 * response command.
 * @param PIN   Ver.: since ha-1.2-05-3520-29
 */
boolean emberAfDoorLockClusterUnlockDoorCallback(int8u* PIN)
{
  doorLockState_t doorLockStatus;

  // Check if the pin is correct
  boolean isPinCorrect = checkDoorLockPin(PIN);

  if (TRUE == isPinCorrect) {
      doorLockStatus = DOOR_UNLOCKED;

      // We will set the Door Lock State attribute to "Unlocked"
      emberAfWriteServerAttribute(DOOR_LOCK_ENDPOINT,
                                           ZCL_DOOR_LOCK_CLUSTER_ID,
                                           ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                           &doorLockStatus,
                                           ZCL_ENUM8_ATTRIBUTE_TYPE);

      // Turn off the LED to simulate the door unlocking
      halClearLed(1);

      // Send a Door Lock Response with status success
      emberAfFillCommandDoorLockClusterUnlockDoorResponse(EMBER_ZCL_STATUS_SUCCESS);
      emberAfSendResponse();
  }
  else
  {
      // If the pin is incorrect, send a Door Lock Response with status failure
      emberAfFillCommandDoorLockClusterUnlockDoorResponse(EMBER_ZCL_STATUS_FAILURE);
      emberAfSendResponse();
  }

  return TRUE;
}

/** @brief Door Lock Cluster Set Pin
 * This function is called by the application framework when the node
 * receives an Set Pin Command.
 *
 * In this function we implement the logic to set and store the received pin,
 * as well as sending the appropriate ZCL response.
 *
 * @param userId   Ver.: always
 * @param userStatus   Ver.: always
 * @param userType   Ver.: always
 * @param pin   Ver.: always
 */
boolean emberAfDoorLockClusterSetPinCallback(int16u userId,
                                             int8u userStatus,
                                             int8u userType,
                                             int8u* pin)
{
  uint8_t sendPinOverTheAirRead;
  bool sendPinOverTheAir;

  // Check if the pin can be sent over the air
  emberAfReadServerAttribute(DOOR_LOCK_ENDPOINT,
                                       ZCL_DOOR_LOCK_CLUSTER_ID,
                                       ZCL_SEND_PIN_OVER_THE_AIR_ATTRIBUTE_ID,
                                       &sendPinOverTheAirRead,
                                       sizeof(sendPinOverTheAirRead));

  uint8_t maxPinLength;
  uint8_t minPinLength;

  sendPinOverTheAir = (bool) sendPinOverTheAirRead;

  //Get the max pin length
  emberAfReadServerAttribute(DOOR_LOCK_ENDPOINT,
                             ZCL_DOOR_LOCK_CLUSTER_ID,
                             ZCL_MAX_PIN_LENGTH_ATTRIBUTE_ID,
                             &maxPinLength,
                             sizeof(maxPinLength));

  //Get the min pin length
  emberAfReadServerAttribute(DOOR_LOCK_ENDPOINT,
                             ZCL_DOOR_LOCK_CLUSTER_ID,
                             ZCL_MIN_PIN_LENGTH_ATTRIBUTE_ID,
                             &minPinLength,
                             sizeof(minPinLength));

  if ((sendPinOverTheAir == TRUE) && (pin[0] <= maxPinLength) && (pin[0] >= minPinLength))
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
  }

  return TRUE;
}

/** @brief Door Lock Cluster Get Pin
 *
 * This function is called by the application framework when the node
 * receives a Get Pin Command.
 *
 * In this function we implement the logic to return the value of the pin
 * to the original sender.
 *
 * @param userId   Ver.: always
 */
boolean emberAfDoorLockClusterGetPinCallback(int16u userId)
{


  uint8_t sendPinOverTheAir;
  bool isPinInUse = FALSE;

  halCommonGetToken(&isPinInUse, TOKEN_DOOR_LOCK_PIN_IN_USE);

  // Check if the pin can be sent over the air
  emberAfReadServerAttribute(DOOR_LOCK_ENDPOINT,
                                       ZCL_DOOR_LOCK_CLUSTER_ID,
                                       ZCL_SEND_PIN_OVER_THE_AIR_ATTRIBUTE_ID,
                                       &sendPinOverTheAir,
                                       sizeof(sendPinOverTheAir));

  if (isPinInUse == TRUE)
  {
      if (sendPinOverTheAir == TRUE)
      {
          doorLockPin_t storedPin;

        // Put the stored pin into a buffer to send
        halCommonGetToken(&storedPin, TOKEN_DOOR_LOCK_PIN);

        // Send the Get Pin response containing the pin
        emberAfFillCommandDoorLockClusterGetPinResponse(0,
                                                        0,
                                                        0,
                                                        storedPin);
        emberAfSendResponse();
      }
      else
      {
          // Send the Default Response with status "Not Authorized"
          emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_AUTHORIZED);
          emberAfSendResponse();
      }
  }
  else
  {
      // Send the Default Response with status "Invalid Field"
      emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
      emberAfSendResponse();
  }

  return TRUE;
}

/** @brief Door Lock Cluster Clear Pin
 *
 * In this function we implement the logic to clear the value of the pin.
 *
 * @param userId   Ver.: always
 */
boolean emberAfDoorLockClusterClearPinCallback(int16u userId)
{
  uint8_t sendPinOverTheAir;

  // Check if the pin can be sent over the air
  emberAfReadServerAttribute(DOOR_LOCK_ENDPOINT,
                                       ZCL_DOOR_LOCK_CLUSTER_ID,
                                       ZCL_SEND_PIN_OVER_THE_AIR_ATTRIBUTE_ID,
                                       &sendPinOverTheAir,
                                       sizeof(sendPinOverTheAir));

  if ((sendPinOverTheAir == TRUE) && (userId == 0))
  {
      doorLockPin_t newPin = DOOR_LOCK_DEFAULT_PIN;
      boolean isPinInUse = FALSE;

      // Set the pin tokens
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

/** @brief Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * We will use this function to get and initialize the values for the
 * Door Lock cluster attributes.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 */
void emberAfClusterInitCallback(int8u endpoint,
                                EmberAfClusterId clusterId)
{

  if (clusterId == ZCL_DOOR_LOCK_CLUSTER_ID)
    {
      uint8_t lockType = DOOR_LOCK_TYPE_DEAD_BOLT;
      uint8_t actuatorEnabled = DOOR_LOCK_ACTUATOR_DISABLED;
      uint8_t maxPinLength = DOOR_LOCK_PIN_STRING_MAX_LENGTH;
      uint8_t minPinLength = DOOR_LOCK_PIN_STRING_MIN_LENGTH;
      uint8_t sendPinOverTheAir = TRUE;

      // Set the Lock Type to Dead Bolt
      emberAfWriteServerAttribute(DOOR_LOCK_ENDPOINT,
                                           ZCL_DOOR_LOCK_CLUSTER_ID,
                                           ZCL_LOCK_TYPE_ATTRIBUTE_ID,
                                           &lockType,
                                           ZCL_ENUM8_ATTRIBUTE_TYPE);

      // Set the Actuator Enabled to FALSE
      emberAfWriteServerAttribute(DOOR_LOCK_ENDPOINT,
                                           ZCL_DOOR_LOCK_CLUSTER_ID,
                                           ZCL_ACTUATOR_ENABLED_ATTRIBUTE_ID,
                                           &actuatorEnabled,
                                           ZCL_BOOLEAN_ATTRIBUTE_TYPE);

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

      // Set the Send Pin Over the Air to TRUE
      emberAfWriteServerAttribute(DOOR_LOCK_ENDPOINT,
                                                 ZCL_DOOR_LOCK_CLUSTER_ID,
                                                 ZCL_SEND_PIN_OVER_THE_AIR_ATTRIBUTE_ID,
                                                 &sendPinOverTheAir,
                                                 ZCL_BOOLEAN_ATTRIBUTE_TYPE);

      doorLockState_t lockStatus;

      // Read the Door Lock State
      emberAfReadServerAttribute(DOOR_LOCK_ENDPOINT,
                                           ZCL_DOOR_LOCK_CLUSTER_ID,
                                           ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                           &lockStatus,
                                           sizeof(lockStatus));

      // Set the LED accordingly
      if (DOOR_LOCKED == lockStatus)
      {
          halSetLed(1);
      }
      else
      {
          halClearLed(1);
      }
    }
}

/** @brief Check Door Lock Pin
 *
 * This function checks if the door lock pin is correct. It will return TRUE if
 * the pin is correct or the pin token is set to the default pin and FALSE if
 * the pin is not correct.
 *
 * @param PIN
 */
boolean checkDoorLockPin(int8u* PIN)
{
  bool isPinCorrect = FALSE;
  bool isPinInUse = FALSE;
  uint8_t maxPinLength;
  uint8_t minPinLength;

  halCommonGetToken(&isPinInUse, TOKEN_DOOR_LOCK_PIN_IN_USE);

  // If no pin is initialized, we will return TRUE
  if (isPinInUse == TRUE)
  {
      //Get the max pin length
      emberAfReadServerAttribute(DOOR_LOCK_ENDPOINT,
                                 ZCL_DOOR_LOCK_CLUSTER_ID,
                                 ZCL_MAX_PIN_LENGTH_ATTRIBUTE_ID,
                                 &maxPinLength,
                                 sizeof(maxPinLength));

      //Get the min pin length
      emberAfReadServerAttribute(DOOR_LOCK_ENDPOINT,
                                 ZCL_DOOR_LOCK_CLUSTER_ID,
                                 ZCL_MIN_PIN_LENGTH_ATTRIBUTE_ID,
                                 &minPinLength,
                                 sizeof(minPinLength));

      doorLockPin_t storedPin;

      // Get the Pin from the token
      halCommonGetToken(&storedPin, TOKEN_DOOR_LOCK_PIN);

      // First check that the PIN is not longer than the max length
      if ((storedPin[0] <= maxPinLength) && (storedPin[0] >= minPinLength))
      {
          // Check the input pin against the stored pin
          int8_t result = memcmp(storedPin, PIN, storedPin[0]);

          if (0 == result)
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


