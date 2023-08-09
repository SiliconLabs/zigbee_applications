# Zigbee Human Detection #

## Overview ##

This project aims to implement human detection using the MLX90640 low-resolution IR camera. The detection integrates into a Zigbee application, emulating an ultrasonic presence sensor (0x0406) in an always-on Zigbee router device. Besides updating the cluster attributes, the device can bind and report whether a human is present. The device also reports whether a human is present on an SSD1306 OLED display.

The following picture shows the system view of how it works.

![system](images/system_connection.png)

## Gecko SDK version ##

- Gecko SDK Suite 4.3.1
- [Third Party Hardware Drivers v1.5.0](https://github.com/SiliconLabs/third_party_hw_drivers_extension)

## Hardware Required ##

- Zigbee Coordinator (host + NCP architecture)

  - NCP: [EFR32xG24 Dev Kit (BRD2601B)](https://www.silabs.com/development-tools/wireless/efr32xg24-dev-kit)
  - Host: Windows-based (Docker). For details regarding setting up Docker for windows, refer to the app note: [AN1389: Running Zigbee Host Applications in a Docker Container](https://www.silabs.com/documents/public/application-notes/an1389-running-host-applications-in-docker-containers.pdf)

- Zigbee Router device (SoC architecture)

  - [EFR32xG24 Dev Kit (BRD2601B)](https://www.silabs.com/development-tools/wireless/efr32xg24-dev-kit)

  - [SparkFun Micro OLED Breakout (Qwiic) board](https://www.sparkfun.com/products/14532)

  - [Sparkfun MLX90640 IR Array (MLX90640 FIR sensor)](https://www.sparkfun.com/products/14844)

## Connections Required ##

Zigbee Coordinator: The EFR32xG24 Dev Kit (NCP) can be connected to the Desktop (Host) via a Micro USB cable.

![board](images/coordinator_connection.png)

Zigbee Router device: The SparkFun Micro OLED Breakout (Qwiic) board and the Sparkfun MLX90640 IR Array can be easily connected with the EFR32xG24 Dev Kit by using the Qwiic cable.

![board](images/connection.png)

**Note:**

*If your router device implements on the SparkFun Thing Plus Matter - BRD2704A board*. It does not have an internal button (which is used to join or leave the network without CLI commands). To connect an external button to the board, you should use a ceramic capacitor (ex: Ceramic Capacitor 104) and a resistor to avoid the anti-vibration button used in the project as below.

![button](images/button_connection.png)

## Setup ##

To test this application, you can either create a project based on an example project or start with a "Zigbee - SoC ZigbeeMinimal" project based on your hardware.

### Create a project based on an example project ###

- **NCP project**
    1. From the Launcher Home, add your hardware to MyProducts, click on it, and click on the **EXAMPLE PROJECTS & DEMOS** tab. Find the example project with filter "NCP ncp-uart-hw".

    2. Click **Create** button on the **Zigbee - NCP ncp-uart-hw** example. This example project creation dialog pops up -> click Create and Finish and Project should be generated.

    ![create_ncp](images/create_ncp.png)

    3. Build and flash this example to your board.

- **Router project**
    1. From the Launcher Home, add your hardware to MyProducts, click on it, and click on the **EXAMPLE PROJECTS & DEMOS** tab. Find the example project with the filter "human".

    2. Click **Create** button on the **Zigbee - Human Detection (Router)** example. This example project creation dialog pops up -> click Create and Finish and Project should be generated.

    ![create_router](images/create_router.png)

    3. Build and flash this example to your board.

- **Zigbee Gateway Host project**
    1. From the Launcher Home, add "Linux 32 Bit" to MyProducts, click on it, and click on the **EXAMPLE PROJECTS & DEMOS** tab. Find the example project with the filter "human".

    2. Click **Create** button on the **Zigbee - Human Detection (Host)** example. This example project creation dialog pops up -> click Create and Finish and Project should be generated.

    ![create_coor](images/create_coor.png)

    3. To build this project, please refer to the app note: [AN1389: Running Zigbee Host Applications in a Docker Container](https://www.silabs.com/documents/public/application-notes/an1389-running-host-applications-in-docker-containers.pdf)

### Start with a "Zigbee - SoC ZigbeeMinimal" project ###

*If you want to create the projects from scratch for a different radio board or kit, follow these steps: The relevant source code files for the custom application code of the Host and the Router are located in the [src](src) folder.*

1. Create the projects

    - **NCP project**
      - Create a **Zigbee - NCP ncp-uart-hw** project for the EFR32xG24 Dev Kit (xG24-DK2601B) using Simplicity Studio v5.

    - **Router project**
      - Create a **Zigbee - SoC ZigbeeMinimal** project for the EFR32xG24 Dev Kit (xG24-DK2601B) using Simplicity Studio v5.

    - **Zigbee Gateway Host project**
      - On the Simplicity Studio launcher add a product with the name Linux 32 Bit from the My Products on the left side then create a **Zigbee - Host Gateway** project with **Copy contents**.

2. Configuring the projects

   2.1 Host project configuration

      - Use the default cluster configuration.
      - Select Z3Gateway.slcp and install the following component for the device:

        - [Zigbee] → [Zigbee 3.0] → [Find and Bind Target]

   2.2 Router project configuration

      - Load the model file into the project:

        - Create a tflite directory inside the config directory of the project and then copy the [.tflite](config/tflite/ir_human_detection.tflite) model file into it. The project configurator provides a tool that will automatically convert .tflite files into sl_tflite_micro_model source and header files.

      - Cluster configuration:

        - Make sure endpoint 1 has Device ID is HA Occupancy Sensor (0x0107) and Profile ID is 0x0104.

          ![Cluster_configurator](images/cluster_configurator.png)

      - Save the Cluster configuration.

      - Open the .slcp file. Select the SOFTWARE COMPONENTS tab and install the software components:

        - [Zigbee] → [Utility] → [Zigbee Device Config] → Configure as Router
        - [Zigbee] → [Cluster Library] → [Common] → [Reporting]
        - [Zigbee] → [Zigbee 3.0] → [Network Steering]
        - [Zigbee] → [Zigbee 3.0] → [Find and Bind Initiator]
        - [Platform] → [Driver] → [Button] → [Simple Button] → [btn0]
        - [Platform] → [Driver] → [LED] → [Simple LED] → [led0]
        - [Service] → [IO Stream] → [IO Stream: USART] → [vcom]

        - [Machine Learning] → [Kernels] → [TensorFlow Lite Micro] → Configure this component to use 20000 Tensor Arena size.

          ![tflite_configure](images/tflite_configure.png)

        - [Platform] → [Toolchain] → [Memory configuration] → Set this component to use 10240 Stack size and 12288 Heap size.

          ![mem_config](images/mem_config.png)

        - [Third Party Hardware Drivers] → [Display & LED] → [SSD1306 - Micro OLED Breakout (Sparkfun) - I2C] → use default configuaration

          ![ssd1306_config](images/ssd1306_config.png)

        - [Third Party Hardware Drivers] → [Service] → [GLIB - OLED Graphics Library]

        - [Third Party Hardware Drivers] → [Sensors] → [MLX90640 - IR Array Breakout (Sparkfun)]

        - Open [Platform] → [Driver] → [I2C] → [I2CSPM] → [inst0] → Set this component to use I2C1 peripheral, SCL to PC04 pin, SDA to PC05 pin and speed mode to fast mode (400kbit/s).

          ![i2c instance](images/i2c_instance.png)

**NOTE:**

- Make sure that the SDK extension already be installed. If not please follow [this documentation](https://github.com/SiliconLabs/third_party_hw_drivers_extension/blob/master/README.md).
- SDK Extension must be enabled for the project to install [SSD1306 - Micro OLED Breakout (Sparkfun) - I2C], [GLIB - OLED Graphics Library], and [MLX90640 - IR Array Breakout (Sparkfun)] components. Selecting [SSD1306 - Micro OLED Breakout (Sparkfun) - I2C] and [MLX90640 - IR Array Breakout (Sparkfun)] components will also include the "I2CSPM" component with an unconfigured instance: **inst0**.
- You need to create the bootloader project and flash it to the device before flashing the application. When flashing the application image to the device, use the .hex or .s37 output file. Flashing the .bin files may overwrite (erase) the bootloader.

## How It Works ##

### System Overview Diagram ###

![system overview](images/system_overview.png)

### Application Workflows ##

#### Startup and initialization ####

![initialization](images/initialization.png)

#### Application event loop ####

![loop](images/app_loop.png)

#### Zigbee integration & Button UI ####

![zigbee](images/zigbee_ui.png)

#### User Configuration ####

```c
// These are parameters that are optionally embedded
// into the .tflite model file
typedef struct AppSettings
{
  // norm_img = img * rescale
  float samplewise_norm_rescale;
  // norm_img = (img - mean(img)) / std(img)
  bool samplewise_norm_mean_and_std;
  // Drop all inference results older than this value
  uint32_t average_window_duration_ms;
  // The minimum number of inference results to average
  uint32_t minimum_count;
  // Minimum averaged model output threshold for a class
  // to be considered detected, 0-255; 255 = highest confidence
  uint8_t detection_threshold;
  // The number of samples that are different than the last detected sample
  // for a new detection to occur
  uint32_t suppression_count;
} app_settings_t;
```

## Testing ##

### Network formation ###

First, we form a centralized network on the coordinator, details regarding Z3 network formation can be found in the following [KBA](https://community.silabs.com/s/article/how-to-form-zigbee-3-0-network-with-emberznet-stack-x?language=en_US). The following command can be used in the Docker command line:

`plugin network-creator start 1`

After formation, you can use the info command to see the details of the network (EUI64, panID, nodeID, extended panID...) and the NCP endpoint configurations. The following image shows an example of the expected output for both commands:

![create network](images/network_creator.png)

Now we need to open the network for other devices to join, we use the following command:

`plugin network-creator-security open-network`

![open network](images/network_open.png)

The network will automatically close after 180 seconds. Now we need to allow our coordinator to act as a "find and bind" target. This way our Router will add a binding entry to its table immediately. We use the following command:

`plugin find_and_bind target 1`

### Joining the Router and observing reported measurements ###

Lastly, press button 0 in the Router board, this will trigger the network steering process of the device allowing it to join the coordinator's network, identify the coordinator as a "find and bind" target and create a new binding entry in its binding table which as mentioned before is a requirement for the "Reporting" plugin. From this point onwards the Router sends a report to the coordinator for changes in human presence. The coordinators' application will parse the received payload and print the value in the right format, this can be seen in the function `emberAfReportAttributesCallback()`. The following picture shows the expected CLI output log from the host and Router:

![host attr](images/host_attr.png)

![router report](images/router_report.png)

### Display ###

![display](images/display.png)

**Note:** score is the probability score of the detected gesture as a uint8 (i.e. 255 = highest confidence that the correct gesture was detected)

## Human Detection Model ##

### Model overview ###

Before continuing with this project, it is recommended to review the [MLTK Overview](https://siliconlabs.github.io/mltk/docs/overview.html), which provides an overview of the core concepts used by this project.

Image classification is one of the most important applications of deep learning and Artificial Intelligence. Image classification refers to assigning labels to images based on certain characteristics or features present in them. The algorithm identifies these features and uses them to differentiate between different images and assign labels to them [[1]](https://www.simplilearn.com/tutorials/deep-learning-tutorial/guide-to-building-powerful-keras-image-classification-models).

In this project, we have a dataset with two different image types:

- **Human** - Images of a human
- **Not Human** - Random images not containing human

We assign an ID, a.k.a. **label**, 0 and 1, to each of these classes.  
We then "train" a machine learning model so that when we input an image from one of the classes given to the model, the model's output is the corresponding class ID. In this way, at runtime on the embedded device when the camera captures an image of a human, the ML model predicts its corresponding class ID which the firmware application uses accordingly. i.e.

![model overview](images/model_overview.png)

### Dataset Model ###

The most important part of a machine learning model is the dataset that was used to train the model.

Class: [Human](dataset/human.zip)

![thumbs up](images/dataset_human.png)

Class: [Nothing](dataset/not_human.zip)

![thumbs up](images/dataset_nothing.png)

### Model Input ###

The model input should have the shape:  `<image height>  x <image width> x <image channels>` (24,32,1)

The datatype should be `float32`.

### Model Input Normalization ###

The application supports "normalizing" the input.

If the `samplewise_norm.rescale` [model parameter](https://siliconlabs.github.io/mltk/docs/guides/model_parameters.html#imagedatasetmixin)
is given, then each element in the image is multiplied by this scaling factor, i.e.:

```c
model_input_tensor = img * samplewise_norm.rescale
```

If the `samplewise_norm.mean_and_std` [model parameter](https://siliconlabs.github.io/mltk/docs/guides/model_parameters.html#imagedatasetmixin)
is given, then each element in the image is centered about its mean and scaled by its standard deviation, i.e.:

```c
model_input_tensor = (img  - mean(img)) / std(img)
```

In both these cases, the model input data type must be `float32`.

### Model Output ###

The model output should have the shape `1 x <number of classes>`  
where `<number of classes>` should be the number of classes that the model is able to detect.

The datatype should be `float32`:

- 0: human

- 1: Not human

### Model Evaluation ###

```txt
Name: ir_human_detection
Model Type: classification
Overall accuracy: 99.432%
Class accuracies:
- not_human = 99.625%
- human = 95.890%
Average ROC AUC: 99.242%
Class ROC AUC:
- not_human = 99.242%
- human = 99.242%
```

![Model Evaluation](images/ir_human_detection-roc.png)
![Model Evaluation](images/ir_human_detection-precision_vs_recall.png)
![Model Evaluation](images/ir_human_detection-tpr.png)
![Model Evaluation](images/ir_human_detection-fpr.png)
![Model Evaluation](images/ir_human_detection-tfp_fpr.png)