# Joining Policies : With Install-Code
![Type badge](https://img.shields.io/badge/Type-Virtual%20application-green)
![Technology badge](https://img.shields.io/badge/Technology-Zigbee-green)
![License badge](https://img.shields.io/badge/License-Zlib-green)
![SDK badge](https://img.shields.io/badge/SDK-v4.1.0-green)

## 1. Summary
This Project has the goal to demonstrate packet exchanges when a device join a network based on install-code.
Each device has a token in FLASH memory which contains an install-code. From this install-code, we obtain a derived link key by AES-MMO algorithm.
This key is then added manually (not in the air) to the transient link key table of TC.
When a device is joining, the TC is then sending the NWK key encrypted with this derived link key. It is then unnecessary to store this link key.

## 2. Gecko SDK version
Gecko SDK Suite 4.1.0 or later
## 3. Hardware Required
* 3x Wireless Starter Kit Main Board
* 3x BRD4180A
## 4. Connections Required
Connect the radio boards to the WSTK mainboards. Connect your desired gateway device via serial connection to a computer.

## 5. Running the Applications
Build and flash the JoinWithCode_CO application to one board (coordinator).
Build and flash the JoinWithCode_Ro application to another one (Router).
Build and flash the JoinWithCode_SED application to the last one (SED).
## Preparing the Boards
You MUST SET a custom Install-code with the program : program_install_code.bat
To use it : You can choose your install-code by changing DEFAULT_INSTALL_CODE

You need to use this program for both the SED and Router board.
Please, Make sure to have only one board connected when executing the script.
## Using the Application
* Form the network on the coordinator by sending **form** through the serial.
* If you want to capture packets, use **keys print** to get the NWK key and add it to your keys.
* Use **open {Index Transient Entry} {EUI64 of Joining Node} {INSTALL CODE + CRC}** to Open the network
* Send **join** to both the Router and SED to allow them to join the network

## 6. Traces & other documents
This directory also contains trace captures on Network Analyzer and Wireshark directly in the repositories
* trace_joining_router_sed.isd : Capture for NA
* trace_joining_router_sed.pcapng : Capture for Wireshark

**To use Wireshark Capture**
You need to add the derived link key of the install-code and the well-known key to your wireshark keys to decode packets.
Go to : [Edit -> Preferences -> Protocols -> ZigBee -> Edit] and add :
**5A:69:67:42:65:65:41:6C:6C:69:61:6E:63:65:30:39** as the well-known key
**66:B6:90:09:81:E1:EE:3C:A4:20:6B:6B:86:1C:02:BB** as the derived key

To get more informations : [AN1089: Using Installation Codes with Zigbee Devices](https://www.silabs.com/documents/public/application-notes/an1089-using-installation-codes-with-zigbee-devices.pdf)
