# Optimization on EM2 Current Consumption of the Sleepy Z3Swtich Example Project #

## Summary ##

A concrete guide to build a sleepy end device from the Z3Switch example project on radio board EFR32MG12 is provided in this article. Sample test results on EM2 current and essential troubleshooting approaches are included.

## Gecko SDK version ##

Gecko SDK Suite 3.0.

## Hardware Required ##

* Wireless Starter Kit Main Board (BRD4001)
* EFR32xG22 2.4GHz 10 dBm Radio Board (BRD4162A Rev A01)

## Connections Required ##

NA

## Setup ##

### Setup a Sleepy Z3Switch sample based on the BRD4162A  
1. Create a Z3Switch example project for BRD4162A.
2. Change the device type to sleepy end device.
3. Enable the EEPROM POWERDOWN plugin to power down SPI Flash. 
4. Select "Local storage" under bootloader configuration, otherwise the code used to power down SPI Flash will not take effect.  
![zigbee](doc/change_bootloader_config.png)  
5. Disable the uart print funtion
    1. Set "Port for application serial communication" to "None", and disable USART0.  
    ![zigbee](doc/disable_serial_comm.png)  
    2. Disable "Enable Command Line For Legacy CLI" in the ZCL Framework Core plugin.  
    3. Un-check the "Enable debug printing" checkbox and change the "Use Command Set" option to "None".  
    ![zigbee](doc/disable_debug_printing.png)  
6. Peripherals configuration
    1. Configurate CMU, choose "LFXO" as the LFA/B/E clock source.  
    ![zigbee](doc/change_cmu_clock_sources.png)
    2. Enable DCDC, set the "Bypass DCDC" option to "False".  
    3. Enable EMU and set the "EM2/3 voltage scaling level" option to "Low Power".
    4. Enable SPI Flash, otherwise compile errors after enabling the EEPROM POWERDOWN plugin are expected to occur.
    5. Disable all of the unused peripherals.  
    ![zigbee](doc/enable_emu.png)  
7. Enable the "Main Init" callback in the callbacks tab, add function call "EMU_PeripheralRetention(emuPeripheralRetention_ALL, false)" in order to enable "allow power down of the peripherals during EM2".  
For more information about the disable of EM23 Peripheral Retention, please refer to Chapter 10.3.10 from the EFR32MG12 reference manual.  
```
#include "em_emu.h"
```
```
void emberAfMainInitCallback(void)
{
  EMU_PeripheralRetention(emuPeripheralRetention_ALL, false);
}
```
8. Generate and build the project.  

## How It Works ##

Energy Profiler is used to implement the EM2 current test. In accordance with "AEM Accuracy and Performance" section from [UG172](https://www.silabs.com/documents/public/user-guides/ug172-brd4320a-user-guide.pdf), when measuring currents below 250 uA, the accuracy is 1 uA. For more precise results, it is necessary to measure the current using a high-accuracy DC analyzer.  
Before current measurement, it is recommended to let the switch join a centralized network and pair with a light, further more, use command "aem calibrate" to run AEM calibration first.  
![zigbee](doc/aem_calibrate.png)  
The screenshot below contains an event that an ON command was sent to the light. Currently the EM2 current of the switch is about 2.54 uA.  
![zigbee](doc/current_measurement_result.png)  

## .sls Projects Used ##

* zigbee_sed_z3switch_mg12.sls

## How to Port to Another Part ##

* Import the .sls file into Simplicity Studio
* Open the .isc file of each project, turn to "General" tab, hit button "Edit Architecture", then select the board and part.

## Special Notes ##

1. Thanks to a technical bug of Simplicity Studio version 5.0.0.0, the current measured may stay at about 90 uA. In such situation, a re-flash of the .s37 firmware before a reset is needed to recover. This issue is expected to be fixed upon future releases, please refer to UID 519744 in future release notes.  
2. The EM2 current may stay at about 3 mA after flashing the firmware, keep capturing and slide the power source to BAT then back to AEM on the bottom left of the main board.   

