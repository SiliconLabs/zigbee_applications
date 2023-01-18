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
 *  open it to join with well-known key
 *    - form : Create the Network
 *    - open : Open the Network
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

static EmberNodeId nodeTable[NODETABLE_SIZE] = {0};
static int currentIndex = 0;
static uint8_t groupName[] = {6,'L','i','g','h','t','s'};

// Create the CLI_Command_info
static const sl_cli_command_info_t myFormCreate_command =
    SL_CLI_COMMAND(eventNetworkFormHandler,
                   "Form and Create custom Network",
                   "No argument",
                   {SL_CLI_ARG_END, });
static const sl_cli_command_info_t myOpen_command =
    SL_CLI_COMMAND(networkOpenHandler,
                   "Open the network with well-known key",
                   "No Argument",
                   {SL_CLI_ARG_END, });
static const sl_cli_command_info_t myGroupCreate_command =
    SL_CLI_COMMAND(groupCreateHandler,
                  "Create the Group with Bindings",
                  "No argument",
                  {SL_CLI_ARG_END, });

// Create the entries
const sl_cli_command_entry_t my_cli_commands[] = {
    {"form", &myFormCreate_command, false},
    {"group", &myGroupCreate_command, false},
    {"open", &myOpen_command, false},
    {NULL, NULL, false},
};

// Create the group of entries
sl_cli_command_group_t my_cli_command_group = {
    {NULL},
    false,
    my_cli_commands
};

/***************************************************************************//**
 * Functions & events.
 ******************************************************************************/

/**
 * Callback when Network Creator is completed
 * @param network
 * @param usedSecondaryChannels
 */
void emberAfPluginNetworkCreatorCompleteCallback(const EmberNetworkParameters *network,
                                                 bool usedSecondaryChannels)
{
  // Launch Open Network Event
  emberAfCorePrintln("PanId : %d",network->panId);
}

/**
 * CLI Function to form the network when : form
 * @param arguments
 */
void eventNetworkFormHandler(sl_cli_command_arg_t *arguments)
{
  EmberStatus status;
  emberAfCorePrintln("Network formation");

  EmberNetworkStatus state;

  // Network Creation
  state = emberAfNetworkState();

  if (state != EMBER_JOINED_NETWORK)
  {
      status = emberAfPluginNetworkCreatorStart(true);
      emberAfCorePrintln("%p network %p: 0x%X", "Form", "start", status);
  }
  else
  {
      emberAfCorePrintln("Network already created");
  }
}

/**
 * CLI Function to open the network with to join with well-known key
 * @param arguments
 */
void networkOpenHandler(sl_cli_command_arg_t *arguments)
{
  EmberStatus status;
  EmberNetworkStatus state;

  emberAfCorePrintln("Open Network with Well-Known key");
  state = emberAfNetworkState();

  // Check if Network Created
  if (state != EMBER_JOINED_NETWORK)
  {
      emberAfCorePrintln("Network not Joined, cannot open the network");
  }
  else
  {
      emberAfCorePrintln("Network UP, opening process launched");
      status = emberAfPluginNetworkCreatorSecurityOpenNetwork();
      emberAfCorePrintln("Network Open with Key : 0x%X", status);
  }
}

/**
 * Custom CLI command to create group with Servers
 * @param arguments
 */
void groupCreateHandler(sl_cli_command_arg_t *arguments)
{
  for(uint8_t i = 0; i < currentIndex; i++)
  {
      // Prepare the command
      emberAfFillCommandGroupsClusterAddGroup(GROUP_ID,groupName);
      emberAfSetCommandEndpoints(emberAfPrimaryEndpoint(), 1);
      // Send the command
      emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeTable[i]);
  }
}

/**
 *
 * @param handle for node joined TC
 */
void emberAfTrustCenterJoinCallback(EmberNodeId newNodeId,
                                    EmberEUI64 newNodeEui64,
                                    EmberNodeId parentOfNewNode,
                                    EmberDeviceUpdate status,
                                    EmberJoinDecision decision)
{
  uint8_t cond = 1;
  emberAfCorePrintln("\n_______________TC Node joined Callback__________________");
  emberAfCorePrintln("Child 0x%x%x has %s the channel", ((newNodeId >> 8) & 0xff),
                     (newNodeId & 0xff), ((status) != EMBER_DEVICE_LEFT) ? "joined" : "left");
  // JOINING
  if((status != EMBER_DEVICE_LEFT) && (currentIndex < NODETABLE_SIZE))
  {

      // Check if already in the network
      for (uint8_t i = 0; i < currentIndex; i++)
      {
        if(newNodeId == nodeTable[i])
        {
          cond = 0;
        }
      }
      if(cond == 1)
      {
          nodeTable[currentIndex] = newNodeId;
          emberAfCorePrintln("Current NodeID : 0x%x%x",(newNodeId >> 8) & 0xff,
                                   (newNodeId & 0xff));
          currentIndex += 1;
      }
  }
  emberAfCorePrintln("________________________________________________________\n");
}

