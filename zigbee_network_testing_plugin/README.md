# Zigbee Network Testing Plugin #

## Summary ##
This plugin provides the cli commands for network testing on Host application,and it depends on the device table plugin. It is not avaiable on SOC project. 

plugin network-testing-cli... 
	
*  table-send - Send command to all the devices in the device table. Test Round means the number of times the messages are sent. the command payload can be filled with [zcl cli command](https://docs.silabs.com/zigbee/6.10/zigbee-af-api/zcl-global)

    "uint8_t  destination endpoint"

    "uint16_t  Test Round"
	
* table-sort - Sort device table by Eui64.
* table-bind - Send zdo bind command to all the devices in the device table

	"uint8_t  endpoint of the device"
    
   "uint16_t  Cluster Id"
   
* table-active - Send zdo active endpoint request command to the devices whose endpoint is 0 in the device table. 

* table-simple - Send zdo simple descriptor command to all the devices in the device table.

	"uint8_t  endpoint of the device"
   

## Gecko SDK version ##

Gecko SDK Suite 3.2

## Hardware Required ##

* Wireless Starter Kit Main Board (BRD4001)
* Any EFR32 radio board.


## How to use ##
1. Copy the network-testing-cli folder to C:\SiliconLabs\SimplicityStudio\v5\developer\sdks\gecko_sdk_suite\v3.2\protocol\zigbee\app\framework\plugin-host.
2. Launch the Simplicity Studio V5 and generate a Z3GatewayHost project.
3. Network Testing plugin will be shown in Plugins window.
![zigbee](doc/network_testing_plugin.PNG)
4. Enable the plugin, you will see the following cli commands.                
![cli](doc/cli.PNG)
5. Send the toggle command to the devices in device table with following cli commands.
    
    * zcl on-off toggle
    * plugin network-testing-cli table-send  1 1

### How to use DISCOVER\_DEVICES\_MANUALLY macro###
Network Testing Plugin depends on the device table plugin. device table plugin will send the active endpoint request and simple descriptor request to the joining device to discover the active endpoints and clusters after the device join the network. it easily results in missing packet if many devices join the network simultaneously. To avoid this issue, DISCOVER\_DEVICES\_MANUALLY macro is designed to discover the active endpoints and clusters manually with the CLI command. 

DISCOVER\_DEVICES\_MANUALLY macro is commented out by default. to enable this macro, you also have to comment out emberAfTrustCenterJoinCallback() and comment out the content of emAfPluginDeviceTablePreZDOMessageReceived() in device-table-discovery.c

After all of devices join the network. issue the following CLI command to discover the active endpoints and clusters for all devices in the device table. Please note "plugin network-testing-cli table-active" is only available to check the devices whose endpoint is 0 in device table.
  
  * plugin network-testing-cli table-active
  * plugin network-testing-cli table-simple "uint8_t  endpoint of the device"


