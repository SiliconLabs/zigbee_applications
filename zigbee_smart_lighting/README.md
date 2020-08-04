# Zigbee Motion Sensor PIR Example #

## Summary ##

This project shows the implementation of PIR sensor with Zigbee. The PIR sensor on the occupancy sensor EXP board enables the internal ADC of ZGM130S to take periodic measurements. CRYOTIMER is set to signal the ADC using PRS. The Op-Amp is configured to use the external one on the board. A simple motion detection algorithm is implemented to post-process ADC samples. Whenever certain motion of the human body is detected, the system will either turn on the light or the alarm. The setup will have at least 2 nodes, 1 for motion detection, 1 for light control. A Zigbee gateway may be involved.

![zigbee](doc/Model.png)

In figure above, node 1 is the Zigbee Gateway. Node 2 is the motion sensor. Node 3 is the light. Upon motion detection, node 2 will notify the gateway and turn on the light.

The figure below illustrates the working flow of this demo.

![zigbee](doc/Flow_Steps.png)

## Gecko SDK version ##

v2.7.3

## Hardware Required ##

For the sensor node (Node 2):

- BRD4001A WSTK board
- BRD4162A Radio board with EFR32MG12
- BRD8030A Occupancy sensor EXP board

For the light node (Node 3):

- BRD4001A WSTK board
- BRD4162A Radio board with EFR32MG12

## Setup ##

On the sensor node: Connect the occupancy sensor EXP board to the WSTK board through the expansion header. Then, you should program the EFR32MG12 with the Zigbee_SmartLight_Switch_ZR.sls project.

On the light node: Program the EFR32MG12 with Zigbee_SmartLight_Light_ZC.sls project.

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

- Zigbee_SmartLight_Light_ZC.sls
- Zigbee_SmartLight_Sensor_ZR.sls

## Special Notes ##

The implemention of PIR sensor driver on this demo bases on two bellow links:

- Z-Wave Motion Sensor PIR Example:
<https://github.com/SiliconLabs/z_wave_applications_staging/tree/master/z_wave_motion_sensor_pir_application>

- Silicon Labs Occupancy Sensor EXP:
<https://github.com/SiliconLabs/occupancy-sensor-exp.git>
