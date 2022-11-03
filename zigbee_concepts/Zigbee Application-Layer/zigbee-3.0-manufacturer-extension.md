# Zigbee 3.0: Manufacturer’s Extensions

There are some standard profilers and clusters can be used in Zigbee specification, it makes the Zigbee devices in the market built by different manufactures can be compatible with other. But manufacturers are free to extend the standard, so we will find how to extend it?

## Types of Manufacturer Specific Extensions

All communications regarding manufacturer specific extensions shall be transmitted with the manufacturer specific sub-field of the frame control field set to 1 and the manufacturer code included in the frame. This is because different manufactures may specify the same cluster.

![Figure 1](resources\manufacture01.png)

Usually, there are four ways to add manufacturer specific extensions within Zigbee.

1. Using the Private Profile

    - Zigbee devices must have the same profile ID in order to interact on the application layer with each other

    - Use a private Zigbee profile only if you are planning to build a completely closed system

    - The devices using private profile will not be interoperable with any other Zigbee devices using other profiles

2. Extending the Manufacturer Specific Clusters

    - __Must__ use cluster ID range 0xfc00 – 0xffff

    - __Must__ have an associated two-byte manufacturer code

    - All commands and attributes within a manufacturer-specific cluster are also considered manufacturer-specific

3. Extending the Manufacturer Specific Commands in Standard Zigbee Cluster

    - __May__ use the entire command ID range 0x00 – 0xff

    - __Must__ provide an associated two-byte manufacturer code

4. Extending the Manufacturer Specific Attributes in Standard Zigbee Cluster

    - __May__ use the entire attribute address range 0x0000 – 0xffff

    - __Must__ provide an associated two-byte manufacturer code

    - For reading a manufacturer-specific attribute you __must__ use the following functions

        - emberAfReadManufacturerSpecificServerAttribute

        - emberAfReadManufacturerSpecificClientAttribute

    - There are manufacturer-specific attribute changed callbacks that are independent from the standard attribute callbacks

        - emberAfXXXXClusterServerManufacturerSpecificAttributeChangedCallback

## Adding Manufacturer’s Extensions to Simplicity Studio

There are only two steps to apply the manufacturer’s extensions to Simplicity Studio.

1. Create a custom xml and put it to stack directory

    - Example: app\zcl\custom-enhancements.xml

2. Add the custom xml to the zcl-config.zap tab under project.slcp > Configugation Tools > Zigbee Cluster Configurator

    - Click ZCL Extensions > Add Custom ZCL

    - Save and reopen the .slcp file

![Figure 2](resources\manufacturer02.png)

__Note:__ This is used for EmberZnet 7.x

## Custom Manufacturer Specific Clusters

This is an example to introduce how to add a custom manufacturer specific clusters. We need prepare a xml file by ourselves firstly. Usually we should set the properties appropriately in the xml file. The properties includes __configurator__, __cluster__, __name__, __domain__, __description__, __code__, __define__, __client__, __server__, __attribute__, __command__. After the xml file is prepared, then put it to the correct directory and add it to Simplicity Studio.

You can see how these properties displayed in Simplicity Studio via below pictures

Properties: __cluster__, __name__, __domain__, __code__, __client__, __server__

![Figure 3](resources\manufacturer03.png)

Properties: __description__, __attribute__

![Figure 4](resources\manufacturer04.png)

Properties: __command__

![Figure 5](resources\manufacturer05.png)

## Custom Manufacturer Specific Attributes in On-Off Cluster

As we know, the on-off cluster is a standard cluster which defined in Zigbee Cluster Library, it has some attributes like on/off, global scene control, on time, off wait time and so on. If we want to add some custom attributes to the cluster, we need prepare the xml file and set the properties appropriately. After install the custom xml to Simplicity Studio we will see the specific attributes in on-off cluster.

Below is an example to introduce how to add custom manufacturer specific attributes in standard Zigbee on-off cluster.

![Figure 6](resources\manufacturer06.png)

## Custom Manufacturer Specific Commands in On-Off Cluster

The on-off cluster is a standard cluster which defined in Zigbee Cluster Library, it has some commands like on/off/toggle and so on.

After install the custom xml to Simplicity Studio we will see the specific commands in on-off cluster.

Below is an example to introduce how to add custom manufacturer specific commands in standard Zigbee on-off cluster.

![Figure 7](resources\manufacturer07.png)
