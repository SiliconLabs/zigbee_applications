# Zigbee Battery Monitor Example #

## Summary ##
EmberZnet SDK has provided a plugin "Battery Monitor" to simplify the development of Zigbee battery monitor application. It works fine on series 1 SoCs, but doesn't work on series 2. The root cause is that the plugin uses ADC0 to read the voltage of the battery, but for series 2, there is only IADC. 

This project will provide an updated plugin "Battery Monitor V2" which supports both series 1 SoCs and series 2 SoCs. The detailed steps will also be provided to implement a Zigbee battery monitor application with this plugin.

Here is how the plugin works:
<div align="center">
    <img src="doc/rac-prs-gpio.png">
</div>
<br>

1. We use the PRS to associate the radio signal `Radio Tx Complete` to a GPIO pin. So that when a packet is sent to the air, the GPIO pin will go high.
2. Then we configure the GPIO to generate an interrupt when the level goes high.
3. When serving the interrupt, an event is activated. Then we start to read the ADC or IADC.
4. The PRS channel and the GPIO pin used here are both configurable.


## Gecko SDK version ##
Gecko SDK Suite 3.2.

## Hardware Required ##

Kit | Radio Board | Purpose
---------|----------|---------
 BRD4001 <p>Wireless Starter Kit Main Board | BRD4161A <p>EFR32MG12 2.4GHz 19 dBm Radio Board | Demonstrate the battery monitor application on series 1
 BRD4001 <p>Wireless Starter Kit Main Board | BRD4181A <p>EFR32xG21 2.4GHz 10 dBm Radio Board | Demonstrate the battery monitor application on EFR32MG21
 BRD4001 <p>Wireless Starter Kit Main Board | BRD4182A <p>EFR32xG22 2.4 GHz 6 dBm Radio Board | Demonstrate the battery monitor application on EFR32MG22

## Connections Required ##
NA

## Setup ##
1. Copy the directory `battery-monitor-v2` under the src directory to the path `v3.2\util\plugin\plugin-afv2` of your SDK, then restart Simplicity Studio V5.
2. Create a `ZigbeeMinimal` sample project;
3. In **"ZCL Clusters"** tab, select endpoint 1, then enable the server side of cluster `Power Configuration`, enable the attribute `battery voltage`, make sure the reporting of attribute `battery voltage` is enabled; Set the maximum reporting interval of battery voltage to **10** seconds to make the test simpler.
4. Select the plugin `Power Configuration Cluster Server`;
5. Select the plugin `Battery Monitor V2`, in the properties, choose a PRS channel and a GPIO pin to associate the **TX_ACTIVE** signal of the radio with the ADC or IADC. For series 1, the location of the GPIO will also need to be configured. 
6. Enable the plugin `Reporting`, so that the battery voltage can be reported.
7. Save and generate the project, then build and test.


## How It Works ##
1. Join the sleepy end device into the network.
2. Bind the endpoint 1 of the sleepy end device to the coordinator.
3. The sleepy end device will report its battery voltage in an interval of less than 10 seconds.

## .sls Projects Used ##
- [zigbee_battery_monitor_series1.sls](SimplicityStudio/zigbee_battery_monitor_series1.sls)
- [zigbee_battery_monitor_mg21.sls](SimplicityStudio/zigbee_battery_monitor_mg21.sls)
- [zigbee_battery_monitor_mg22.sls](SimplicityStudio/zigbee_battery_monitor_mg22.sls)

## How to Port to Another Part ##
- Import the .sls file into Simplicity Studio
- Open the .isc file of each project, turn to "General" tab, hit button "Edit Architecture", then select the board and part.

## Special Notes ##
NA
