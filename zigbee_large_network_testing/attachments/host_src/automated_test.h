/***************************************************************************//**
 * @file automated_test.h
 * @brief Zigbee Large Network Testing example - Automated test header
 * @version 1.0.0
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
******************************************************************************/

#ifndef AUTOMATED_TEST_H_
#define AUTOMATED_TEST_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "af.h"

/// Filename for storing local device database
#define SL_NODES_FILE "nodes.txt"

/// Maximum size of local device database
#define SL_NODES_INVENTORY_MAX_SIZE 128

/// Delay of consecutive On/Off messages that are sent
#define SL_TRAFFIC_DT_MS 1000

/// String to print before debug messages
#define SL_LARGE_NETWORK_TESTING_DEBUG "SL_LARGE_NETWORK_TESTING_DEBUG"

/// String to print before error messages
#define SL_LARGE_NETWORK_TESTING_ERROR "SL_LARGE_NETWORK_TESTING_ERROR"

#ifdef __cplusplus
extern "C" {
#endif

/// Command handlers
void sl_start_command(void);
void sl_stop_command(void);
void sl_inventory_command(void);

/// App function declarations
void sl_large_network_testing_trust_center_join_callback(EmberNodeId new_node_id, 
                                                      EmberDeviceUpdate status);
void sl_large_network_testing_main_init_callback(void);
uint8_t sl_remove_node(EmberNodeId node_id);
uint8_t sl_add_node(EmberNodeId new_node_id);
uint8_t sl_is_in_inventory(EmberNodeId node_id);
uint8_t sl_write_nodes_to_file(void);
uint8_t sl_get_devices(void);
void sl_toggle_sent_callback(EmberOutgoingMessageType type, 
                             uint16_t indexOrDestination, 
                             EmberApsFrame *apsFrame, 
                             uint16_t msgLen, 
                             uint8_t *message,
                             EmberStatus status);
uint8_t sl_custom_rand(void);
void sl_custom_traffic_event_handler(void);

#ifdef __cplusplus
}
#endif

#endif // AUTOMATED_TEST_H_