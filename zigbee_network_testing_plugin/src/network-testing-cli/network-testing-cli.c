/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *******************************************************************************
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/



#include "app/framework/plugin/network-testing-cli/network-testing.h"
#include "app/framework/util/common.h"
#include "af.h"

void emberAfPluginNetworkTestingTableSendCommand(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint16_t round = (uint16_t)emberUnsignedCommandArgument(1);
  setDestEndpoint(endpoint);
  setDeviceIndex(0);
  setTestRound(round);
  setTestCommandType(ZclCommand);
  emberEventControlSetActive(emberAfPluginNetworkTestingCliSendPacketEventControl);
}

void emberAfPluginNetworkTestingTableBindCommand(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint16_t clusterId = (uint16_t)emberUnsignedCommandArgument(1);
  setTestRound(1);
  setDestEndpoint(endpoint);
  setBindClusterId(clusterId);
  setDeviceIndex(0);
  setTestCommandType(ZdoBindRequest);
  emberEventControlSetActive(emberAfPluginNetworkTestingCliSendPacketEventControl);
}

void emberAfPluginDeviceTableSortByEui64Command(void)
{
  emberAfDeviceTableSortByEui64();
}
