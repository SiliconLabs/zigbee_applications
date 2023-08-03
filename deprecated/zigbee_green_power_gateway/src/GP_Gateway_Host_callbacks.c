/***************************************************************************//**
* @file GP_Gateway_Host_callback.c
* @brief (optional)
* @version (optional)
*******************************************************************************
* # License
* <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
* DEPRECATION NOTICE
* This code has been deprecated. It has been provided for historical reference
* only and should not be used. This code will not be maintained.
* This code is subject to the quality disclaimer at the point in time prior
* to deprecation and superseded by this deprecation notice.
*
******************************************************************************/

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include "af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/util.h"

#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "stack/include/trust-center.h"
#include <stdlib.h>

// ID of the Green Power Toggle command
#define GP_CMD_TOGGLE  0x22
// the number of tokens that can be written using ezspSetToken and read
// using ezspGetToken
#define MFGSAMP_NUM_EZSP_TOKENS 8
// the size of the tokens that can be written using ezspSetToken and
// read using ezspGetToken
#define MFGSAMP_EZSP_TOKEN_SIZE 8
// the number of manufacturing tokens
#define MFGSAMP_NUM_EZSP_MFG_TOKENS 11
// the size of the largest EZSP Mfg token, EZSP_MFG_CBKE_DATA
// please refer to app/util/ezsp/ezsp-enum.h
#define MFGSAMP_EZSP_TOKEN_MFG_MAXSIZE 92

EmberStatus emberAfTrustCenterStartNetworkKeyUpdate(void);

// Forward declarations of custom cli command functions
static void mfgappTokenDump(void);
static void changeNwkKeyCommand(void);
static void printNextKeyCommand(void);
static void versionCommand(void);
static void setTxPowerCommand(void);
static void setTarget(void);

uint8_t targetBindingIndex = 0;
bool targetSet = false;

EmberCommandEntry emberAfCustomCommands[] = {
  //emberCommandEntryAction("print_srt", printSourceRouteTable, "", ""),// Concentrator plugin, should eb used. for this
  emberCommandEntryAction("tokdump", mfgappTokenDump, "", ""),
  emberCommandEntryAction("changeNwkKey", changeNwkKeyCommand, "", ""),
  emberCommandEntryAction("printNextKey", printNextKeyCommand, "", ""),
  emberCommandEntryAction("version", versionCommand, "", ""),
  emberCommandEntryAction("txPower", setTxPowerCommand, "s", ""),
  emberCommandEntryAction("set-target", setTarget, "v", "Sets the target of the GP message"),
  emberCommandEntryTerminator()
};

//// ******* test of token dump code

// the manufacturing tokens are enumerated in app/util/ezsp/ezsp-protocol.h
// the names are enumerated here to make it easier for the user
const char * ezspMfgTokenNames[] =
{
  "EZSP_MFG_CUSTOM_VERSION...",
  "EZSP_MFG_STRING...........",
  "EZSP_MFG_BOARD_NAME.......",
  "EZSP_MFG_MANUF_ID.........",
  "EZSP_MFG_PHY_CONFIG.......",
  "EZSP_MFG_BOOTLOAD_AES_KEY.",
  "EZSP_MFG_ASH_CONFIG.......",
  "EZSP_MFG_EZSP_STORAGE.....",
  "EZSP_STACK_CAL_DATA.......",
  "EZSP_MFG_CBKE_DATA........",
  "EZSP_MFG_INSTALLATION_CODE"
};

// IAS ACE Server side callbacks
bool emberAfIasAceClusterArmCallback(uint8_t armMode,
                                     uint8_t* armDisarmCode,
                                     uint8_t zoneId)
{
  uint16_t armDisarmCodeLength = emberAfStringLength(armDisarmCode);
  EmberNodeId sender = emberGetSender();
  uint16_t i;

  emberAfAppPrint("IAS ACE Arm Received %x", armMode);

  // Start i at 1 to skip over leading character in the byte array as it is the
  // length byte
  for (i = 1; i < armDisarmCodeLength; i++) {
    emberAfAppPrint("%c", armDisarmCode[i]);
  }
  emberAfAppPrintln(" %x", zoneId);

  emberAfFillCommandIasAceClusterArmResponse(armMode);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, sender);

  return true;
}

bool emberAfIasAceClusterBypassCallback(uint8_t numberOfZones,
                                        uint8_t* zoneIds,
                                        uint8_t* armDisarmCode)
{
  EmberNodeId sender = emberGetSender();
  uint8_t i;

  emberAfAppPrint("IAS ACE Cluster Bypass for zones ");

  for (i = 0; i < numberOfZones; i++) {
    emberAfAppPrint("%d ", zoneIds[i]);
  }
  emberAfAppPrintln("");

  emberAfFillCommandIasAceClusterBypassResponse(numberOfZones,
                                                zoneIds,
                                                numberOfZones);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, sender);

  return true;
}

// Called to dump all of the tokens. This dumps the indices, the names,
// and the values using ezspGetToken and ezspGetMfgToken. The indices
// are used for read and write functions below.
static void mfgappTokenDump(void)
{
  EmberStatus status;
  uint8_t tokenData[MFGSAMP_EZSP_TOKEN_MFG_MAXSIZE];
  uint8_t index, i, tokenLength;

  // first go through the tokens accessed using ezspGetToken
  emberAfCorePrintln("(data shown little endian)");
  emberAfCorePrintln("Tokens:");
  emberAfCorePrintln("idx  value:");
  for (index = 0; index < MFGSAMP_NUM_EZSP_TOKENS; index++) {
    // get the token data here
    status = ezspGetToken(index, tokenData);
    emberAfCorePrint("[%d]", index);
    if (status == EMBER_SUCCESS) {
      // Print out the token data
      for (i = 0; i < MFGSAMP_EZSP_TOKEN_SIZE; i++) {
        emberAfCorePrint(" %X", tokenData[i]);
      }

      emberSerialWaitSend(APP_SERIAL);
      emberAfCorePrintln("");
    } else {
      // handle when ezspGetToken returns an error
      emberAfCorePrintln(" ... error 0x%x ...",
                         status);
    }
  }

  // now go through the tokens accessed using ezspGetMfgToken
  // the manufacturing tokens are enumerated in app/util/ezsp/ezsp-protocol.h
  // this file contains an array (ezspMfgTokenNames) representing the names.
  emberAfCorePrintln("Manufacturing Tokens:");
  emberAfCorePrintln("idx  token name                 len   value");
  for (index = 0; index < MFGSAMP_NUM_EZSP_MFG_TOKENS; index++) {
    // ezspGetMfgToken returns a length, be careful to only access
    // valid token indices.
    tokenLength = ezspGetMfgToken(index, tokenData);
    emberAfCorePrintln("[%x] %p: 0x%x:", index,
                       ezspMfgTokenNames[index], tokenLength);

    // Print out the token data
    for (i = 0; i < tokenLength; i++) {
      if ((i != 0) && ((i % 8) == 0)) {
        emberAfCorePrintln("");
        emberAfCorePrint("                                    :");
        emberSerialWaitSend(APP_SERIAL);
      }
      emberAfCorePrint(" %X", tokenData[i]);
    }
    emberSerialWaitSend(APP_SERIAL);
    emberAfCorePrintln("");
  }
  emberAfCorePrintln("");
}

static void changeNwkKeyCommand(void)
{
  EmberStatus status = emberAfTrustCenterStartNetworkKeyUpdate();

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Change Key Error %x", status);
  } else {
    emberAfCorePrintln("Change Key Success");
  }
}

static void dcPrintKey(uint8_t label, uint8_t *key)
{
  uint8_t i;
  emberAfCorePrintln("key %x: ", label);
  for (i = 0; i < EMBER_ENCRYPTION_KEY_SIZE; i++) {
    emberAfCorePrint("%x", key[i]);
  }
  emberAfCorePrintln("");
}

static void printNextKeyCommand(void)
{
  EmberKeyStruct nextNwkKey;
  EmberStatus status;

  status = emberGetKey(EMBER_NEXT_NETWORK_KEY,
                       &nextNwkKey);

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Error getting key");
  } else {
    dcPrintKey(1, nextNwkKey.key.contents);
  }
}

static void versionCommand(void)
{
  emberAfCorePrintln("Version:  0.1 Alpha");
  emberAfCorePrintln(" %s", __DATE__);
  emberAfCorePrintln(" %s", __TIME__);
  emberAfCorePrintln("");
}

static void setTxPowerCommand(void)
{
  int8_t dBm = (int8_t)emberSignedCommandArgument(0);

  emberSetRadioPower(dBm);
}

/*
 * -- Green Power gateway application --
*/

/** @brief Setting GP message target
 * 	This function is being called when the custom command "set-target" is issued.
 * 	The first parameter in the custom CLI command is a NodeId.
 * 	This NodeId is paired with a binding table index.
 * 	The function finds the paired binding table index and
 * 	stores it in targetBindingIndex global variable.
 */
void setTarget(void)
{
	EmberNodeId selectedNodeId = (uint16_t)emberUnsignedCommandArgument(0);
	/*Sending command with binding table requires binding index.
	 * The binding index belonging to the nodeID needs to be found and stored in targetBindingIndex.
	*/
	uint8_t i;
	for (i = 0; i < EMBER_BINDING_TABLE_SIZE; i++){
		EmberBindingTableEntry binding;
		if (emberGetBinding(i, &binding) == EMBER_SUCCESS
			&& binding.type == EMBER_UNICAST_BINDING
			&& binding.clusterId == ZCL_ON_OFF_CLUSTER_ID
			&& selectedNodeId == emberGetBindingRemoteNodeId(i)){
			targetBindingIndex = i;
		    targetSet = true;
		    break;
		 }
	}

	if(i == EMBER_BINDING_TABLE_SIZE) //If no table index was found.
		emberAfCorePrintln("Node [0x%04X] has no On/Off cluster binding with the Green Power Gateway", selectedNodeId);
	else
		emberAfCorePrintln("Target nodeID set:[0x%04X]", selectedNodeId);
}


/** @brief Gp Notification
 *
 * This function is called by the Green Power Sink Plugin to notify the
 * application of a Green Power Gp Notification of an incomming gpd command.
 * Return true to handle in application.
 *
 * @param options from the incoming Gp Notification Command Ver.: always
 * @param addr GPD address        Ver.: always
 * @param gpdSecurityFrameCounter Ver.: always
 * @param gpdCommandId            Ver.: always
 * @param gpdCommandPayload first byte is length of the payload Ver.: always
 * @param gppShortAddress         Ver.: always
 * @param gppDistance             Ver.: always
 */
bool emberAfGreenPowerClusterGpNotificationForwardCallback(uint16_t options,
                                                           EmberGpAddress * addr,
                                                           uint32_t gpdSecurityFrameCounter,
                                                           uint8_t gpdCommandId,
                                                           uint8_t * gpdCommandPayload,
                                                           uint16_t gppShortAddress,
                                                           uint8_t  gppDistance)
{
	if(targetSet){ // Check if there's a valid target.
		if(gpdCommandId == GP_CMD_TOGGLE){ // Check incoming command.
			/* Sending toggle command.
			* Here you can change emberAfFillCommandOnOffClusterToggle() to your desired cluster specific command.
			*/
			emberAfFillCommandOnOffClusterToggle();
			emberAfSendCommandUnicast(EMBER_OUTGOING_VIA_BINDING, targetBindingIndex);
			emberAfCorePrintln("Sending Toggle command to node [0x%04X]", emberGetBindingRemoteNodeId(targetBindingIndex));
		}
		else
			emberAfCorePrintln("GP message was not toggle command");
	}
		else
			emberAfCorePrintln("No target nodeID has been set yet");

	return false;
}
