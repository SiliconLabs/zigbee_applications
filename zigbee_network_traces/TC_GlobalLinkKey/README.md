# TC Policies : Unique Global Link Key
![Type badge](https://img.shields.io/badge/Type-Virtual%20application-green)
![Technology badge](https://img.shields.io/badge/Technology-Zigbee-green)
![License badge](https://img.shields.io/badge/License-Zlib-green)
![SDK badge](https://img.shields.io/badge/SDK-v4.1.0-green)

## 1. Summary
When joining a network and to be allowed to communicate end-to-end with the trust center, each node requires a TC Link key. This link key can be unique, hashed link key based on a global key which is hashed by the ID of the corresponding node, or the same link key for every node which means the "end-to-end" communication can be decrypted by every node of the network.

## 2. Gecko SDK version
Gecko SDK Suite 4.1.0 or later
## 3. Hardware Required
* 3x Wireless Starter Kit Main Board 
* 3x BRD4180A
## 4. Connections Required
Connect the radio boards to the WSTK mainboards. Connect your desired gateway device via serial connection to a computer. 

## 5. SDK Modification
In order to achieve what we are doing, it is **IMPORTANT** to change a function in the network-creator-security which enables the 
hashed link keys. You can find in "network-creator-security.c" in the SDK, in the function emberAfPluginNetworkCreatorSecurityStart()
```
// Use hashed link keys for improved storage and speed.
state.bitmask |= EMBER_TRUST_CENTER_USES_HASHED_LINK_KEY;
```
This is the line you are supposed to comment in order to disable the use of Hashed Link Keys.

## 6. Running the Applications
Build and flash the globalTCLink_CO application to one board (coordinator).
Build and flash the globalTCLink_Ro application to another one (Router).
Build and flash the globalTCLink_SED application to the last one (SED).

## Using the Application
* Form the network on the coordinator by sending **form** through the serial.
* If you want to capture packets, use **keys print** to get the NWK key and add it to your keys.
* Use **open** to Open the network
* Send **join** to both the Router and SED to allow them to join the network

## Interpretation
It is possible to see in the traces that the same link key is send to both the Router and SED.

**For the router** :
![alt text](doc/RouterLinkKeyPacket.PNG "Router Transport Link Key")

**For the SED** : 
![alt text](doc/SEDLinkKeyPacket.PNG "SED Transport Link Key")

It's also possible to check that the 2 TC Link keys are the same by printing them with **keys print** on both the Router and SED.
## 7. Traces & other documents
This directory also contains trace captures on Network Analyzer and Wireshark directly in the repositories
* trace_global_link_key.isd : Capture for NA
* trace_global_link_key.pcapng : Capture for Wireshark   

**To use Wireshark Capture**
You need to add the well-known key to your wireshark keys to decode packets.\
Go to : [Edit -> Preferences -> Protocols -> ZigBee -> Edit] and add :
**5A:69:67:42:65:65:41:6C:6C:69:61:6E:63:65:30:39** as the well-known key\

To get more informations : [AN1233: Zigbee Security](https://www.silabs.com/documents/public/application-notes/an1233-zigbee-security.pdf)