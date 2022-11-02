# ZigBee Virtual UART example #

![Type badge](https://img.shields.io/badge/dynamic/json?url=https://raw.githubusercontent.com/SiliconLabs/application_examples_ci/master/zigbee_applications/zigbee_virtual_uart_common.json&label=Type&query=type&color=green)
![Technology badge](https://img.shields.io/badge/dynamic/json?url=https://raw.githubusercontent.com/SiliconLabs/application_examples_ci/master/zigbee_applications/zigbee_virtual_uart_common.json&label=Technology&query=technology&color=green)
![License badge](https://img.shields.io/badge/dynamic/json?url=https://raw.githubusercontent.com/SiliconLabs/application_examples_ci/master/zigbee_applications/zigbee_virtual_uart_common.json&label=License&query=license&color=green)
![Build badge](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/SiliconLabs/application_examples_ci/master/zigbee_applications/zigbee_virtual_uart_build_status.json)
![SDK badge](https://img.shields.io/badge/dynamic/json?url=https://raw.githubusercontent.com/SiliconLabs/application_examples_ci/master/zigbee_applications/zigbee_virtual_uart_common.json&label=SDK&query=sdk&color=green)

## 1. Summary ##

The Virtual UART(VUART) interface provides a high-performance application data interface that does not require any additional I/O pins apart from the debug interface. It is based on SEGGER's Real-Time Transfer (RTT) technology and uses Serial Wire Output (SWO) to get application data from the device, and a shared memory interface to send data to the target application. The Wireless Starter Kit makes the VUART interface available on TCP/IP port 4900.

VUART, also called “two-way debug”, allows normal serial APIs to still be used on the port being used by the debug channel (SerialWire interface) for debug input and output.

EmberZNet supports VUART functionality through the Debug Basic Library plugin and the Debug JTAG plugin. For the EFR32, VUART functionality not only requires the plugins configured as in the previous section but also some configuration through the Hardware Configurator. In the GPIO peripheral, the SerialWire Output Pin must be enabled (can also be configured through the Debug JTAG plugin interface). The Serial VUART peripheral must be enabled and the VUART type must be set to VUART via SWO.

This example demonstrates how to create a ZigBee project that uses VUART for Command Line Interface and Debug Print.

## 2. Gecko SDK version ##

Gecko SDK Suite 2.7

## 3. Hardware Required ##

- Wireless Starter Kit Main Board (BRD4001) with adapter firmware version 1.4.4 and later
- EFR32MG12 2.4 GHz 19 dBm Radio Board (BRD4161A)

## 4. Connections Required ##

Connect the BRD4161A radio board to the BRD4001 WSTK mainboard.

Connect the BRD4001 WSTK mainboard to the PC through the USB or the Ethernet Interface.

## 5. Setup ##

### 5.1 Create ZigBee Virtual UART application ###

- Create an EmberZNet ZigBee project using Z3Light as the template. In this example, Z3LightSoc_VUART is used as the project name.
- In the Serial plugin, change the value of "Port for application serial communication" to "VUART" and generate the project
- In the hardware configurator, change the "VUART type" in the "Virtual UART" tab to "VUART via SWO" and save the hardware configurator file.
- For EmberZNet SDK version 6.7.0 - 6.7.5, there is a SEGER_RTT alignment issue breaking the VUART input through SEGGER's RTT. To fix this issue, a workaround is adding `#define SEGGER_RTT_ALIGNMENT 1024` in file `~\gecko_sdk_suite\v2.7\util\third_party\segger\systemview\Config\SEGGER_RTT_Conf.h`. This step can be skipped if EmberZNet SDK version 6.7.0 - 6.7.5 is not used.
- Build the project
- Load the project into the target board

A pre-built Z3LightSoc_VUART.sls project for brd4161 is provided under the SimplicityStudio directory.

### 5.2 Set up the VUART connection through the Ethernet port ###

- Set the IP address of the WSTK main board. Refer to [UG162: Simplicity Commander Reference Guide](https://www.silabs.com/documents/public/user-guides/ug162-simplicity-commander-reference-guide.pdf) for more information
- Configure the Ethernet adapter in the computer to be able to detect the WSTK through the Ethernet interface. Refer to https://wiki.segger.com/Setting_up_Ethernet_interface for more information
- Connect the WSTK to the PC via Ethernet interface
- In Simplicity Studio → Window → Preference → Simplicity Studio → Device Manager, enable all the locators. In Device Manager → TCP/IP Adapters, enable all the selections
- There should be a new debug adapter showing with the IP address configured in the first step. Use this adapter to launch console
- Use serial 0 panel to communicate with the Radio Board through VUART

### 5.2 Set up the VUART connection through the USB ###

- Use the silink.exe to map the VUART port to TCP/IP port 4900. For example, in the Windows Command Prompt, use command `C:\SiliconLabs\SimplicityStudio\v4\developer\adapter_packs\silink\silink.exe -sn 000440031200 -automap 4900 -trace=false -polldelay=5000`. 000440031200 is the serial number of the WSTK mainboard which appears on the WSTK display
- Use a terminal program(for example, YAT) to connect to TCP/IP port 4900 to communicate with the Radio Board through VUART
- **Or** use serial 0 in Simplicity Studio and the two steps above can be skipped

## 6. How It Works ##

When VUART support is enabled, the serial output sent to serial port 0(TCP/IP port 4900) is encapsulated in the debug channel protocol and sent bi-directionally via the SWO and SWDIO lines through the debug adapter(WSTK). The raw serial output will be displayed by Simplicity Studio’s Device Console tool, and will also appear on TCP/IP port 4900 of the debug adapter. Similarly, data sent to port 4900 of the adapter will be encapsulated in the debug channel protocol and sent to the node. The raw input data can then also be read using normal serial APIs.

VUART provides an additional port for the output with debug builds that would otherwise not be available.

## 7. .sls Projects Used ##

Project | Comment
-|-|
z3lightsoc_vuart_mg12.sls | ZigBee application for BRD4161 supporting VUART

## 8. How to Port to Another Part ##

- Follow the steps in section 5.1 to create a ZigBee project with the desired board and part selected in the Appbuilder

## 9. Special Notes ##

Virtual UART is different from the virtual COM port support, known as “VCOM”, provided in the EFR32 Board Support Package (BSP) via com_device.h. VCOM routes the physical serial port connections of USART0 back through the WSTK’s onboard TTL-to-USB converter for use as a communications port by a USB host. VCOM and VUART are independent of one another and can be enabled separately, together, or not at all.

The following behaviors for VUART differ from normal serial UART behavior:

- emberSerialWaitSend() does not wait for data to finish transmitting
- emberSerialGuaranteedPrintf() is not guaranteed
- EMBER_SERIALn_BLOCKING might not block

More serial output might be dropped than normal depending on how busy the processor is with other stack functions.
