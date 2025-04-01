********
![zigbee](files/zigbee.png)
********

# Zigbee Boot Camp

We have prepared a series of hands-on modules to implement the Zigbee protocol and familiarize you with Simplicity Studio v5. You should complete the following modules:

| Time | Content | Description |
|:---- |:---- |:---- |    
| 1 hour | [Hands on : Forming and Joining](Zigbee-Hands-on-Forming-and-Joining) | Form a basic centralized Zigbee network using a zigbee minimal sample application |
| 1 hour | [Hands on : Sending on/off commands](Zigbee-Hands-on-Sending-OnOff-Commands) | Use a Zigbee ED to control LED status with ZCL commands |
| 1 hour | [Hands on : Using Event](Zigbee-Hands-on-Using-Event) | Execute custom code to control LED blink |
| 1 hour | [Hands on : Non-volatile Data Storage](Zigbee-Hands-on-Non-volatile-Data-Storage) | Implement custom tokens |
*************

Each module builds on the former module so we strongly recommend completing these in order. Before starting the modules, make sure you have Simplicity Studio v5 installed and configured. 

NOTE: QSG-180 is designed for developers who are new to Zigbee EmberZNet and the Silicon Labs development hardware. It provides basic information on configuring, building, and installing applications for the EFR32MG family of SoCs. Completing QSG-180 prior to the Zigbee Boot Camp is recommended.


## Summary

The following boot camp is focused on providing a hands-on experience on the development of IoT technologies with the Silicon Labs EmberZNet Stack and Simplicity Studio IDE. We will give a brief introduction of Zigbee and then you can expect to work through a series of workshops to develop your own Zigbee network. You will also have the ability to reference other Silicon Labs documentation for more detailed learning.

By the end of the hands-on tutorials you will know how to use the Simplicity Studio IDE to build, customize, and implement applications that utilize Zigbee. 

## SDK version

This bootcamp was created for SiSDK v2024.12.1. 

## Hardware Required

- 2 WPK/WSTK + 2 EFR32MG24 (BRD4187C)

## Connections Required

Connection is required from the Development PC to 2 Radio Boards. This can be done via USB or Ethernet connection.  

## Setup 

The following sections will go over how to configure the 2 Radio Boards into a Zigbee Light and a Zigbee Switch. 

## How It Works

The following sections will also go over how to run the 2 Radio Boards into a Zigbee Light and a Zigbee Switch. 

## Reference
- [Zigbee Onboarding Roadmap][Zigbee Onboarding Roadmap]
- [QSG-180: Quick Start Guide using SSv5](https://www.silabs.com/documents/public/quick-start-guides/qsg180-zigbee-emberznet-7x-quick-start-guide.pdf)
- [UG103-01 Fundamentals: Wireless Network](https://www.silabs.com/documents/public/user-guides/ug103-01-fundamentals-wireless-network.pdf)
- [UG103-02 Fundamentals: Zigbee](https://www.silabs.com/documents/public/user-guides/ug103-02-fundamentals-zigbee.pdf)
- [AN0822 Simplicity Studio User Guide](https://docs.silabs.com/simplicity-studio-5-users-guide/latest/ss-5-users-guide-overview/)
- [Silicon Labs: Zigbee - Application Framework API Reference Documentation](https://docs.silabs.com/zigbee/latest/)

[Zigbee Onboarding Roadmap]: files/Silicon-Labs-ZigBee-Onboarding-Roadmap.pdf
