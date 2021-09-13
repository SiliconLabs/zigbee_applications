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



#include "app/framework/include/af.h"
#include "app/framework/plugin/device-table/device-table-internal.h"
#include "app/framework/plugin/device-table/device-table.h"
#include "app/framework/plugin/concentrator/concentrator-support.h"
#include "stack/include/zigbee-device-stack.h"
#include "app/framework/plugin-host/network-testing-cli/network-testing.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"

extern EmberApsFrame globalApsFrame;
extern uint8_t appZclBuffer[];
extern uint16_t appZclBufferLen;
EmberEventControl emberAfPluginNetworkTestingCliReceivedAckEventControl;
EmberEventControl emberAfPluginNetworkTestingCliSendPacketEventControl;
static boolean receiveAck;
static CommandType testCommandType;
static uint16_t deviceIndex,currentDeviceIndex,bindClusterId,testRound;
static uint8_t destEndpoint;

void setDeviceIndex(uint16_t index)
{
  deviceIndex = index;
}

static uint16_t getDeviceIndex(void)
{
  return deviceIndex;
}

void setDestEndpoint(uint8_t endpoint)
{
  destEndpoint = endpoint;
}

static uint8_t getDestEndpoint(void)
{
  return destEndpoint;
}

void setBindClusterId(uint16_t clusterId)
{
  bindClusterId = clusterId;
}

static uint16_t getBindClusterId(void)
{
  return bindClusterId;
}

void setTestRound(uint16_t round)
{
  testRound = round;
}

uint16_t getTestRound(void)
{
  return testRound;
}

void setTestCommandType(CommandType commandType)
{
  testCommandType = commandType;
}

CommandType getTestCommandType(void)
{
  return testCommandType;
}

bool emAfPluginNetworkTestPreZDOMessageReceivedCallback(EmberNodeId emberNodeId,
                                                EmberApsFrame* apsFrame,
                                                uint8_t* message,
                                                uint16_t length)
{
  switch (apsFrame->clusterId) {
     case BIND_RESPONSE:
        if(emberNodeId==emberAfDeviceTableGetNodeIdFromIndex(currentDeviceIndex)){
          receiveAck = TRUE;
          emberEventControlSetInactive(emberAfPluginNetworkTestingCliReceivedAckEventControl);
          emberEventControlSetActive(emberAfPluginNetworkTestingCliSendPacketEventControl);
        }
        emberAfCorePrintln("BIND_RESPONSE: %2x:  ", emberNodeId);
      break;
    default:
      break;
  }
  return false;
}

static void sendMessageSentCallback(EmberOutgoingMessageType type,
                                            uint16_t indexOrDestination,
                                            EmberApsFrame *apsFrame,
                                            uint16_t msgLen,
                                            uint8_t *message,
                                            EmberStatus status)
{
  if (status == EMBER_SUCCESS) {
	  emberAfCorePrintln("Received Ack %2x", indexOrDestination);
    if(indexOrDestination==emberAfDeviceTableGetNodeIdFromIndex(currentDeviceIndex)){
      receiveAck = TRUE;
      emberEventControlSetInactive(emberAfPluginNetworkTestingCliReceivedAckEventControl);
      emberEventControlSetActive(emberAfPluginNetworkTestingCliSendPacketEventControl);
    }
  }
}

static void emberAfDeviceTableCommandIndexSendWithCallback(uint16_t index, EmberAfMessageSentFunction callback)
{	
  EmberStatus status;
  uint8_t endpoint = emAfDeviceTableGetFirstEndpointFromIndex(index);
  EmberNodeId nodeId = emberAfDeviceTableGetNodeIdFromIndex(index);
  globalApsFrame.sourceEndpoint = emberAfPrimaryEndpoint();
  globalApsFrame.destinationEndpoint = endpoint;
  status = emberAfSendUnicastWithCallback(EMBER_OUTGOING_DIRECT,
                                           nodeId,
                                           &globalApsFrame,
                                           appZclBufferLen,
                                           appZclBuffer,
                                           callback);
  emberAfCorePrintln("device table send status: 0x%X, 0x%2X, %d",
					 status,
					 nodeId,
					 endpoint);
}

static void emberAfDeviceTableCommandIndexBindRequest(uint16_t index)
{
  EmberStatus status;
  EmberEUI64 sourceEui, destEui;  // names relative to binding sent over-the-air
  EmberNodeId target = emberAfDeviceTableGetNodeIdFromIndex(index);
  uint8_t sourceEndpoint = emAfDeviceTableGetFirstEndpointFromIndex(index);
  uint8_t destinationEndpoint = emberAfPrimaryEndpoint();;
  uint16_t clusterId = getBindClusterId();
  emberAfDeviceTableGetEui64FromNodeId(target, sourceEui);
  emberAfGetEui64(destEui);
  status = emberBindRequest(target,           // who gets the bind req
                            sourceEui,       // source eui IN the binding
                            sourceEndpoint,
                            clusterId,
                            UNICAST_BINDING, // binding type
                            destEui,         // destination eui IN the binding
                            0,               // groupId for new binding
                            destinationEndpoint,
                            EMBER_AF_DEFAULT_APS_OPTIONS);
  emberAfCorePrintln("ZDO bind req  0x%X, 0x%2X", status, target);
}

void emberAfPluginNetworkTestingCliSendPacketEventHandler(void)
{
  if(getTestRound()>0){
    emberEventControlSetInactive(emberAfPluginNetworkTestingCliSendPacketEventControl);
    while((EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID==emberAfDeviceTableGetNodeIdFromIndex(getDeviceIndex()))||(getDestEndpoint() !=emAfDeviceTableGetFirstEndpointFromIndex(getDeviceIndex()))){
      setDeviceIndex(getDeviceIndex()+1);
      if((getDeviceIndex() >= EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE)){
        break;
      }
    }
    if(getDeviceIndex() < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE){
      currentDeviceIndex = getDeviceIndex();
      //emberAfCorePrintln("currentDeviceIndex %d", currentDeviceIndex);

      switch (testCommandType) {
        case ZclCommand:
             emberAfDeviceTableCommandIndexSendWithCallback(currentDeviceIndex,sendMessageSentCallback);
          break;
        case ZdoBindRequest:
             emberAfDeviceTableCommandIndexBindRequest(currentDeviceIndex);
          break;
        default:
          emberAfCorePrintln("Unsupported Test Command Type  %2x", testCommandType);
          break;
      }
      setDeviceIndex(getDeviceIndex()+1);
      receiveAck = FALSE;
      emberAfEventControlSetDelayMS(&emberAfPluginNetworkTestingCliReceivedAckEventControl,5000);
    }
    else{
      setDeviceIndex(0);
      setTestRound(getTestRound()-1);
      if(getTestRound()>0)
        emberEventControlSetActive(emberAfPluginNetworkTestingCliSendPacketEventControl);
    }
  }
}

void emberAfPluginNetworkTestingCliReceivedAckEventHandler(void)
{
	emberEventControlSetInactive(emberAfPluginNetworkTestingCliReceivedAckEventControl);
	if(receiveAck == FALSE)
	{
		emberSendManyToOneRouteRequest(EMBER_AF_PLUGIN_CONCENTRATOR_CONCENTRATOR_TYPE, EMBER_AF_PLUGIN_CONCENTRATOR_MAX_HOPS);
		emberIeeeAddressRequestToTarget(emberAfDeviceTableGetNodeIdFromIndex(currentDeviceIndex),
										false,   // report kids?
										0,       // child start index
										EMBER_APS_OPTION_NONE,
										EMBER_BROADCAST_ADDRESS);
		emberAfEventControlSetDelayMS(&emberAfPluginNetworkTestingCliSendPacketEventControl,3000);
	}
	else
	{
		emberEventControlSetActive(emberAfPluginNetworkTestingCliSendPacketEventControl);
	}
}

static uint64_t Eui64ToUint64(EmberEUI64 eui64)
{
  uint64_t eui64Temp=0;
  for (uint16_t i = 0; i < EUI64_SIZE; i++)
    eui64Temp += (uint64_t)eui64[i] << (i*8);
  return eui64Temp;
}

void emberAfDeviceTableSortByEui64(void)
{
  EmberAfPluginDeviceTableEntry temp;
  EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();

  for (uint16_t i = 0; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE-1; i++){
       for (uint16_t j = 0; j < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE-1-i; j++) {
            if (Eui64ToUint64(deviceTable[j].eui64) > Eui64ToUint64(deviceTable[j+1].eui64)) {
               MEMCOPY(&temp, &deviceTable[j+1], sizeof(EmberAfPluginDeviceTableEntry));
               MEMCOPY(&deviceTable[j+1], &deviceTable[j], sizeof(EmberAfPluginDeviceTableEntry));
               MEMCOPY(&deviceTable[j], &temp, sizeof(EmberAfPluginDeviceTableEntry));
           }
       }
   }
  emAfDeviceTableSave();
}
