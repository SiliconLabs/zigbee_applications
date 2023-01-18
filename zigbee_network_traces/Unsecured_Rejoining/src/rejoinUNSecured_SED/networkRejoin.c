/***************************************************************************//**
 * @file networkRejoin.c
 * @brief File with Events and Function to rejoin a network.
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
 *  This file grants you a set of custom CLI commands to rejoin a network
 *    - rejoin : Launch the rejoin process
 *
 *  HOW TO USE :
 *    Include the header file in your app.c
 *    Create the emberAfMainInitCallback() into the app.c file and initialize
 *    the command group : my_rejoin_command_group, and events if necessary
 *    (refer to app.c of this project as an example)
 ******************************************************************************/

/***************************************************************************//**
 * Variable declarations / Includes.
 ******************************************************************************/

#include "networkRejoin.h"

static const sl_cli_command_info_t myRejoinCommand =
  SL_CLI_COMMAND(myRejoinHandler,
                 "Function to rejoin the network",
                 "None",
                 {SL_CLI_ARG_END});
                
static const sl_cli_command_entry_t my_cli_commands[] = {
  {"rejoin", &myRejoinCommand, false},
  {NULL, NULL, false},
};

sl_cli_command_group_t my_rejoin_command_group = {
  {NULL},
  false,
  my_cli_commands
};

/***************************************************************************//**
 * Functions & events.
 ******************************************************************************/

/**
 * @brief CLI Handler to rejoin a network with join command
 * 
 * @param arguments CLI arguments after the rejoin
 */
void myRejoinHandler(sl_cli_command_arg_t *arguments)
{
  // Launch the join process
  EmberStatus status;
  status = emberRejoinNetwork(false);

  // Check the status of the network Steering
  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("Start of Network Steering successful");
  } else {
    emberAfCorePrintln("ERROR to start Network Steering");
  }
}
