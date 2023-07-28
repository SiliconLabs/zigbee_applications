/***************************************************************************//**
 * @file queries.h
 * @brief Zigbee Large Network Testing example - Queries header
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

#ifndef QUERIES_H_
#define QUERIES_H_

#include "app/framework/include/af.h"
#include "app/framework/plugin/counters/counters.h"

/// Delay between the consecutive neighbor table entry messages
#define SL_NEXT_NEIGHBOR_TABLE_ENTRY_SEND_DELAY_MS 200

/// Delay between the consecutive child table entry messages
#define SL_NEXT_CHILD_TABLE_ENTRY_SEND_DELAY_MS 200

/// Delay between the consecutive route table entry messages
#define SL_NEXT_ROUTE_TABLE_ENTRY_SEND_DELAY_MS 200

/// Delay between re-sending a failed entry
#define SL_RETRY_TIME_MS 10

#ifdef __cplusplus
extern "C" {
#endif

/// App function declarations
bool emberAfLargeNetworkDiagnosticsGetCountersCallback(void);
bool emberAfLargeNetworkDiagnosticsGetNeighborTableCallback(void);
bool emberAfLargeNetworkDiagnosticsGetChildTableCallback(void);
bool emberAfLargeNetworkDiagnosticsGetRoutingTableCallback(void);
void sl_send_next_neighbor_table_entry(void);
void sl_send_next_child_table_entry(void);
void sl_send_next_route_table_entry(void);

#ifdef __cplusplus
}
#endif

#endif //QUERIES_H_