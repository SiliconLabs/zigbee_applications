/***************************************************************************//**
 * @file networkJoin.h
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

#ifndef NETWORKJOIN_H_
#define NETWORKJOIN_H_

/***************************************************************************//**
 * INCLUDES & DEFINES
 ******************************************************************************/

#include "sl_cli_types.h"
#include "app/framework/include/af.h"
#include "stdbool.h"
#include "ember-types.h"
#include "sl_cli_handles.h"
#include "af-security.h"
#include "network-steering.h"

#define INFO_DELAY_MS 1000
#define REJOIN_DELAY_MS 15000

/***************************************************************************//**
 * Function Prototypes
 ******************************************************************************/

void myJoinHandler(sl_cli_command_arg_t *arguments);
void myEventRejoinHandler(sl_zigbee_event_context_t *context);
void myEventRouterInfoHandler(sl_zigbee_event_context_t *context);

/***************************************************************************//**
 * Variables
 ******************************************************************************/

extern sl_zigbee_event_t eventRouterInfoCtrl;
extern sl_zigbee_event_t eventRejoinCtrl;
extern sl_cli_command_group_t my_cli_command_group;



#endif /* NETWORKJOIN_H_ */
