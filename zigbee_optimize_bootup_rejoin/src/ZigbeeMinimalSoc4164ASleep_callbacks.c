/***************************************************************************//**
 * @file ZigbeeMinimalSoc4164ASleep_callbacks.c
 * @brief Callback implementation for ZigbeeMinimal sample application.
 * @version 1.0.1
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
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

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include "app/framework/include/af.h"

typedef enum
{
	REJOIN_ON_BOOT_STATE_IDLE,
	REJOIN_ON_BOOT_STATE_CURRENT_CH,
	REJOIN_ON_BOOT_STATE_ALL_CH,
	REJOIN_ON_BOOT_STATE_DONE,
}REJOIN_ON_BOOT_STATE;

REJOIN_ON_BOOT_STATE g_rejoin_on_boot_state = REJOIN_ON_BOOT_STATE_IDLE;
uint8_t g_currentch = 0xFF;

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
bool emberAfStackStatusCallback(EmberStatus status)
{
  // This value is ignored by the framework.
  return false;
}

/** @brief Complete
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to EMBER_SUCCESS to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt. Ver.: always
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID. Ver.: always
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network. Ver.: always
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete. Ver.: always
 */
void emberAfPluginNetworkSteeringCompleteCallback(EmberStatus status,
                                                  uint8_t totalBeacons,
                                                  uint8_t joinAttempts,
                                                  uint8_t finalState)
{
  emberAfCorePrintln("%p network %p: 0x%X", "Join", "complete", status);
}

EmberEventControl customEventData;
void customEventHandler()
{
	EmberStatus status;

	emberEventControlSetInactive(customEventData);

	if (REJOIN_ON_BOOT_STATE_CURRENT_CH == g_rejoin_on_boot_state) {
		status = emberFindAndRejoinNetworkWithReason(1, // Network Key is known
												   1 << g_currentch,
												   EMBER_REJOIN_DUE_TO_END_DEVICE_REBOOT);
		emberAfCorePrintln("[%d] status=%X", __LINE__, status);

		g_rejoin_on_boot_state = REJOIN_ON_BOOT_STATE_ALL_CH;

		if (EMBER_SUCCESS == status) {
			emberEventControlSetDelayMS(customEventData, 500);
		} else {
			emberEventControlSetActive(customEventData);
		}
	} else if (REJOIN_ON_BOOT_STATE_ALL_CH == g_rejoin_on_boot_state) {
		status = emberFindAndRejoinNetworkWithReason(true, // Network Key is known
													EMBER_ALL_802_15_4_CHANNELS_MASK,
												   EMBER_REJOIN_DUE_TO_END_DEVICE_REBOOT);
		emberAfCorePrintln("[%d] status=%X", __LINE__, status);
	} else {
		//unreachable branch
	}
}

void emberAfMainInitCallback(void)
{
    tokTypeStackNodeData data;
    EmberNodeType nodeType = EMBER_UNKNOWN_DEVICE;
    EmberStatus status;

    status = emberAfGetNodeType(&nodeType);
    if (status != EMBER_SUCCESS) {
      return;
    }

    // only end device need rejoin
    if (EMBER_SLEEPY_END_DEVICE != nodeType && EMBER_END_DEVICE != nodeType) {
    	return;
    }

    memset(&data, 0xFF, sizeof(data));
    halCommonGetToken(&data, TOKEN_STACK_NODE_DATA);

    if (data.panId != 0 && data.panId != 0xFFFF
    		&& data.zigbeeNodeId != 0xFFFF && data.zigbeeNodeId != 0xFFFE) {

    	g_rejoin_on_boot_state = REJOIN_ON_BOOT_STATE_CURRENT_CH;
    	g_currentch = data.radioFreqChannel;
    	emberEventControlSetActive(customEventData);
    }
}

EmberPacketAction emberAfIncomingPacketFilterCallback(EmberZigbeePacketType packetType,
                                                      int8u* packetData,
                                                      int8u* size_p,
                                                      void* data)
{
	uint8_t cmd;
	uint8_t status;
	EmberNodeId nodeID;

	if (EMBER_ZIGBEE_PACKET_TYPE_NWK_COMMAND != packetType) {
		return EMBER_ACCEPT_PACKET;
	}

//	emberAfCorePrintln("[%d] %X %X %X %X", __LINE__, packetData[0], packetData[1], packetData[2], packetData[3]);

	cmd = packetData[0];
	nodeID = packetData[2] << 8 | packetData[1];
	status = packetData[3];
	if (7 == cmd && 0 == status && emberGetNodeId() == nodeID) {  // cmd=7 means rejoin response
		g_rejoin_on_boot_state = REJOIN_ON_BOOT_STATE_DONE;
		emberEventControlSetInactive(customEventData);
	}

	return EMBER_ACCEPT_PACKET;
}
