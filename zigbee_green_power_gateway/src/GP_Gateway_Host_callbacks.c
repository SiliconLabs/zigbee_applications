// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include "app/framework/include/af.h"
#include EMBER_AF_API_GREEN_POWER_SERVER
#include EMBER_AF_API_NETWORK_STEERING
#include PLATFORM_HEADER
#include "app/framework/plugin/ota-server-policy/ota-server-policy.h"

#define GP_CMD_TOGGLE  0x22

uint8_t targetBindingIndex = 0;
bool targetSet = false;

/** @brief Setting GP message target
 * 	This function is being called when the custom command "set-target" is issued.
 * 	The first parameter in the custom CLI command is a NodeId.
 * 	This NodeId is paired with a binding table index.
 * 	The function finds the paired binding table index and
 * 	stores it in targetBindingIndex global variable.
 */
void setTarget(void)
{
	EmberNodeId selectedNodeId = (uint16_t)emberUnsignedCommandArgument(0);

	//going through the binding table
	uint8_t i;
	for (i = 0; i < EMBER_BINDING_TABLE_SIZE; i++){
		EmberBindingTableEntry binding;
		if (emberGetBinding(i, &binding) == EMBER_SUCCESS
			&& binding.type == EMBER_UNICAST_BINDING
			&& binding.clusterId == ZCL_ON_OFF_CLUSTER_ID
			&& selectedNodeId == emberGetBindingRemoteNodeId(i)){
			targetBindingIndex = i;
		    targetSet = true;
		    break;
		 }
	}

	if(i == EMBER_BINDING_TABLE_SIZE) //if no table index was found
		emberAfCorePrintln("Node [0x%04X] has no On/Off cluster binding with the Green Power Gateway", selectedNodeId);
	else
		emberAfCorePrintln("Target nodeID set:[0x%04X]", selectedNodeId);
}

EmberCommandEntry emberAfCustomCommands[] = {
  /* Custom CLI commands */
   emberCommandEntryAction("set-target", setTarget, "v", "Sets the target of the GP message"),
  emberCommandEntryTerminator()
};

/** @brief Gp Notification
 *
 * This function is called by the Green Power Sink Plugin to notify the
 * application of a Green Power Gp Notification of an incomming gpd command.
 * Return true to handle in application.
 *
 * @param options from the incoming Gp Notification Command Ver.: always
 * @param addr GPD address        Ver.: always
 * @param gpdSecurityFrameCounter Ver.: always
 * @param gpdCommandId            Ver.: always
 * @param gpdCommandPayload first byte is length of the payload Ver.: always
 * @param gppShortAddress         Ver.: always
 * @param gppDistance             Ver.: always
 */
bool emberAfGreenPowerClusterGpNotificationForwardCallback(uint16_t options,
                                                           EmberGpAddress * addr,
                                                           uint32_t gpdSecurityFrameCounter,
                                                           uint8_t gpdCommandId,
                                                           uint8_t * gpdCommandPayload,
                                                           uint16_t gppShortAddress,
                                                           uint8_t  gppDistance)
{
	if(targetSet){
		if(gpdCommandId == GP_CMD_TOGGLE){
			// sending the toggle command
			emberAfFillCommandOnOffClusterToggle();
			emberAfSendCommandUnicast(EMBER_OUTGOING_VIA_BINDING, targetBindingIndex);
			emberAfCorePrintln("Sending Toggle command to node [0x%04X]", emberGetBindingRemoteNodeId(targetBindingIndex));
		}
		else
			emberAfCorePrintln("GP message was not toggle command");
	}
		else
			emberAfCorePrintln("No target nodeID has been set yet");

	return false;
}


