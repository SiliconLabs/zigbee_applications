/***************************************************************************//**
 * @file appLinkKey.c
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
 *  This file allows the user to request an application link key to communicate
 *  with another device in the network specified by its eui64.
 *    - appLKey {eui64 of the remote node}
 *
 *  HOW TO USE :
 *   Include the header to the app.c
 *   In the emberAfMainInitCallback(), initialize the custom command group such
 *   as :
 *   sl_cli_command_add_command_group(sl_cli_handles[0],
 *   &my_request_cli_command_group);
 *   (refer to app.c of this project as an example).
 ******************************************************************************/

/***************************************************************************//**
 * Variable declarations / Includes.
 ******************************************************************************/

#include "appLinkKey.h"

static const sl_cli_command_info_t myRequestLinkCommand =
  SL_CLI_COMMAND(myRequestAppLinkKeyHandler,
                 "Function to request App Link Key",
                 "EUI64 of the other node",
                 { SL_CLI_ARG_HEX, SL_CLI_ARG_END });

static const sl_cli_command_entry_t my_request_cli_commands[] = {
  { "appLKey", &myRequestLinkCommand, false },
  { NULL, NULL, false },
};

sl_cli_command_group_t my_request_cli_command_group = {
  { NULL },
  false,
  my_request_cli_commands
};

/***************************************************************************//**
 * Functions & events.
 ******************************************************************************/

/**
 * @brief CLI Handler to request the TC an Application Link Key
 *
 * @param args
 */
void myRequestAppLinkKeyHandler(sl_cli_command_arg_t *args)
{
  EmberStatus status;
  EmberEUI64 eui64;
  EmberAfApplicationTask currentTask;
  sl_zigbee_copy_eui64_arg(args, 0, eui64, true);

  /* Fast Poll Mode enabled */
  // Waiting for the APS Link Key
  currentTask = EMBER_AF_FORCE_SHORT_POLL;
  emberAfAddToCurrentAppTasks(currentTask);
  emberAfSetWakeTimeoutBitmaskCallback(currentTask);

  // Request the Link key
  status = emberRequestLinkKey(eui64);
  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("Successfully requested the Link key");
  }
}
