/***************************************************************************//**
 * @file networkCreation.c
 * @brief File with Events and Function to create & open the network.
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

/***************************************************************************//**
 * DESCRIPTION OF THE FILE
 *  This file grants you a set of custom CLI commands to form a network and
 *  open it for install-code
 *    - form : Create the Network
 *    - open : Allows to add a Transient Key to the TC to allow a specific device
 *    to join with its install-code key
 *
 *  HOW TO USE :
 *    Include this file and corresponding header files
 *    Create the emberAfMainInitCallback() into the app.c file and initialize
 *    the command group : my_cli_command_group, and events
 *    (refer to app.c of this project as an example)
 ******************************************************************************/

/***************************************************************************//**
 * Variable declarations / Includes.
 ******************************************************************************/

#include "networkCreation.h"

sl_zigbee_event_t networkFormCtrl;
sl_zigbee_event_t networkOpenCtrl;
static EmberNodeId nodeTable[3] = {0};
static int currentIndex = 0;

/// Create the CLI_Command_info
static const sl_cli_command_info_t myFormCreate_command =
  SL_CLI_COMMAND(eventNetworkFormHandler,
                 "Form and Create custom Network",
                 "No argument",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t myOpenInstallCode_command =
  SL_CLI_COMMAND(networkOpenwithCodeHandler,
                 "Open the network with Install-code",
                 "index<7:0>" SL_CLI_UNIT_SEPARATOR "eui64" SL_CLI_UNIT_SEPARATOR "install-code" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_HEX, SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

/// Create the entries
const sl_cli_command_entry_t my_cli_commands[] = {
  {"form", &myFormCreate_command, false},
  {"open", &myOpenInstallCode_command, false},
  {NULL, NULL, false},
};

/// Create the group of entries
sl_cli_command_group_t my_cli_command_group = {
  {NULL},
  false,
  my_cli_commands
};

/***************************************************************************//**
 * Functions & events.
 ******************************************************************************/

/**
 * @brief Callback when Network Creator is completed
 *
 * @param network Network parameters
 * @param usedSecondaryChannels Flag indicating use of Secondary channels
 */
void emberAfPluginNetworkCreatorCompleteCallback(const EmberNetworkParameters *network,
                                                 bool usedSecondaryChannels)
{
  // Launch Open Network Event
  emberAfCorePrintln("PanId : %d",network->panId);
}

void emberAfTrustCenterJoinCallback(EmberNodeId newNodeId,
                                    EmberEUI64 newNodeEui64,
                                    EmberNodeId parentOfNewNode,
                                    EmberDeviceUpdate status,
                                    EmberJoinDecision decision)
{
  emberAfCorePrintln("\n_______________TC Node joined Callback__________________");
  emberAfCorePrintln("Child 0x%x%x has %s the channel", ((newNodeId >> 8) & 0xff),
                     (newNodeId & 0xff), ((status) != EMBER_DEVICE_LEFT) ? "joined" : "left");
  // JOINING ?
  if(status != EMBER_DEVICE_LEFT) {
      nodeTable[currentIndex] = newNodeId;
      emberAfCorePrintln("Current NodeID : 0x%x%x",(newNodeId >> 8) & 0xff,
                         (newNodeId & 0xff));
      currentIndex += 1;
  }
  emberAfCorePrintln("________________________________________________________\n");
}

/**
 * @brief CLI Function to form the network when : form
 *
 * @param arguments CLI arguments when forming Network
 */
void eventNetworkFormHandler(sl_cli_command_arg_t *arguments)
{
  EmberStatus status;
  emberAfCorePrintln("Network formation");

  EmberNetworkStatus state;

  // Network Creation
  state = emberAfNetworkState();

  if (state != EMBER_JOINED_NETWORK) {
    status = emberAfPluginNetworkCreatorStart(true);
    emberAfCorePrintln("%p network %p: 0x%X", "Form", "start", status);
  } else {
    emberAfCorePrintln("Network already created");
  }
}

/**
 * @brief CLI Function to open the network with Install code
 *
 * @param arguments CLI arguments after open
 */
void networkOpenwithCodeHandler(sl_cli_command_arg_t *arguments)
{
  EmberStatus status;
  EmberEUI64 eui64;
  EmberKeyData keyData;
  EmberNetworkStatus state;
  size_t len = 18; // 16 bytes
  uint8_t code[18];

  emberAfCorePrintln("Open Network with Install Code");

  sl_zigbee_copy_eui64_arg(arguments, 1, eui64, true);
  sl_zigbee_copy_hex_arg(arguments, 2, code, sizeof(code), false);
  // Get the derived key from the code
  status = emAfInstallCodeToKey(code, len, &keyData);

  state = emberAfNetworkState();
  // Check if Network Created
  if (state != EMBER_JOINED_NETWORK) {
    emberAfCorePrintln("Network not Joined, cannot open the network");
  } else {
    emberAfCorePrintln("Network UP, opening process launched");
    emberAfCorePrintln("Derived key from Install-Code : ");
    status = emberAfPluginNetworkCreatorSecurityOpenNetworkWithKeyPair(eui64, keyData);
    emberAfCorePrintln("Network Open with Key : 0x%X", status);
  }
}
