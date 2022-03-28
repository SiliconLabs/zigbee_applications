/***************************************************************************//**
 * @file sl_cli_command_table.c
 * @brief Declarations of relevant command structs for cli framework.
 * @version x.y.z
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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
 ******************************************************************************/

#include <stdlib.h>

#include "sl_cli_config.h"
#include "sl_cli_command.h"
#include "sl_cli_arguments.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *****************************   TEMPLATED FILE   ******************************
 ******************************************************************************/

/*******************************************************************************
 * Example syntax (.slcc or .slcp) for populating this file:
 *
 *   template_contribution:
 *     - name: cli_command          # Register a command
 *       value:
 *         name: status             # Name of command
 *         handler: status_command  # Function to be called. Must be defined
 *         help: "Prints status"    # Optional help description
 *         shortcuts:               # Optional shorcut list
 *           - name: st
 *         argument:                # Argument list, if apliccable
 *           - type: uint8          # Variable type
 *             help: "Channel"      # Optional description
 *           - type: string
 *             help: "Text"
 *     - name: cli_group            # Register a group
 *       value:
 *         name: shell              # Group name
 *         help: "Shell commands"   # Optional help description
 *         shortcuts:               # Optional shorcuts
 *           - name: sh
 *     - name: cli_command
 *       value:
 *         name: repeat
 *         handler: repeat_cmd
 *         help: "Repeat commands"
 *         shortcuts:
 *           - name: r
 *           - name: rep
 *         group: shell            # Associate command with group
 *         argument:
 *           - type: string
 *             help: "Text"
 *           - type: additional
 *             help: "More text"
 *
 * For subgroups, an optional unique id can be used to allow a particular name to
 * be used more than once. In the following case, from the command line the
 * following commands are available:
 *
 * >  root_1 shell status
 * >  root_2 shell status
 *
 *     - name: cli_group            # Register a group
 *       value:
 *         name: root_1             # Group name
 *
 *     - name: cli_group            # Register a group
 *       value:
 *         name: root_2             # Group name
 *
 *    - name: cli_group             # Register a group
 *       value:
 *         name: shell              # Group name
 *         id: shell_root_1         # Optional unique id for group
 *         group: root_1            # Add group to root_1 group
 *
 *    - name: cli_group             # Register a group
 *       value:
 *         name: shell              # Group name
 *         id: shell_root_2         # Optional unique id for group
 *         group: root_2            # Add group to root_1 group
 *
 *    - name: cli_command           # Register a command
 *       value:
 *         name: status
 *         handler: status_1
 *         group: shell_root_1      # id of subgroup
 *
 *    - name: cli_command           # Register a command
 *       value:
 *         name: status
 *         handler: status_2
 *         group: shell_root_2      # id of subgroup
 *
 ******************************************************************************/

// Provide function declarations
void emberAfPluginIdleSleepStatusCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginIdleSleepStayAwakeCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginIdleSleepAwakeWhenNotJoinedCommand(sl_cli_command_arg_t *arguments);
void emAfCliInfoCommand(sl_cli_command_arg_t *arguments);
void printAllLibraryStatus(sl_cli_command_arg_t *arguments);
void emAfCliBsendCommand(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_send_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_zcl_read_cli_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_zcl_write_cli_command(sl_cli_command_arg_t *arguments);
void resetCommand(sl_cli_command_arg_t *arguments);
void emAfCliRawCommand(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_send_command(sl_cli_command_arg_t *arguments);
void emAfCliSendUsingMulticastBindingCommand(sl_cli_command_arg_t *arguments);
void emAfCliTimesyncCommand(sl_cli_command_arg_t *arguments);
void sli_get_pti_radio_config(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_config_cca_mode_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_version_command(sl_cli_command_arg_t *arguments);
void endpointPrint(sl_cli_command_arg_t *arguments);
void enableDisableEndpoint(sl_cli_command_arg_t *arguments);
void enableDisableEndpoint(sl_cli_command_arg_t *arguments);
void printEvents(sl_cli_command_arg_t *arguments);
void getSetMfgToken(sl_cli_command_arg_t *arguments);
void getSetMfgToken(sl_cli_command_arg_t *arguments);
void emberAfPluginCountersPrintCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginCounterPrintCounterTypeCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginCountersSimplePrintCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginCountersClear(sl_cli_command_arg_t *arguments);
void emberAfPluginCountersPrintThresholdsCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginCountersSetThresholdCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginCountersResetThresholds(sl_cli_command_arg_t *arguments);
void emberAfPluginCountersSendRequestCommand(sl_cli_command_arg_t *arguments);
void sli_zigbee_debug_print_enable_stack_type_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_debug_print_enable_core_type_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_debug_print_enable_app_type_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_debug_print_enable_zcl_type_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_debug_print_enable_legacy_af_debug_type_command(sl_cli_command_arg_t *arguments);
void emberAfPluginEndDeviceSupportStatusCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginEndDeviceSupportPollCompletedCallbackCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginEndDeviceSupportForceShortPollCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginFindAndBindInitiatorStartCommand(sl_cli_command_arg_t *arguments);
void emAfPluginIdentifyCliPrint(sl_cli_command_arg_t *arguments);
void emAfInterpanEnableCommand(sl_cli_command_arg_t *arguments);
void emAfInterpanDisableCommand(sl_cli_command_arg_t *arguments);
void emAfInterpanFragmentTestCommand(sl_cli_command_arg_t *arguments);
void emAfInterpanSetMessageTimeoutCommand(sl_cli_command_arg_t *arguments);
void emAfInterpanGroupShortCommand(sl_cli_command_arg_t *arguments);
void emAfInterpanGroupShortCommand(sl_cli_command_arg_t *arguments);
void emAfInterpanLongCommand(sl_cli_command_arg_t *arguments);
void emAfMfglibStartCommand(sl_cli_command_arg_t *arguments);
void emAfMfglibStopCommand(sl_cli_command_arg_t *arguments);
void emAfMfglibToneStartCommand(sl_cli_command_arg_t *arguments);
void emAfMfglibToneStopCommand(sl_cli_command_arg_t *arguments);
void emAfMfglibStatusCommand(sl_cli_command_arg_t *arguments);
void emAfMfglibSetChannelCommand(sl_cli_command_arg_t *arguments);
void emAfMfglibSetPowerAndModeCommand(sl_cli_command_arg_t *arguments);
void emAfMfglibStreamStartCommand(sl_cli_command_arg_t *arguments);
void emAfMfglibStreamStopCommand(sl_cli_command_arg_t *arguments);
void emAfMfglibSendCommand(sl_cli_command_arg_t *arguments);
void emAfMfglibSendCommand(sl_cli_command_arg_t *arguments);
void emAfMfglibSendMessageCommand(sl_cli_command_arg_t *arguments);
void emAfMfglibSleepCommand(sl_cli_command_arg_t *arguments);
void emAfMfglibProgramEuiCommand(sl_cli_command_arg_t *arguments);
void emAfMfglibEnableMfglib(sl_cli_command_arg_t *arguments);
void emAfMfglibSetOptions(sl_cli_command_arg_t *arguments);
////MFG UPDATED CODE START -----------------------------------------------------------------------------------------------------------------
void emAfMfglibClearPackets(sl_cli_command_arg_t *arguments);
void emAfMfglibSetPower(sl_cli_command_arg_t *arguments);
void emAfMfglibGetPower(sl_cli_command_arg_t *arguments);
void emAfMfglibSetCcaThresholdReg(sl_cli_command_arg_t *arguments);
void emAfMfglibGetCcaThresholdReg(sl_cli_command_arg_t *arguments);
void emAfMfglibGetCtuneValueReg(sl_cli_command_arg_t *arguments);
void emAfMfglibSetCtuneValueReg(sl_cli_command_arg_t *arguments);
void emAfMfglibGetCcaThresholdTok(sl_cli_command_arg_t *arguments);
void emAfMfglibSetCcaThresholdTok(sl_cli_command_arg_t *arguments);
void emAfMfglibGetCtuneValueTok(sl_cli_command_arg_t *arguments);
void emAfMfglibSetCtuneValueTok(sl_cli_command_arg_t *arguments);
void emAfMfglibSetGpio(sl_cli_command_arg_t *arguments);
void emAfMfglibGetGpio(sl_cli_command_arg_t *arguments);
void emAfMfglibGpioHelp(sl_cli_command_arg_t *arguments);
void emAfMfglibTokDump(sl_cli_command_arg_t *arguments);
void emAfMfglibSleepTest(sl_cli_command_arg_t *arguments);
void emAfMfglibEnterBootloader(sl_cli_command_arg_t *arguments);
void emAfMfglibPERTest(sl_cli_command_arg_t *arguments);
void emAfMfglibContinuousPacket(sl_cli_command_arg_t *arguments);
void emAfMfglibStopContinuous(sl_cli_command_arg_t *arguments);
void emAfMfglibReceiveMode(sl_cli_command_arg_t *arguments);
void emAfMfglibReceiveStart(sl_cli_command_arg_t *arguments);
void emAfMfglibReceiveStop(sl_cli_command_arg_t *arguments);
void emAfMfglibClearPacketCounter(sl_cli_command_arg_t *arguments);
void emAfMfglibGetPackets(sl_cli_command_arg_t *arguments);
////MFG UPDATED CODE END--------------------------------------------------------------------------------------------------------------------

void emberAfPluginNetworkCreatorStartCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginNetworkCreatorStop(sl_cli_command_arg_t *arguments);
void emberAfPluginNetworkCreatorFormCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginNetworkCreatorChannelMaskCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginNetworkCreatorChannelMaskCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginNetworkCreatorChannelMaskCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginNetworkCreatorStatusCommand(sl_cli_command_arg_t *arguments);
void emAfPluginNetworkCreatorSecuritySetJoiningLinkKeyCommand(sl_cli_command_arg_t *arguments);
void emAfPluginNetworkCreatorSecurityClearJoiningLinkKeyCommand(sl_cli_command_arg_t *arguments);
void emAfPluginNetworkCreatorSecurityOpenOrCloseNetworkCommand(sl_cli_command_arg_t *arguments);
void emAfPluginNetworkCreatorSecurityOpenOrCloseNetworkCommand(sl_cli_command_arg_t *arguments);
void emAfPluginNetworkCreatorSecurityOpenNetworkWithKeyCommand(sl_cli_command_arg_t *arguments);
void emAfPluginNetworkCreatorSecurityConfigureDistributedKey(sl_cli_command_arg_t *arguments);
void emberAfPluginNetworkSteeringStatusCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginNetworkSteeringStartCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginNetworkSteeringStopCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginNetworkSteeringSetPreconfiguredKeyCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginNetworkSteeringChannelSetCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginNetworkSteeringChannelAddOrSubtractCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginNetworkSteeringChannelAddOrSubtractCommand(sl_cli_command_arg_t *arguments);
void emberAfPluginSetTCLinkKeyUpdateTimerCommand(sl_cli_command_arg_t *arguments);
void networkFormCommand(sl_cli_command_arg_t *arguments);
void networkJoinCommand(sl_cli_command_arg_t *arguments);
void networkPermitJoinCommand(sl_cli_command_arg_t *arguments);
void networkLeaveCommand(sl_cli_command_arg_t *arguments);
void networkRejoinCommand(sl_cli_command_arg_t *arguments);
void networkRejoinDiffDeviceTypeCommand(sl_cli_command_arg_t *arguments);
void networkExtendedPanIdCommand(sl_cli_command_arg_t *arguments);
void networkCheckPjoinCommand(sl_cli_command_arg_t *arguments);
void networkPermitJoinCommand(sl_cli_command_arg_t *arguments);
void findJoinableNetworkCommand(sl_cli_command_arg_t *arguments);
void findUnusedPanIdCommand(sl_cli_command_arg_t *arguments);
void networkChangeChannelCommand(sl_cli_command_arg_t *arguments);
void networkSetCommand(sl_cli_command_arg_t *arguments);
void networkInitCommand(sl_cli_command_arg_t *arguments);
void networkIdCommand(sl_cli_command_arg_t *arguments);
void changeKeepAliveModeCommand(sl_cli_command_arg_t *arguments);
void networkChangeChildTimeoutOptionMaskCommand(sl_cli_command_arg_t *arguments);
void networkMultiPhyStartCommand(sl_cli_command_arg_t *arguments);
void networkMultiPhyStopCommand(sl_cli_command_arg_t *arguments);
void keysPrintCommand(sl_cli_command_arg_t *arguments);
void keysClearCommand(sl_cli_command_arg_t *arguments);
void optionBindingTablePrintCommand(sl_cli_command_arg_t *arguments);
void optionBindingTableClearCommand(sl_cli_command_arg_t *arguments);
void optionBindingTableSetCommand(sl_cli_command_arg_t *arguments);
void optionPrintRxCommand(sl_cli_command_arg_t *arguments);
void optionPrintRxCommand(sl_cli_command_arg_t *arguments);
void optionRegisterCommand(sl_cli_command_arg_t *arguments);
void optionDiscoveryTargetCommand(sl_cli_command_arg_t *arguments);
void optionDiscoverCommand(sl_cli_command_arg_t *arguments);
void optionEndDeviceBindCommand(sl_cli_command_arg_t *arguments);
void optionApsRetryCommand(sl_cli_command_arg_t *arguments);
void optionApsRetryCommand(sl_cli_command_arg_t *arguments);
void optionApsRetryCommand(sl_cli_command_arg_t *arguments);
void optionApsSecurityCommand(sl_cli_command_arg_t *arguments);
void optionApsSecurityCommand(sl_cli_command_arg_t *arguments);
void optionSecurityAllowTrustCenterRejoinUsingWellKnownKey(sl_cli_command_arg_t *arguments);
void optionSecurityAllowTrustCenterRejoinUsingWellKnownKeyTimeout(sl_cli_command_arg_t *arguments);
void optionSecuritySetKeyRequestPolicy(sl_cli_command_arg_t *arguments);
void changeKeyCommand(sl_cli_command_arg_t *arguments);
void changeKeyCommand(sl_cli_command_arg_t *arguments);
void optionLinkCommand(sl_cli_command_arg_t *arguments);
void optionInstallCodeCommand(sl_cli_command_arg_t *arguments);
void zdoRouteRequestCommand(sl_cli_command_arg_t *arguments);
void zdoPowerDescriptorRequestCommand(sl_cli_command_arg_t *arguments);
void zdoMgmtLqiCommand(sl_cli_command_arg_t *arguments);
void zdoMgmtBindCommand(sl_cli_command_arg_t *arguments);
void zdoLeaveRequestCommand(sl_cli_command_arg_t *arguments);
void zdoUnbindGroupCommand(sl_cli_command_arg_t *arguments);
void zdoUnbindUnicastCommand(sl_cli_command_arg_t *arguments);
void zdoActiveEpCommand(sl_cli_command_arg_t *arguments);
void zdoBindCommand(sl_cli_command_arg_t *arguments);
void zdoNodeCommand(sl_cli_command_arg_t *arguments);
void zdoMatchCommand(sl_cli_command_arg_t *arguments);
void zdoSimpleCommand(sl_cli_command_arg_t *arguments);
void zdoIeeeAddressRequestCommand(sl_cli_command_arg_t *arguments);
void zdoNwkAddressRequestCommand(sl_cli_command_arg_t *arguments);
void zdoNetworkUpdateScanCommand(sl_cli_command_arg_t *arguments);
void zdoNetworkUpdateSetCommand(sl_cli_command_arg_t *arguments);
void zdoNetworkUpdateScanCommand(sl_cli_command_arg_t *arguments);
void zdoNetworkUpdateChannelCommand(sl_cli_command_arg_t *arguments);
void zdoNetworkUpdateChannelCommand(sl_cli_command_arg_t *arguments);
void zdoAddClusterCommand(sl_cli_command_arg_t *arguments);
void zdoClearClusterCommand(sl_cli_command_arg_t *arguments);
void zdoAddClusterCommand(sl_cli_command_arg_t *arguments);
void zdoClearClusterCommand(sl_cli_command_arg_t *arguments);
void emberAfPrintAttributeTable(sl_cli_command_arg_t *arguments);
void printTimeCommand(sl_cli_command_arg_t *arguments);
void formNetwork(sl_cli_command_arg_t *arguments);
void scanTouchLink(sl_cli_command_arg_t *arguments);
void scanTouchLink(sl_cli_command_arg_t *arguments);
void scanTouchLink(sl_cli_command_arg_t *arguments);
void abortTouchLink(sl_cli_command_arg_t *arguments);
void initiateTouchLink(sl_cli_command_arg_t *arguments);
void setIdentifyDuration(sl_cli_command_arg_t *arguments);
void endpointInformation(sl_cli_command_arg_t *arguments);
void getGroupIdentifiersRequest(sl_cli_command_arg_t *arguments);
void getEndpointListRequest(sl_cli_command_arg_t *arguments);
void printZllTokens(sl_cli_command_arg_t *arguments);
void setScanChannel(sl_cli_command_arg_t *arguments);
void setSecondaryScanChannel(sl_cli_command_arg_t *arguments);
void setScanMask(sl_cli_command_arg_t *arguments);
void statusCommand(sl_cli_command_arg_t *arguments);
void joinable(sl_cli_command_arg_t *arguments);
void unused(sl_cli_command_arg_t *arguments);
void emberAfZllResetToFactoryNew(sl_cli_command_arg_t *arguments);
void noTouchlinkForNFN(sl_cli_command_arg_t *arguments);
void noResetForNFN(sl_cli_command_arg_t *arguments);
void disable(sl_cli_command_arg_t *arguments);
void enable(sl_cli_command_arg_t *arguments);
void setRxOn(sl_cli_command_arg_t *arguments);
void cancelRxOn(sl_cli_command_arg_t *arguments);
void rxOnStatus(sl_cli_command_arg_t *arguments);

// Command structs. Names are in the format : cli_cmd_{command group name}_{command name}
// In order to support hyphen in command and group name, every occurence of it while
// building struct names will be replaced by "_hyphen_"
static const sl_cli_command_info_t cli_cmd_idle_hyphen_sleep_status = \
  SL_CLI_COMMAND(emberAfPluginIdleSleepStatusCommand,
                 "Display the sleep status",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_idle_hyphen_sleep_force_hyphen_awake = \
  SL_CLI_COMMAND(emberAfPluginIdleSleepStayAwakeCommand,
                 "Set or clear force awake",
                  "" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_idle_hyphen_sleep_awake_hyphen_when_hyphen_not_hyphen_joined = \
  SL_CLI_COMMAND(emberAfPluginIdleSleepAwakeWhenNotJoinedCommand,
                 "Set or clear stay awake when not joined",
                  "" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__info = \
  SL_CLI_COMMAND(emAfCliInfoCommand,
                 "Prints information about the network state, clusters, and endpoints.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__libs = \
  SL_CLI_COMMAND(printAllLibraryStatus,
                 "Lists which optional libraries of the stack are implemented on this device.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__bsend = \
  SL_CLI_COMMAND(emAfCliBsendCommand,
                 "Sends a message.",
                  "Source endpoint" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__send = \
  SL_CLI_COMMAND(sli_zigbee_cli_send_command,
                 "Sends a message.",
                  "Destination" SL_CLI_UNIT_SEPARATOR "Source endpoint" SL_CLI_UNIT_SEPARATOR "Destination endpoint" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__read = \
  SL_CLI_COMMAND(sli_zigbee_zcl_read_cli_command,
                 "Reads a message.",
                  "Endpoint" SL_CLI_UNIT_SEPARATOR "Cluster ID" SL_CLI_UNIT_SEPARATOR "Attribute ID" SL_CLI_UNIT_SEPARATOR "1 if server direction, 0 if client direction" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__write = \
  SL_CLI_COMMAND(sli_zigbee_zcl_write_cli_command,
                 "Writes a message.",
                  "Endpoint" SL_CLI_UNIT_SEPARATOR "Cluster ID" SL_CLI_UNIT_SEPARATOR "Attribute ID" SL_CLI_UNIT_SEPARATOR "Mask" SL_CLI_UNIT_SEPARATOR "Data type" SL_CLI_UNIT_SEPARATOR "Data byte" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__reset = \
  SL_CLI_COMMAND(resetCommand,
                 "Resets the node.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__raw = \
  SL_CLI_COMMAND(emAfCliRawCommand,
                 "Creates a message by specifying the raw bytes. Use the send command to send the message once it has been created. Ex: raw 0x000F {00 0A 00 11 22 33 44 55} sends a message to cluster 15 (0x000F) of length 8 which includes the ZCL header.",
                  "ClusterId" SL_CLI_UNIT_SEPARATOR "Data" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__send_multicast = \
  SL_CLI_COMMAND(sli_zigbee_cli_send_command,
                 "Sends a pre-buffered multicast message to a given group ID from a given endpoint.",
                  "groupId" SL_CLI_UNIT_SEPARATOR "src-endpoint" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__send_hyphen_using_hyphen_multicast_hyphen_binding = \
  SL_CLI_COMMAND(emAfCliSendUsingMulticastBindingCommand,
                 "When sending using a binding, specifies whether a multicast binding should be used.",
                  "useMulticastBinding" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__timesync = \
  SL_CLI_COMMAND(emAfCliTimesyncCommand,
                 "Sends a read attr for the time of the device specified. It sets a flag so when it gets the response it writes the time to its own time attr.",
                  "Id" SL_CLI_UNIT_SEPARATOR "srcEndpoint" SL_CLI_UNIT_SEPARATOR "destEndpoint" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__get_hyphen_pti_hyphen_radio_hyphen_config = \
  SL_CLI_COMMAND(sli_get_pti_radio_config,
                 "Returns PTI radio config information to determine the PHY type being used by RAIL. return value is enum RAIL_IEEE802154_PtiRadioConfig_t e.g  RAIL_IEEE802154_PTI_RADIO_CONFIG_2P4GHZ = 0x00U RAIL_IEEE802154_PTI_RADIO_CONFIG_2P4GHZ_ANTDIV = 0x01U RAIL_IEEE802154_PTI_RADIO_CONFIG_2P4GHZ_COEX = 0x02U RAIL_IEEE802154_PTI_RADIO_CONFIG_2P4GHZ_ANTDIV_COEX = 0x03U",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__config_hyphen_cca_hyphen_mode = \
  SL_CLI_COMMAND(sli_zigbee_cli_config_cca_mode_command,
                 "Set the configured 802.15.4 CCA mode in the radio. See documentation regarding RAIL_IEEE802154_CcaMode_t.",
                  "A RAIL_IEEE802154_CcaMode_t value" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__version = \
  SL_CLI_COMMAND(sli_zigbee_cli_version_command,
                 "Shows the version of the software.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_endpoints_print = \
  SL_CLI_COMMAND(endpointPrint,
                 "Print status of endpoints.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_endpoints_enable = \
  SL_CLI_COMMAND(enableDisableEndpoint,
                 "Enables the endpoint for ZCL messages.",
                  "Endpoint index" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_endpoints_disable = \
  SL_CLI_COMMAND(enableDisableEndpoint,
                 "Disable the endpoint for ZCL messages.",
                  "Endpoint index" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__events = \
  SL_CLI_COMMAND(printEvents,
                 "Print active events.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfg_hyphen_token_get = \
  SL_CLI_COMMAND(getSetMfgToken,
                 "Print MFG token.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfg_hyphen_token_set = \
  SL_CLI_COMMAND(getSetMfgToken,
                 "Set MFG token, CANNOT BE UNSET BY CLI.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_counters_print = \
  SL_CLI_COMMAND(emberAfPluginCountersPrintCommand,
                 "Prints all counter values and clears them.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_counters_print_hyphen_counter_hyphen_type = \
  SL_CLI_COMMAND(emberAfPluginCounterPrintCounterTypeCommand,
                 "Prints the value of this particular counter.",
                  "The counter type to print" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_counters_simple_hyphen_print = \
  SL_CLI_COMMAND(emberAfPluginCountersSimplePrintCommand,
                 "Prints all counter values.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_counters_clear = \
  SL_CLI_COMMAND(emberAfPluginCountersClear,
                 "Clears all counter values.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_counters_print_hyphen_thresholds = \
  SL_CLI_COMMAND(emberAfPluginCountersPrintThresholdsCommand,
                 "Prints the thresholds of all the counters.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_counters_set_hyphen_threshold = \
  SL_CLI_COMMAND(emberAfPluginCountersSetThresholdCommand,
                 "Sets a threshold value for a particular type of counter.",
                  "Type of counter" SL_CLI_UNIT_SEPARATOR "Threshold Value" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_counters_reset_hyphen_thresholds = \
  SL_CLI_COMMAND(emberAfPluginCountersResetThresholds,
                 "Resets all thresholds values to 0xFFFF.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_counters_send_hyphen_request = \
  SL_CLI_COMMAND(emberAfPluginCountersSendRequestCommand,
                 "Sends a request for OTA counters.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_enable_type_stack = \
  SL_CLI_COMMAND(sli_zigbee_debug_print_enable_stack_type_command,
                 "Enable/disable debug `stack` print type.",
                  "Enable/disable" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_enable_type_core = \
  SL_CLI_COMMAND(sli_zigbee_debug_print_enable_core_type_command,
                 "Enable/disable debug `core` print type.",
                  "Enable/disable" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_enable_type_app = \
  SL_CLI_COMMAND(sli_zigbee_debug_print_enable_app_type_command,
                 "Enable/disable debug `app` print type.",
                  "Enable/disable" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_enable_type_zcl = \
  SL_CLI_COMMAND(sli_zigbee_debug_print_enable_zcl_type_command,
                 "Enable/disable debug `zcl` print type.",
                  "Enable/disable" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_enable_type_legacy_af_debug = \
  SL_CLI_COMMAND(sli_zigbee_debug_print_enable_legacy_af_debug_type_command,
                 "Enable/disable debug `legacy app framework debug` print type.",
                  "Enable/disable" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_end_hyphen_device_hyphen_support_status = \
  SL_CLI_COMMAND(emberAfPluginEndDeviceSupportStatusCommand,
                 "Displays the status of the End Devices polling.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_end_hyphen_device_hyphen_support_poll_hyphen_completed_hyphen_callback = \
  SL_CLI_COMMAND(emberAfPluginEndDeviceSupportPollCompletedCallbackCommand,
                 "Sets whether the devices poll completed callback function is enabled.",
                  "The value indicating whether the devices poll completed callback function is enabled" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_INT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_end_hyphen_device_hyphen_support_force_hyphen_short_hyphen_poll = \
  SL_CLI_COMMAND(emberAfPluginEndDeviceSupportForceShortPollCommand,
                 "Sets whether the CLI forces the device to short poll.",
                  "The value indicating whether the device should short poll" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_INT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_find_and_bind_initiator = \
  SL_CLI_COMMAND(emberAfPluginFindAndBindInitiatorStartCommand,
                 "Makes this node start the initiator part of the finding and binding process.",
                  "The endpoint on which to begin the Finding and Binding initiator process" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_identify_print = \
  SL_CLI_COMMAND(emAfPluginIdentifyCliPrint,
                 "Print which endpoints are reporting.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_interpan_enable = \
  SL_CLI_COMMAND(emAfInterpanEnableCommand,
                 "Enables inter-PAN globally.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_interpan_disable = \
  SL_CLI_COMMAND(emAfInterpanDisableCommand,
                 "Disables inter-PAN globally.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_interpan_fragment_hyphen_test = \
  SL_CLI_COMMAND(emAfInterpanFragmentTestCommand,
                 "Sends a message of specified length of random values to target device over inter-PAN.",
                  "The PAN ID that the target is located on" SL_CLI_UNIT_SEPARATOR "The targets EUI64 (big endian)" SL_CLI_UNIT_SEPARATOR "The cluster ID that the sample message should contain" SL_CLI_UNIT_SEPARATOR "The length of the randomly-filled message to be sent across inter-PAN" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_HEX, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_interpan_set_hyphen_msg_hyphen_timeout = \
  SL_CLI_COMMAND(emAfInterpanSetMessageTimeoutCommand,
                 "Sets the timeout for inter-PAN messages sent and received.",
                  "Message timeout in seconds" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_interpan_group = \
  SL_CLI_COMMAND(emAfInterpanGroupShortCommand,
                 "Send an interpan message to a group id.",
                  "Group ID" SL_CLI_UNIT_SEPARATOR "Destination PAN ID" SL_CLI_UNIT_SEPARATOR "Destination Profile ID" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_interpan_short = \
  SL_CLI_COMMAND(emAfInterpanGroupShortCommand,
                 "Send an interpan message to a short id.",
                  "Short ID" SL_CLI_UNIT_SEPARATOR "Destination PAN ID" SL_CLI_UNIT_SEPARATOR "Destination Profile ID" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_interpan_long = \
  SL_CLI_COMMAND(emAfInterpanLongCommand,
                 "Send an interpan message to an 8-byte EUI64.",
                  "Long ID" SL_CLI_UNIT_SEPARATOR "Destination PAN ID" SL_CLI_UNIT_SEPARATOR "Destination Profile ID" SL_CLI_UNIT_SEPARATOR "Encrypt Options" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_HEX, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_start = \
  SL_CLI_COMMAND(emAfMfglibStartCommand,
                 "Start the mfglib test mode.",
                  "Use the Rx callback." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_INT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_stop = \
  SL_CLI_COMMAND(emAfMfglibStopCommand,
                 "Stop the mfglib test mode.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_tone_start = \
  SL_CLI_COMMAND(emAfMfglibToneStartCommand,
                 "Starts a tone test.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_tone_stop = \
  SL_CLI_COMMAND(emAfMfglibToneStopCommand,
                 "Stops the tone test.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_status = \
  SL_CLI_COMMAND(emAfMfglibStatusCommand,
                 "Prints the current status of the manufacturing library.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_set_hyphen_channel = \
  SL_CLI_COMMAND(emAfMfglibSetChannelCommand,
                 "Sets the channel used by the manufacturing library for testing.",
                  "The 802.15.4 channel number." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_set_hyphen_power = \
  SL_CLI_COMMAND(emAfMfglibSetPowerAndModeCommand,
                 "Set the power level and mode for the manufacturing test.",
                  "The power level" SL_CLI_UNIT_SEPARATOR "The power mode" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_INT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_stream_start = \
  SL_CLI_COMMAND(emAfMfglibStreamStartCommand,
                 "Start the stream test.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_stream_stop = \
  SL_CLI_COMMAND(emAfMfglibStreamStopCommand,
                 "Stop the stream test.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_send_random = \
  SL_CLI_COMMAND(emAfMfglibSendCommand,
                 "Send a series of test packets with random data..",
                  "The number of packets to send." SL_CLI_UNIT_SEPARATOR "The length of the packet to send." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_send_test = \
  SL_CLI_COMMAND(emAfMfglibSendCommand,
                 "Send a series of test packets with fixed data.",
                  "The number of packets to send." SL_CLI_UNIT_SEPARATOR "The length of the packet to send." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_send_message = \
  SL_CLI_COMMAND(emAfMfglibSendMessageCommand,
                 "Send a test message with the specified data in it.",
                  "The hex data to send, up to 16 bytes." SL_CLI_UNIT_SEPARATOR "The number of packets to send." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_HEX, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_sleep = \
  SL_CLI_COMMAND(emAfMfglibSleepCommand,
                 "Sleep.",
                  "Sleep duration (mS)." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_programEui = \
  SL_CLI_COMMAND(emAfMfglibProgramEuiCommand,
                 "Program EUI.",
                  "IEEE address to program." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_mfgenable = \
  SL_CLI_COMMAND(emAfMfglibEnableMfglib,
                 "Enable mfglib.",
                  "1 = enable, 0 = disable." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_set_hyphen_options = \
  SL_CLI_COMMAND(emAfMfglibSetOptions,
                 "Sets options in the manufacturing library for testing.",
                  "The options bitmask (0 = normal transmit, 1 = CSMA transmit)." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

////MFG UPDATED CODE START -----------------------------------------------------------------------------------------------------------------

static const sl_cli_command_info_t cli_cmd_mfglib_clear_hyphen_rx_hyphen_packets = \
  SL_CLI_COMMAND(emAfMfglibClearPackets,
                 "reset the number of received packets to 0",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_set_hyphen_deci_hyphen_dbm = \
  SL_CLI_COMMAND(emAfMfglibSetPower,
                 "Sets the power of radio transmissions(deci-dBm)",
                  "The power level" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_INT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_get_hyphen_deci_hyphen_dbm = \
  SL_CLI_COMMAND(emAfMfglibGetPower,
                 "Gets the power of radio transmissions(deci-dBm)",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_set_hyphen_cca = \
  SL_CLI_COMMAND(emAfMfglibSetCcaThresholdReg,
                 "Sets the temporary register value of the CCA threshold",
                  "The CCA threshold" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_INT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_get_hyphen_cca = \
  SL_CLI_COMMAND(emAfMfglibGetCcaThresholdReg,
                 "Gets the temporary register value of the CCA threshold",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_get_hyphen_ctune_hyphen_reg = \
  SL_CLI_COMMAND(emAfMfglibGetCtuneValueReg,
                 "Gets the temporary register of the CTUNE value",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_set_hyphen_ctune_hyphen_reg = \
  SL_CLI_COMMAND(emAfMfglibSetCtuneValueReg,
                 "Sets the temporary register of the CTUNE value",
                  "The CTUNE value" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_get_hyphen_cca_hyphen_tok = \
  SL_CLI_COMMAND(emAfMfglibGetCcaThresholdTok,
                 "Gets the token value of the CCA threshold",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_set_hyphen_cca_hyphen_tok = \
  SL_CLI_COMMAND(emAfMfglibSetCcaThresholdTok,
                 "Sets the token value of the CCA threshold",
                  "The CCA Threshold token" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_INT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_get_hyphen_ctune_hyphen_tok = \
  SL_CLI_COMMAND(emAfMfglibGetCtuneValueTok,
                 "Gets the token of the CTUNE value",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_set_hyphen_ctune_hyphen_tok = \
  SL_CLI_COMMAND(emAfMfglibSetCtuneValueTok,
                 "Sets the token of the CTUNE value",
                  "The CTUNE value token" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_set_hyphen_gpio = \
  SL_CLI_COMMAND(emAfMfglibSetGpio,
                 "Sets various gpio settings",
                  "port" SL_CLI_UNIT_SEPARATOR "pin" SL_CLI_UNIT_SEPARATOR "mode" SL_CLI_UNIT_SEPARATOR "out" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT32, SL_CLI_ARG_UINT32, SL_CLI_ARG_UINT32, SL_CLI_ARG_UINT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_get_hyphen_gpio = \
  SL_CLI_COMMAND(emAfMfglibGetGpio,
                 "Gets GPIO mode at a specified GPIO port and pin",
                  "port" SL_CLI_UNIT_SEPARATOR "pin" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT32, SL_CLI_ARG_UINT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_gpio_hyphen_help = \
  SL_CLI_COMMAND(emAfMfglibGpioHelp,
                 "Gives information of GPIO command inputs",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_tok_hyphen_dump = \
  SL_CLI_COMMAND(emAfMfglibTokDump,
                 "Dumps all tokens",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_set_hyphen_sleep = \
  SL_CLI_COMMAND(emAfMfglibSleepTest,
                 "Enters the specified sleep mode",
                  "mode" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_enter_hyphen_bl = \
  SL_CLI_COMMAND(emAfMfglibEnterBootloader,
                 "Enters the bootloader mode",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_per_hyphen_test = \
  SL_CLI_COMMAND(emAfMfglibPERTest,
                 "Sends specified # of packets at the specified interval to all devices on the same network set to receive;PER is displayed on the receiving device",
                  "The number of packets to send." SL_CLI_UNIT_SEPARATOR "The length of the interval between packets in microseconds." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_contPack_hyphen_start = \
  SL_CLI_COMMAND(emAfMfglibContinuousPacket,
                 "Starts sending continuous packets at a 1000 millisecond interval",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_contPack_hyphen_stop = \
  SL_CLI_COMMAND(emAfMfglibStopContinuous,
                 "Stops sending continuous packets",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_receive_hyphen_mode = \
  SL_CLI_COMMAND(emAfMfglibReceiveMode,
                 "Sets the print mode for incoming packets",
                  "The number of packets" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_INT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_receivePER_hyphen_start = \
  SL_CLI_COMMAND(emAfMfglibReceiveStart,
                 "Sets up receive mode for PER test by clearing packets and setting # of incoming packets",
                  "The number of expected incoming packets" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_receivePER_hyphen_stop = \
  SL_CLI_COMMAND(emAfMfglibReceiveStop,
                 "Stops receive mode and prints out PER data",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_clear_hyphen_tx_hyphen_packets = \
  SL_CLI_COMMAND(emAfMfglibClearPacketCounter,
                 "Clears the packet Counter(number of transmitted packets)",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mfglib_get_hyphen_tx_hyphen_packets = \
  SL_CLI_COMMAND(emAfMfglibGetPackets,
                 "Prints packet Counter(number of transmitted packets)",
                  "",
                 {SL_CLI_ARG_END, });

////MFG UPDATED CODE END--------------------------------------------------------------------------------------------------------------------


static const sl_cli_command_info_t cli_cmd_network_hyphen_creator_start = \
  SL_CLI_COMMAND(emberAfPluginNetworkCreatorStartCommand,
                 "Starts the network formation process.",
                  "Whether or not to form a centralized network. If this value is false, the device will attempt to join a distributed network." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_INT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_hyphen_creator_stop = \
  SL_CLI_COMMAND(emberAfPluginNetworkCreatorStop,
                 "Stops the network formation process.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_hyphen_creator_form = \
  SL_CLI_COMMAND(emberAfPluginNetworkCreatorFormCommand,
                 "Forms a network with specified parameters.",
                  "Whether or not to form a centralized network. If this value is false, the device will attempt to join a distributed network." SL_CLI_UNIT_SEPARATOR "PanID of the network to be formed" SL_CLI_UNIT_SEPARATOR "Tx power of the network to be formed" SL_CLI_UNIT_SEPARATOR "Channel of the network to be formed" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_INT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_INT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mask_add = \
  SL_CLI_COMMAND(emberAfPluginNetworkCreatorChannelMaskCommand,
                 "Adds a channel to the channel mask of choice.",
                  "The mask to which to add the channel. 1 chooses the primary channel mask, any other argument chooses the secondary channel mask." SL_CLI_UNIT_SEPARATOR "The channel to add to the channel mask" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mask_subtract = \
  SL_CLI_COMMAND(emberAfPluginNetworkCreatorChannelMaskCommand,
                 "Subtracts a channel from the channel mask of choice.",
                  "The mask from which to subtract the channel. 1 chooses the primary channel mask. Any other argument chooses the secondary channel mask." SL_CLI_UNIT_SEPARATOR "The channel to subtract from the channel mask" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_mask_set = \
  SL_CLI_COMMAND(emberAfPluginNetworkCreatorChannelMaskCommand,
                 "Sets a channel mask.",
                  "The mask  to set. 1 chooses the primary channel mask. Any other argument chooses the secondary channel mask." SL_CLI_UNIT_SEPARATOR "The bit mask to which to set the chosen channel mask." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_hyphen_creator_status = \
  SL_CLI_COMMAND(emberAfPluginNetworkCreatorStatusCommand,
                 "Prints the status of the network-creator component.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_hyphen_creator_hyphen_security_set_hyphen_joining_hyphen_link_hyphen_key = \
  SL_CLI_COMMAND(emAfPluginNetworkCreatorSecuritySetJoiningLinkKeyCommand,
                 "Sets the link key that a specific joining device will use when joining the network. This command can be also be used to add install code-derived link keys. If all FFs are entered for the EUI64 for the joining device, then this link key will be used for all joining devices without a joining key entry.",
                  "The EUI64 of the joining device" SL_CLI_UNIT_SEPARATOR "The link key that the joining device will use to enter the network" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_HEX, SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_hyphen_creator_hyphen_security_clear_hyphen_joining_hyphen_link_hyphen_keys = \
  SL_CLI_COMMAND(emAfPluginNetworkCreatorSecurityClearJoiningLinkKeyCommand,
                 "Clears all of the joining link keys stored in the stack.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_hyphen_creator_hyphen_security_open_hyphen_network = \
  SL_CLI_COMMAND(emAfPluginNetworkCreatorSecurityOpenOrCloseNetworkCommand,
                 "Opens the network for joining.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_hyphen_creator_hyphen_security_close_hyphen_network = \
  SL_CLI_COMMAND(emAfPluginNetworkCreatorSecurityOpenOrCloseNetworkCommand,
                 "Closes the network for joining.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_hyphen_creator_hyphen_security_open_hyphen_with_hyphen_key = \
  SL_CLI_COMMAND(emAfPluginNetworkCreatorSecurityOpenNetworkWithKeyCommand,
                 "Opens the network that would only allow the node with the specified EUI and link key pair to join.",
                  "The EUI64 of the joining device" SL_CLI_UNIT_SEPARATOR "The link key that the joining device will use to enter the network" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_HEX, SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_hyphen_creator_hyphen_security_set_hyphen_distributed_hyphen_key = \
  SL_CLI_COMMAND(emAfPluginNetworkCreatorSecurityConfigureDistributedKey,
                 "Sets the TC Link key for a distributed network.",
                  "The pre-configured distributed key that the joining device will use to enter the network" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_hyphen_steering_status = \
  SL_CLI_COMMAND(emberAfPluginNetworkSteeringStatusCommand,
                 "Displays the current status of the network steering process.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_hyphen_steering_start = \
  SL_CLI_COMMAND(emberAfPluginNetworkSteeringStartCommand,
                 "Starts the network steering process.",
                  "A mask of options for indicating specific behavior within the network-steering process." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_hyphen_steering_stop = \
  SL_CLI_COMMAND(emberAfPluginNetworkSteeringStopCommand,
                 "Stops the network steering process.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_hyphen_steering_pre_hyphen_configured_hyphen_key_hyphen_set = \
  SL_CLI_COMMAND(emberAfPluginNetworkSteeringSetPreconfiguredKeyCommand,
                 "Sets the pre-configured key so that the joining device can enter the network.",
                  "Sets the preconfigured key" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_shell_mask_1_set = \
  SL_CLI_COMMAND(emberAfPluginNetworkSteeringChannelSetCommand,
                 "Sets either the primary or secondary channel mask.",
                  "The channel mask to subtract the channel from" SL_CLI_UNIT_SEPARATOR "The value to set the channel mask to." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_shell_mask_1_add = \
  SL_CLI_COMMAND(emberAfPluginNetworkSteeringChannelAddOrSubtractCommand,
                 "Adds a channel to either the primary or secondary channel mask of the network-steering component.",
                  "The channel mask to add a channel to" SL_CLI_UNIT_SEPARATOR "The channel to add to the mask" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_shell_mask_1_subtract = \
  SL_CLI_COMMAND(emberAfPluginNetworkSteeringChannelAddOrSubtractCommand,
                 "Subtracts a channel from either the primary or secondary channel mask of the network-steering component.",
                  "The channel mask to subtract the channel from" SL_CLI_UNIT_SEPARATOR "The channel to subtract the mask from" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_update_hyphen_tc_hyphen_link_hyphen_key_timer = \
  SL_CLI_COMMAND(emberAfPluginSetTCLinkKeyUpdateTimerCommand,
                 "This sets the the amount of time between subsequent trust center link key updates in milliseconds.",
                  "The amount of time between subsequent trust center link key updates in milliseconds" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_form = \
  SL_CLI_COMMAND(networkFormCommand,
                 "Forms a network on a given channel, with a given TX Power and PAN ID.",
                  "The channel on which to form the network" SL_CLI_UNIT_SEPARATOR "One-byte signed value indicating the TX Power that the radio should be set to" SL_CLI_UNIT_SEPARATOR "The PAN ID on which to form the network" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_INT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_join = \
  SL_CLI_COMMAND(networkJoinCommand,
                 "Joins a network on a given channel, with a given TX Power and PAN ID.",
                  "The channel on which to join the network" SL_CLI_UNIT_SEPARATOR "One-byte signed value indicating the TX Power that the radio should be set to" SL_CLI_UNIT_SEPARATOR "The PAN ID on which to join the network" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_INT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_pjoin = \
  SL_CLI_COMMAND(networkPermitJoinCommand,
                 "Turns permit joining on for the amount of time indicated.",
                  "A single byte indicating how long the device should have permit joining turn on for. A value of 0xff turns permit join indefinitely." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_leave = \
  SL_CLI_COMMAND(networkLeaveCommand,
                 "Leaves a network.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_rejoin = \
  SL_CLI_COMMAND(networkRejoinCommand,
                 "ReJoins a network.",
                  "Boolean network key availability" SL_CLI_UNIT_SEPARATOR "Channel mask" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_INT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_rejoin_hyphen_diff_hyphen_device_hyphen_type = \
  SL_CLI_COMMAND(networkRejoinDiffDeviceTypeCommand,
                 "Rejoins an existing network in a secure or insecure manner with a different device type.",
                  "Boolean network key availability" SL_CLI_UNIT_SEPARATOR "Channel mask" SL_CLI_UNIT_SEPARATOR "An enumeration indicating the device type to rejoin as.The stack only accepts EMBER_END_DEVICE and EMBER_SLEEPY_END_DEVICE." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_INT32, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_extpanid = \
  SL_CLI_COMMAND(networkExtendedPanIdCommand,
                 "Writes the extended pan ID for the device.",
                  "extpanid" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_isopen = \
  SL_CLI_COMMAND(networkCheckPjoinCommand,
                 "Checks network pjoin status.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_broad_hyphen_pjoin = \
  SL_CLI_COMMAND(networkPermitJoinCommand,
                 "Permits joining on the network for a given number of seconds AND broadcasts a ZDO Mgmt Permit Joining request to all routers.",
                  "A single byte indicating how long the device should have permit joining turned on for. A value of 0xff turns on permit join indefinitely." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_find_joinable = \
  SL_CLI_COMMAND(findJoinableNetworkCommand,
                 "findJoinableNetwork",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_find_unused = \
  SL_CLI_COMMAND(findUnusedPanIdCommand,
                 "findUnusedPanId",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_change_hyphen_channel = \
  SL_CLI_COMMAND(networkChangeChannelCommand,
                 "Attempts to change device over to a different channel given in the channel argument.",
                  "The channel to change to" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_set = \
  SL_CLI_COMMAND(networkSetCommand,
                 "Sets the network index used by all future CLI commands.  Before executing a CLI command, the framework switches to this network.  After the command finishes executing, the framework switches back to the previous network.  The CLI uses the same network index until the device resets or it is changed through this command.",
                  "index" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_init = \
  SL_CLI_COMMAND(networkInitCommand,
                 "Initializes a network; this is a test command used for tc-swap-out testing.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_id = \
  SL_CLI_COMMAND(networkIdCommand,
                 "Prints the current Node ID, EUI64, and Pan ID.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_change_hyphen_keep_hyphen_alive_hyphen_mode = \
  SL_CLI_COMMAND(changeKeepAliveModeCommand,
                 "Switches between different keep alive modes supported by a router.",
                  "Keep alive mode" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_timeout_hyphen_option_hyphen_mask = \
  SL_CLI_COMMAND(networkChangeChildTimeoutOptionMaskCommand,
                 "Attempts to change the child timeout option mask to filter out undesirable values (e.g. no more than 3 days).",
                  "timeout option mask" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_multi_hyphen_phy_hyphen_start = \
  SL_CLI_COMMAND(networkMultiPhyStartCommand,
                 "Used to start multi-PHY interface other than native and form the network. The stack uses same PanId as native radio network.",
                  "page" SL_CLI_UNIT_SEPARATOR "channel" SL_CLI_UNIT_SEPARATOR "power" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_INT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_network_multi_hyphen_phy_hyphen_stop = \
  SL_CLI_COMMAND(networkMultiPhyStopCommand,
                 "Terminates the multi-PHY interface",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_keys_print = \
  SL_CLI_COMMAND(keysPrintCommand,
                 "Prints all security keys.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_keys_clear = \
  SL_CLI_COMMAND(keysClearCommand,
                 "Clears all security keys.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_binding_hyphen_table_print = \
  SL_CLI_COMMAND(optionBindingTablePrintCommand,
                 "Prints the binding table to the command line.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_binding_hyphen_table_clear = \
  SL_CLI_COMMAND(optionBindingTableClearCommand,
                 "Clears the binding table.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_binding_hyphen_table_set = \
  SL_CLI_COMMAND(optionBindingTableSetCommand,
                 "Sets the binding table",
                  "index" SL_CLI_UNIT_SEPARATOR "cluster" SL_CLI_UNIT_SEPARATOR "localEp" SL_CLI_UNIT_SEPARATOR "remoteEp" SL_CLI_UNIT_SEPARATOR "eui64" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_print_hyphen_rx_hyphen_msgs_enable = \
  SL_CLI_COMMAND(optionPrintRxCommand,
                 "Enables printing of Rx messages.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_print_hyphen_rx_hyphen_msgs_disable = \
  SL_CLI_COMMAND(optionPrintRxCommand,
                 "Disables printing of Rx messages.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_option_register = \
  SL_CLI_COMMAND(optionRegisterCommand,
                 "",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_option_target = \
  SL_CLI_COMMAND(optionDiscoveryTargetCommand,
                 "",
                  "targetId" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_option_disc = \
  SL_CLI_COMMAND(optionDiscoverCommand,
                 "",
                  "profileId" SL_CLI_UNIT_SEPARATOR "clusterId" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_option_edb = \
  SL_CLI_COMMAND(optionEndDeviceBindCommand,
                 "",
                  "endpointId" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_apsretry_on = \
  SL_CLI_COMMAND(optionApsRetryCommand,
                 "",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_apsretry_off = \
  SL_CLI_COMMAND(optionApsRetryCommand,
                 "",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_apsretry_default = \
  SL_CLI_COMMAND(optionApsRetryCommand,
                 "",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_aps_on = \
  SL_CLI_COMMAND(optionApsSecurityCommand,
                 "",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_aps_off = \
  SL_CLI_COMMAND(optionApsSecurityCommand,
                 "",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_security_set_hyphen_allow_hyphen_trust_hyphen_center_hyphen_rejoin_hyphen_using_hyphen_well_hyphen_known_hyphen_key = \
  SL_CLI_COMMAND(optionSecurityAllowTrustCenterRejoinUsingWellKnownKey,
                 "",
                  "Bool: allow trust center rejoin using well-known key" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_security_set_hyphen_allow_hyphen_trust_hyphen_center_hyphen_rejoin_hyphen_using_hyphen_well_hyphen_known_hyphen_key_hyphen_timeout = \
  SL_CLI_COMMAND(optionSecurityAllowTrustCenterRejoinUsingWellKnownKeyTimeout,
                 "",
                  "timeout" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_security_set_hyphen_key_hyphen_request_hyphen_policy = \
  SL_CLI_COMMAND(optionSecuritySetKeyRequestPolicy,
                 "",
                  "TC link key request policy" SL_CLI_UNIT_SEPARATOR "App link key request policy" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_changekey_link = \
  SL_CLI_COMMAND(changeKeyCommand,
                 "",
                  "key" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_changekey_network = \
  SL_CLI_COMMAND(changeKeyCommand,
                 "",
                  "key" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_option_link = \
  SL_CLI_COMMAND(optionLinkCommand,
                 "",
                  "index" SL_CLI_UNIT_SEPARATOR "eui64" SL_CLI_UNIT_SEPARATOR "key" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_HEX, SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_option_install_hyphen_code = \
  SL_CLI_COMMAND(optionInstallCodeCommand,
                 "",
                  "index" SL_CLI_UNIT_SEPARATOR "eui64" SL_CLI_UNIT_SEPARATOR "installCode" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_HEX, SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_route = \
  SL_CLI_COMMAND(zdoRouteRequestCommand,
                 "Sends a ZDO route request command to the target.",
                  "target" SL_CLI_UNIT_SEPARATOR "index" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_power = \
  SL_CLI_COMMAND(zdoPowerDescriptorRequestCommand,
                 "Sends a ZDO Power Descriptor Request to the target device.",
                  "target" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_mgmt_hyphen_lqi = \
  SL_CLI_COMMAND(zdoMgmtLqiCommand,
                 "Sends a ZDO MGMT-LQI (LQI Table) Request to the target device.",
                  "target" SL_CLI_UNIT_SEPARATOR "startIndex" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_mgmt_hyphen_bind = \
  SL_CLI_COMMAND(zdoMgmtBindCommand,
                 "Sends a ZDO MGMT-Bind (Binding Table) Request to the target device.",
                  "target" SL_CLI_UNIT_SEPARATOR "startIndex" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_leave = \
  SL_CLI_COMMAND(zdoLeaveRequestCommand,
                 "Sends a ZDO Management Leave command to the target device.",
                  "target" SL_CLI_UNIT_SEPARATOR "removeChildren" SL_CLI_UNIT_SEPARATOR "rejoin" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_unbind_group = \
  SL_CLI_COMMAND(zdoUnbindGroupCommand,
                 "Sends an unbind request for a multicast binding to the target device.",
                  "target" SL_CLI_UNIT_SEPARATOR "source eui64" SL_CLI_UNIT_SEPARATOR "source endpoint" SL_CLI_UNIT_SEPARATOR "clusterID" SL_CLI_UNIT_SEPARATOR "groupAddress" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_HEX, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_unbind_unicast = \
  SL_CLI_COMMAND(zdoUnbindUnicastCommand,
                 "Sends an unbind request for a unicast binding to the target device.",
                  "target" SL_CLI_UNIT_SEPARATOR "source eui64" SL_CLI_UNIT_SEPARATOR "source endpoint" SL_CLI_UNIT_SEPARATOR "clusterID" SL_CLI_UNIT_SEPARATOR "destinationEUI64" SL_CLI_UNIT_SEPARATOR "destination endpoint" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_HEX, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_HEX, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_active = \
  SL_CLI_COMMAND(zdoActiveEpCommand,
                 "Sends an active endpoint request to the device with the given short ID.",
                  "nodeId" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_bind = \
  SL_CLI_COMMAND(zdoBindCommand,
                 "Sends a ZDO Bind command to a device specified in the command arguments.",
                  "Destination" SL_CLI_UNIT_SEPARATOR "Source endpoint" SL_CLI_UNIT_SEPARATOR "Destination endpoint" SL_CLI_UNIT_SEPARATOR "clusterID" SL_CLI_UNIT_SEPARATOR "remoteEUI64" SL_CLI_UNIT_SEPARATOR "destEUI64" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_HEX, SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_node = \
  SL_CLI_COMMAND(zdoNodeCommand,
                 "Sends a node descriptor request to a given target device.",
                  "target" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_match = \
  SL_CLI_COMMAND(zdoMatchCommand,
                 "Sends a matchDescriptorsRequest to the given destination with the given profile.",
                  "nodeId" SL_CLI_UNIT_SEPARATOR "profile" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_simple = \
  SL_CLI_COMMAND(zdoSimpleCommand,
                 "Sends a simple descriptor request for the short address and endpoint specified.",
                  "dest" SL_CLI_UNIT_SEPARATOR "targetEndpoint" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_ieee = \
  SL_CLI_COMMAND(zdoIeeeAddressRequestCommand,
                 "Requests an IEEE address based on a given node ID.",
                  "nodeId" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zdo_nwk = \
  SL_CLI_COMMAND(zdoNwkAddressRequestCommand,
                 "Sends a network address request for the given IEEE address.",
                  "ieee" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_HEX, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_nwk_hyphen_upd_scan_hyphen_chan_hyphen_mask = \
  SL_CLI_COMMAND(zdoNetworkUpdateScanCommand,
                 "Performs an energy scan on given channel mask.",
                  "targetNodeId" SL_CLI_UNIT_SEPARATOR "scanDuration" SL_CLI_UNIT_SEPARATOR "scanCount" SL_CLI_UNIT_SEPARATOR "channelMask" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_nwk_hyphen_upd_set = \
  SL_CLI_COMMAND(zdoNetworkUpdateSetCommand,
                 "Broadcasts the ID of the new network manager and active channels.",
                  "nwkMgeId" SL_CLI_UNIT_SEPARATOR "channelMask" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_nwk_hyphen_upd_scan = \
  SL_CLI_COMMAND(zdoNetworkUpdateScanCommand,
                 "Performs an energy scan.",
                  "targetNodeId" SL_CLI_UNIT_SEPARATOR "scanDuration" SL_CLI_UNIT_SEPARATOR "scanCount" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_nwk_hyphen_upd_chanPg = \
  SL_CLI_COMMAND(zdoNetworkUpdateChannelCommand,
                 "Sends an update channel page req. The stack sends nwk enhanced update req for non-zero page.",
                  "channel" SL_CLI_UNIT_SEPARATOR "page" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_nwk_hyphen_upd_chan = \
  SL_CLI_COMMAND(zdoNetworkUpdateChannelCommand,
                 "Sends an update channel request.",
                  "channel" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_out_hyphen_cl_hyphen_list_add = \
  SL_CLI_COMMAND(zdoAddClusterCommand,
                 "Adds clusters to the known client (out) clusters on this device.",
                  "clusterId" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_out_hyphen_cl_hyphen_list_clear = \
  SL_CLI_COMMAND(zdoClearClusterCommand,
                 "Clears the ZDO list of client (out) clusters.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_in_hyphen_cl_hyphen_list_add = \
  SL_CLI_COMMAND(zdoAddClusterCommand,
                 "Adds clusters to the known client (in) clusters on this device.",
                  "clusterId" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_in_hyphen_cl_hyphen_list_clear = \
  SL_CLI_COMMAND(zdoClearClusterCommand,
                 "Clears the ZDO list of client (in) clusters.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_print_attr = \
  SL_CLI_COMMAND(emberAfPrintAttributeTable,
                 "Prints attribute.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_print_time = \
  SL_CLI_COMMAND(printTimeCommand,
                 "Prints time.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_form = \
  SL_CLI_COMMAND(formNetwork,
                 "Forms a ZLL network.",
                  "The channel on which to form the network" SL_CLI_UNIT_SEPARATOR "The power setting for network transmissions" SL_CLI_UNIT_SEPARATOR "The PAN identifier for the network" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_INT8, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_scan_device = \
  SL_CLI_COMMAND(scanTouchLink,
                 "Initiates a Touchlink for the purpose of retrieving information about a target device.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_scan_identify = \
  SL_CLI_COMMAND(scanTouchLink,
                 "Initiates a Touchlink for the purpose of causing a target device to identify itself.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_scan_reset = \
  SL_CLI_COMMAND(scanTouchLink,
                 "Initiates a Touchlink for the purpose of resetting a target device.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_abort = \
  SL_CLI_COMMAND(abortTouchLink,
                 "Aborts the Touchlink procedure.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_link = \
  SL_CLI_COMMAND(initiateTouchLink,
                 "Initiates the Touchlink procedure.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_identify = \
  SL_CLI_COMMAND(setIdentifyDuration,
                 "Sets the duration that a target device should remain in identify mode during touchlinking.",
                  "The duration (in tenths of a second) of identify mode or 0xFFFF to indicate that the target should use its own application-specific duration." SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_info = \
  SL_CLI_COMMAND(endpointInformation,
                 "Sends an EndpointInformationRequest to a client.",
                  "The network address of the device to which the request will be sent" SL_CLI_UNIT_SEPARATOR "The source endpoint from which the request will be sent" SL_CLI_UNIT_SEPARATOR "The destination endpoint to which the request will be sent" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_groups = \
  SL_CLI_COMMAND(getGroupIdentifiersRequest,
                 "Sends a GroupIdentifiersRequest to a server.",
                  "The network address of the device to which the request will be sent" SL_CLI_UNIT_SEPARATOR "The source endpoint from which the request will be sent" SL_CLI_UNIT_SEPARATOR "The destination endpoint to which the request will be sent" SL_CLI_UNIT_SEPARATOR "The group table index at which to start retrieving data" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_endpoints = \
  SL_CLI_COMMAND(getEndpointListRequest,
                 "Sends a GetEndpointListRequest to a server.",
                  "The network address of the device to which the request will be sent" SL_CLI_UNIT_SEPARATOR "The source endpoint from which the request will be sent" SL_CLI_UNIT_SEPARATOR "The destination endpoint to which the request will be sent" SL_CLI_UNIT_SEPARATOR "The endpoint index at which to start retrieving data" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT16, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_tokens = \
  SL_CLI_COMMAND(printZllTokens,
                 "Print the ZLL tokens.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_channel = \
  SL_CLI_COMMAND(setScanChannel,
                 "Sets the scan channel used by the ZLL Commissioning plugin.",
                  "The primary channel to be used" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_secondary_hyphen_channel = \
  SL_CLI_COMMAND(setSecondaryScanChannel,
                 "Sets the scan channel used by the ZLL Commissioning plugin.",
                  "The secondary channel to be used" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_mask = \
  SL_CLI_COMMAND(setScanMask,
                 "Sets the scan channel set used by the ZLL Commissioning component. An index of 0 sets the primary ZLL channel set, 1 is the +1 channel set, 2 is the +2 channel set, 3 is the +3 channel set, and 4 is all Zigbee channels.",
                  "The index of the channel mask to be used" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT8, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_status = \
  SL_CLI_COMMAND(statusCommand,
                 "Prints the ZLL channel set and tokens",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_joinable = \
  SL_CLI_COMMAND(joinable,
                 "Scans for joinable networks and attempts to join if a network is found.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_unused = \
  SL_CLI_COMMAND(unused,
                 "Scans for an unused PAN identifier and forms a new ZLL network.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_reset = \
  SL_CLI_COMMAND(emberAfZllResetToFactoryNew,
                 "Resets the local device to factory new.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_notouchlink_hyphen_nfn = \
  SL_CLI_COMMAND(noTouchlinkForNFN,
                 "Disables Touchlinking (stealing) for an NFN device.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_noreset_hyphen_nfn = \
  SL_CLI_COMMAND(noResetForNFN,
                 "Disables reset for an NFN device on a centralized security network.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_disable = \
  SL_CLI_COMMAND(disable,
                 "Disables Touchlinking. This overrides the notouchlink-nfn and noreset-nfn commands.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_enable = \
  SL_CLI_COMMAND(enable,
                 "Enables Touchlinking. This overrides the notouchlink-nfn and noreset-nfn commands.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_set_hyphen_rx_hyphen_on = \
  SL_CLI_COMMAND(setRxOn,
                 "Sets Rx On When Idle duration.",
                  "The duration for the Rx On period" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_cancel_hyphen_rx_hyphen_on = \
  SL_CLI_COMMAND(cancelRxOn,
                 "Cancels Rx On When Idle.",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_zll_hyphen_commissioning_rx_hyphen_on_hyphen_active = \
  SL_CLI_COMMAND(rxOnStatus,
                 "Gets Rx On When Idle status.",
                  "",
                 {SL_CLI_ARG_END, });


// Create group command tables and structs if cli_groups given
// in template. Group name is suffixed with _group_table for tables
// and group commands are cli_cmd_grp_( group name )
static const sl_cli_command_entry_t idle_hyphen_sleep_group_table[] = {
  { "status", &cli_cmd_idle_hyphen_sleep_status, false },
  { "force-awake", &cli_cmd_idle_hyphen_sleep_force_hyphen_awake, false },
  { "awake-when-not-joined", &cli_cmd_idle_hyphen_sleep_awake_hyphen_when_hyphen_not_hyphen_joined, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_idle_hyphen_sleep = \
  SL_CLI_COMMAND_GROUP(idle_hyphen_sleep_group_table, "Commands to control idling/sleeping of the device");

static const sl_cli_command_entry_t endpoints_group_table[] = {
  { "print", &cli_cmd_endpoints_print, false },
  { "enable", &cli_cmd_endpoints_enable, false },
  { "disable", &cli_cmd_endpoints_disable, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_endpoints = \
  SL_CLI_COMMAND_GROUP(endpoints_group_table, "endpoint related commands.");

static const sl_cli_command_entry_t mfg_hyphen_token_group_table[] = {
  { "get", &cli_cmd_mfg_hyphen_token_get, false },
  { "set", &cli_cmd_mfg_hyphen_token_set, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_mfg_hyphen_token = \
  SL_CLI_COMMAND_GROUP(mfg_hyphen_token_group_table, "Get/set MFG token.");

static const sl_cli_command_entry_t aps_group_table[] = {
  { "on", &cli_cmd_aps_on, false },
  { "off", &cli_cmd_aps_off, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_aps = \
  SL_CLI_COMMAND_GROUP(aps_group_table, "Security aps related commands.");

static const sl_cli_command_entry_t security_group_table[] = {
  { "set-allow-trust-center-rejoin-using-well-known-key", &cli_cmd_security_set_hyphen_allow_hyphen_trust_hyphen_center_hyphen_rejoin_hyphen_using_hyphen_well_hyphen_known_hyphen_key, false },
  { "set-allow-trust-center-rejoin-using-well-known-key-timeout", &cli_cmd_security_set_hyphen_allow_hyphen_trust_hyphen_center_hyphen_rejoin_hyphen_using_hyphen_well_hyphen_known_hyphen_key_hyphen_timeout, false },
  { "set-key-request-policy", &cli_cmd_security_set_hyphen_key_hyphen_request_hyphen_policy, false },
  { "mfg-token", &cli_cmd_grp_mfg_hyphen_token, false },
  { "aps", &cli_cmd_grp_aps, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_security = \
  SL_CLI_COMMAND_GROUP(security_group_table, "security related commands");

static const sl_cli_command_entry_t counters_group_table[] = {
  { "print", &cli_cmd_counters_print, false },
  { "print-counter-type", &cli_cmd_counters_print_hyphen_counter_hyphen_type, false },
  { "simple-print", &cli_cmd_counters_simple_hyphen_print, false },
  { "clear", &cli_cmd_counters_clear, false },
  { "print-thresholds", &cli_cmd_counters_print_hyphen_thresholds, false },
  { "set-threshold", &cli_cmd_counters_set_hyphen_threshold, false },
  { "reset-thresholds", &cli_cmd_counters_reset_hyphen_thresholds, false },
  { "send-request", &cli_cmd_counters_send_hyphen_request, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_counters = \
  SL_CLI_COMMAND_GROUP(counters_group_table, "counters related commands.");

static const sl_cli_command_entry_t enable_type_group_table[] = {
  { "stack", &cli_cmd_enable_type_stack, false },
  { "core", &cli_cmd_enable_type_core, false },
  { "app", &cli_cmd_enable_type_app, false },
  { "zcl", &cli_cmd_enable_type_zcl, false },
  { "legacy_af_debug", &cli_cmd_enable_type_legacy_af_debug, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_enable_type = \
  SL_CLI_COMMAND_GROUP(enable_type_group_table, "");

static const sl_cli_command_entry_t zigbee_print_group_table[] = {
  { "enable_type", &cli_cmd_grp_enable_type, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_zigbee_print = \
  SL_CLI_COMMAND_GROUP(zigbee_print_group_table, "");

static const sl_cli_command_entry_t end_hyphen_device_hyphen_support_group_table[] = {
  { "status", &cli_cmd_end_hyphen_device_hyphen_support_status, false },
  { "poll-completed-callback", &cli_cmd_end_hyphen_device_hyphen_support_poll_hyphen_completed_hyphen_callback, false },
  { "force-short-poll", &cli_cmd_end_hyphen_device_hyphen_support_force_hyphen_short_hyphen_poll, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_end_hyphen_device_hyphen_support = \
  SL_CLI_COMMAND_GROUP(end_hyphen_device_hyphen_support_group_table, "end-device-support related commands.");

static const sl_cli_command_entry_t find_and_bind_group_table[] = {
  { "initiator", &cli_cmd_find_and_bind_initiator, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_find_and_bind = \
  SL_CLI_COMMAND_GROUP(find_and_bind_group_table, "Find and bind related commands.");

static const sl_cli_command_entry_t identify_group_table[] = {
  { "print", &cli_cmd_identify_print, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_identify = \
  SL_CLI_COMMAND_GROUP(identify_group_table, "identify related commands");

static const sl_cli_command_entry_t interpan_group_table[] = {
  { "enable", &cli_cmd_interpan_enable, false },
  { "disable", &cli_cmd_interpan_disable, false },
  { "fragment-test", &cli_cmd_interpan_fragment_hyphen_test, false },
  { "set-msg-timeout", &cli_cmd_interpan_set_hyphen_msg_hyphen_timeout, false },
  { "group", &cli_cmd_interpan_group, false },
  { "short", &cli_cmd_interpan_short, false },
  { "long", &cli_cmd_interpan_long, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_interpan = \
  SL_CLI_COMMAND_GROUP(interpan_group_table, "interpan related commands.");

static const sl_cli_command_entry_t tone_group_table[] = {
  { "start", &cli_cmd_tone_start, false },
  { "stop", &cli_cmd_tone_stop, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_tone = \
  SL_CLI_COMMAND_GROUP(tone_group_table, "tone related commands");

static const sl_cli_command_entry_t stream_group_table[] = {
  { "start", &cli_cmd_stream_start, false },
  { "stop", &cli_cmd_stream_stop, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_stream = \
  SL_CLI_COMMAND_GROUP(stream_group_table, "stream related commands");

static const sl_cli_command_entry_t send_group_table[] = {
  { "random", &cli_cmd_send_random, false },
  { "test", &cli_cmd_send_test, false },
  { "message", &cli_cmd_send_message, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_send = \
  SL_CLI_COMMAND_GROUP(send_group_table, "send related commands");

static const sl_cli_command_entry_t mfglib_group_table[] = {
  { "start", &cli_cmd_mfglib_start, false },
  { "stop", &cli_cmd_mfglib_stop, false },
  { "status", &cli_cmd_mfglib_status, false },
  { "set-channel", &cli_cmd_mfglib_set_hyphen_channel, false },
  { "set-power", &cli_cmd_mfglib_set_hyphen_power, false },
  { "sleep", &cli_cmd_mfglib_sleep, false },
  { "programEui", &cli_cmd_mfglib_programEui, false },
  { "mfgenable", &cli_cmd_mfglib_mfgenable, false },
  { "set-options", &cli_cmd_mfglib_set_hyphen_options, false },
  { "tone", &cli_cmd_grp_tone, false },
  { "stream", &cli_cmd_grp_stream, false },
  { "send", &cli_cmd_grp_send, false },
  ////MFG UPDATED CODE START -----------------------------------------------------------------------------------------------------------------
   { "clear-rx-packets", &cli_cmd_mfglib_clear_hyphen_rx_hyphen_packets, false },
   { "set-deci-dbm", &cli_cmd_mfglib_set_hyphen_deci_hyphen_dbm, false },
   { "get-deci-dbm", &cli_cmd_mfglib_get_hyphen_deci_hyphen_dbm, false },
   { "set-cca", &cli_cmd_mfglib_set_hyphen_cca, false },
   { "get-cca", &cli_cmd_mfglib_get_hyphen_cca, false },
   { "get-ctune-reg", &cli_cmd_mfglib_get_hyphen_ctune_hyphen_reg, false },
   { "set-ctune-reg", &cli_cmd_mfglib_set_hyphen_ctune_hyphen_reg, false },
   { "get-cca-tok", &cli_cmd_mfglib_get_hyphen_cca_hyphen_tok, false },
   { "set-cca-tok", &cli_cmd_mfglib_set_hyphen_cca_hyphen_tok, false },
   { "get-ctune-tok", &cli_cmd_mfglib_get_hyphen_ctune_hyphen_tok, false },
   { "set-ctune-tok", &cli_cmd_mfglib_set_hyphen_ctune_hyphen_tok, false },
   { "set-gpio", &cli_cmd_mfglib_set_hyphen_gpio, false },
   { "get-gpio", &cli_cmd_mfglib_get_hyphen_gpio, false },
   { "gpio-help", &cli_cmd_mfglib_gpio_hyphen_help, false },
   { "tok-dump", &cli_cmd_mfglib_tok_hyphen_dump, false },
   { "set-sleep", &cli_cmd_mfglib_set_hyphen_sleep, false },
   { "enter-bl", &cli_cmd_mfglib_enter_hyphen_bl, false },
   { "per-test", &cli_cmd_mfglib_per_hyphen_test, false },
   { "contPack-start", &cli_cmd_mfglib_contPack_hyphen_start, false },
   { "contPack-stop", &cli_cmd_mfglib_contPack_hyphen_stop, false },
   { "receive-mode", &cli_cmd_mfglib_receive_hyphen_mode, false },
   { "receivePER-start", &cli_cmd_mfglib_receivePER_hyphen_start, false },
   { "receivePER-stop", &cli_cmd_mfglib_receivePER_hyphen_stop, false },
   { "clear-tx-packets", &cli_cmd_mfglib_clear_hyphen_tx_hyphen_packets, false },
   { "get-tx-packets", &cli_cmd_mfglib_get_hyphen_tx_hyphen_packets, false },
   ////MFG UPDATED CODE END--------------------------------------------------------------------------------------------------------------------
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_mfglib = \
  SL_CLI_COMMAND_GROUP(mfglib_group_table, "mfglib related commands");

static const sl_cli_command_entry_t mask_group_table[] = {
  { "add", &cli_cmd_mask_add, false },
  { "subtract", &cli_cmd_mask_subtract, false },
  { "set", &cli_cmd_mask_set, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_mask = \
  SL_CLI_COMMAND_GROUP(mask_group_table, "mask related commands");

static const sl_cli_command_entry_t network_hyphen_creator_group_table[] = {
  { "start", &cli_cmd_network_hyphen_creator_start, false },
  { "stop", &cli_cmd_network_hyphen_creator_stop, false },
  { "form", &cli_cmd_network_hyphen_creator_form, false },
  { "status", &cli_cmd_network_hyphen_creator_status, false },
  { "mask", &cli_cmd_grp_mask, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_network_hyphen_creator = \
  SL_CLI_COMMAND_GROUP(network_hyphen_creator_group_table, "network-creator related commands.");

static const sl_cli_command_entry_t network_hyphen_creator_hyphen_security_group_table[] = {
  { "set-joining-link-key", &cli_cmd_network_hyphen_creator_hyphen_security_set_hyphen_joining_hyphen_link_hyphen_key, false },
  { "clear-joining-link-keys", &cli_cmd_network_hyphen_creator_hyphen_security_clear_hyphen_joining_hyphen_link_hyphen_keys, false },
  { "open-network", &cli_cmd_network_hyphen_creator_hyphen_security_open_hyphen_network, false },
  { "close-network", &cli_cmd_network_hyphen_creator_hyphen_security_close_hyphen_network, false },
  { "open-with-key", &cli_cmd_network_hyphen_creator_hyphen_security_open_hyphen_with_hyphen_key, false },
  { "set-distributed-key", &cli_cmd_network_hyphen_creator_hyphen_security_set_hyphen_distributed_hyphen_key, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_network_hyphen_creator_hyphen_security = \
  SL_CLI_COMMAND_GROUP(network_hyphen_creator_hyphen_security_group_table, "network-creator-security related commands.");

static const sl_cli_command_entry_t shell_mask_1_group_table[] = {
  { "set", &cli_cmd_shell_mask_1_set, false },
  { "add", &cli_cmd_shell_mask_1_add, false },
  { "subtract", &cli_cmd_shell_mask_1_subtract, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_shell_mask_1 = \
  SL_CLI_COMMAND_GROUP(shell_mask_1_group_table, "mask related command.");

static const sl_cli_command_entry_t network_hyphen_steering_group_table[] = {
  { "status", &cli_cmd_network_hyphen_steering_status, false },
  { "start", &cli_cmd_network_hyphen_steering_start, false },
  { "stop", &cli_cmd_network_hyphen_steering_stop, false },
  { "pre-configured-key-set", &cli_cmd_network_hyphen_steering_pre_hyphen_configured_hyphen_key_hyphen_set, false },
  { "mask", &cli_cmd_grp_shell_mask_1, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_network_hyphen_steering = \
  SL_CLI_COMMAND_GROUP(network_hyphen_steering_group_table, "network-steering related commands.");

static const sl_cli_command_entry_t update_hyphen_tc_hyphen_link_hyphen_key_group_table[] = {
  { "timer", &cli_cmd_update_hyphen_tc_hyphen_link_hyphen_key_timer, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_update_hyphen_tc_hyphen_link_hyphen_key = \
  SL_CLI_COMMAND_GROUP(update_hyphen_tc_hyphen_link_hyphen_key_group_table, "update-tc-link-key related commands");

static const sl_cli_command_entry_t scan_group_table[] = {
  { "device", &cli_cmd_scan_device, false },
  { "identify", &cli_cmd_scan_identify, false },
  { "reset", &cli_cmd_scan_reset, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_scan = \
  SL_CLI_COMMAND_GROUP(scan_group_table, "scan related commands");

static const sl_cli_command_entry_t zll_hyphen_commissioning_group_table[] = {
  { "form", &cli_cmd_zll_hyphen_commissioning_form, false },
  { "abort", &cli_cmd_zll_hyphen_commissioning_abort, false },
  { "link", &cli_cmd_zll_hyphen_commissioning_link, false },
  { "identify", &cli_cmd_zll_hyphen_commissioning_identify, false },
  { "info", &cli_cmd_zll_hyphen_commissioning_info, false },
  { "groups", &cli_cmd_zll_hyphen_commissioning_groups, false },
  { "endpoints", &cli_cmd_zll_hyphen_commissioning_endpoints, false },
  { "tokens", &cli_cmd_zll_hyphen_commissioning_tokens, false },
  { "channel", &cli_cmd_zll_hyphen_commissioning_channel, false },
  { "secondary-channel", &cli_cmd_zll_hyphen_commissioning_secondary_hyphen_channel, false },
  { "mask", &cli_cmd_zll_hyphen_commissioning_mask, false },
  { "status", &cli_cmd_zll_hyphen_commissioning_status, false },
  { "joinable", &cli_cmd_zll_hyphen_commissioning_joinable, false },
  { "unused", &cli_cmd_zll_hyphen_commissioning_unused, false },
  { "reset", &cli_cmd_zll_hyphen_commissioning_reset, false },
  { "notouchlink-nfn", &cli_cmd_zll_hyphen_commissioning_notouchlink_hyphen_nfn, false },
  { "noreset-nfn", &cli_cmd_zll_hyphen_commissioning_noreset_hyphen_nfn, false },
  { "disable", &cli_cmd_zll_hyphen_commissioning_disable, false },
  { "enable", &cli_cmd_zll_hyphen_commissioning_enable, false },
  { "set-rx-on", &cli_cmd_zll_hyphen_commissioning_set_hyphen_rx_hyphen_on, false },
  { "cancel-rx-on", &cli_cmd_zll_hyphen_commissioning_cancel_hyphen_rx_hyphen_on, false },
  { "rx-on-active", &cli_cmd_zll_hyphen_commissioning_rx_hyphen_on_hyphen_active, false },
  { "scan", &cli_cmd_grp_scan, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_zll_hyphen_commissioning = \
  SL_CLI_COMMAND_GROUP(zll_hyphen_commissioning_group_table, "zll-commissioning related commands");

static const sl_cli_command_entry_t plugin_group_table[] = {
  { "idle-sleep", &cli_cmd_grp_idle_hyphen_sleep, false },
  { "counters", &cli_cmd_grp_counters, false },
  { "end-device-support", &cli_cmd_grp_end_hyphen_device_hyphen_support, false },
  { "find_and_bind", &cli_cmd_grp_find_and_bind, false },
  { "identify", &cli_cmd_grp_identify, false },
  { "interpan", &cli_cmd_grp_interpan, false },
  { "mfglib", &cli_cmd_grp_mfglib, false },
  { "network-creator", &cli_cmd_grp_network_hyphen_creator, false },
  { "network-creator-security", &cli_cmd_grp_network_hyphen_creator_hyphen_security, false },
  { "network-steering", &cli_cmd_grp_network_hyphen_steering, false },
  { "update-tc-link-key", &cli_cmd_grp_update_hyphen_tc_hyphen_link_hyphen_key, false },
  { "zll-commissioning", &cli_cmd_grp_zll_hyphen_commissioning, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_plugin = \
  SL_CLI_COMMAND_GROUP(plugin_group_table, "");

static const sl_cli_command_entry_t find_group_table[] = {
  { "joinable", &cli_cmd_find_joinable, false },
  { "unused", &cli_cmd_find_unused, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_find = \
  SL_CLI_COMMAND_GROUP(find_group_table, "network find commands.");

static const sl_cli_command_entry_t network_group_table[] = {
  { "form", &cli_cmd_network_form, false },
  { "join", &cli_cmd_network_join, false },
  { "pjoin", &cli_cmd_network_pjoin, false },
  { "leave", &cli_cmd_network_leave, false },
  { "rejoin", &cli_cmd_network_rejoin, false },
  { "rejoin-diff-device-type", &cli_cmd_network_rejoin_hyphen_diff_hyphen_device_hyphen_type, false },
  { "extpanid", &cli_cmd_network_extpanid, false },
  { "isopen", &cli_cmd_network_isopen, false },
  { "broad-pjoin", &cli_cmd_network_broad_hyphen_pjoin, false },
  { "change-channel", &cli_cmd_network_change_hyphen_channel, false },
  { "set", &cli_cmd_network_set, false },
  { "init", &cli_cmd_network_init, false },
  { "id", &cli_cmd_network_id, false },
  { "change-keep-alive-mode", &cli_cmd_network_change_hyphen_keep_hyphen_alive_hyphen_mode, false },
  { "timeout-option-mask", &cli_cmd_network_timeout_hyphen_option_hyphen_mask, false },
  { "multi-phy-start", &cli_cmd_network_multi_hyphen_phy_hyphen_start, false },
  { "multi-phy-stop", &cli_cmd_network_multi_hyphen_phy_hyphen_stop, false },
  { "find", &cli_cmd_grp_find, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_network = \
  SL_CLI_COMMAND_GROUP(network_group_table, "Network related commands.");

static const sl_cli_command_entry_t keys_group_table[] = {
  { "print", &cli_cmd_keys_print, false },
  { "clear", &cli_cmd_keys_clear, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_keys = \
  SL_CLI_COMMAND_GROUP(keys_group_table, "Security keys related commands.");

static const sl_cli_command_entry_t binding_hyphen_table_group_table[] = {
  { "print", &cli_cmd_binding_hyphen_table_print, false },
  { "clear", &cli_cmd_binding_hyphen_table_clear, false },
  { "set", &cli_cmd_binding_hyphen_table_set, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_binding_hyphen_table = \
  SL_CLI_COMMAND_GROUP(binding_hyphen_table_group_table, "Binding table related commands.");

static const sl_cli_command_entry_t print_hyphen_rx_hyphen_msgs_group_table[] = {
  { "enable", &cli_cmd_print_hyphen_rx_hyphen_msgs_enable, false },
  { "disable", &cli_cmd_print_hyphen_rx_hyphen_msgs_disable, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_print_hyphen_rx_hyphen_msgs = \
  SL_CLI_COMMAND_GROUP(print_hyphen_rx_hyphen_msgs_group_table, "Enables/Disables printing of Rx messages.");

static const sl_cli_command_entry_t apsretry_group_table[] = {
  { "on", &cli_cmd_apsretry_on, false },
  { "off", &cli_cmd_apsretry_off, false },
  { "default", &cli_cmd_apsretry_default, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_apsretry = \
  SL_CLI_COMMAND_GROUP(apsretry_group_table, "APS retry flag force commands");

static const sl_cli_command_entry_t option_group_table[] = {
  { "register", &cli_cmd_option_register, false },
  { "target", &cli_cmd_option_target, false },
  { "disc", &cli_cmd_option_disc, false },
  { "edb", &cli_cmd_option_edb, false },
  { "link", &cli_cmd_option_link, false },
  { "install-code", &cli_cmd_option_install_hyphen_code, false },
  { "binding-table", &cli_cmd_grp_binding_hyphen_table, false },
  { "print-rx-msgs", &cli_cmd_grp_print_hyphen_rx_hyphen_msgs, false },
  { "apsretry", &cli_cmd_grp_apsretry, false },
  { "security", &cli_cmd_grp_security, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_option = \
  SL_CLI_COMMAND_GROUP(option_group_table, "Option related commands.");

static const sl_cli_command_entry_t changekey_group_table[] = {
  { "link", &cli_cmd_changekey_link, false },
  { "network", &cli_cmd_changekey_network, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_changekey = \
  SL_CLI_COMMAND_GROUP(changekey_group_table, "changekey related commands.");

static const sl_cli_command_entry_t unbind_group_table[] = {
  { "group", &cli_cmd_unbind_group, false },
  { "unicast", &cli_cmd_unbind_unicast, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_unbind = \
  SL_CLI_COMMAND_GROUP(unbind_group_table, "Zdo unbind related commands.");

static const sl_cli_command_entry_t nwk_hyphen_upd_group_table[] = {
  { "scan-chan-mask", &cli_cmd_nwk_hyphen_upd_scan_hyphen_chan_hyphen_mask, false },
  { "set", &cli_cmd_nwk_hyphen_upd_set, false },
  { "scan", &cli_cmd_nwk_hyphen_upd_scan, false },
  { "chanPg", &cli_cmd_nwk_hyphen_upd_chanPg, false },
  { "chan", &cli_cmd_nwk_hyphen_upd_chan, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_nwk_hyphen_upd = \
  SL_CLI_COMMAND_GROUP(nwk_hyphen_upd_group_table, "Zdo nwk-upd related commands.");

static const sl_cli_command_entry_t out_hyphen_cl_hyphen_list_group_table[] = {
  { "add", &cli_cmd_out_hyphen_cl_hyphen_list_add, false },
  { "clear", &cli_cmd_out_hyphen_cl_hyphen_list_clear, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_out_hyphen_cl_hyphen_list = \
  SL_CLI_COMMAND_GROUP(out_hyphen_cl_hyphen_list_group_table, "Zdo out-cl-list related commands.");

static const sl_cli_command_entry_t in_hyphen_cl_hyphen_list_group_table[] = {
  { "add", &cli_cmd_in_hyphen_cl_hyphen_list_add, false },
  { "clear", &cli_cmd_in_hyphen_cl_hyphen_list_clear, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_in_hyphen_cl_hyphen_list = \
  SL_CLI_COMMAND_GROUP(in_hyphen_cl_hyphen_list_group_table, "Zdo in-cl-list related commands.");

static const sl_cli_command_entry_t zdo_group_table[] = {
  { "route", &cli_cmd_zdo_route, false },
  { "power", &cli_cmd_zdo_power, false },
  { "mgmt-lqi", &cli_cmd_zdo_mgmt_hyphen_lqi, false },
  { "mgmt-bind", &cli_cmd_zdo_mgmt_hyphen_bind, false },
  { "leave", &cli_cmd_zdo_leave, false },
  { "active", &cli_cmd_zdo_active, false },
  { "bind", &cli_cmd_zdo_bind, false },
  { "node", &cli_cmd_zdo_node, false },
  { "match", &cli_cmd_zdo_match, false },
  { "simple", &cli_cmd_zdo_simple, false },
  { "ieee", &cli_cmd_zdo_ieee, false },
  { "nwk", &cli_cmd_zdo_nwk, false },
  { "unbind", &cli_cmd_grp_unbind, false },
  { "nwk-upd", &cli_cmd_grp_nwk_hyphen_upd, false },
  { "out-cl-list", &cli_cmd_grp_out_hyphen_cl_hyphen_list, false },
  { "in-cl-list", &cli_cmd_grp_in_hyphen_cl_hyphen_list, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_zdo = \
  SL_CLI_COMMAND_GROUP(zdo_group_table, "Zdo related commands.");

static const sl_cli_command_entry_t print_group_table[] = {
  { "attr", &cli_cmd_print_attr, false },
  { "time", &cli_cmd_print_time, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_print = \
  SL_CLI_COMMAND_GROUP(print_group_table, "print related commands.");

// Create root command table
const sl_cli_command_entry_t sl_cli_default_command_table[] = {
  { "info", &cli_cmd__info, false },
  { "libs", &cli_cmd__libs, false },
  { "bsend", &cli_cmd__bsend, false },
  { "send", &cli_cmd__send, false },
  { "read", &cli_cmd__read, false },
  { "write", &cli_cmd__write, false },
  { "reset", &cli_cmd__reset, false },
  { "raw", &cli_cmd__raw, false },
  { "send_multicast", &cli_cmd__send_multicast, false },
  { "send-using-multicast-binding", &cli_cmd__send_hyphen_using_hyphen_multicast_hyphen_binding, false },
  { "timesync", &cli_cmd__timesync, false },
  { "get-pti-radio-config", &cli_cmd__get_hyphen_pti_hyphen_radio_hyphen_config, false },
  { "config-cca-mode", &cli_cmd__config_hyphen_cca_hyphen_mode, false },
  { "version", &cli_cmd__version, false },
  { "events", &cli_cmd__events, false },
  { "endpoints", &cli_cmd_grp_endpoints, false },
  { "security", &cli_cmd_grp_security, false },
  { "zigbee_print", &cli_cmd_grp_zigbee_print, false },
  { "plugin", &cli_cmd_grp_plugin, false },
  { "network", &cli_cmd_grp_network, false },
  { "net", &cli_cmd_grp_network, true },
  { "keys", &cli_cmd_grp_keys, false },
  { "option", &cli_cmd_grp_option, false },
  { "opt", &cli_cmd_grp_option, true },
  { "plugin", &cli_cmd_grp_plugin, false },
  { "plug", &cli_cmd_grp_plugin, true },
  { "changekey", &cli_cmd_grp_changekey, false },
  { "changek", &cli_cmd_grp_changekey, true },
  { "zdo", &cli_cmd_grp_zdo, false },
  { "print", &cli_cmd_grp_print, false },
  { NULL, NULL, false },
};


#ifdef __cplusplus
}
#endif
