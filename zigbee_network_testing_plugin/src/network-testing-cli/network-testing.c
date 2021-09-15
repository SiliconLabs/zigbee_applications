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

// Enable DISCOVER_DEVICES_MANUALLY option needs to comment out emberAfTrustCenterJoinCallback() and
// comment out the content of emAfPluginDeviceTablePreZDOMessageReceived() in device-table-discovery.c
//#define DISCOVER_DEVICES_MANUALLY

//for simple descriptor response
#define SIMPLE_DESCRIPTOR_RESPONSE_ENDPOINT_OFFSET                           \
  (EMBER_AF_ZDO_RESPONSE_OVERHEAD                                            \
   + 2 /* address of interest */                      + 1 /* length value */ \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_PROFILE_ID_OFFSET \
  (SIMPLE_DESCRIPTOR_RESPONSE_ENDPOINT_OFFSET        \
   + 1 /* endpoint */                                \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET \
  (SIMPLE_DESCRIPTOR_RESPONSE_PROFILE_ID_OFFSET + 2)

#define SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX \
  (SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET                    \
   + 2  /* device ID length */                                    \
   + 1  /* version (4-bits), reserved (4-bits) */                 \
  )

#define SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_INDEX \
  (SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX + 1)

// For Active endpoint response
#define ACTIVE_ENDPOINT_RESPONSE_COUNT_OFFSET \
  (EMBER_AF_ZDO_RESPONSE_OVERHEAD             \
   + 2) // Address of Interest

#define ACTIVE_ENDPOINT_RESPONSE_NODE_ID_OFFSET \
  (EMBER_AF_ZDO_RESPONSE_OVERHEAD)

#define ACTIVE_ENDPOINT_RESPONSE_LIST_OFFSET \
  (ACTIVE_ENDPOINT_RESPONSE_NODE_ID_OFFSET   \
   + 3)
// in out cluster types
#define CLUSTER_IN               0
#define CLUSTER_OUT              1
#define NUMBER_OF_CLUSTER_IN_OUT 2

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

static void parseActiveEndpointsResponse(EmberNodeId emberNodeId,
                                                  EmberApsFrame* apsFrame,
                                                  uint8_t* message,
                                                  uint16_t length)
{
#ifdef DISCOVER_DEVICES_MANUALLY
  uint8_t i;
  uint16_t index;
  EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();
  emberAfCorePrintln("number of ep: %d",
                     message[ACTIVE_ENDPOINT_RESPONSE_COUNT_OFFSET]);
  for (i = 0; i < message[ACTIVE_ENDPOINT_RESPONSE_COUNT_OFFSET]; i++) {
    emberAfCorePrintln("ep: %d", message[ACTIVE_ENDPOINT_RESPONSE_LIST_OFFSET + i]);
    if(emAfDeviceTableFindIndexNodeIdEndpoint(emberNodeId,message[ACTIVE_ENDPOINT_RESPONSE_LIST_OFFSET + i])==EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX){
        index = emAfDeviceTableFindIndexNodeIdEndpoint(emberNodeId,0);
       if(index != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX){
         deviceTable[index].endpoint = message[ACTIVE_ENDPOINT_RESPONSE_LIST_OFFSET + i];
       }
       else{
         index = emAfDeviceTableFindFreeDeviceTableIndex();
         if (index == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
           emberAfCorePrintln("Error: Device Table Full");
         }
         else{
           deviceTable[index].nodeId = emberNodeId;
           emberAfDeviceTableGetEui64FromNodeId(emberNodeId, deviceTable[index].eui64);
           deviceTable[index].endpoint = message[ACTIVE_ENDPOINT_RESPONSE_LIST_OFFSET + i];
         }
       }
    }
  }
  emAfDeviceTableSave();
#endif //DISCOVER_DEVICES_MANUALLY
}

static void parseSimpleDescriptorResponse(EmberNodeId nodeId,
                                                   uint8_t* message,
                                                   uint16_t length)
{
#ifdef DISCOVER_DEVICES_MANUALLY
  uint8_t endpoint;
  EmberAfPluginDeviceTableEntry *pEntry;
  uint8_t clusterIndex = 0;
  uint16_t endpointIndex;
  uint8_t i, currentClusterType, ClusterCount;
  uint8_t msgArrayIndex = SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_INDEX;

  endpoint = message[SIMPLE_DESCRIPTOR_RESPONSE_ENDPOINT_OFFSET];

  endpointIndex = emAfDeviceTableFindIndexNodeIdEndpoint(nodeId,endpoint);
  if(endpointIndex != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX){
    pEntry = emberAfDeviceTableFindDeviceTableEntry(endpointIndex);
    pEntry->deviceId =
      emberFetchLowHighInt16u(message
                              + SIMPLE_DESCRIPTOR_RESPONSE_DEVICE_ID_OFFSET);
    emberAfCorePrintln("device id: %2x:  ", pEntry->deviceId);
    ClusterCount = message[SIMPLE_DESCRIPTOR_RESPONSE_INPUT_CLUSTER_LIST_COUNT_INDEX];

    for (currentClusterType = CLUSTER_IN;
         currentClusterType < NUMBER_OF_CLUSTER_IN_OUT;
         currentClusterType++) {
      for (i = 0; i < ClusterCount; i++) {
        pEntry->clusterIds[clusterIndex] =
          HIGH_LOW_TO_INT(message[msgArrayIndex + 1], message[msgArrayIndex]);
        clusterIndex++;
        msgArrayIndex += 2; //advance the index by 2 bytes for each 16 bit
                            //cluster id
      }
      if (currentClusterType == CLUSTER_IN) {
        pEntry->clusterOutStartPosition = clusterIndex;
        // This is the output cluster count
        ClusterCount = message[msgArrayIndex++];
      }
    }
  emAfDeviceTableSave();
  }
#endif //DISCOVER_DEVICES_MANUALLY
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
      case ACTIVE_ENDPOINTS_RESPONSE:
        emberAfCorePrintln("Active Endpoints Response");
        if(emberNodeId==emberAfDeviceTableGetNodeIdFromIndex(currentDeviceIndex)){
          receiveAck = TRUE;
          emberEventControlSetInactive(emberAfPluginNetworkTestingCliReceivedAckEventControl);
          emberEventControlSetActive(emberAfPluginNetworkTestingCliSendPacketEventControl);
          parseActiveEndpointsResponse(emberNodeId, apsFrame, message, length);
        }
        break;
      case SIMPLE_DESCRIPTOR_RESPONSE:
        emberAfCorePrintln("Simple Descriptor Response");
        if(emberNodeId==emberAfDeviceTableGetNodeIdFromIndex(currentDeviceIndex)){
          receiveAck = TRUE;
          emberEventControlSetInactive(emberAfPluginNetworkTestingCliReceivedAckEventControl);
          emberEventControlSetActive(emberAfPluginNetworkTestingCliSendPacketEventControl);
          parseSimpleDescriptorResponse(emberNodeId,message,length);
        }
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
  if(status == EMBER_SUCCESS) {
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

static void emberAfDeviceTableCommandIndexActiveEndpointRequest(uint16_t index)
{
  EmberStatus status;
  EmberNodeId target = emberAfDeviceTableGetNodeIdFromIndex(index);
  status = emberActiveEndpointsRequest(target,EMBER_AF_DEFAULT_APS_OPTIONS);
  emberAfCorePrintln("ZDO active endpoint  req  0x%X, 0x%2X", status, target);
}

static void emberAfDeviceTableCommandIndexSimpleDescriptorRequest(uint16_t index)
{
  EmberStatus status;
  EmberNodeId target = emberAfDeviceTableGetNodeIdFromIndex(index);
  uint8_t endpoint = emAfDeviceTableGetFirstEndpointFromIndex(index);
  status = emberSimpleDescriptorRequest(target,endpoint,EMBER_AF_DEFAULT_APS_OPTIONS);
  emberAfCorePrintln("ZDO simple descriptor  req  0x%X, 0x%2X", status, target);
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
          case ZdoActiveEndpointRequest:
               emberAfDeviceTableCommandIndexActiveEndpointRequest(currentDeviceIndex);
            break;
          case ZdoSimpleDescriptorRequest:
               emberAfDeviceTableCommandIndexSimpleDescriptorRequest(currentDeviceIndex);
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
  if(receiveAck == FALSE){
    emberSendManyToOneRouteRequest(EMBER_AF_PLUGIN_CONCENTRATOR_CONCENTRATOR_TYPE, EMBER_AF_PLUGIN_CONCENTRATOR_MAX_HOPS);
    emberIeeeAddressRequestToTarget(emberAfDeviceTableGetNodeIdFromIndex(currentDeviceIndex),
                    false,   // report kids?
                    0,       // child start index
                    EMBER_APS_OPTION_NONE,
                    EMBER_BROADCAST_ADDRESS);
    emberAfEventControlSetDelayMS(&emberAfPluginNetworkTestingCliSendPacketEventControl,3000);
  }
  else{
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

void emAfPluginNetworkTestZigbeeKeyEstablishmentCallback (EmberEUI64 partner,
                                        EmberKeyStatus status)
{
#ifdef DISCOVER_DEVICES_MANUALLY
  uint16_t index;
  if(status == EMBER_TC_REQUESTER_VERIFY_KEY_SUCCESS)
    {
      if(emberAfDeviceTableGetFirstIndexFromEui64(partner)==EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX){
        EmberNodeId nodeId = emberLookupNodeIdByEui64(partner);
        emberAfCorePrintln("TRUST CENTER LINK KEY ESTABLISHED %2x", nodeId);
        if (nodeId != EMBER_ERR_FATAL){
          index = emAfDeviceTableFindFreeDeviceTableIndex();
          if (index == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
            // Error case... no more room in the index table
            emberAfCorePrintln("Error: Device Table Full");
          }
          else{
            EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();
            deviceTable[index].nodeId = nodeId;
            MEMCOPY( deviceTable[index].eui64, partner, EUI64_SIZE);
            emAfDeviceTableSave();
          }
        }
        else{
            emberAfCorePrintln("Error: Can't Find Node ID");
        }
     }
    }
#endif //DISCOVER_DEVICES_MANUALLY
}

#ifdef DISCOVER_DEVICES_MANUALLY
/** @brief Trust Center Join
 *
 * This callback is called from within the application framework's
 * implementation of emberTrustCenterJoinHandler or
 * ezspTrustCenterJoinHandler. This callback provides the same arguments
 * passed to the TrustCenterJoinHandler. For more information about the
 * TrustCenterJoinHandler please see documentation included in
 * stack/include/trust-center.h.
 *
 * @param newNodeId   Ver.: always
 * @param newNodeEui64   Ver.: always
 * @param parentOfNewNode   Ver.: always
 * @param status   Ver.: always
 * @param decision   Ver.: always
 */
void emberAfTrustCenterJoinCallback(EmberNodeId newNodeId,
                                    EmberEUI64 newNodeEui64,
                                    EmberNodeId parentOfNewNode,
                                    EmberDeviceUpdate status,
                                    EmberJoinDecision decision)
{
 if(status==EMBER_DEVICE_LEFT) {
   uint16_t index = emberAfDeviceTableGetFirstIndexFromEui64(newNodeEui64);
   if(index != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
      emAfPluginDeviceTableDeleteEntry(index);
      emAfDeviceTableSave();
   }
 }
}
#endif //DISCOVER_DEVICES_MANUALLY
