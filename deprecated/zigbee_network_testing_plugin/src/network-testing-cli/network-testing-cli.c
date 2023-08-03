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
 * DEPRECATION NOTICE
 * This code has been deprecated. It has been provided for historical reference
 * only and should not be used. This code will not be maintained.
 * This code is subject to the quality disclaimer at the point in time prior
 * to deprecation and superseded by this deprecation notice.
 ******************************************************************************/



#include "app/framework/plugin-host/network-testing-cli/network-testing.h"
#include "app/framework/util/common.h"
#include "af.h"

extern bool zclCmdIsBuilt;

void emberAfPluginNetworkTestingTableSendCommand(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint16_t round = (uint16_t)emberUnsignedCommandArgument(1);

  // check that cmd is built
  if (zclCmdIsBuilt == false) {
    emberAfCorePrintln("no cmd");
    return;
  }
  if (getTestRound() > 0) {
    emberAfCorePrintln("Testing is already running");
    return;
  }
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

void emberAfPluginNetworkTestingTableActiveEndpointsCommand(void)
{
  setTestRound(1);
  setDeviceIndex(0);
  setDestEndpoint(0);
  setTestCommandType(ZdoActiveEndpointRequest);
  emberEventControlSetActive(emberAfPluginNetworkTestingCliSendPacketEventControl);
}

void emberAfPluginNetworkTestingTableSimpleDescriptorCommand(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  setTestRound(1);
  setDeviceIndex(0);
  setDestEndpoint(endpoint);
  setTestCommandType(ZdoSimpleDescriptorRequest);
  emberEventControlSetActive(emberAfPluginNetworkTestingCliSendPacketEventControl);
}

void emberAfPluginDeviceTableSortByEui64Command(void)
{
  emberAfDeviceTableSortByEui64();
}
