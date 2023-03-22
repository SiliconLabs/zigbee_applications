/***************************************************************************//**
 * @file networkJoin.c
 * @brief File with Events and Function to join the network.
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
 *  This file grants you a set of custom CLI commands to join a network
 *    - join : Launch Network Steering process to join the network
 *
 *  HOW TO USE :
 *    Include the header file in your app.c
 *    Create the emberAfMainInitCallback() into the app.c file and initialize
 *    the command group : my_cli_command_group, and events if necessary
 *    (refer to app.c of this project as an example)
 ******************************************************************************/

/***************************************************************************//**
 * Variable declarations / Includes.
 ******************************************************************************/

#include "networkJoin.h"

sl_zigbee_event_t eventRouterInfoCtrl;

static const sl_cli_command_info_t myJoinCommand =
  SL_CLI_COMMAND(myJoinHandler,
                 "Function to join the network with install-code",
                 "None",
                 { SL_CLI_ARG_END });
const sl_cli_command_entry_t my_cli_commands[] = {
  { "join", &myJoinCommand, false },
  { NULL, NULL, false },
};

sl_cli_command_group_t my_cli_command_group = {
  { NULL },
  false,
  my_cli_commands
};

/***************************************************************************//**
 * Functions & events.
 ******************************************************************************/

/**
 * @brief CLI Handler to join a network with join command
 *
 * @param arguments CLI arguments
 */
void myJoinHandler(sl_cli_command_arg_t *arguments)
{
  // Launch the join process
  EmberStatus status;
  status = emberAfPluginNetworkSteeringStart();

  // Check the status of the network Steering
  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("Start of Network Steering successful");
  } else {
    emberAfCorePrintln("ERROR to start Network Steering");
  }
}

/**
 * @brief Event to print info of the router
 *
 * @param context Context of the event
 */
void myEventRouterInfoHandler(sl_zigbee_event_context_t *context)
{
  EmberEUI64 eui64;
  emberAfGetEui64(eui64);

  // Display the eui64 info
  emberAfPrintBigEndianEui64(eui64);
}
