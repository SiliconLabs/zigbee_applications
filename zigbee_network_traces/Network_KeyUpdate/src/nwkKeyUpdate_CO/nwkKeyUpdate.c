/***************************************************************************//**
 * @file nwkKeyUpdate.c
 * @brief File with Events and Function to update the network key
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
 *  This file grants you a set of custom CLI commands to update the NWK key of
 *  the current network.
 *
 *  HOW TO USE :
 *    Include this file and corresponding header files
 *    Create the emberAfMainInitCallback() into the app.c file and initialize
 *    the command group : my_updateNwk_command_group,
 *    (look at the app.c of this project)
 ******************************************************************************/

/***************************************************************************//**
 * Variable declarations / Includes.
 ******************************************************************************/

#include "nwkKeyUpdate.h"

sl_zigbee_event_t networkUpdateCtrl;

static const sl_cli_command_info_t myNwkUpdate_command =
  SL_CLI_COMMAND(networkKeyUpdateHandler,
                  "Network Update Handler",
                  "No Argument",
                  {SL_CLI_ARG_END, });

/// Create the entries
static const sl_cli_command_entry_t my_updateNwk_cli_commands[] = {
  {"updateNwk", &myNwkUpdate_command, false},
  {NULL, NULL, false},
};

/// Create the group of entries
sl_cli_command_group_t my_updateNwk_command_group = {
  {NULL},
  false,
  my_updateNwk_cli_commands
};

/***************************************************************************//**
 * Functions & events.
 ******************************************************************************/

/**
 * @brief Callback when Network Key update complete
 * 
 * @param status
 */
void emberAfNetworkKeyUpdateCompleteCallback(EmberStatus status)
{
  EmberKeyStruct nwkKey;
  emberAfCorePrintln("Network Key Update Complete : ",
                     (status == EMBER_SUCCESS) ? "Success" : "Failed");
  if(status == EMBER_SUCCESS) {
    // Display new NWK Key
    emberGetKey(EMBER_CURRENT_NETWORK_KEY,&nwkKey);
    emberAfCorePrint("Current NWK Key : ");
    emberAfPrintZigbeeKey(nwkKey.key.contents);
  }
}

/**
 * @brief Custom CLI to launch a Network Key Update
 * The TC waits 9 sec before switching to this new key
 * 
 * @param context
 */
void networkKeyUpdateHandler(sl_cli_command_arg_t *arguments)
{
  EmberStatus status;

  status = emberAfTrustCenterStartNetworkKeyUpdate();
  // Check
  if(status == EMBER_SUCCESS)
  {
      emberAfCorePrintln("Successfully updated NWK key");
  }
  else
  {
      emberAfCorePrintln("Failed to update NWK Key");
  }
}
