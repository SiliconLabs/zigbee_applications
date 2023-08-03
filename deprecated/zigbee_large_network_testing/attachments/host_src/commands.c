/***************************************************************************//**
 * @file commands.c
 * @brief Zigbee Large Network Testing example - Commands
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
 * DEPRECATION NOTICE
 * This code has been deprecated. It has been provided for historical reference
 * only and should not be used. This code will not be maintained.
 * This code is subject to the quality disclaimer at the point in time prior
 * to deprecation and superseded by this deprecation notice.
******************************************************************************/

#include "commands.h"
#include "automated_test.h"

/// Counter name strings from the couters plugin
extern const char *titleStrings[];

/// Static functions declarations
static void sl_get_counter_command(void);
static void sl_get_table_command(void);

EmberCommandEntry emberAfCustomCommands[] = {
  emberCommandEntryAction("get-counter",
                          sl_get_counter_command,
                          "v",
                          "Get counters OTA from a remote device"),
  emberCommandEntryAction("get-table",
                          sl_get_table_command,
                          "vb",
                          "Get tables OTA from a remote device"),
  emberCommandEntryAction("start", 
                          sl_start_command, 
                          "", 
                          "Start random unicast commands"),
  emberCommandEntryAction("stop",
                          sl_stop_command,
                          "",
                          "Stop random unicast commands"),
  emberCommandEntryAction("inventory",
                          sl_inventory_command, 
                          "", 
                          "Print device inventory"),
  emberCommandEntryTerminator()
};

/**************************************************************************//**
 * @brief Copy EUI64 between two buffers
 * @note Copy happens from MSB format to LSB format
 * 
 * @param from Buffer from where to copy
 * @param to Buffer where to copy
*****************************************************************************/
void sl_copy_eui64(uint8_t *from, uint8_t *to)
{
  uint8_t i;
  uint8_t eui_size = sizeof(EmberEUI64);
  for (i = 0; i < eui_size; i++) {
    to[i] = from[eui_size-1-i];
  }
}

/**************************************************************************//**
 * @brief Print counters
 * @note EMBER_COUNTER_TYPE_COUNT counters will be printed 
 * 
 * @param counters Array holding coounter values
*****************************************************************************/
void sl_print_counters(uint16_t *counters)
{
  uint8_t i;
  for (i = 0; i < EMBER_COUNTER_TYPE_COUNT; i++) {
    emberAfCorePrintln("%u) %p: %u", i, titleStrings[i], counters[i]);
  }
}

// -------------------------------
// Cluster command response callbacks

/**************************************************************************//** 
 * @brief Large network diagnostics Cluster Get Counters Response
 *
 * @param counters   Ver.: always
*****************************************************************************/
bool emberAfLargeNetworkDiagnosticsGetCountersResponseCallback(uint8_t *counters)
{
  EmberApsFrame *emberAfGetCommandApsFrame();
  emberAfCorePrintln("Received counters");
  sl_print_counters((uint16_t*)counters);

  return true;
}

/**************************************************************************//** 
 * @brief Large network diagnostics Cluster Get Neighbor Table Response 
 *
 * @param shortId   Ver.: always
 * @param averageLqi   Ver.: always
 * @param inCost   Ver.: always
 * @param outCost   Ver.: always
 * @param age   Ver.: always
 * @param longId   Ver.: always
*****************************************************************************/
bool emberAfLargeNetworkDiagnosticsGetNeighborTableResponseCallback(uint16_t shortId,
                                                                    uint8_t averageLqi,
                                                                    uint8_t inCost,
                                                                    uint8_t outCost,
                                                                    uint8_t age,
                                                                    uint8_t *longId)
{
  EmberNeighborTableEntry neighbor;
  neighbor.shortId = shortId;
  neighbor.averageLqi = averageLqi;
  neighbor.inCost = inCost;
  neighbor.outCost = outCost;
  neighbor.age = age;

  sl_copy_eui64(longId, neighbor.longId);

  emberAfCorePrintln("Received neighbor table entry");
  emberAfCorePrint("shortId: 0x%2X\t"
                   "averageLqi: %u\t"
                   "inCost: %u\t"
                   "outCost: %u\t"
                   "age: %u\tlongId: 0x", 
                    neighbor.shortId,
                    neighbor.averageLqi, 
                    neighbor.inCost, 
                    neighbor.outCost, 
                    neighbor.age);

  for (uint8_t i = 0; i < EUI64_SIZE; i++) {
    emberAfCorePrint("%X", neighbor.longId[i]);
  }
  emberAfCorePrintln("");

  return true;
}

/**************************************************************************//** 
 * @brief Large network diagnostics Cluster Get Child Table Response
 *
 * @param longId   Ver.: always
 * @param type   Ver.: always
 * @param id   Ver.: always
 * @param phy   Ver.: always
 * @param power   Ver.: always
 * @param timeout   Ver.: always
*****************************************************************************/
bool emberAfLargeNetworkDiagnosticsGetChildTableResponseCallback(uint8_t *longId,
                                                                 uint8_t type,
                                                                 uint16_t id,
                                                                 uint8_t phy,
                                                                 uint8_t power,
                                                                 uint8_t timeout)
{
  emberAfCorePrintln("Received child data");
  EmberChildData child;
  sl_copy_eui64(longId, child.eui64);
  child.type = type;
  child.id = id;
  child.phy = phy;
  child.power = power;
  child.timeout = timeout;
  emberAfCorePrintln("Received child table entry");
  emberAfCorePrint("shortId: 0x%2X\t"
                   "type: %u\t"
                   "PHY: %u\t"
                   "power: %u\t"
                   "timeout: %u\t"
                   "longId: 0x", 
                    child.id,
                    child.type,
                    child.phy,
                    child.power,
                    child.timeout);

  for (uint8_t i = 0; i < EUI64_SIZE; i++) {
    emberAfCorePrint("%X", child.eui64[i]);
  }
  emberAfCorePrintln("");

  return true;
}

/**************************************************************************//** 
 * @brief Large network diagnostics Cluster Get Routing Table Response
 *
 * @param destination   Ver.: always
 * @param nextHop   Ver.: always
 * @param status   Ver.: always
 * @param age   Ver.: always
 * @param concentratorType   Ver.: always
 * @param routeRecordState   Ver.: always
*****************************************************************************/
bool emberAfLargeNetworkDiagnosticsGetRoutingTableResponseCallback(uint16_t destination,
                                                                   uint16_t nextHop,
                                                                   uint8_t status,
                                                                   uint8_t age,
                                                                   uint8_t concentratorType,
                                                                   uint8_t routeRecordState)
{
  emberAfCorePrintln("Received route data");
  EmberRouteTableEntry route;
  
  route.destination = destination;
  route.nextHop = nextHop;
  route.status = status;
  route.age = age;
  route.concentratorType = concentratorType;
  route.routeRecordState = routeRecordState;

  emberAfCorePrintln("Received route table entry");
  emberAfCorePrintln("received route table entry: destination: 0x%2X\t"
                     "nextHop: 0x%2X\t"
                     "status: %u\t"
                     "age: %u\t"
                     " concentratorType: %u\t"
                     "routeRecordState: %u\t", 
                     route.destination, 
                     route.nextHop, 
                     route.status,
                     route.age,
                     route.concentratorType, 
                     route.routeRecordState);

  return true;
}


// -------------------------------
// Custom CLI commands' definitions

/**************************************************************************//**
 * @brief Get counter command handler
 * 
*****************************************************************************/
static void sl_get_counter_command(void)
{
  EmberNodeId remote = (EmberNodeId)emberUnsignedCommandArgument(0);
  emberAfCorePrintln("Requesting counters from remote device 0x%X", remote);

  emberAfFillCommandLargeNetworkDiagnosticsGetCounters()
  emberAfSetCommandEndpoints(1, 1);

  EmberStatus status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, remote);
  emberAfCorePrintln("Custom message sent: 0x%X", status);
}

/**************************************************************************//**
 * @brief Get tables command handler
 * 
*****************************************************************************/
static void sl_get_table_command(void)
{
  EmberNodeId remote = (EmberNodeId)emberUnsignedCommandArgument(0);
  char table[32];
  emberCopyStringArgument(1, table, 32, 0);
  if (!strcmp(table, "child")) {
    emberAfFillCommandLargeNetworkDiagnosticsGetChildTable();
  } else if (!strcmp(table, "neighbor") || !strcmp(table, "neigh")) {
    emberAfFillCommandLargeNetworkDiagnosticsGetNeighborTable();
  } else if (!strcmp(table, "routing")) {
    emberAfFillCommandLargeNetworkDiagnosticsGetRoutingTable();
  } else {
    emberAfCorePrintln("Unrecognized table type: %s", table);
  }

  emberAfSetCommandEndpoints(1, 1);
  EmberStatus status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, remote);
  emberAfCorePrintln("Custom message sent: 0x%X", status);
}