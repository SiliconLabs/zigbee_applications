/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *******************************************************************************
 * DEPRECATION NOTICE
 * This code has been deprecated. It has been provided for historical reference
 * only and should not be used. This code will not be maintained.
 * This code is subject to the quality disclaimer at the point in time prior
 * to deprecation and superseded by this deprecation notice.
 ******************************************************************************/

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.
#include <stdint.h>
#include <stdio.h>
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/plugin/network-creator-security/network-creator-security.h"
#include "app/framework/plugin/network-creator/network-creator.h"
#include "app/framework/plugin/reporting/reporting.h"
#include "sl_bt_rtos_adaptation.h"
#include "gatt_db.h"
#ifdef EMBER_AF_PLUGIN_DMP_UI_DEMO
 #include "app/framework/plugin/dmp-ui-demo/dmp-ui.h"
#elif defined(EMBER_AF_PLUGIN_DMP_UI_DEMO_STUB)
 #include "app/framework/plugin/dmp-ui-demo-stub/dmp-ui-stub.h"
#endif

/* Write response codes*/
#define ES_WRITE_OK                         0
#define ES_READ_OK                          0
#define ES_ERR_CCCD_CONF                    0x81
#define ES_ERR_PROC_IN_PROGRESS             0x80
#define ES_NO_CONNECTION                    0xFF
// Advertisement data
#define UINT16_TO_BYTES(n)        ((uint8_t) (n)), ((uint8_t)((n) >> 8))
#define UINT16_TO_BYTE0(n)        ((uint8_t) (n))
#define UINT16_TO_BYTE1(n)        ((uint8_t) ((n) >> 8))
// Ble TX test macros and functions
#define BLE_TX_TEST_DATA_SIZE   2
// We need to put the device name into a scan response packet,
// since it isn't included in the 'standard' beacons -
// I've included the flags, since certain apps seem to expect them
#define DEVNAME "DMP%02X%02X"
#define DEVNAME_LEN 8  // incl term null
#define UUID_LEN 16 // 128-bit UUID
#define SOURCE_ADDRESS_LEN 8

#define OTA_SCAN_RESPONSE_DATA        0x04
#define OTA_ADVERTISING_DATA          0x02
#define PUBLIC_DEVICE_ADDRESS         0
#define STATIC_RANDOM_ADDRESS         1
#define LE_GAP_NON_RESOLVABLE         0x04

#define LE_GAP_MAX_DISCOVERABLE_MODE   0x04
#define LE_GAP_MAX_CONNECTABLE_MODE    0x03
#define LE_GAP_MAX_DISCOVERY_MODE      0x02
#define BLE_INDICATION_TIMEOUT         30000
#define BUTTON_LONG_PRESS_TIME_MSEC    3000

// Use dmpUiZigBeePjoin() UI function to make the PANId flash on the UI
// when we are Identifying. NOTE: PJoin itself is NOT actually enabled
// by either of these calls!
#define uiPanIdFlash(x)   dmpUiZigBeePjoin(x)

static void toggleOnoffAttribute(void);
static bool writeIdentifyTime(uint16_t identifyTime);
static void startIdentifying(void);
static void stopIdentifying(void);
static void readLightState(uint8_t connection);
static void readTriggerSource(uint8_t connection);
static void readSourceAddress(uint8_t connection);
static void writeLightState(uint8_t connection, uint8array *writeValue);
static void enableBleAdvertisements(void);
static uint8_t bleConnectionInfoTableFindUnused(void);
static bool bleConnectionInfoTableIsEmpty(void);
static void bleConnectionInfoTablePrintEntry(uint8_t index);
static uint8_t bleConnectionInfoTableLookup(uint8_t connHandle);
static void printBleAddress(uint8_t *address);
static void setDefaultReportEntry(void);
static bool startPjoinAndIdentifying(uint16_t identifyTime);
static void startIdentifyOnAllChildNodes(uint16_t identifyTime);

// Event function forward declarations
void bleEventHandler(void);
void bleTxEventHandler(void);
void lcdMainMenuDisplayEventHandler(void);
void buttonEventHandler(void);

enum {
  DMP_LIGHT_OFF,
  DMP_LIGHT_ON
};
// Light state
enum {
  HANDLE_DEMO = 0,
  HANDLE_IBEACON = 1,
  HANDLE_EDDYSTONE = 2,

  MAX_ADV_HANDLES = 3
};

uint8_t adv_handle[MAX_ADV_HANDLES];

static gatt_client_config_flag_t ble_lightState_config = gatt_disable;
static gatt_client_config_flag_t ble_triggerSrc_config = gatt_disable;
static gatt_client_config_flag_t ble_bleSrc_config = gatt_disable;

static uint8_t ble_lightState = DMP_LIGHT_OFF;
static uint8_t ble_lastEvent = DMP_UI_DIRECTION_INVALID;
static uint8_t SourceAddress[SOURCE_ADDRESS_LEN];
static uint8_t SwitchEUI[SOURCE_ADDRESS_LEN];
static EmberEUI64 lightEUI;
static uint8_t activeBleConnections = 0;
static uint8_t lastButton;
static bool longPress = false;
static uint16_t bleNotificationsPeriodMs;
static DmpUiLightDirection_t lightDirection = DMP_UI_DIRECTION_INVALID;
static bool identifying = false;
static bool leavingNwk = false;

// Event control struct declarations
EmberEventControl networkEventControl;
EmberEventControl bleEventControl;
EmberEventControl bleTxEventControl;
EmberEventControl zigbeeEventControl;
EmberEventControl lcdMainMenuDisplayEventControl;
EmberEventControl lcdPermitJoinEventControl;
EmberEventControl buttonEventControl;

struct {
  bool inUse;
  bool isMaster;
  uint8_t connectionHandle;
  uint8_t bondingHandle;
  uint8_t remoteAddress[6];
} bleConnectionTable[EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS];

/** GATT Server Attribute User Read Configuration.
 *  Structure to register handler functions to user read events. */
typedef struct {
  uint16_t charId; /**< ID of the Characteristic. */
  void (*fctn)(uint8_t connection); /**< Handler function. */
} AppCfgGattServerUserReadRequest_t;

/** GATT Server Attribute Value Write Configuration.
 *  Structure to register handler functions to characteristic write events. */
typedef struct {
  uint16_t charId; /**< ID of the Characteristic. */
  /**< Handler function. */
  void (*fctn)(uint8_t connection, uint8array * writeValue);
} AppCfgGattServerUserWriteRequest_t;

// iBeacon structure and data
static struct {
  uint8_t flagsLen; /* Length of the Flags field. */
  uint8_t flagsType; /* Type of the Flags field. */
  uint8_t flags; /* Flags field. */
  uint8_t mandataLen; /* Length of the Manufacturer Data field. */
  uint8_t mandataType; /* Type of the Manufacturer Data field. */
  uint8_t compId[2]; /* Company ID field. */
  uint8_t beacType[2]; /* Beacon Type field. */
  uint8_t uuid[16]; /* 128-bit Universally Unique Identifier (UUID). The UUID is an identifier for the company using the beacon*/
  uint8_t majNum[2]; /* Beacon major number. Used to group related beacons. */
  uint8_t minNum[2]; /* Beacon minor number. Used to specify individual beacons within a group.*/
  uint8_t txPower; /* The Beacon's measured RSSI at 1 meter distance in dBm. See the iBeacon specification for measurement guidelines. */
} iBeaconData = {
/* Flag bits - See Bluetooth 4.0 Core Specification , Volume 3, Appendix C, 18.1 for more details on flags. */
  2, /* length  */
  0x01, /* type */
  0x04 | 0x02, /* Flags: LE General Discoverable Mode, BR/EDR is disabled. */

/* Manufacturer specific data */
  26, /* length of field*/
  0xFF, /* type of field */

/* The first two data octets shall contain a company identifier code from
 * the Assigned Numbers - Company Identifiers document */
  { UINT16_TO_BYTES(0x004C) },

/* Beacon type */
/* 0x0215 is iBeacon */
  { UINT16_TO_BYTE1(0x0215), UINT16_TO_BYTE0(0x0215) },

/* 128 bit / 16 byte UUID - generated specially for the DMP Demo */
  { 0x00, 0x47, 0xe7, 0x0a, 0x5d, 0xc1, 0x47, 0x25, 0x87, 0x99, 0x83, 0x05, 0x44,
    0xae, 0x04, 0xf6 },

/* Beacon major number - not used for this application */
  { UINT16_TO_BYTE1(256), UINT16_TO_BYTE0(256) },

/* Beacon minor number  - not used for this application*/
  { UINT16_TO_BYTE1(0), UINT16_TO_BYTE0(0) },

/* The Beacon's measured RSSI at 1 meter distance in dBm */
/* 0xC3 is -61dBm */
// TBD: check?
  0xC3
};

static struct {
  uint8_t flagsLen; /**< Length of the Flags field. */
  uint8_t flagsType; /**< Type of the Flags field. */
  uint8_t flags; /**< Flags field. */
  uint8_t serLen; /**< Length of Complete list of 16-bit Service UUIDs. */
  uint8_t serType; /**< Complete list of 16-bit Service UUIDs. */
  uint8_t serviceList[2]; /**< Complete list of 16-bit Service UUIDs. */
  uint8_t serDataLength; /**< Length of Service Data. */
  uint8_t serDataType; /**< Type of Service Data. */
  uint8_t uuid[2]; /**< 16-bit Eddystone UUID. */
  uint8_t frameType; /**< Frame type. */
  uint8_t txPower; /**< The Beacon's measured RSSI at 0 meter distance in dBm. */
  uint8_t urlPrefix; /**< URL prefix type. */
  uint8_t url[10]; /**< URL. */
} eddystone_data = {
/* Flag bits - See Bluetooth 4.0 Core Specification , Volume 3, Appendix C, 18.1 for more details on flags. */
  2, /* length  */
  0x01, /* type */
  0x04 | 0x02, /* Flags: LE General Discoverable Mode, BR/EDR is disabled. */
/* Service field length */
  0x03,
/* Service field type */
  0x03,
/* 16-bit Eddystone UUID */
  { UINT16_TO_BYTES(0xFEAA) },
/* Eddystone-TLM Frame length */
  0x10,
/* Service Data data type value */
  0x16,
/* 16-bit Eddystone UUID */
  { UINT16_TO_BYTES(0xFEAA) },
/* Eddystone-URL Frame type */
  0x10,
/* Tx power */
  0x00,
/* URL prefix - standard */
  0x00,
/* URL */
  { 's', 'i', 'l', 'a', 'b', 's', '.', 'c', 'o', 'm' }
};

static struct {
  uint16_t txDelayMs;
  uint8_t connHandle;
  uint16_t characteristicHandle;
  uint8_t size;
} bleTxTestParams;

struct responseData_t {
  uint8_t flagsLen; /**< Length of the Flags field. */
  uint8_t flagsType; /**< Type of the Flags field. */
  uint8_t flags; /**< Flags field. */
  uint8_t shortNameLen; /**< Length of Shortened Local Name. */
  uint8_t shortNameType; /**< Shortened Local Name. */
  uint8_t shortName[DEVNAME_LEN]; /**< Shortened Local Name. */
  uint8_t uuidLength; /**< Length of UUID. */
  uint8_t uuidType; /**< Type of UUID. */
  uint8_t uuid[UUID_LEN]; /**< 128-bit UUID. */
};

static struct responseData_t responseData = { 2, /* length (incl type) */
                                              0x01, /* type */
                                              0x04 | 0x02, /* Flags: LE General Discoverable Mode, BR/EDR is disabled. */
                                              DEVNAME_LEN + 1, // length of local name (incl type)
                                              0x08, // shortened local name
                                              { 'D', 'M', '0', '0', ':', '0', '0' },
                                              UUID_LEN + 1, // length of UUID data (incl type)
                                              0x06, // incomplete list of service UUID's
                                              // custom service UUID for silabs lamp in little-endian format
                                              { 0xc9, 0x1b, 0x80, 0x3d, 0x61, 0x50, 0x0c, 0x97, 0x8d, 0x45, 0x19,
                                                0x7d, 0x96, 0x5b, 0xe5, 0xba } };

static const AppCfgGattServerUserReadRequest_t appCfgGattServerUserReadRequest[] =
{
  { gattdb_light_state, readLightState },
  { gattdb_trigger_source, readTriggerSource },
  { gattdb_source_address, readSourceAddress },
  { 0, NULL }
};

static const AppCfgGattServerUserWriteRequest_t appCfgGattServerUserWriteRequest[] =
{
  { gattdb_light_state, writeLightState },
  { 0, NULL }
};

size_t appCfgGattServerUserReadRequestSize = COUNTOF(appCfgGattServerUserReadRequest) - 1;
size_t appCfgGattServerUserWriteRequestSize = COUNTOF(appCfgGattServerUserWriteRequest) - 1;

/**
 * Custom CLI.  This command tree is executed by typing "custom <command>"
 * See app/util/serial/command-interpreter2.h for more detail on writing commands.
 **/
/*  Example sub-menu */
//  extern void doSomethingFunction(void);
//  static EmberCommandEntry customSubMenu[] = {
//    emberCommandEntryAction("do-something", doSomethingFunction, "", "Do something description"),
//    emberCommandEntryTerminator()
//  };
//  extern void actionFunction(void);
static void readLightState(uint8_t connection)
{
  uint16_t sent_data_len;
  emberAfCorePrintln("Light state = %d\r\n", ble_lightState);
  /* Send response to read request */
  sl_status_t status =  sl_bt_gatt_server_send_user_read_response(connection,
                                                                  gattdb_light_state,
                                                                  ES_READ_OK,
                                                                  sizeof(ble_lightState),
                                                                  &ble_lightState,
                                                                  &sent_data_len);

  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Failed to readLightState");
  }
}

static void readTriggerSource(uint8_t connection)
{
  uint16_t sent_data_len;
  emberAfCorePrintln("Last event = %d\r\n", ble_lastEvent);

  /* Send response to read request */
  sl_status_t status =  sl_bt_gatt_server_send_user_read_response(connection,
                                                                  gattdb_trigger_source,
                                                                  ES_READ_OK,
                                                                  sizeof(ble_lastEvent),
                                                                  &ble_lastEvent,
                                                                  &sent_data_len);
  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Failed to readTriggerSource");
  }
}

static void readSourceAddress(uint8_t connection)
{
  uint16_t sent_data_len;
  emberAfCorePrintln("readSourceAddress");

  /* Send response to read request */
  sl_status_t status =  sl_bt_gatt_server_send_user_read_response(connection,
                                                                  gattdb_source_address,
                                                                  ES_READ_OK,
                                                                  sizeof(SourceAddress),
                                                                  SourceAddress,
                                                                  &sent_data_len);

  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Failed to readSourceAddress");
  }
}

static void writeLightState(uint8_t connection, uint8array *writeValue)
{
  emberAfCorePrintln("Light state write; %d\r\n", writeValue->data[0]);

  lightDirection = DMP_UI_DIRECTION_BLUETOOTH;

  emberAfWriteAttribute(emberAfPrimaryEndpoint(),
                        ZCL_ON_OFF_CLUSTER_ID,
                        ZCL_ON_OFF_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        (int8u *) &writeValue->data[0],
                        ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  sl_status_t status = sl_bt_gatt_server_send_user_write_response(connection,
                                                                  gattdb_light_state,
                                                                  ES_WRITE_OK);

  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Failed to writeLightState");
    return;
  }

  uint8_t index = bleConnectionInfoTableLookup(connection);

  if (index != 0xFF) {
    memset(SourceAddress, 0, sizeof(SourceAddress));
    for (int i = 0; i < SOURCE_ADDRESS_LEN - 2; i++) {
      SourceAddress[2 + i] =
        bleConnectionTable[index].remoteAddress[5 - i];
    }
  }
}

static void notifyLight(uint8_t lightState)
{
  ble_lightState = lightState;
  uint16_t sent_data_len;
  sl_status_t status;

  if (ble_lightState_config == gatt_indication) {
    emberAfCorePrintln("notifyLight : Light state = %d\r\n", lightState);
    /* Send notification/indication data */
    for (int i = 0; i < EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS; i++) {
      if (bleConnectionTable[i].inUse
          && bleConnectionTable[i].connectionHandle) {
        status = sl_bt_gatt_server_send_characteristic_notification(
          bleConnectionTable[i].connectionHandle,
          gattdb_light_state,
          sizeof(lightState),
          &lightState,
          &sent_data_len);
        if (status != SL_STATUS_OK) {
          emberAfCorePrintln("Failed to notifyLightState");
          return;
        }
      }
    }
  }
}

static void notifyTriggerSource(uint8_t connection, uint8_t triggerSource)
{
  sl_status_t status;
  uint16_t sent_data_len;
  if (ble_triggerSrc_config == gatt_indication) {
    emberAfCorePrintln("notifyTriggerSource :Last event = %d\r\n",
                       triggerSource);
    /* Send notification/indication data */
    status = sl_bt_gatt_server_send_characteristic_notification(connection,
                                                                gattdb_trigger_source,
                                                                sizeof(triggerSource),
                                                                &triggerSource,
                                                                &sent_data_len);

    if (status != SL_STATUS_OK) {
      emberAfCorePrintln("Failed to notifyTriggerSource");
      return;
    }
  }
}
static void notifySourceAddress(uint8_t connection)
{
  sl_status_t status;
  uint16_t sent_data_len;

  if (ble_triggerSrc_config == gatt_indication) {
    /* Send notification/indication data */
    status = sl_bt_gatt_server_send_characteristic_notification(connection,
                                                                gattdb_source_address,
                                                                sizeof(SourceAddress),
                                                                SourceAddress,
                                                                &sent_data_len);
    if (status != SL_STATUS_OK) {
      emberAfCorePrintln("Failed to notifySourceAddress");
      return;
    }
  }
}

static void toggleOnoffAttribute(void)
{
  EmberStatus status;
  int8u data;
  status = emberAfReadAttribute(emberAfPrimaryEndpoint(),
                                ZCL_ON_OFF_CLUSTER_ID,
                                ZCL_ON_OFF_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                (int8u*) &data,
                                sizeof(data),
                                NULL);

  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    if (data == 0x00) {
      data = 0x01;
    } else {
      data = 0x00;
    }

    lightDirection = DMP_UI_DIRECTION_SWITCH;
    emberAfGetEui64(lightEUI);
    for (int i = 0; i < SOURCE_ADDRESS_LEN; i++) {
      SourceAddress[i] = lightEUI[(SOURCE_ADDRESS_LEN - 1) - i];
    }
  } else {
    emberAfAppPrintln("read onoff attr: 0x%x", status);
  }

  status = emberAfWriteAttribute(emberAfPrimaryEndpoint(),
                                 ZCL_ON_OFF_CLUSTER_ID,
                                 ZCL_ON_OFF_ATTRIBUTE_ID,
                                 CLUSTER_MASK_SERVER,
                                 (int8u *) &data,
                                 ZCL_BOOLEAN_ATTRIBUTE_TYPE);
  emberAfAppPrintln("write to onoff attr: 0x%x", status);
}

static void setDefaultReportEntry(void)
{
  EmberAfPluginReportingEntry reportingEntry;
  emberAfClearReportTableCallback();
  reportingEntry.direction = EMBER_ZCL_REPORTING_DIRECTION_REPORTED;
  reportingEntry.endpoint = emberAfPrimaryEndpoint();
  reportingEntry.clusterId = ZCL_ON_OFF_CLUSTER_ID;
  reportingEntry.attributeId = ZCL_ON_OFF_ATTRIBUTE_ID;
  reportingEntry.mask = CLUSTER_MASK_SERVER;
  reportingEntry.manufacturerCode = EMBER_AF_NULL_MANUFACTURER_CODE;
  reportingEntry.data.reported.minInterval = 0x0001;
  reportingEntry.data.reported.maxInterval = 0x001E; // 30S report interval for SED.
  reportingEntry.data.reported.reportableChange = 0; // onoff is boolean type so it is unused
  emberAfPluginReportingConfigureReportedAttribute(&reportingEntry);
}

static bool writeIdentifyTime(uint16_t identifyTime)
{
  EmberAfStatus status =
    emberAfWriteServerAttribute(emberAfPrimaryEndpoint(),
                                ZCL_IDENTIFY_CLUSTER_ID,
                                ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                                (uint8_t *)&identifyTime,
                                sizeof(identifyTime));

  return (status == EMBER_ZCL_STATUS_SUCCESS);
}

static void startIdentifying(void)
{
  writeIdentifyTime(180);
}

static void stopIdentifying(void)
{
  writeIdentifyTime(0);

  uiPanIdFlash(false);
}

void emberAfPluginIdentifyStartFeedbackCallback(uint8_t endpoint,
                                                uint16_t identifyTime)
{
  if (identifyTime > 0) {
    identifying = true;
    emberAfAppPrintln("Start Identifying for %dS", identifyTime);
    uiPanIdFlash(true);
    emberAfSetDefaultPollControlCallback(EMBER_AF_SHORT_POLL);  // Use short poll while identifying.
  }
}

void emberAfPluginIdentifyStopFeedbackCallback(uint8_t endpoint)
{
  if (identifying) {
    identifying = false;
    emberAfAppPrintln("Stop Identifying");
    uiPanIdFlash(false);
    emberAfSetDefaultPollControlCallback(EMBER_AF_LONG_POLL); // Revert to long poll when we stop identifying.
  }
}

void buttonEventHandler(void)
{
  emberEventControlSetInactive(buttonEventControl);

  if (lastButton == BUTTON0) {
    toggleOnoffAttribute();
  } else if (lastButton == BUTTON1) {
    EmberNetworkStatus state = emberAfNetworkState();
    if (state == EMBER_NO_NETWORK) {
      // Join Network
      emberAfAppPrintln("Button- Join Nwk");

      EmberEUI64 wildcardEui64 = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, };
      EmberKeyData centralizedKey = { 0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C, 0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 };
      emberAddTransientLinkKey(wildcardEui64, &centralizedKey);
      emberAfPluginNetworkSteeringStart();
      dmpUiDisplayZigBeeState(DMP_UI_JOINING);
    } else {
      // already on NWK
      if (!leavingNwk) { // Ignore button1 events while leaving.
        if (!longPress) {
          if (identifying) {
            emberAfAppPrintln("Button- Identify stop");
            stopIdentifying();
          } else if (state == EMBER_JOINED_NETWORK) {
            emberAfAppPrintln("Button- Identify start");
            startIdentifying();
          }
        } else {
          leavingNwk = true;
          emberAfAppPrintln("Button- Leave Nwk");
          emberLeaveNetwork();
          emberClearBindingTable();
        }
      }
    }
  }
}

void lcdMainMenuDisplayEventHandler(void)
{
  emberEventControlSetInactive(lcdMainMenuDisplayEventControl);

  // dmpUiClrLcdDisplayMainScreen();
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
boolean emberAfStackStatusCallback(EmberStatus status)
{
  EmberNetworkStatus nwkState = emberAfNetworkState();

  emberAfCorePrintln("Stack status=0x%X, nwkState=%d", status, emberAfNetworkState());

  switch (nwkState) {
    case EMBER_JOINED_NETWORK:
      startIdentifying();
      break;
    case EMBER_NO_NETWORK:
      leavingNwk = false; // leave has completed.
      stopIdentifying();
      break;
    case EMBER_JOINED_NETWORK_NO_PARENT:
      stopIdentifying();
      break;
    default:
      break;
  }

  return false;
}

/** @brief Complete
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to EMBER_SUCCESS to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt. Ver.: always
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID. Ver.: always
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network. Ver.: always
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete. Ver.: always
 */
void emberAfPluginNetworkSteeringCompleteCallback(EmberStatus status,
                                                  uint8_t totalBeacons,
                                                  uint8_t joinAttempts,
                                                  uint8_t finalState)
{
  if (status == EMBER_SUCCESS) {
    setDefaultReportEntry();
    startIdentifying();
    dmpUiDisplayZigBeeState(DMP_UI_NETWORK_UP);
  } else {
    stopIdentifying();
    dmpUiDisplayZigBeeState(DMP_UI_NO_NETWORK);
  }
}

static void bleConnectionInfoTableInit(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS; i++) {
    bleConnectionTable[i].inUse = false;
  }
}

static uint8_t bleConnectionInfoTableLookup(uint8_t connHandle)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS; i++) {
    if (bleConnectionTable[i].inUse
        && bleConnectionTable[i].connectionHandle == connHandle) {
      return i;
    }
  }
  return 0xFF;
}

/** @brief Main Init
 *
 * This function is called from the application's main function. It gives the
 * application a chance to do any initialization required at system startup. Any
 * code that you would normally put into the top of the application's main()
 * routine should be put into this function. This is called before the clusters,
 * plugins, and the network are initialized so some functionality is not yet
 * available.
   Note: No callback in the Application Framework is
 * associated with resource cleanup. If you are implementing your application on
 * a Unix host where resource cleanup is a consideration, we expect that you
 * will use the standard Posix system calls, including the use of atexit() and
 * handlers for signals such as SIGTERM, SIGINT, SIGCHLD, SIGPIPE and so on. If
 * you use the signal() function to register your signal handler, please mind
 * the returned value which may be an Application Framework function. If the
 * return value is non-null, please make sure that you call the returned
 * function from your handler to avoid negating the resource cleanup of the
 * Application Framework itself.
 *
 */
void emberAfMainInitCallback(void)
{
  dmpUiInit();
  dmpUiDisplayHelp();
  emberEventControlSetDelayMS(lcdMainMenuDisplayEventControl, 10000);
  bleConnectionInfoTableInit();
}

/** @brief On/off Cluster Server Post Init
 *
 * Following resolution of the On/Off state at startup for this endpoint, perform any
 * additional initialization needed; e.g., synchronize hardware state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginOnOffClusterServerPostInitCallback(uint8_t endpoint)
{
}

/** @brief On/off Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
void emberAfOnOffClusterServerAttributeChangedCallback(int8u endpoint,
                                                       EmberAfAttributeId attributeId)
{
  EmberStatus status;
  int8u data;

  if (attributeId == ZCL_ON_OFF_ATTRIBUTE_ID) {
    status = emberAfReadAttribute(endpoint,
                                  ZCL_ON_OFF_CLUSTER_ID,
                                  ZCL_ON_OFF_ATTRIBUTE_ID,
                                  CLUSTER_MASK_SERVER,
                                  (int8u*) &data,
                                  sizeof(data),
                                  NULL);

    if (status == EMBER_ZCL_STATUS_SUCCESS) {
      if (data == 0x00) {
        halClearLed(BOARDLED0);
        halClearLed(BOARDLED1);
        dmpUiLightOff();
        notifyLight(DMP_LIGHT_OFF);
      } else {
        halSetLed(BOARDLED0);
        halSetLed(BOARDLED1);
        dmpUiLightOn();
        notifyLight(DMP_LIGHT_ON);
      }
      if ((lightDirection == DMP_UI_DIRECTION_BLUETOOTH)
          || (lightDirection == DMP_UI_DIRECTION_SWITCH)) {
        dmpUiUpdateDirection(lightDirection);
      } else {
        lightDirection = DMP_UI_DIRECTION_ZIGBEE;
        dmpUiUpdateDirection(lightDirection);
        for (int i = 0; i < SOURCE_ADDRESS_LEN; i++) {
          SourceAddress[i] = SwitchEUI[(SOURCE_ADDRESS_LEN - 1) - i];
        }
      }
      ble_lastEvent = lightDirection;
      lightDirection = DMP_UI_DIRECTION_INVALID;
    }
  }
}

/** @brief
 *
 * This function is called from the BLE stack to notify the application of a
 * stack event.
 */
void emberAfPluginBleEventCallback(sl_bt_msg_t *evt)
{
  switch (SL_BT_MSG_ID(evt->header)) {
    /* This event indicates that a remote GATT client is attempting to read a value of an
     *  attribute from the local GATT database, where the attribute was defined in the GATT
     *  XML firmware configuration file to have type="user". */

    case sl_bt_evt_gatt_server_user_read_request_id:
      for (int i = 0; i < appCfgGattServerUserReadRequestSize; i++) {
        if ((appCfgGattServerUserReadRequest[i].charId
             == evt->data.evt_gatt_server_user_read_request.characteristic)
            && (appCfgGattServerUserReadRequest[i].fctn)) {
          appCfgGattServerUserReadRequest[i].fctn(
            evt->data.evt_gatt_server_user_read_request.connection);
        }
      }
      break;

    /* This event indicates that a remote GATT client is attempting to write a value of an
     * attribute in to the local GATT database, where the attribute was defined in the GATT
     * XML firmware configuration file to have type="user".  */

    case sl_bt_evt_gatt_server_user_write_request_id:
      for (int i = 0; i < appCfgGattServerUserWriteRequestSize; i++) {
        if ((appCfgGattServerUserWriteRequest[i].charId
             == evt->data.evt_gatt_server_characteristic_status.characteristic)
            && (appCfgGattServerUserWriteRequest[i].fctn)) {
          appCfgGattServerUserWriteRequest[i].fctn(
            evt->data.evt_gatt_server_user_read_request.connection,
            &(evt->data.evt_gatt_server_attribute_value.value));
        }
      }
      break;

    case sl_bt_evt_system_boot_id: {
      bd_addr ble_address;
      uint8_t type;
      sl_status_t status = sl_bt_system_hello();
      emberAfCorePrintln("BLE hello: %s",
                         (status == SL_STATUS_OK) ? "success" : "error");

      status = sl_bt_system_get_identity_address(&ble_address, &type);
      emberAfCorePrint("BLE address: ");
      printBleAddress(ble_address.addr);
      emberAfCorePrintln("");

      sl_bt_advertiser_create_set(&adv_handle[HANDLE_DEMO]);
      sl_bt_advertiser_create_set(&adv_handle[HANDLE_IBEACON]);
      sl_bt_advertiser_create_set(&adv_handle[HANDLE_EDDYSTONE]);

      // start advertising
      enableBleAdvertisements();
    }
    break;
    case sl_bt_evt_gatt_server_characteristic_status_id: {
      sl_bt_evt_gatt_server_characteristic_status_t *StatusEvt =
        (sl_bt_evt_gatt_server_characteristic_status_t*) &(evt->data);
      if (StatusEvt->status_flags == gatt_server_confirmation) {
        emberAfCorePrintln(
          "characteristic= %d , GAT_SERVER_CLIENT_CONFIG_FLAG = %d\r\n",
          StatusEvt->characteristic, StatusEvt->client_config_flags);
        if (StatusEvt->characteristic == gattdb_light_state) {
          notifyTriggerSource(StatusEvt->connection, ble_lastEvent);
        } else if (StatusEvt->characteristic == gattdb_trigger_source) {
          notifySourceAddress(StatusEvt->connection);
        }
      } else if (StatusEvt->status_flags == gatt_server_client_config) {
        if (StatusEvt->characteristic == gattdb_light_state) {
          ble_lightState_config = (gatt_client_config_flag_t)StatusEvt->client_config_flags;
        } else if (StatusEvt->characteristic == gattdb_trigger_source) {
          ble_triggerSrc_config = (gatt_client_config_flag_t)StatusEvt->client_config_flags;
        } else if (StatusEvt->characteristic == gattdb_source_address) {
          ble_bleSrc_config = (gatt_client_config_flag_t)StatusEvt->client_config_flags;
        }
        emberAfCorePrintln(
          "SERVER : ble_lightState_config= %d , ble_triggerSrc_config = %d , ble_bleSrc_config = %d\r\n",
          ble_lightState_config,
          ble_triggerSrc_config,
          ble_bleSrc_config);
      }
    }
    break;
    case sl_bt_evt_connection_opened_id: {
      emberAfCorePrintln("sl_bt_evt_connection_opened_id \n");
      sl_bt_evt_connection_opened_t *conn_evt =
        (sl_bt_evt_connection_opened_t*) &(evt->data);
      uint8_t index = bleConnectionInfoTableFindUnused();
      if (index == 0xFF) {
        emberAfCorePrintln("MAX active BLE connections");
        assert(index < 0xFF);
      } else {
        bleConnectionTable[index].inUse = true;
        bleConnectionTable[index].isMaster = (conn_evt->master > 0);
        bleConnectionTable[index].connectionHandle = conn_evt->connection;
        bleConnectionTable[index].bondingHandle = conn_evt->bonding;
        memcpy(bleConnectionTable[index].remoteAddress,
               conn_evt->address.addr, 6);

        activeBleConnections++;
        //preferred phy 1: 1M phy, 2: 2M phy, 4: 125k coded phy, 8: 500k coded phy
        //accepted phy 1: 1M phy, 2: 2M phy, 4: coded phy, ff: any
        sl_bt_connection_set_preferred_phy(conn_evt->connection, test_phy_1m, 0xff);
        enableBleAdvertisements();
        emberAfCorePrintln("BLE connection opened");
        bleConnectionInfoTablePrintEntry(index);
        emberAfCorePrintln("%d active BLE connection",
                           activeBleConnections);
      }
    }
    break;
    case sl_bt_evt_connection_phy_status_id: {
      sl_bt_evt_connection_phy_status_t *conn_evt =
        (sl_bt_evt_connection_phy_status_t *)&(evt->data);
      // indicate the PHY that has been selected
      emberAfCorePrintln("now using the %dMPHY\r\n",
                         conn_evt->phy);
    }
    break;
    case sl_bt_evt_connection_closed_id: {
      sl_bt_evt_connection_closed_t *conn_evt =
        (sl_bt_evt_connection_closed_t*) &(evt->data);
      uint8_t index = bleConnectionInfoTableLookup(conn_evt->connection);
      assert(index < 0xFF);

      bleConnectionTable[index].inUse = false;
      if ( activeBleConnections ) {
        --activeBleConnections;
      }
      // restart advertising, set connectable
      enableBleAdvertisements();
      if (bleConnectionInfoTableIsEmpty()) {
        dmpUiBluetoothConnected(false);
      }
      emberAfCorePrintln(
        "BLE connection closed, handle=0x%x, reason=0x%2x : [%d] active BLE connection",
        conn_evt->connection, conn_evt->reason, activeBleConnections);
    }
    break;
    case sl_bt_evt_scanner_scan_report_id: {
      sl_bt_evt_scanner_scan_report_t *scan_evt =
        (sl_bt_evt_scanner_scan_report_t*) &(evt->data);
      emberAfCorePrint("Scan response, address type=0x%x, address: ",
                       scan_evt->address_type);
      printBleAddress(scan_evt->address.addr);
      emberAfCorePrintln("");
    }
    break;
    case sl_bt_evt_sm_list_bonding_entry_id: {
      sl_bt_evt_sm_list_bonding_entry_t * bonding_entry_evt =
        (sl_bt_evt_sm_list_bonding_entry_t*) &(evt->data);
      emberAfCorePrint("Bonding handle=0x%x, address type=0x%x, address: ",
                       bonding_entry_evt->bonding,
                       bonding_entry_evt->address_type);
      printBleAddress(bonding_entry_evt->address.addr);
      emberAfCorePrintln("");
    }
    break;
    case sl_bt_evt_gatt_service_id: {
      sl_bt_evt_gatt_service_t* service_evt =
        (sl_bt_evt_gatt_service_t*) &(evt->data);
      uint8_t i;
      emberAfCorePrintln(
        "GATT service, conn_handle=0x%x, service_handle=0x%4x",
        service_evt->connection, service_evt->service);
      emberAfCorePrint("UUID=[");
      for (i = 0; i < service_evt->uuid.len; i++) {
        emberAfCorePrint("0x%x ", service_evt->uuid.data[i]);
      }
      emberAfCorePrintln("]");
    }
    break;
    case sl_bt_evt_gatt_characteristic_id: {
      sl_bt_evt_gatt_characteristic_t* char_evt =
        (sl_bt_evt_gatt_characteristic_t*) &(evt->data);
      uint8_t i;
      emberAfCorePrintln(
        "GATT characteristic, conn_handle=0x%x, char_handle=0x%2x, properties=0x%x",
        char_evt->connection,
        char_evt->characteristic,
        char_evt->properties);
      emberAfCorePrint("UUID=[");
      for (i = 0; i < char_evt->uuid.len; i++) {
        emberAfCorePrint("0x%x ", char_evt->uuid.data[i]);
      }
      emberAfCorePrintln("]");
    }
    break;
    case sl_bt_evt_gatt_characteristic_value_id: {
      sl_bt_evt_gatt_characteristic_value_t* char_evt =
        (sl_bt_evt_gatt_characteristic_value_t*) &(evt->data);
      uint8_t i;

      if (char_evt->att_opcode == gatt_handle_value_indication) {
        sl_bt_gatt_send_characteristic_confirmation(
          char_evt->connection);
      }
      emberAfCorePrintln(
        "GATT (client) characteristic value, handle=0x%x, characteristic=0x%2x, att_op_code=0x%x",
        char_evt->connection,
        char_evt->characteristic,
        char_evt->att_opcode);
      emberAfCorePrint("value=[");
      for (i = 0; i < char_evt->value.len; i++) {
        emberAfCorePrint("0x%x ", char_evt->value.data[i]);
      }
      emberAfCorePrintln("]");
    }
    break;
    case sl_bt_evt_gatt_server_attribute_value_id: {
      sl_bt_evt_gatt_server_attribute_value_t* attr_evt =
        (sl_bt_evt_gatt_server_attribute_value_t*) &(evt->data);

      uint8_t i;
      emberAfCorePrintln(
        "GATT (server) attribute value, handle=0x%x, attribute=0x%2x, att_op_code=0x%x",
        attr_evt->connection,
        attr_evt->attribute,
        attr_evt->att_opcode);
      emberAfCorePrint("value=[");
      for (i = 0; i < attr_evt->value.len; i++) {
        emberAfCorePrint("0x%x ", attr_evt->value.data[i]);
      }
      emberAfCorePrintln("]");
      // Forward the attribute over the ZigBee network.
      emberAfWriteAttribute(emberAfPrimaryEndpoint(),
                            ZCL_ON_OFF_CLUSTER_ID,
                            ZCL_ON_OFF_ATTRIBUTE_ID,
                            CLUSTER_MASK_SERVER,
                            (int8u *) attr_evt->value.data,
                            ZCL_BOOLEAN_ATTRIBUTE_TYPE);

      lightDirection = DMP_UI_DIRECTION_BLUETOOTH;
    }
    break;
    case sl_bt_evt_connection_parameters_id: {
      sl_bt_evt_connection_parameters_t* param_evt =
        (sl_bt_evt_connection_parameters_t*) &(evt->data);
      emberAfCorePrintln(
        "BLE connection parameters are updated, handle=0x%x, interval=0x%2x, latency=0x%2x, timeout=0x%2x, security=0x%x, txsize=0x%2x",
        param_evt->connection,
        param_evt->interval,
        param_evt->latency,
        param_evt->timeout,
        param_evt->security_mode,
        param_evt->txsize);
      dmpUiBluetoothConnected(true);
    }
    break;
    case sl_bt_evt_gatt_procedure_completed_id: {
      sl_bt_evt_gatt_procedure_completed_t* proc_comp_evt =
        (sl_bt_evt_gatt_procedure_completed_t*) &(evt->data);
      emberAfCorePrintln("BLE procedure completed, handle=0x%x, result=0x%2x",
                         proc_comp_evt->connection, proc_comp_evt->result);
    }
    break;
    default:
      break;
  }
}

/** @brief Message Sent
 *
 * This function is called by the application framework from the message sent
 * handler, when it is informed by the stack regarding the message sent status.
 * All of the values passed to the emberMessageSentHandler are passed on to this
 * callback. This provides an opportunity for the application to verify that its
 * message has been sent successfully and take the appropriate action. This
 * callback should return a bool value of true or false. A value of true
 * indicates that the message sent notification has been handled and should not
 * be handled by the application framework.
 *
 * @param type   Ver.: always
 * @param indexOrDestination   Ver.: always
 * @param apsFrame   Ver.: always
 * @param msgLen   Ver.: always
 * @param message   Ver.: always
 * @param status   Ver.: always
 */
boolean emberAfMessageSentCallback(EmberOutgoingMessageType type,
                                   int16u indexOrDestination,
                                   EmberApsFrame* apsFrame,
                                   int16u msgLen,
                                   int8u* message,
                                   EmberStatus status)
{
  return false;
}

/** @brief Hal Button Isr
 *
 * This callback is called by the framework whenever a button is pressed on the
 * device. This callback is called within ISR context.
 *
 * @param button The button which has changed state, either BUTTON0 or BUTTON1
 * as defined in the appropriate BOARD_HEADER.  Ver.: always
 * @param state The new state of the button referenced by the button parameter,
 * either ::BUTTON_PRESSED if the button has been pressed or ::BUTTON_RELEASED
 * if the button has been released.  Ver.: always
 */
void emberAfHalButtonIsrCallback(int8u button, int8u state)
{
  static uint16_t buttonPressTime;
  uint16_t currentTime = 0;
  if (state == BUTTON_PRESSED) {
    if (button == BUTTON1) {
      buttonPressTime = halCommonGetInt16uMillisecondTick();
    }
  } else if (state == BUTTON_RELEASED) {
    if (button == BUTTON1) {
      currentTime = halCommonGetInt16uMillisecondTick();
      longPress = ((currentTime - buttonPressTime) > BUTTON_LONG_PRESS_TIME_MSEC);
    }
    lastButton = button;
    emberEventControlSetActive(buttonEventControl);
  }
}

/** @brief Pre Command Received
 *
 * This callback is the second in the Application Framework's message processing
 * chain. At this point in the processing of incoming over-the-air messages, the
 * application has determined that the incoming message is a ZCL command. It
 * parses enough of the message to populate an EmberAfClusterCommand struct. The
 * Application Framework defines this struct value in a local scope to the
 * command processing but also makes it available through a global pointer
 * called emberAfCurrentCommand, in app/framework/util/util.c. When command
 * processing is complete, this pointer is cleared.
 *
 * @param cmd   Ver.: always
 */
boolean emberAfPreCommandReceivedCallback(EmberAfClusterCommand* cmd)
{
  if ((cmd->commandId == ZCL_ON_COMMAND_ID)
      || (cmd->commandId == ZCL_OFF_COMMAND_ID)
      || (cmd->commandId == ZCL_TOGGLE_COMMAND_ID)) {
    memset(SwitchEUI, 0, SOURCE_ADDRESS_LEN);
    emberLookupEui64ByNodeId(cmd->source, SwitchEUI);
    emberAfCorePrintln(
      "SWITCH ZCL toggle/on/off EUI [%x %x %x %x %x %x %x %x]",
      SwitchEUI[7],
      SwitchEUI[6],
      SwitchEUI[5],
      SwitchEUI[4],
      SwitchEUI[3],
      SwitchEUI[2],
      SwitchEUI[1],
      SwitchEUI[0]);
  }
  return false;
}

/** @brief Pre Message Received
 *
 * This callback is the first in the Application Framework's message processing
 * chain. The Application Framework calls it when a message has been received
 * over the air but has not yet been parsed by the ZCL command-handling code. If
 * you wish to parse some messages that are completely outside the ZCL
 * specification or are not handled by the Application Framework's command
 * handling code, you should intercept them for parsing in this callback.

 *   This callback returns a Boolean value indicating whether or not the message
 * has been handled. If the callback returns a value of true, then the
 * Application Framework assumes that the message has been handled and it does
 * nothing else with it. If the callback returns a value of false, then the
 * application framework continues to process the message as it would with any
 * incoming message.
   Note:   This callback receives a pointer to an
 * incoming message struct. This struct allows the application framework to
 * provide a unified interface between both Host devices, which receive their
 * message through the ezspIncomingMessageHandler, and SoC devices, which
 * receive their message through emberIncomingMessageHandler.
 *
 * @param incomingMessage   Ver.: always
 */
boolean emberAfPreMessageReceivedCallback(EmberAfIncomingMessage* incomingMessage)
{
  return false;
}

/** @brief Pre Message Send
 *
 * This function is called by the framework when it is about to pass a message
 * to the stack primitives for sending.   This message may or may not be ZCL,
 * ZDO, or some other protocol.  This is called prior to
   any ZigBee
 * fragmentation that may be done.  If the function returns true it is assumed
 * the callback has consumed and processed the message.  The callback must also
 * set the EmberStatus status code to be passed back to the caller.  The
 * framework will do no further processing on the message.
   If the
 * function returns false then it is assumed that the callback has not processed
 * the mesasge and the framework will continue to process accordingly.
 *
 * @param messageStruct The structure containing the parameters of the APS
 * message to be sent.  Ver.: always
 * @param status A pointer to the status code value that will be returned to the
 * caller.  Ver.: always
 */
boolean emberAfPreMessageSendCallback(EmberAfMessageStruct* messageStruct,
                                      EmberStatus* status)
{
  return false;
}

/** @brief Trust Center Join
 *
 * This callback is called from within the application framework's
 * implementation of emberTrustCenterJoinHandler or ezspTrustCenterJoinHandler.
 * This callback provides the same arguments passed to the
 * TrustCenterJoinHandler. For more information about the TrustCenterJoinHandler
 * please see documentation included in stack/include/trust-center.h.
 *
 * @param newNodeId   Ver.: always
 * @param newNodeEui64   Ver.: always
 * @param parentOfNewNode   Ver.: always
 * @param status   Ver.: always
 * @param decision   Ver.: always
 */
void emberAfTrustCenterJoinCallback(EmberNodeId newNodeId,
                                    EmberEUI64 newNodeEui64,
                                    EmberNodeId parentOfNewNode,
                                    EmberDeviceUpdate status,
                                    EmberJoinDecision decision)
{
  if (status == EMBER_DEVICE_LEFT) {
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++) {
      EmberBindingTableEntry entry;
      emberGetBinding(i, &entry);
      if ((entry.type == EMBER_UNICAST_BINDING)
          && (entry.clusterId == ZCL_ON_OFF_CLUSTER_ID)
          && ((MEMCOMPARE(entry.identifier, newNodeEui64, EUI64_SIZE)
               == 0))) {
        emberDeleteBinding(i);
        emberAfAppPrintln("deleted binding entry: %d", i);
        break;
      }
    }
  }
}

/** @brief Pre ZDO Message Received
 *
 * This function passes the application an incoming ZDO message and gives the
 * appictation the opportunity to handle it. By default, this callback returns
 * false indicating that the incoming ZDO message has not been handled and
 * should be handled by the Application Framework.
 *
 * @param emberNodeId   Ver.: always
 * @param apsFrame   Ver.: always
 * @param message   Ver.: always
 * @param length   Ver.: always
 */
boolean emberAfPreZDOMessageReceivedCallback(EmberNodeId emberNodeId,
                                             EmberApsFrame* apsFrame,
                                             int8u* message,
                                             int16u length)
{
  return false;
}

/** @brief Ok To Sleep
 *
 * This function is called by the Idle/Sleep plugin before sleeping. It is
 * called with interrupts disabled. The application should return true if the
 * device may sleep or false otherwise.
 *
 * @param durationMs The maximum duration in milliseconds that the device will
 * sleep. Ver.: always
 */
bool emberAfPluginIdleSleepOkToSleepCallback(uint32_t durationMs)
{
  // if no serial input pending
  if (emberSerialReadAvailable(APP_SERIAL) != 0) {
    return false;
  }

  // if not in middle of reading a command
  if (emberCommandInterpreterBusy()) {
    return false;
  }

  // Don't allow sleeping while identifying.
  // (Device polls at ShortPoll rate while identifying)
  return !identifying;
}

void bleEventHandler(void)
{
  emberEventControlSetDelayMS(bleEventControl, bleNotificationsPeriodMs);
}

#ifdef EMBER_AF_ENABLE_CUSTOM_COMMANDS
static void printBleConnectionTableCommand(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS; i++) {
    if (bleConnectionTable[i].inUse) {
      bleConnectionInfoTablePrintEntry(i);
    }
  }
}

static void enableBleNotificationsCommand(void)
{
  bleNotificationsPeriodMs = emberUnsignedCommandArgument(0);
#if defined(DMP_DEBUG)
  emberAfCorePrintln("BLE notifications enabled");
#endif //DMP_DEBUG
  bleEventHandler();
}

static void disableBleNotificationsCommand(void)
{
#if defined(DMP_DEBUG)
  emberAfCorePrintln("BLE notifications disabled");
#endif //DMP_DEBUG
  emberEventControlSetInactive(bleEventControl);
}
#endif

void bleTxEventHandler(void);

EmberCommandEntry emberAfCustomCommands[] = {
#ifdef EMBER_AF_ENABLE_CUSTOM_COMMANDS
  emberCommandEntryAction("print-ble-connections",
                          printBleConnectionTableCommand,
                          "",
                          "Print BLE connections info"),
  emberCommandEntryAction("enable-ble-notifications",
                          enableBleNotificationsCommand,
                          "v",
                          "Enable BLE temperature notifications"),
  emberCommandEntryAction("disable-ble-notifications",
                          disableBleNotificationsCommand,
                          "",
                          "Disable BLE temperature notifications"),
#endif //EMBER_AF_ENABLE_CUSTOM_COMMANDS
  emberCommandEntryTerminator(),
};

//------------------------------------------------------------------------------

// BLE connection info table functions
static uint8_t bleConnectionInfoTableFindUnused(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS; i++) {
    if (!bleConnectionTable[i].inUse) {
      return i;
    }
  }
  return 0xFF;
}

static bool bleConnectionInfoTableIsEmpty(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS; i++) {
    if (bleConnectionTable[i].inUse) {
      return false;
    }
  }
  return true;
}

static void bleConnectionInfoTablePrintEntry(uint8_t index)
{
  assert(index < EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS
         && bleConnectionTable[index].inUse);
  emberAfCorePrintln("**** Connection Info index[%d]****", index);
  emberAfCorePrintln("connection handle 0x%x",
                     bleConnectionTable[index].connectionHandle);
  emberAfCorePrintln("bonding handle = 0x%x",
                     bleConnectionTable[index].bondingHandle);
  emberAfCorePrintln("local node is %s",
                     (bleConnectionTable[index].isMaster) ? "master" : "slave");
  emberAfCorePrint("remote address: ");
  printBleAddress(bleConnectionTable[index].remoteAddress);
  emberAfCorePrintln("");
  emberAfCorePrintln("*************************");
}

static void printBleAddress(uint8_t *address)
{
  emberAfCorePrint("[%X %X %X %X %X %X]",
                   address[5], address[4], address[3],
                   address[2], address[1], address[0]);
}

//------------------------------------------------------------------------------

void bleTxEventHandler(void)
{
  uint8_t txData[BLE_TX_TEST_DATA_SIZE];
  uint8_t i;

  for (i = 0; i < BLE_TX_TEST_DATA_SIZE; i++) {
    txData[i] = i;
  }

  sl_status_t status = sl_bt_gatt_write_characteristic_value(
    bleTxTestParams.connHandle,
    bleTxTestParams.characteristicHandle,
    BLE_TX_TEST_DATA_SIZE,
    txData);

  emberEventControlSetDelayMS(bleTxEventControl, bleTxTestParams.txDelayMs);
}

static void BeaconAdvertisements(uint16_t devId)
{
  static uint8_t *advData;
  static uint8_t advDataLen;
  sl_status_t status;

  iBeaconData.minNum[0] = UINT16_TO_BYTE1(devId);
  iBeaconData.minNum[1] = UINT16_TO_BYTE0(devId);

  advData = (uint8_t*) &iBeaconData;
  advDataLen = sizeof(iBeaconData);
  /* Set custom advertising data */
  status = sl_bt_advertiser_set_data(adv_handle[HANDLE_IBEACON], 0, advDataLen, advData);
  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Error sl_bt_advertiser_set_data code: ", status);
    return;
  }

  status = sl_bt_advertiser_set_channel_map(adv_handle[HANDLE_IBEACON], 7);
  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Error sl_bt_advertiser_set_channel_map code: ", status);
    return;
  }

  status = sl_bt_advertiser_set_timing(adv_handle[HANDLE_IBEACON],   // handle
                                       (100 / 0.625), //100ms min adv interval in terms of 0.625ms
                                       (100 / 0.625), //100ms max adv interval in terms of 0.625ms
                                       0,   // duration : continue advertisement until stopped
                                       0);   // max_events :continue advertisement until stopped
  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Error iBeacon sl_bt_advertiser_set_timing code: ", status);
    return;
  }

  status = sl_bt_advertiser_set_configuration(adv_handle[HANDLE_IBEACON], LE_GAP_NON_RESOLVABLE);
  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Error iBeacon sl_bt_advertiser_set_configuration code: ", status);
    return;
  }

  status = sl_bt_advertiser_start(adv_handle[HANDLE_IBEACON],
                                  advertiser_user_data,
                                  advertiser_non_connectable);
  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Error iBeacon sl_bt_advertiser_start code: ", status);
    return;
  }

  advData = (uint8_t*) &eddystone_data;
  advDataLen = sizeof(eddystone_data);
  /* Set custom advertising data */
  status = sl_bt_advertiser_set_data(adv_handle[HANDLE_EDDYSTONE], 0, advDataLen, advData);
  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Error eddystone sl_bt_advertiser_set_data code: ", status);
    return;
  }

  status = sl_bt_advertiser_set_channel_map(adv_handle[HANDLE_EDDYSTONE], 7);
  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Error eddystone sl_bt_advertiser_set_channel_map code: ", status);
    return;
  }

  status = sl_bt_advertiser_set_timing(adv_handle[HANDLE_EDDYSTONE],   // handle
                                       (100 / 0.625), //100ms min adv interval in terms of 0.625ms
                                       (100 / 0.625), //100ms max adv interval in terms of 0.625ms
                                       0,   // duration : continue advertisement until stopped
                                       0);   // max_events :continue advertisement until stopped
  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Error eddystone sl_bt_advertiser_set_timing code: ", status);
    return;
  }

  status = sl_bt_advertiser_set_configuration(adv_handle[HANDLE_EDDYSTONE], LE_GAP_NON_RESOLVABLE);
  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Error eddystone sl_bt_advertiser_set_configuration code: ", status);
    return;
  }

  status = sl_bt_advertiser_start(adv_handle[HANDLE_EDDYSTONE],
                                  advertiser_user_data,
                                  advertiser_non_connectable);
  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Error eddystone sl_bt_advertiser_start code: ", status);
    return;
  }
}

static void enableBleAdvertisements(void)
{
  int16_t tx_power_set_value = 0;
  int16_t tx_power_get_value = 0xFF;
  sl_status_t status;

  /* Set transmit power to 0 dBm */
  status = sl_bt_system_set_max_tx_power(tx_power_set_value, &tx_power_get_value);
  if ( status != SL_STATUS_OK ) {
    emberAfCorePrintln("Unable to set Tx Power to %d. Errorcode:", tx_power_set_value, status);
    return;
  }

  /* Create the device Id and name based on the 16-bit truncated bluetooth address
     Copy to the local GATT database - this will be used by the BLE stack
     to put the local device name into the advertisements, but only if we are
     using default advertisements */
  uint16_t devId;
  bd_addr ble_address;
  static char devName[DEVNAME_LEN];

  status = sl_bt_system_get_identity_address(&ble_address, 0);   //0 : public address
  if ( status != SL_STATUS_OK ) {
    emberAfCorePrintln("Unable to get BLE address. Errorcode: %d", status);
    return;
  }
  devId = ((uint16_t)ble_address.addr[1] << 8) + (uint16_t)ble_address.addr[0];

  snprintf(devName, DEVNAME_LEN, DEVNAME, devId >> 8, devId & 0xff);
  emberAfCorePrintln("devName = %s", devName);
  status = sl_bt_gatt_server_write_attribute_value(gattdb_device_name,
                                                   0,
                                                   strnlen(devName, DEVNAME_LEN),
                                                   (uint8_t *)devName);

  if ( status != SL_STATUS_OK ) {
    emberAfCorePrintln("Unable to sl_bt_gatt_server_write_attribute_value device name. Errorcode: %d", status);
    return;
  }

  dmpUiSetBleDeviceName(devName);   //LCD display

  /* Copy the shortened device name to the response data, overwriting
     the default device name which is set at compile time */
  MEMCOPY(((uint8_t*) &responseData) + 5, devName, 8);

  /* Set the advertising data and scan response data*/
  /* Note that the Wireless Gecko mobile app filters by a specific UUID and
     if the advertising data is not set, the device will not be found on the app*/
  status = sl_bt_advertiser_set_data(adv_handle[HANDLE_DEMO],
                                     0,             //advertising packets
                                     sizeof(responseData),
                                     (uint8_t*) &responseData);

  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Unable to set adv data sl_bt_advertiser_set_data. Errorcode: %d", status);
    return;
  }

  status = sl_bt_advertiser_set_data(adv_handle[HANDLE_DEMO],
                                     1,             //scan response packets
                                     sizeof(responseData),
                                     (uint8_t*) &responseData);

  if (status != SL_STATUS_OK) {
    emberAfCorePrintln("Unable to set scan response data sl_bt_advertiser_set_data. Errorcode: %d", status);
    return;
  }

  /* Set advertising parameters : channel map, min, max intervals, etc */
  status = sl_bt_advertiser_set_channel_map(adv_handle[HANDLE_DEMO], 7);
  if (status != SL_STATUS_OK) {
    return;
  }

  status = sl_bt_advertiser_set_timing(adv_handle[HANDLE_DEMO],
                                       (100 / 0.625), //100ms min adv interval in terms of 0.625ms
                                       (100 / 0.625), //100ms max adv interval in terms of 0.625ms
                                       0,   // duration : continue advertisement until stopped
                                       0);   // max_events :continue advertisement until stopped
  if (status != SL_STATUS_OK) {
    return;
  }
  status = sl_bt_advertiser_set_report_scan_request(adv_handle[HANDLE_DEMO], 1);   //scan request reported as events
  if (status != SL_STATUS_OK) {
    return;
  }
  /* Start advertising in user mode and enable connections*/
  status = sl_bt_advertiser_start(adv_handle[HANDLE_DEMO],
                                  advertiser_user_data,
                                  advertiser_connectable_scannable);

  emberAfCorePrintln("BLE custom advertisements enabled");
  BeaconAdvertisements(devId);
}
