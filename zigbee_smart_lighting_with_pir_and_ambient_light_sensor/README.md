# Zigbee Smart Lighting with PIR and Ambient light sensor #

## 1. Introduction ##

In this demo, the light is controlled automatically by the PIR sensor and Ambient light sensor (using [PIR Occupancy board](https://www.silabs.com/development-tools/sensors/occupancy-sensor-kit)). If there is a motion and the ambient light is dark, the light will be turned on. The light will be turned off if there is no motion or the ambient light is bright.

This document also shows how to **integrate Motion sensor (PIR) driver and Ambient light sensor (ALS) driver into Zigbee application** with EmberZNet stack.

The **PIR sensor** is read by **using ADC** to get data from sensor and run the **motion detect algorithm** to detect the motion. CRYOTIMER is set to trigger the ADC using PRS. The Op-Amp is configured to use the external one on the sensor board. A simple motion detection algorithm is implemented to post-process ADC samples. After run the algorithm, PIR driver will return motion status to application by running **callback function**.

The **Ambient light sensor** is read by **using I2C** to get data from **Si115x sensor** then **calculate the lux value**. Reading process of this sensor consists of **two phases**. The **first phase** is **sending the start command** to sensor over I2C, then wait for the sampling and processing in hardware. When reading process in hardware is completed, a trigger will send back from the sensor over sensor interrupt pin. When receive this signal by using **external interrupt**, the second phase will be start. The **second phase** is **reading data** from the sensor over I2C and **calculate lux value**. This process is configured to run every 1 second. Same with the PIR driver, after get lux value, ALS driver will return it to application by running **callback function**.

The figure below shows the operation of this demo application. The **Coordinator node** will **form a network** and **open it** to allow light node and sensor node to **join**. When a motion is detected, the **sensor node** will **update** occupancy attribute and ambient light attribute, then **report** them to coordinator node. And if there is a motion and the ambient light value is less than a threshold value at the same time, the LED in the light node will be turned on.

![zigbee](doc/model.png)

This document includes two part:

- First part is how to **setup and run** this demo
- Second part is how to **integrate the PIR sensor driver and ALS driver with Zigbee network**.

## 2. Gecko SDK version ##

- Gecko SDK Suite 3.0.0

## 3. Hardware Required ##

For the occupancy sensor node:

- BRD4001A WSTK board
- BRD4162A Radio board with EFR32MG12
- BRD8030A Occupancy sensor EXP board

For the coordinator node:

- BRD4001A WSTK board
- BRD4162A Radio board with EFR32MG12

For the light node:

- BRD4001A WSTK board
- BRD4162A Radio board with EFR32MG12

## 4. Setup and how it works ##

The figure below shows how to setup this demo and how it works.

![work flow](doc/work_flow.png)

### 4.1. Setup ###

#### 4.1.1 Import, build and flash ####

1. Connect the occupancy sensor EXP board to the WSTK board through the expansion header  
![connect hardware](doc/sensor_board.png)

2. Import project **zigbee_sensor_node.sls** to your Simplicity Studio.  

    Select **File** -> **Import**
![import project](doc/import_1.png)  

    Link to folder include project **.sls** file. Write the project name and click **Finish**.  
![import finish](doc/import_2.png)  

    Build project: Right click in your project -> **Build Project**.  
![build](doc/build.png)  

    Download the binary file to sensor board. Right click to the **.hex** file -> **Flash to Device ...**  
![flash](doc/flash.png)  

3. Import project **zigbee_coordinator_node.sls**, build and download the binary file to coordinator board same as Sensor board.

4. Import project **zigbee_light_node.sls**, build and download the binary file to light board same as Sensor board.

#### 4.1.2. Run and debug ####

After flash binary file to three nodes:

1. Open three console.  
In Debug Adapter window, right click to your board, choose **Lauch Console...**  
![lauch console](doc/lauch_console.png)  
The console will appear like this. Select **Serial 1** Tab.  
![lauch console](doc/lauch_console_2.png)

2. Push button PB0 in Coordinator board.  
The coordinator will start first, it forms a network and open it to allow another device join the network.  
After that, it will start find and bind target.  
You can see the information in debug console.  
![coordinator](doc/run_step_1_coordinator_start.png)

3. When the network open success, push button PB0 in Light node.
It will join to coordinator's network.
After that, it will start find and bind target.  
You can see the information in debug console.  
![coordinator](doc/run_step_2_light_start.png)

4. Push button PB0 in Sensor node.  
Sensor node will start final because it is an initiator. See Cluster section in [Introduction of Zigbee Basic](https://github.com/SiliconLabs/IoT-Developer-Boot-Camp/wiki/Introduction-of-Zigbee-Basic)  
After join to coordinator's network, it will start find and bind initiator.  
You can see the information in debug console.  
![coordinator](doc/run_step_3_sensor_start.png)

5. From now they will keep running automatically. When the sensor detected a motion, it will get the ambient light value, compare with threshold value, control the light and update to the coordinator.  
You can see the information in debug console.  
![coordinator](doc/debug.png)

### 4.2. How it's work ###

- To form a network, the Coordinator use function `EmberStatus emberAfPluginNetworkCreatorStart(bool centralizedNetwork)` in plugin **network-creator**.

    ```C
    void emberAfHalButtonIsrCallback(uint8_t button, uint8_t state)
    {
        if (state == BUTTON_RELEASED) {
            if(!appStart){
                appStart = true;
                emberEventControlSetActive(appStartEventControl);
            }
        }
    }
    ```

    ```C
    void appStartEventHandler()
    {
        emberEventControlSetInactive(appStartEventControl);
        emberAfCorePrintln("Coordinator start \n");

        EmberStatus status = emberAfPluginNetworkCreatorStart(true);
        emberAfCorePrintln("%p network %p: 0x%X", "Form", "start", status);
    }
    ```

    The function `appStartEventHandler()` is handler for event `appStartEventControl`. It been set in `emberAfHalButtonIsrCallback()` function. The function `emberAfHalButtonIsrCallback()` is called by the framework when you push a button PB0 or PB1.

- To open a network, the Coordinator use function `EmberStatus emberAfPluginNetworkCreatorSecurityOpenNetwork(void)` in plugin network-creator-security.

    ```C
    void reopenNetworkEventHandler()
    {
        emberEventControlSetInactive(reopenNetworkEventControl);

        EmberStatus status =  emberAfPluginNetworkCreatorSecurityOpenNetwork();
        emberAfCorePrintln("Open network: 0x%X \n", status);

        emberEventControlSetDelayMS(reopenNetworkEventControl, NETWORK_CLOSE_TIMEOUT); // reopen after about 250s
    }
    ```

    Note that the network will automatic close after about 250s according to your configuration. So that after that time to allow another device join network, you need to add schedule event to re-open the network.  
    So you will see the handler `reopenNetworkEventHandler()` set active it's event `reopenNetworkEventControl` by itself. See this in [Zigbee Hands on Using Event](https://github.com/SiliconLabs/IoT-Developer-Boot-Camp/wiki/Zigbee-Hands-on-Using-Event).  

- To join network, light node and sensor node use function `EmberStatus emberAfPluginNetworkSteeringStart(void)` in plugin **network-steering**.  

    ```C
    void appStartEventHandler()
    {
        emberEventControlSetInactive(appStartEventControl);
        emberAfCorePrintln("Sensor node start \n");

        appStart = true;

        EmberStatus status = emberAfPluginNetworkSteeringStart();
        emberAfCorePrintln("Steering network: 0x%X \n", status);
    }
    ```

    When you push the button in the light node, the process is same with the coordinator node. But with the sensor it there is a bit different.

    In the sensor node, the Ambient light sensor using pin PF7 as sensor interrupt, so that the button PB7 cannot be use with button plugin.

    The event `appStartEventControl` is set in `void button_irq_handler(uint8_t pin)`.

    ```C
    void button_irq_handler(uint8_t pin)
    {
        /* if btn0 press */
        if (pin == BSP_GPIO_PB0_PIN) {
            if(!appStart){
                appStart = true;
                emberEventControlSetActive(appStartEventControl);
            }
        }
    }
    ```

    And the `button_irq_handler(uint8_t pin)` is register to **gpiointerrupt** driver in `emberAfMainInitCallback(void)`

    ```C
    GPIOINT_CallbackRegister(BSP_GPIO_PB0_PIN, button_irq_handler);
    ```

- Finding and binding.  
The concept can find in [Introduction of Zigbee Basic](https://github.com/SiliconLabs/IoT-Developer-Boot-Camp/wiki/Introduction-of-Zigbee-Basic).

    To start find and bind target, the coordinator node and light node use function `EmberAfStatus emberAfPluginFindAndBindTargetStart(uint8_t endpoint)` in plugin **find-and-bind-target**.

    To start find and bind initiator, the sensor node use function `EmberAfStatus emberAfPluginFindAndBindTargetStart(uint8_t endpoint)` in plugin **find-and-bind-initiator**.

    The find and bind can only start after the node join to a network.

- Usage of the sensor driver:

    Both two sensor need to be initialized by call the initial function. These function is called when the find and bind complete.  

    ```C
    void emberAfPluginFindAndBindInitiatorCompleteCallback(EmberStatus status)
    {
        emberAfCorePrintln("Find and bind initiator %p: 0x%X", "complete", status);
        if(status == EMBER_SUCCESS){
            findAndBindSuccsess = true;
            emberEventControlSetActive(initSensorEventControl);
        }
    }
    ```

    ```C
    void initSensorEventHandler()
    {
        emberEventControlSetInactive(initSensorEventControl);
        emberAfCorePrintln("Init ambient light sensor \n");
        als_init(&get_lux_completed_callback);

        emberAfCorePrintln("Init pir sensor \n");
        pir_init(&motion_detection_callback);
    }
    ```

    `motion_detection_callback` is callback function after PIR driver run motion detection algorithm.  

    ```C
    void motion_detection_callback(bool motion)
    {
        if(motion){
            if(findAndBindSuccsess){
                emberEventControlSetActive(motionDetectedEventControl);
            }
        }
    }
    ```

    `get_lux_completed_callback` is callback function after ALS driver get lux value. `lux_value` is static variable in application.  

    ```C
    void get_lux_completed_callback(uint16_t enter_lux_value)
    {
        lux_value = enter_lux_value;
    }
    ```

## 5. How to integrate the PIR sensor driver and ambient light sensor driver into EmberZnet stack ##

The base driver is on [Silicon Labs Occupancy Sensor EXP](https://github.com/SiliconLabs/occupancy-sensor-exp.git). The structure of this driver need to refactor to compatible with Zigbee stack.  

In this demo, we will start with Zigbee minimal project because it provides a minimal functional subset to work with ZigBee 3.0 network and will be fast to edit.

The steps are:

- Create Zigbee minimal project.  
- Import base driver and necessary files.  
- Configure project: ZCL, plugins, callbacks, Events.  
- Refactor base driver.  

### 5.1. Create Zigbee Minimal project ###

You can see in [Zigbee Hands on Forming and Joining](https://github.com/SiliconLabs/IoT-Developer-Boot-Camp/wiki/Zigbee-Hands-on-Forming-and-Joining)

- Select **File** -> **New** -> **Project..** -> **Silicon Labs AppBuilder Project**  
![zigbee](doc/create_new_project_1.png)

- Select Slilicon Labs Zigbee and then choose the stack version for SoC, for this example is **EmberZNet 6.8.0.1 GA SoC 6.8.0.1**  
![zigbee](doc/create_new_project_2.png)

- Select **ZigbeeMinimal**  
![zigbee](doc/create_new_project_3.png)

- Write the Project's name and then choose the board, for this example is **BRD4162A REV A02** and then check the Configuration for project, for this example is **Default GNU ARM v7.2.1**. Finally click **Finish**  
![zigbee](doc/create_new_project_4.png)

### 5.2. Import file ###

Download and copy the source folder to your project folder from [Silicon Labs Occupancy Sensor EXP](https://github.com/SiliconLabs/occupancy-sensor-exp.git). Remove application files, just keep driver files and configure file.  

![zigbee](doc/import_files.png)  

This driver also uses some **emlib** libraries, you need to add them to your project.  
These libraries are: **em_letimer.c**, **em_opamp.c**, **em_pcnt.c**.

Right click in **emlib** folder in the project structure, select **New** -> **File**.  
![zigbee](doc/add_files_1.png)  

Select **Advance** -> tick to **Link to file in the file system** -> **Browse...**.  
Browse to SDK folder **C:\SiliconLabs\SimplicityStudio\v5\developer\sdks\gecko_sdk_suite\v3.0\platform\emlib\src** and select one by one each file: **em_letimer.c**, **em_opamp**, **em_pcnt.c**.  
![zigbee](doc/add_files_2.png)  

### 5.3. Configure Zigbee Minimal project ###

- ZCL tab  

    Sensor node need three Clusters are: On/Off cluster, Illuminance Measurement and Occupancy Sensing, so in this demo we custom a device type base on Home Automation devices:  

    In **ZCL device type**, select **Zigbee Custom**  
    ![zcl](doc/zcl_1.png)  

    In **Device template**, select **HA devices** -> **HA Occupancy Sensor**  
    ![zcl](doc/zcl_2.png)  

    In **Cluster table**, select **On/Off cluster** in **client side**, **Illuminance Measurement** and **Occupancy Sensing** both in **server side**  
    ![zcl](doc/zcl_3.png)  
    ![zcl](doc/zcl_4.png)  

- Zigbee Stack tab  

    In **Zigbee device type**, select **Router**  
    ![stack](doc/zigbee_stack.png)  

- Printing tab

    Enable debug printing.  
    ![printing](doc/printing_tab.png)  

- Plugins tab  

    Add plugin to your project by tick in the check box. These plugins below are used:  

    Under **Common Clusters** tick: **Basic Server Cluster**, **Identify Cluster** and **Reporting**.  

    Under **HAL** tick: **i2c-driver**.  

    Under **Stack libraries** tick: **Binding table library**, **Zigbee PRO library**.  

    Under **Utility** tick: **General Response Commands**, **mbedtls**.  

    Under **Zigbee 3.0** tick: **find-and-bind-initiator**, **network-steering** and **update-tc-link-key**.  

    Some plugins are explained in [Building a Zigbee 3.0 Switch and Light from Scratch](https://www.silabs.com/support/training/zigbee-application-layer-concepts/building-a-zigbee-3-0-switch-and-light-from-scratch)  

    The **Reporting** plugin is used for report sensor value to another node. And **i2c-driver** plugin is used for the ambient light sensor.  

- Callback tab  

    Use a framework callback by tick in check box and implement it in your source code. These callbacks below are used:

    Under **Non-cluster related** tick: **MainInit**.  
    The function `void emberAfMainInitCallback(void)` is called in start of `main()` function and it is called before the clusters, network and plugins are initialized so note that some functions are not available. It is usually used for initialize your project. In this demo it is used for initialize the button. Then when you push the button, button handler will run and initial network. When the network is up, the sensor is initialized.  

    Under **Plugin-specific callbacks** tick: **Find and Bind Initiator Complete** and **Network Steering Complete**.  
    The function `void emberAfPluginNetworkSteeringCompleteCallback(EmberStatus status, uint8_t totalBeacons, uint8_t joinAttempts, uint8_t finalState)` is called after network steering process is complete, it notify to your application that network steering process is successful or not.  

    The function `void emberAfPluginFindAndBindInitiatorCompleteCallback(EmberStatus status)` is called after find and bind initiator process is complete, it notify to your application that  find and bind initiator process is successful or not.  

- Include tab  

    In Event Configuration table, add events and the handlers for them. See at [Zigbee Hands on Using Event](https://github.com/SiliconLabs/IoT-Developer-Boot-Camp/wiki/Zigbee-Hands-on-Using-Event).  

    ![Events](doc/event_tab.png)  

    The `ambientLightSensorInterruptEventControl` and `ambientLightSensorReadDataEventControl` are used for Ambient light sensor and the others are used for application.  

### 5.4. The PIR sensor driver ###

- Add callback for the motion detection event.

Application can know when driver run the motion detection algorithm and know if there is a motion or not by the parameter of the callback function.  

The code below is added in `detect_motion()` function to run the callback.

```C
bool detect_motion()
{
  ...
  if (lock_out_counter > 0) {
    lock_out_counter -= (num_samples < lock_out_counter) ? num_samples : lock_out_counter;
    if (lock_out_counter == 0) {
        motion_off();
        (*motion_detection_callback)(false);
    }
  }

  /* Assert motion. */
  if (motion) {
    lock_out_counter = config.motionOnTime;
    motion_on();
    (*motion_detection_callback)(true);
  }

  ...
}
```

`motion_detection_callback` is static function pointer in PIR driver. It is assigned by callback function in application, which is registered to PIR driver over initial function.  

```C
void pir_init(motion_detection_callback_t callback_registration)
{
  /* Enable LDO to startup VPIR. */
  GPIO_PinOutSet(LDO_SHDN_B_PORT, LDO_SHDN_B_PIN);

  PIR_Init_TypeDef pirInit = PIR_INIT_DEFAULT;
  pirInit.opampMode = pirOpampModeExternal;

  initial(&pirInit, true);

  // set callback function for motion detection event
  motion_detection_callback = callback_registration;

  pir_start();
}
```

The figure below shows the structure of the PIR driver.  

![PIR structure](doc/PIR_structure.png)  

### 5.5. The ambient light sensor driver ###

With the ALS driver, it need more changes.

- Add the schedule event in order to driver can run automatic.  

As described above, get lux value need to phase.

First phase is send start command over I2C:

```C
void ambientLightSensorReadDataEventHandler()
{
    emberEventControlSetInactive(ambientLightSensorReadDataEventControl);
    Si11xxReadFromRegister(si115xHandle, REG_IRQ_STATUS);   // ensure INT is cleared
    if (mode == si1150ModeAuto) {
        Si115xStart(si115xHandle);
    } else {
        Si115xForce(si115xHandle);
    }
}
```

Then driver need to wait for second phase start. Next step will show how the second phase start.

- Using **em_gpio** and **gpiointerrupt** library to implement the sensor interrupt handler.

```C
void als_init(get_lux_completed_callback_t callback_registration)
{
    ...
    //gpio pinmode set
    GPIO_PinModeSet(SENSOR_INT_PORT, SENSOR_INT_PIN, gpioModeInput, 1);

    //register callback
    GPIOINT_CallbackRegister(SENSOR_INT_PIN, sensor_int_irq_handler);

    //interrupt configure
    GPIO_IntConfig(SENSOR_INT_PORT, SENSOR_INT_PIN, false, true, true);

    NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
    NVIC_EnableIRQ(GPIO_EVEN_IRQn);
    NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
    NVIC_EnableIRQ(GPIO_ODD_IRQn);
    ...
}
```

`sensor_int_irq_handler` is handler when receive interrupt signal from sensor. It is registered to **gpiointerrupt** by `GPIOINT_CallbackRegister(SENSOR_INT_PIN, sensor_int_irq_handler);`

```C
void sensor_int_irq_handler(uint8_t pin)
{
    if (pin == SENSOR_INT_PIN) {
        emberEventControlSetActive(ambientLightSensorInterruptEventControl);
    }
}
```

You can see `sensor_int_irq_handler` will start the second phase of get lux value by set active `ambientLightSensorInterruptEventControl`.

```C
void ambientLightSensorInterruptEventHandler()
{
    emberEventControlSetInactive(ambientLightSensorInterruptEventControl);
    uint32_t lux = read_lux(si115xHandle);

    get_lux_completed_callback(lux);

    /* repeat read lux value every 1s */
    emberEventControlSetDelayMS(ambientLightSensorReadDataEventControl, PERIOD_READ_LUX_MS);
}
```

You also see that after get lux value, driver will run the callback function `get_lux_completed_callback(lux);`

`get_lux_completed_callback` is static function pointer in ALS driver. It is assigned by callback function in application, which is registered to ALS driver over initial function.  

```C
void als_init(get_lux_completed_callback_t callback_registration)
{
    ...
    get_lux_completed_callback = callback_registration;
    ...
}
```

The figure below shows the structure of the ALS driver.  

![ALS structure](doc/ALS_structure.png)  

## 6. Reference ##

The implementation of PIR driver and ambient light sensor in this demo is based on this example:

- [Silicon Labs Occupancy Sensor EXP](https://github.com/SiliconLabs/occupancy-sensor-exp.git)

Refer links:

- [Zigbee Motion Sensor PIR Example](https://github.com/SiliconLabs/zigbee_applications/tree/master/zigbee_smart_lighting)

- [Z-Wave Motion Sensor PIR Example](https://github.com/SiliconLabs/z_wave_applications/tree/master/z_wave_motion_sensor_pir_application)

About setup network, on/off cluster can be found here:

- [Building a Zigbee 3.0 Switch and Light from Scratch](https://www.silabs.com/support/training/zigbee-application-layer-concepts/building-a-zigbee-3-0-switch-and-light-from-scratch)

About create zigbee project in Simplicity Studio and network security can be found here:

- [Zigbee Hands on Forming and Joining](https://github.com/SiliconLabs/IoT-Developer-Boot-Camp/wiki/Zigbee-Hands-on-Forming-and-Joining)

About using event in Simplicity Studio:

- [Zigbee Hands on Using Event](https://github.com/SiliconLabs/IoT-Developer-Boot-Camp/wiki/Zigbee-Hands-on-Using-Event)  
