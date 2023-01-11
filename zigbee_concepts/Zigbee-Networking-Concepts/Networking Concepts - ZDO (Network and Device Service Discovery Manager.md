# Networking Concepts: ZDO (Network and device and Service Discovery Manager)

## Zigbee Device Object

The ZDO is a group of messages, concepts, and primitives that are all device specific, this way another device can get to know another device on the network. A good example is an end device that is requesting the network Key from the coordinator or the coordinator wants to get more information about the number of endpoints on a device. All these actions we collect in the ZDO group, we can distinguish the following categories in this group:

- __Network Manager:__ Handles network activities such as network discovery, leaving/joining a network, resetting a network connection and creating a network.

- __Device and Service Discovery:__ Handles device and service discovery, so other devices in the network can get to know the device capabilities.

- __Binding Manager:__ Handles end device binding, binding and unbinding activities. This is used to make binding between 2 endpoints on separate devices. This way we can bind a switch to a dedicated light. This will be handled in the “Networking Concepts: Binding” presentation.

- __Security Manager:__ Handles security services such as key loading, key establishment, key transport, and authentication. This will be handled in more detail in the “Networking Concepts: ZigBee security” presentation.

Depending on the device not all of these subcategories are mandatory. We will dive a little deeper in the device and service manager and the network manager. The binding manager and the security manager will be discussed separately.

| Name | Status |
|------|--------|
|Network Manager| Mandatory|
|Device and Service Discovery| Mandatory|
|Biding Manager  | Optional|
|Security Manager| Optional|

## Network Manager

Main Functions:
- Faciliates the scan procedure (Scan all channels for network creation or to find a joinable network)
- Supports orphan and rejoining the network
- Detects and reports interference to support channel changing

One of the tasks for the Network Manager is to form or join a network, before forming a network it will scan the channels to asses which channel is the best to create a network. The best channel is the channel with the least interference, this can come from other Zigbee networks on the channel but can also be from other network types in the 2.4 GHz band, think here about Bluetooth or WIFI. First, it will scan the predefined channels and after that, it will scan all the channels to find a quiet channel. This has a downside that a joining device can’t know up front where to find the network, It will use the same principle as the with the network creation to find a joinable network. It needs also to do a scan to find the network could be joined. For the joining (association) procedure we advise to use install code based association, this is a key based joining procedure.

Another task of the network manager is to manage the orphan and rejoining process. If a device is losing communication with the network, which can be caused for example by a power loss. If they still have the correct network key the device is able to join the network again with a rejoin request. This is the same procedure as the association procedure only without the association broadcast. Also for rejoining counts that the network may be on a different channel, here we do the same scan procedure again.

When an end device is losing communication with the network, it will try to find the parent with an orphan scan. This scan will search for the parent on different channels. If a rejoin or an orphan scan is not successful the device shall try to join the network through the association process.

Also, the task of the network manager is to report the interference on the channel. If there is too much interference on a channel the device will notify the coordinator. Which may decide to move the channel to another one where there is less interference. The interference measurement when the network is created is just a spot check, so it will not guarantee future interference. The algorithm to decide if the coordinator is switching channels is up to the user. Before switching channels the coordinator is broadcasting a network update request, so the whole network knows we move to another channel. If one of the device for some reason doesn’t hear the broadcast it will do a channel scan and rejoin the network.

All the functionality of the network manager is provided by the plugins in the stack. 

| Component |
|--------|
| Network Steering |
| Network Creator |
| ZigBee Pro Stack |
| ZigBee Pro Leaf Stack |

- __The Network Steering Component__ allows you to join a network, this will also provide the network scan.

- __The Network Creator__ allows you to create a network this is mostly used for coordinators.

- __The ZigBee Pro Stack__ will provide the base stack for Zigbee

- __The ZigBee Pro Leaf Stack__ will provide the base stack for Zigbee end devices.

## Device and Service Discovery

Main Functions:
- Device Discovery:
```
IEEE Request: zdoIeeeAddressRequestCommand
Network address Request: zdoNwkAddressRequestCommand
```
- Service Discovery
```
Node or power descriptor: emberNodeDescriptorRequest
Active endpoints of a device: emberActiveEndpointsRequest
Simple descriptor: emberSimpleDescriptorRequest
Match descriptor: matchDescriptorsRequest
```
- Routers and coordinators can store information & messages for sleepy end devices


After joining a network we don’t know anything about the device or his service. Whith the device discovery you can obtain:

- __The IEEE address__ is the unique EUI64 address

- __The network address of the device__ is the 16 bits short address of the device

This gives other devices in the network the opportunity to discover the EUI64 of a device whit the corresponding short address or the other way around.

With the service discovery, other devices in the network can obtain the services that the device is providing, it is nice to know if a device is a light or a motion detector. We can request the power description of the device, this will return the power mode of the device, sleepy or not sleepy, available power sources and the current power source and his level.

We can request the active endpoints of the device, whith this information we can ask a simple descriptor of the endpoints. This will give the device id, profile id and the clusters that are available on that particular endpoint. Whit this information you can paint a good picture of what the endpoint is capable of, also this information is used to form a binding to an endpoint.

Routers and coordinators can have the ability to store messages and information about a sleepy end device. This is to minimize the messages towards the sleepy end device, this extent the battery life of the device. The stack stores the short id and the EUI64 of the sleepy end device, to reduce messages. Also, parents have the ability to store complete messages for the sleepy end device, this way when the sleepy end device is waking up it can read the message and reply on it if needed.
