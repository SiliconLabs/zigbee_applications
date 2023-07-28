# Multicast On/Off Lights with Switch
![Type badge](https://img.shields.io/badge/Type-Virtual%20application-green)
![Technology badge](https://img.shields.io/badge/Technology-Zigbee-green)
![License badge](https://img.shields.io/badge/License-Zlib-green)
![SDK badge](https://img.shields.io/badge/SDK-v4.1.0-green)

## 1. Summary
The following article will demonstrate how to create a multicast command to send to a specific
groupId. It is sending ON/Off command to lights present in the network.
It also provides files to create Network (for Coordinator), joining, and create groups.

## 2. Gecko SDK version
Gecko SDK Suite 4.1.0 or later
## 3. Hardware Required
* 3x Wireless Starter Kit Main Board
* 3x BRD4180A - 1 switch & 2 lights
## 4. Connections Required
Connect the radio boards to the WSTK mainboards. Connect your desired gateway device via serial connection to a computer.

## 5. Running the Applications
Build and flash the multicastGroup_CO application to your board (switch).
Build and flash the multicastGroup_RO1 to the 2 other boards (lights).

## Using the Application
* Form the network on the coordinator (switch) by sending **form** through the serial.
* If you want to capture packets, use **keys print** to get the NWK key and add it to the keys.
* Use **open** to Open the network
* Send **join** to the 2 lights to allow them to join the network.

Now, our network has been initialized, we can start to create the group, the group used is :
GROUP_ID = 0x4123  & Group_Name = "Lights"

* Now, send **group** on the coordinator to send a command to the 2 other devices which joined.
* Then, you can now press the BTN0 on the switch to change the state of the LED 0.

## 6. Traces & other documents
This directory also contains trace captures on Network Analyzer and Wireshark directly in the repositories
* multicast_process_trace.isd : Capture for NA
* multicast_process_trace.pcapng : Capture for Wireshark

**To use Wireshark Capture**
You need to add the well-known key to your wireshark keys to decode packets.
Go to : [Edit -> Preferences -> Protocols -> ZigBee -> Edit] and add :
**5A:69:67:42:65:65:41:6C:6C:69:61:6E:63:65:30:39** as the well-known key
