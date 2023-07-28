/***************************************************************************//**
 * @file commands.h
 * @brief Zigbee Large Network Testing example - Commands Header
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

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "af.h"
#include "app/framework/plugin/counters/counters-ota.h"
#include "automated_test.h"

#ifdef __cplusplus
extern "C" {
#endif

/// App function declarations
void sl_copy_eui64(uint8_t *from, uint8_t *to);
void sl_print_counters(uint16_t *counters);
bool emberAfLargeNetworkDiagnosticsGetCountersResponseCallback(uint8_t *counters);
bool emberAfLargeNetworkDiagnosticsGetNeighborTableResponseCallback(uint16_t shortId,
                                                            uint8_t averageLqi,
                                                            uint8_t inCost,
                                                            uint8_t outCost,
                                                            uint8_t age,
                                                            uint8_t *longId);
bool emberAfLargeNetworkDiagnosticsGetChildTableResponseCallback(uint8_t *longId,
                                                              uint8_t type,
                                                              uint16_t id,
                                                              uint8_t phy,
                                                              uint8_t power,
                                                              uint8_t timeout);
bool emberAfLargeNetworkDiagnosticsGetRoutingTableResponseCallback(uint16_t destination,
                                                      uint16_t nextHop,
                                                      uint8_t status,
                                                      uint8_t age,
                                                      uint8_t concentratorType,
                                                      uint8_t routeRecordState);

#ifdef __cplusplus
}
#endif

#endif // COMMANDS_H_