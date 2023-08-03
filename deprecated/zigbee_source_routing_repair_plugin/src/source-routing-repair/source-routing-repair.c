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



#include "app/framework/include/af.h"
#include "app/framework/plugin/concentrator/concentrator-support.h"

EmberEventControl emberAfPluginSourceRoutingRepairMyEventControl;
enum {
  SOURCE_ROUTING_REPAIR_STATE_MTO             = 0x00,
  SOURCE_ROUTING_REPAIR_STATE_BROADCAST       = 0x01,
};
typedef uint8_t SourceRoutingRepairState;
static SourceRoutingRepairState RepairState;


void SendBroadcastMessage(void)
{
  uint8_t ZclBuffer[5];
  uint16_t ZclBufferLen = 0;
  EmberApsFrame ApsFrame;
  ApsFrame.clusterId = ZCL_BASIC_CLUSTER_ID;
  ApsFrame.sourceEndpoint = emberAfPrimaryEndpointForCurrentNetworkIndex();
//Just send the message to the broadcast endpoint 0xff, as we don't know the destination endpoint.
  ApsFrame.destinationEndpoint = 0xff;
  ApsFrame.options = EMBER_AF_DEFAULT_APS_OPTIONS;


  ZclBuffer[ZclBufferLen++] = (ZCL_GLOBAL_COMMAND
                           | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                           | ZCL_DISABLE_DEFAULT_RESPONSE_MASK);
  ZclBuffer[ZclBufferLen++] = emberAfNextSequence();
  ZclBuffer[ZclBufferLen++] = ZCL_READ_ATTRIBUTES_COMMAND_ID;
  ZclBuffer[ZclBufferLen++] = LOW_BYTE(ZCL_VERSION_ATTRIBUTE_ID);
  ZclBuffer[ZclBufferLen++] = HIGH_BYTE(ZCL_VERSION_ATTRIBUTE_ID);
  emberAfSendBroadcast(EMBER_SLEEPY_BROADCAST_ADDRESS,&ApsFrame,ZclBufferLen,&ZclBuffer[0]);
}

void customPluginSourceRoutingRepairInitCallback(void)
{
#ifdef EMBER_AF_PLUGIN_CONCENTRATOR
  RepairState = SOURCE_ROUTING_REPAIR_STATE_MTO;
  emberEventControlSetActive(emberAfPluginSourceRoutingRepairMyEventControl);
  //emberAfCorePrintln("Source Routing Repair Plugin Init");
#endif
}

void emberAfPluginSourceRoutingRepairMyEventHandler(void)
{
  emberEventControlSetInactive(emberAfPluginSourceRoutingRepairMyEventControl);
  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
	  if(RepairState==SOURCE_ROUTING_REPAIR_STATE_MTO)
		{
		  emberSendManyToOneRouteRequest(EMBER_AF_PLUGIN_CONCENTRATOR_CONCENTRATOR_TYPE, EMBER_AF_PLUGIN_CONCENTRATOR_MAX_HOPS);
		  //emberAfCorePrintln("Source Routing Repair Plugin Sending Many-To-One Request");
		  RepairState = SOURCE_ROUTING_REPAIR_STATE_BROADCAST;
		  emberEventControlSetDelayMS(emberAfPluginSourceRoutingRepairMyEventControl, EMBER_AF_PLUGIN_SOURCE_ROUTING_REPAIR_BROADCAST_DELAY);
		}
	  else if(RepairState==SOURCE_ROUTING_REPAIR_STATE_BROADCAST)
		{
		  SendBroadcastMessage();
		  //emberAfCorePrintln("Source Routing Repair Plugin Sending Broadcast Message");
		}
  }
}
