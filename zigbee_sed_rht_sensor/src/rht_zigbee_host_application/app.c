/***************************************************************************//**
 * @file app.c
 * @brief RHT Sensor Zigbee Gateway Example
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

#include "af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/util.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "stack/include/trust-center.h"
#include "zap-cluster-command-parser.h"
#include <stdlib.h>

// The number of tokens that can be written using ezspSetToken and read using
// ezspGetToken.
#define MFGSAMP_NUM_EZSP_TOKENS 8
// The size of the tokens that can be written using ezspSetToken and read using
// ezspGetToken.
#define MFGSAMP_EZSP_TOKEN_SIZE 8
// The number of manufacturing tokens.
#define MFGSAMP_NUM_EZSP_MFG_TOKENS 11
// The size of the largest EZSP Mfg token, EZSP_MFG_CBKE_DATA please refer to
// app/util/ezsp/ezsp-enum.h.
#define MFGSAMP_EZSP_TOKEN_MFG_MAXSIZE 92

extern EmberStatus emberAfTrustCenterStartNetworkKeyUpdate(void);

//----------------------
// ZCL commands handling

static void ias_ace_cluster_arm_command_handler(uint8_t armMode,
                                                uint8_t* armDisarmCode,
                                                uint8_t zoneId)
{
  uint16_t armDisarmCodeLength = emberAfStringLength(armDisarmCode);
  EmberNodeId sender = emberGetSender();
  uint16_t i;

  sl_zigbee_app_debug_print("IAS ACE Arm Received %04X", armMode);

  // Start i at 1 to skip over leading character in the byte array as it is the
  // length byte
  for (i = 1; i < armDisarmCodeLength; i++) {
    sl_zigbee_app_debug_print("%c", armDisarmCode[i]);
  }
  sl_zigbee_app_debug_print(" %02X\n", zoneId);

  emberAfFillCommandIasAceClusterArmResponse(armMode);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, sender);
}

static void ias_ace_cluster_bypass_command_handler(uint8_t numberOfZones,
                                                   uint8_t* zoneIds,
                                                   uint8_t* armDisarmCode)
{
  EmberNodeId sender = emberGetSender();
  uint8_t i;

  sl_zigbee_app_debug_print("IAS ACE Cluster Bypass for zones ");

  for (i = 0; i < numberOfZones; i++) {
    sl_zigbee_app_debug_print("%d ", zoneIds[i]);
  }
  sl_zigbee_app_debug_print("\n");

  emberAfFillCommandIasAceClusterBypassResponse(numberOfZones,
                                                zoneIds,
                                                numberOfZones);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, sender);
}

static uint32_t zcl_ias_ace_cluster_server_command_handler(sl_service_opcode_t opcode,
                                                           sl_service_function_context_t *context)
{
  (void)opcode;

  EmberAfClusterCommand *cmd = (EmberAfClusterCommand *)context->data;

  switch (cmd->commandId) {
    case ZCL_ARM_COMMAND_ID:
    {
      sl_zcl_ias_ace_cluster_arm_command_t cmd_data;

      if (zcl_decode_ias_ace_cluster_arm_command(cmd, &cmd_data)
          != EMBER_ZCL_STATUS_SUCCESS) {
        return EMBER_ZCL_STATUS_UNSUP_COMMAND;
      }

      ias_ace_cluster_arm_command_handler(cmd_data.armMode,
                                          cmd_data.armDisarmCode,
                                          cmd_data.zoneId);
      return EMBER_ZCL_STATUS_SUCCESS;
    }
    case ZCL_BYPASS_COMMAND_ID:
    {
      sl_zcl_ias_ace_cluster_bypass_command_t cmd_data;

      if (zcl_decode_ias_ace_cluster_bypass_command(cmd, &cmd_data)
          != EMBER_ZCL_STATUS_SUCCESS) {
        return EMBER_ZCL_STATUS_UNSUP_COMMAND;
      }

      ias_ace_cluster_bypass_command_handler(cmd_data.numberOfZones,
                                             cmd_data.zoneIds,
                                             cmd_data.armDisarmCode);

      return EMBER_ZCL_STATUS_SUCCESS;
    }
  }

  return EMBER_ZCL_STATUS_UNSUP_COMMAND;
}

//----------------------
// Implemented Callbacks

/** @brief Init
 * Application init function
 */
void emberAfMainInitCallback(void)
{
  // Subscribe to ZCL commands for the IAS_ACE cluster, server side.
  sl_zigbee_subscribe_to_zcl_commands(ZCL_IAS_ACE_CLUSTER_ID,
                                      0xFFFF,
                                      ZCL_DIRECTION_CLIENT_TO_SERVER,
                                      zcl_ias_ace_cluster_server_command_handler);
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

  // Custom processing done only for cluster 0x405 and 0x402 (RH and Temperature
  // measurement clusters)
  if (clusterId == ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID ||
      clusterId == ZCL_TEMP_MEASUREMENT_CLUSTER_ID) {

    attribute = (buffer[bufLen-1] << 8) | (buffer[bufLen-2]);

    if (clusterId == ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID) {
      emberAfCorePrintln("RH: %2d.%2d %%",
                         attribute/100,
                         (attribute % 100));
    } else {
      emberAfCorePrintln("Temperature: %2d.%2d C",
                         attribute/100,
                         (attribute % 100));
    }
    return true;
  }

  return false;
}

#ifdef SL_CATALOG_CLI_PRESENT
//-------------------------------------
// Custom CLI commands and related code

// The manufacturing tokens are enumerated in app/util/ezsp/ezsp-protocol.h.
// The names are enumerated here to make it easier for the user.
static const char * ezspMfgTokenNames[] =
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

// Called to dump all of the tokens. This dumps the indices, the names,
// and the values using ezspGetToken and ezspGetMfgToken. The indices
// are used for read and write functions below.
void mfgappTokenDump(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  EmberStatus status;
  uint8_t tokenData[MFGSAMP_EZSP_TOKEN_MFG_MAXSIZE];
  uint8_t index, i, tokenLength;

  // first go through the tokens accessed using ezspGetToken
  sl_zigbee_app_debug_print("(data shown little endian)\n");
  sl_zigbee_app_debug_print("Tokens:\n");
  sl_zigbee_app_debug_print("idx  value:\n");
  for (index = 0; index < MFGSAMP_NUM_EZSP_TOKENS; index++) {
    // get the token data here
    status = ezspGetToken(index, tokenData);
    sl_zigbee_app_debug_print("[%d]", index);
    if (status == EMBER_SUCCESS) {
      // Print out the token data
      for (i = 0; i < MFGSAMP_EZSP_TOKEN_SIZE; i++) {
        sl_zigbee_app_debug_print(" %02X", tokenData[i]);
      }
      sl_zigbee_app_debug_print("\n");
    } else {
      // handle when ezspGetToken returns an error
      sl_zigbee_app_debug_print(" ... error 0x%02X ...\n", status);
    }
  }

  // now go through the tokens accessed using ezspGetMfgToken
  // the manufacturing tokens are enumerated in app/util/ezsp/ezsp-protocol.h
  // this file contains an array (ezspMfgTokenNames) representing the names.
  sl_zigbee_app_debug_print("Manufacturing Tokens:\n");
  sl_zigbee_app_debug_print("idx  token name                 len   value\n");
  for (index = 0; index < MFGSAMP_NUM_EZSP_MFG_TOKENS; index++) {
    // ezspGetMfgToken returns a length, be careful to only access
    // valid token indices.
    tokenLength = ezspGetMfgToken(index, tokenData);
    sl_zigbee_app_debug_print("[%x] %s: 0x%x:\n",
                              index, ezspMfgTokenNames[index], tokenLength);

    // Print out the token data
    for (i = 0; i < tokenLength; i++) {
      if ((i != 0) && ((i % 8) == 0)) {
        sl_zigbee_app_debug_print("\n                                    :");
      }
      sl_zigbee_app_debug_print(" %02X", tokenData[i]);
    }
    sl_zigbee_app_debug_print("\n");
  }
  sl_zigbee_app_debug_print("\n");
}

void changeNwkKeyCommand(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  EmberStatus status = emberAfTrustCenterStartNetworkKeyUpdate();

  if (status != EMBER_SUCCESS) {
    sl_zigbee_app_debug_print("Change Key Error %x\n", status);
  } else {
    sl_zigbee_app_debug_print("Change Key Success\n");
  }
}

static void dcPrintKey(uint8_t label, uint8_t *key)
{
  uint8_t i;
  sl_zigbee_app_debug_print("key %x: \n", label);
  for (i = 0; i < EMBER_ENCRYPTION_KEY_SIZE; i++) {
    sl_zigbee_app_debug_print("%02X", key[i]);
  }
  sl_zigbee_app_debug_print("\n");
}

void printNextKeyCommand(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  EmberKeyStruct nextNwkKey;
  EmberStatus status;

  status = emberGetKey(EMBER_NEXT_NETWORK_KEY,
                       &nextNwkKey);

  if (status != EMBER_SUCCESS) {
    sl_zigbee_app_debug_print("Error getting key\n");
  } else {
    dcPrintKey(1, nextNwkKey.key.contents);
  }
}

void versionCommand(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  sl_zigbee_app_debug_print("Version:  0.1 Alpha\n");
  sl_zigbee_app_debug_print(" %s\n", __DATE__);
  sl_zigbee_app_debug_print(" %s\n", __TIME__);
  sl_zigbee_app_debug_print("\n");
#ifdef EMBER_TEST
  sl_zigbee_app_debug_print("Print formatter test : 0x%x=0x12, 0x%02x=0x1234 0x%04x=0x12345678\n",
                            0x12, 0x1234, 0x12345678);
#endif
}

void setTxPowerCommand(sl_cli_command_arg_t *arguments)
{
  int8_t dBm = sl_cli_get_argument_int8(arguments, 0);

  emberSetRadioPower(dBm);
}

#endif
