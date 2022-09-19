# Zigbee RTC Time #
![Type badge](https://img.shields.io/badge/Type-Virtual%20application-green)
![Technology badge](https://img.shields.io/badge/Technology-Zigbee-green)
![License badge](https://img.shields.io/badge/License-Zlib-green)
![SDK badge](https://img.shields.io/badge/SDK-v3.2.1-green)

## Summary ##
In a common Zigbee network, the gateway normally has the capability of connecting to the internet, so it can get the date and time through NTP. Therefore, the gateway can act as the time server to provide the time source for the other Zigbee devices. This example demonstrates how we synchronize the date and time in Zigbee network. On the device side, the local date and time will be kept by the plugin **Simple Clock**. 

## Gecko SDK version ##
Gecko SDK Suite v3.2

## Hardware Required ##
- EFR32MG12 2.4GHz 19 dBm Radio Board (BRD4161A Rev A01)

## Connections Required ##
NA

## Setup ##
### Time Server ###
1. Create a **Z3GatewayHost** sample project.
2. In **ZCL** tab, enable the server side of the **Time** cluster.
3. Enable the plugin **Time Server Cluster**.
4. Enable the callback **emberAfGetCurrentTimeCallback**.
5. Generate the project.
6. Add the following source code to the file **\<projectname\>_callbacks.c**.
   ```C

    #define TIME_UNIX_EPOCH (1970u)
    #define TIME_ZIGBEE_EPOCH (2000u)
    #define TIME_SEC_PER_DAY (60u * 60u * 24u)
    #define TIME_ZIGBEE_UNIX_EPOCH_DIFF (TIME_ZIGBEE_EPOCH - TIME_UNIX_EPOCH)
    #define TIME_DAY_COUNT_ZIGBEE_TO_UNIX_EPOCH (TIME_ZIGBEE_UNIX_EPOCH_DIFF * 365u + 7u) ///< 30 years and 7 leap days
    #define TIME_ZIGBEE_EPOCH_OFFSET_SEC (TIME_DAY_COUNT_ZIGBEE_TO_UNIX_EPOCH * TIME_SEC_PER_DAY)

    int32u emberAfGetCurrentTimeCallback(void)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        //Round the micro seconds
        if (tv.tv_usec >= 500000)
        {
            tv.tv_sec++;
        }

        return tv.tv_sec - TIME_ZIGBEE_EPOCH_OFFSET_SEC;
    }
   ```
7. Create a NCP project with the default settings.
8. Build and test.


### Time Client ###
1. Create a **ZigbeeMinimal** sample project.
2. In **ZCL** tab, change the device type of the endpoint 1 to **Zigbee Custom**-->**LO devices**-->**LO On/Off Light**, then enable the client side of the **Time** cluster.
3. Enable the plugin **Simple Clock**.
4. In Callbacks tab, enable the callback **emberAfReadAttributesResponseCallback** and **emberAfStackStatusCallback**.
5. In **Includes** tab, add a custom event **emberTimeSyncEventControl** and its handler **emberTimeSyncEventHandler**.
6. Save and generate the project.
7. Add the following source code snippet to the **\<projectname\>_callbacks.c**.
  
  ```C
  EmberEventControl emberTimeSyncEventControl;

  void emberTimeSyncEventHandler()
  {
    emberEventControlSetInactive(emberTimeSyncEventControl);

    if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
      //send read attribute
      uint8_t timeAttributeIds[] = {
          LOW_BYTE(ZCL_TIME_ATTRIBUTE_ID),
          HIGH_BYTE(ZCL_TIME_ATTRIBUTE_ID)};

      emberAfFillCommandGlobalClientToServerReadAttributes(ZCL_TIME_CLUSTER_ID, timeAttributeIds, sizeof(timeAttributeIds));
      emberAfGetCommandApsFrame()->sourceEndpoint = 1;
      emberAfGetCommandApsFrame()->destinationEndpoint = 1;
      EmberStatus status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, 0x0000);
      emberAfCorePrintln("Query time from the gateway status=0x%X", status);

      emberEventControlSetDelayMS(emberTimeSyncEventControl, 5000);
    }
  }

  bool emberAfStackStatusCallback(EmberStatus status)
  {
    if (status == EMBER_NETWORK_DOWN) {
      emberEventControlSetInactive(emberTimeSyncEventControl);
    } else if (status == EMBER_NETWORK_UP) {
      emberEventControlSetDelayMS(emberTimeSyncEventControl, 5000);
    }

    // This value is ignored by the framework.
    return false;
  }

  boolean emberAfReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                                int8u *buffer,
                                                int16u bufLen)
  {
    if (ZCL_TIME_CLUSTER_ID != clusterId) {
      return false;
    }

    //attribute ID (2B) + status (1B) + date type (0B or 1B) + value (4B)
    if (bufLen < 7) {
      return false;
    }

    if ((emberAfGetInt16u(buffer, 0, bufLen) == ZCL_TIME_ATTRIBUTE_ID) && (emberAfGetInt8u(buffer, 2, bufLen) == EMBER_ZCL_STATUS_SUCCESS)) {
      emberAfSetTime(emberAfGetInt32u(buffer, 4, bufLen));
      emberAfCorePrintln("time sync ok, time: %4x", emberAfGetCurrentTime());

      emberEventControlSetDelayMS(emberTimeSyncEventControl, MILLISECOND_TICKS_PER_DAY);

      return true;
    }

    return false;
  }  
  ```
8.  Build and test.

## How It Works ##
Join the device into the network. The device will query the time from the gateway in about 5 seconds after the network is up. Then you can use the command **print time** to query the local time.
```
zigbee_rtc_time_sync_4161A>Reset info: 0x03 (EXT)
Extended Reset info: 0x0301 (PIN)
init pass
EMBER_NETWORK_UP 0x97D6
NWK Steering stack status 0x90
Query time from the gateway status=0x00
Processing message: len=11 profile=0104 cluster=000A

T00000000:RX len 11, ep 01, clus 0x000A (Time) FC 18 seq 00 cmd 01 payload[00 00 00 E2 4A 31 AD 28 ]

zigbee_rtc_time_sync_4161A>print time
UTC time: 8/16/2021 14:23:10 (28ad314e)
```

## .sls Projects Used ##
- [Timer Server - Z3GatewayHost.sls](SimplicityStudio/Z3GatewayHost.sls)
- [Time Client - zigbee_rtc_time_sync_4161A.sls](SimplicityStudio/zigbee_rtc_time_sync_4161A.sls)

## How to Port to Another Part ##
- Import the .sls file into Simplicity Studio
- Open the .isc file of each project, turn to "General" tab, hit button "Edit Architecture", then select the board and part.

## Special Notes ##
NA
