/***************************************************************************//**
 * @file queries.c
 * @brief Zigbee Large Network Testing example - Queries
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

#include "queries.h"
#include "app/framework/util/af-main.h"

/// Counters from the stack
extern uint16_t emberCounters[EMBER_COUNTER_TYPE_COUNT];

/// Variables to store next indexes to send
uint8_t sl_next_neighbor_table_entry_to_send = 0;
uint8_t sl_next_child_table_entry_to_send = 0;
uint8_t sl_next_route_table_entry_to_send = 0;

/// Variables indicating wether a sending event is in progress
bool sl_next_neighbor_table_entry_send_in_progress = false;
bool sl_next_child_table_entry_send_in_progress = false;
bool sl_next_route_table_entry_send_in_progress = false;

/// Events for sending table entries
EmberEventControl sl_next_neighbor_table_send_event;
EmberEventControl sl_next_child_table_send_event;
EmberEventControl sl_next_route_table_send_event;

/**************************************************************************//**
 * @brief Handling send counters command
 * 
 * @return true if sending was successful
 * @return false if couldn't send message
 *****************************************************************************/
bool emberAfLargeNetworkDiagnosticsGetCountersCallback(void)
{
  emberAfFillCommandLargeNetworkDiagnosticsGetCountersResponse(emberCounters,
                                                               EMBER_COUNTER_TYPE_COUNT);
  emberAfSetCommandEndpoints(1, 1);

  EmberStatus status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, 0x0);
  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("Sent counters");
    return true;
  } else {
    emberAfCorePrintln("Couldn't send counters: 0x%X", status);
    return false;
  }
}

/**************************************************************************//**
 * @brief 
 * 
 * @return true if event could be started
 * @return false if an other sending event is already in progress
 *****************************************************************************/
bool emberAfLargeNetworkDiagnosticsGetNeighborTableCallback(void)
{
  if (!sl_next_neighbor_table_entry_send_in_progress) {
    emberAfCorePrintln("Sending neighbor table entries");
    sl_next_neighbor_table_entry_send_in_progress = true;
    sl_next_neighbor_table_entry_to_send = 0;
    emberEventControlSetDelayMS(sl_next_neighbor_table_send_event,
                                SL_NEXT_NEIGHBOR_TABLE_ENTRY_SEND_DELAY_MS);
    return true;
  } else {
    emberAfCorePrintln("Sending neighbor table entries is already in progress");
    return false;
  }
}

/**************************************************************************//**
 * @brief Handling send child table entries command. This will activate the sending event.
 * 
 * @return true if event could be started
 * @return false if an other sending event is already in progress
 *****************************************************************************/
bool emberAfLargeNetworkDiagnosticsGetChildTableCallback(void)
{
  if (!sl_next_child_table_entry_send_in_progress) {
    emberAfCorePrintln("Sending child table entries");
    sl_next_child_table_entry_send_in_progress = true;
    sl_next_child_table_entry_to_send = 0;
    emberEventControlSetDelayMS(sl_next_child_table_send_event,
                                SL_NEXT_CHILD_TABLE_ENTRY_SEND_DELAY_MS);
    return true;
  } else {
    emberAfCorePrintln("Sending child table entries is already in progress");
    return false;
  }
}

/**************************************************************************//**
 * @brief Handling send routing table entries command. This will activate the 
 * sending event.
 * 
 * @return true if event could be started
 * @return false if an other sending event is already in progress
 *****************************************************************************/
bool emberAfLargeNetworkDiagnosticsGetRoutingTableCallback(void)
{
  if (!sl_next_route_table_entry_send_in_progress) {
    emberAfCorePrintln("Sending route table entries");
    sl_next_route_table_entry_send_in_progress = true;
    sl_next_route_table_entry_to_send = 0;
    emberEventControlSetDelayMS(sl_next_route_table_send_event,
                                SL_NEXT_ROUTE_TABLE_ENTRY_SEND_DELAY_MS);
    return true;
  } else {
    emberAfCorePrintln("Sending route table entries is already in progress");
    return false;
  }
}

/**************************************************************************//**
 * @brief Send the next neighbor table entry and adjust index.
 * 
 *****************************************************************************/
void sl_send_next_neighbor_table_entry(void)
{
  emberAfCorePrintln("Neighbor table entry sending event");
  emberEventControlSetInactive(sl_next_neighbor_table_send_event);

  EmberNeighborTableEntry neighbor;
  EmberStatus status;
  
  uint8_t usedEntries = emberAfGetNeighborTableSize();
  status = emberGetNeighbor(sl_next_neighbor_table_entry_to_send, &neighbor);
  if ((status != EMBER_SUCCESS) || (neighbor.shortId == EMBER_NULL_NODE_ID)) {
    emberAfCorePrintln("Neighbor %d is invalid: 0x%X, status 0x%X",
                       sl_next_neighbor_table_entry_to_send,
                       neighbor.shortId,
                       status);

    sl_next_neighbor_table_entry_to_send++;
    if ((status == EMBER_ERR_FATAL)
      || (sl_next_neighbor_table_entry_to_send >= usedEntries)) {
      sl_next_neighbor_table_entry_send_in_progress = false;
      sl_next_neighbor_table_entry_to_send = 0;
      return;
    }
    emberEventControlSetDelayMS(sl_next_neighbor_table_send_event, SL_RETRY_TIME_MS);
    return;
  }

  emberAfFillCommandLargeNetworkDiagnosticsGetNeighborTableResponse(neighbor.shortId,
                                                                    neighbor.averageLqi,
                                                                    neighbor.inCost,
                                                                    neighbor.outCost,
                                                                    neighbor.age,
                                                                    neighbor.longId);
  emberAfSetCommandEndpoints(1, 1);

  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, 0x0);
  emberAfCorePrintln("Sending neighbor table entry %u (0x%2X) of %u: 0x%X",
                     sl_next_neighbor_table_entry_to_send,
                     neighbor.shortId,
                     usedEntries,
                     status);
  
  sl_next_neighbor_table_entry_to_send++;

  if (status != EMBER_SUCCESS) {
    emberEventControlSetDelayMS(sl_next_neighbor_table_send_event,
                                SL_RETRY_TIME_MS);
    return;
  }

  if (sl_next_neighbor_table_entry_to_send >= usedEntries) {
    sl_next_neighbor_table_entry_send_in_progress = false;
    sl_next_neighbor_table_entry_to_send = 0;
    return;
  }

  emberEventControlSetDelayMS(sl_next_neighbor_table_send_event,
                              SL_NEXT_NEIGHBOR_TABLE_ENTRY_SEND_DELAY_MS);
}

/**************************************************************************//**
 * @brief Send the next child table entry and adjust index.
 * 
 *****************************************************************************/
void sl_send_next_child_table_entry(void)
{
  emberEventControlSetInactive(sl_next_child_table_send_event);

  EmberChildData child;
  EmberStatus status;

  uint8_t usedEntries = emberAfGetChildTableSize();
  status = emberAfGetChildData(sl_next_child_table_entry_to_send, &child);
  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Child %d could not be fetched, status 0x%X", 
                       sl_next_child_table_entry_to_send, 
                       child.id,
                       status);

    sl_next_child_table_entry_to_send++;
    if ((status == EMBER_ERR_FATAL)
       || (status == EMBER_NOT_FOUND)
       || (sl_next_child_table_entry_to_send >= usedEntries)) {
      sl_next_child_table_entry_send_in_progress = false;
      sl_next_child_table_entry_to_send = 0;
      return;
    }
    emberEventControlSetDelayMS(sl_next_child_table_send_event, 
                                SL_RETRY_TIME_MS);
    return;
  }

  emberAfFillCommandLargeNetworkDiagnosticsGetChildTableResponse(child.eui64,
                                                                 child.type,
                                                                 child.id,
                                                                 child.phy,
                                                                 child.power,
                                                                 child.timeout);
  emberAfSetCommandEndpoints(1, 1);

  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, 0x0);
  emberAfCorePrintln("Sending child table entry %u (0x%2X) of %u: 0x%X", 
                     sl_next_child_table_entry_to_send,
                     child.id,
                     usedEntries,
                     status);

  emberAfCorePrint("EUI64: ");
  emberAfPrintBigEndianEui64(child.eui64);
  emberAfCorePrintln("\ttype: %d\tid: 0x%2X\tphy: %u\t power: %u\t,"
                     " timeout: %u\t", 
                     child.type, 
                     child.id, 
                     child.phy, 
                     child.power, 
                     (2 << (child.timeout - 1)));

  sl_next_child_table_entry_to_send++;

  if (status != EMBER_SUCCESS) {
    emberEventControlSetDelayMS(sl_next_child_table_send_event,
                                SL_RETRY_TIME_MS);
    return;
  }

  if (sl_next_child_table_entry_to_send >= usedEntries) {
    sl_next_child_table_entry_send_in_progress = false;
    sl_next_child_table_entry_to_send = 0;
    return;
  }

  emberEventControlSetDelayMS(sl_next_child_table_send_event,
                              SL_NEXT_CHILD_TABLE_ENTRY_SEND_DELAY_MS);
}

/**************************************************************************//**
 * @brief Send the next routing table entry and adjust index.
 * 
 *****************************************************************************/
void sl_send_next_route_table_entry(void)
{
  emberEventControlSetInactive(sl_next_route_table_send_event);

  EmberRouteTableEntry route;
  EmberStatus status;

  uint8_t usedEntries = emberAfGetRouteTableSize();
  status = emberGetRouteTableEntry(sl_next_route_table_entry_to_send, &route);
  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Route %d could not be fetched, status 0x%X", 
                       sl_next_route_table_entry_to_send, 
                       sl_next_route_table_entry_to_send, 
                       status);

    sl_next_route_table_entry_to_send++;
    if ((status == EMBER_ERR_FATAL)
       || (status == EMBER_NOT_FOUND)
       || (sl_next_route_table_entry_to_send >= usedEntries)) {
      emberAfCorePrintln("Terminating route table sending");
      sl_next_route_table_entry_send_in_progress = false;
      sl_next_route_table_entry_to_send = 0;
      return;
    }
    emberEventControlSetDelayMS(sl_next_route_table_send_event,
                                SL_RETRY_TIME_MS);
    return;
  }

  emberAfFillCommandLargeNetworkDiagnosticsGetRoutingTableResponse(route.destination,
                                                                   route.nextHop,
                                                                   route.status,
                                                                   route.age,
                                                                   route.concentratorType,
                                                                   route.routeRecordState);
  emberAfSetCommandEndpoints(1, 1);

  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, 0x0);
  emberAfCorePrintln("Sending route table entry: destination: 0x%2X\t"
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

  sl_next_route_table_entry_to_send++;

  if (status != EMBER_SUCCESS) {
    emberEventControlSetDelayMS(sl_next_route_table_send_event,
                                SL_RETRY_TIME_MS);
    return;
  }

  if (sl_next_route_table_entry_to_send >= usedEntries) {
    sl_next_route_table_entry_send_in_progress = false;
    sl_next_route_table_entry_to_send = 0;
    return;
  }

  emberEventControlSetDelayMS(sl_next_route_table_send_event,
                              SL_NEXT_ROUTE_TABLE_ENTRY_SEND_DELAY_MS);
}
