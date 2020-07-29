# Zigbee Motion Sensor PIR Example #

## Summary ##

This project demostrates the smart lighting or alarm systems in home automation. Whenever certain motion of the human body is detected, the system will either turn on the light or the alarm. The setup will have at least 2 nodes, 1 for motion detection, 1 for light control. A Zigbee gateway may be involved.


## Gecko SDK version ##

v2.7.3

## Hardware Required ##

For the sensor device:

- BRD4001A WSTK board
- BRD4162A Radio board with EFR32MG12
- BRD8030A Occupancy sensor EXP board

For the light device:

- BRD4001A WSTK board
- BRD4162A Radio board with EFR32MG12

## Setup ##

On the sensor device: Connect the occupancy sensor EXP board to the WSTK board through the expansion header. Then, you should program the EFR32MG12 with the Zigbee_SmartLight_ZR.sls project.

On the light device: Create a light application following this guide line: https://github.com/SiliconLabs/Zigbee-Boot-Camp/wiki/Zigbee-Hands-on-Forming-and-Joining.

## How It Works ##

The example application provides a simple UI that depicts the state of the device and offers basic user control. This UI is implemented via the general-purpose LEDs, LCD and buttons built in to the EFR32 WSTK development board.

On the sensor device:

LED #0 shows the overall state of the device and its connectivity. Four states are depicted:
- Off: Not joined to a Zigbee Network.
- Rapid Even Flashing (100ms on/100ms off): Device joining the Network.
- Solid On: Device connected to the Zigbee Network.

LED D1 on Occupancy Sensor Kit shows the state of motion detected. Two states are depicted:
- Solid On: The motion is detected.
- Off: The motion isn’t detected or the sensor is disabled.

Button PB0 can be used to change the state of the motion sensor. The button behaves as a toggle, swapping the state every time it is pressed.
Built-in LCD is used to displayed current state of the motion sensor is enabled or disabled.

On the light device:

LED #0 shows the overall state of the device and its connectivity. Four states are depicted:

- TBD.

LED #1 is used as a light. The state of light is depended on state of motion sensor on the sensor device.

## .sls Projects Used ##

Zigbee_SmartLight_Switch_ZR.sls

## Special Notes ##

N/A
