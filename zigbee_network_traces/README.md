# Basic Network Policies captures Project #
![Type badge](https://img.shields.io/badge/Type-Virtual%20application-green)
![Technology badge](https://img.shields.io/badge/Technology-Zigbee-green)
![License badge](https://img.shields.io/badge/License-Zlib-green)
![SDK badge](https://img.shields.io/badge/SDK-v4.1.0-green)
 
## Summary ##
This repository provides projects and network traces about basic network Policies on Zigbee. 

## List of Projects ##
**Joining Policies** : 
* Joining_InstallCode : To join a network with install-code
* Joining_WellKnownKey : Join with the well-known key "ZigbeeAlliance09"

**Rejoining Policies** :
* Secured_Rejoining : Secured Rejoining of a SED aged out of the child table of the parent
* **UN**Secured_Rejoining : Unsecured rejoining of a SED which missed a NWK Key update

**TC Policies** :
* TC_GlobalLinkKey : Use a global TC link key for all nodes 
* TC_HashedLinkKey : Use a global TC Link key which is hashed by the eui64 of each node
* TC_UniqueLinkKey : Use a random unique TC link key for each node joining the network

**NWK Key Update** :
* Network_KeyUpdate : Demonstrate the network update with broadcast

**App Link Keys**
* appLinkKey : Use a unique APP Link key to secure communication between 2 nodes on a network

**Messaging** :
* Multicast_SwitchLight : Project to send multicast message to a specific Group

## Structure of Projects ##
You can see in all the projects the following structures : 
* Src : Source code files used for the project
* SimplicityStudio : Export files for all the projects
* Traces directly into the directory
* README : Explains how to use the project

## Gecko SDK version ##

v4.1

## Hardware Required ##

* 3x Wireless Starter Kit Main Board 
* 3x BRD4180A

## Connections Required ##

Connect the radio boards to the WSTK mainboards.

## Setup ##

Look at each guidelines present in each projects in this repository.


## How It Works ##

Some Wireshark/Network Analyzer traces are directly available in order to explore the different packet exchanges 

## .sls Projects Used ##

Voir dans chacun des sous-projets