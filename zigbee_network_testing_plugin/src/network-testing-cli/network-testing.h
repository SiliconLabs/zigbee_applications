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
#ifndef NETWORK_TESTING_H
#define NETWORK_TESTING_H


#include "app/framework/include/af.h"
EmberEventControl emberAfPluginNetworkTestingCliReceivedAckEventControl;
EmberEventControl emberAfPluginNetworkTestingCliSendPacketEventControl;
enum {
  ZclCommand             = 0x00,
  ZdoBindRequest         = 0x01,
};
typedef uint8_t CommandType;

void setDeviceIndex(uint16_t index);
void setDestEndpoint(uint8_t endpoint);
void setBindClusterId(uint16_t clusterId);
void setTestRound(uint16_t round);
void setTestCommandType(CommandType commandType);
void emberAfDeviceTableSortByEui64(void);

#endif  // NETWORK_TESTING_H
