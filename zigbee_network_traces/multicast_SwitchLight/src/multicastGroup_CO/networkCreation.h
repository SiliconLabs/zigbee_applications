/***************************************************************************//**
 * @file networkCreation.h
 * @brief Header file
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

#ifndef NETWORKCREATION_H_
#define NETWORKCREATION_H_

/***************************************************************************//**
 * INCLUDES & DEFINES
 ******************************************************************************/

#include "sl_cli_types.h"
#include "app/framework/include/af.h"
#include "network-creator-security.h"
#include "network-creator.h"
#include "stdbool.h"
#include "ember-types.h"
#include "sl_cli_handles.h"
#include "af-security.h"

#define NODETABLE_SIZE (3)
#define SWITCH_ENDPOINT (1)
#define GROUP_ID (0x4123)

/***************************************************************************//**
 * Function Prototypes
 ******************************************************************************/

void networkOpenHandler(sl_cli_command_arg_t *arguments);
void eventNetworkFormHandler(sl_cli_command_arg_t *arguments);
void groupCreateHandler(sl_cli_command_arg_t *arguments);

/***************************************************************************//**
 * Variables
 ******************************************************************************/

extern sl_zigbee_event_t networkFormCtrl;
extern sl_zigbee_event_t networkOpenCtrl;
extern sl_cli_command_group_t my_cli_command_group;

#endif /* NETWORKCREATION_H_ */
