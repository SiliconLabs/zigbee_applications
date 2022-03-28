/***************************************************************************//**
 * @file
 * @brief Commands for executing manufacturing related tests
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
 ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "stack/include/mfglib.h"
#include "app/framework/util/attribute-storage.h"
#ifdef HAL_CONFIG
#include "hal-config.h"
#include "ember-hal-config.h"
#endif
// -----------------------------------------------------------------------------
// Globals
extern EmberStatus mfglibStart(void (*mfglibRxCallback)(uint8_t *packet, uint8_t linkQuality, int8_t rssi));

//MFG UPDATED CODE START -----------------------------------------------------------------------------------------------------------------
#include "rail.h"

uint8_t txData[504] = { 0x0F, 0x0E, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, };
uint16_t txDataLen = 16;
// Data Management
RAIL_DataConfig_t railDataConfig = { .txSource = TX_PACKET_DATA, .rxSource =
    RX_PACKET_DATA, .txMethod = PACKET_MODE, .rxMethod = PACKET_MODE, };

uint32_t rxOverflowDelay = 10 * 1000000; // 10 seconds
RAIL_CsmaConfig_t *csmaConfig = NULL;

extern RAIL_Handle_t emPhyRailHandle;


#define MY_DELAY_IN_MS 1000 //1000 ms = 1 second
static int packetCounter = 0; //used for sent packets
static boolean contPacket = FALSE;
static boolean MODE1 = FALSE;
static boolean MODE2 = FALSE;
static boolean PERtest = FALSE;

static int8_t tempThresh = 0;
static uint16_t expectedPackets = 0;

#ifdef DOXYGEN_SHOULD_SKIP_THIS

/** @brief Enumerations for the possible microcontroller sleep modes.
 * - SLEEPMODE_RUNNING
 *     Everything is active and running.  In practice this mode is not
 *     used, but it is defined for completeness of information.
 * - SLEEPMODE_IDLE
 *     Only the CPU is idled.  The rest of the chip continues running
 *     normally.  The chip will wake from any interrupt.
 * - SLEEPMODE_WAKETIMER
 *     The sleep timer clock sources remain running.  The RC is always
 *     running and the 32kHz XTAL depends on the board header.  Wakeup
 *     is possible from both GPIO and the sleep timer.  System time
 *     is maintained.  The sleep timer is assumed to be configured
 *     properly for wake events.
 * - SLEEPMODE_MAINTAINTIMER
 *     The sleep timer clock sources remain running.  The RC is always
 *     running and the 32kHz XTAL depends on the board header.  Wakeup
 *     is possible from only GPIO.  System time is maintained.
 *       NOTE: This mode is not available on EM2XX chips.
 * - SLEEPMODE_NOTIMER
 *     The sleep timer clock sources (both RC and XTAL) are turned off.
 *     Wakeup is possible from only GPIO.  System time is lost.
 * - SLEEPMODE_HIBERNATE
 *     This maps to EM4 Hibernate on the EFM32/EFR32 devices. RAM is not
 *     retained in SLEEPMODE_HIBERNATE so waking up from this sleepmode
 *     will behave like a reset.
 *       NOTE: This mode is only available on EFM32/EFR32
 */
enum SleepModes
#else
typedef uint8_t SleepModes;
enum
#endif
{
  SLEEPMODE_RUNNING = 0U,
  SLEEPMODE_IDLE = 1U,
  SLEEPMODE_WAKETIMER = 2U,
  SLEEPMODE_MAINTAINTIMER = 3U,
  SLEEPMODE_NOTIMER = 4U,
  SLEEPMODE_HIBERNATE = 5U,

  //The following SleepModes are deprecated on EM2xx and EM3xx chips.  Each
  //micro's halSleep() function will remap these modes to the appropriate
  //replacement, as necessary.
  SLEEPMODE_RESERVED = 6U,
  SLEEPMODE_POWERDOWN = 7U,
  SLEEPMODE_POWERSAVE = 8U,
};

void halSleep(SleepModes sleepMode);
void halInternalSleep(SleepModes sleepMode);

sl_zigbee_event_t packetSend;
void packetSendHandler(void);

//MFG UPDATED CODE END--------------------------------------------------------------------------------------------------------------------


// The max packet size for 802.15.4 is 128, minus 1 byte for the length, and 2 bytes for the CRC.
#define MAX_BUFFER_SIZE 125

// the saved information for the first packet
static uint8_t savedPktLength = 0;
static int8_t savedRssi = 0;
static uint8_t savedLinkQuality = 0;
static uint8_t savedPkt[MAX_BUFFER_SIZE];

static uint16_t mfgCurrentPacketCounter = 0;

static bool inReceivedStream = false;

static bool mfgLibRunning = false;
static bool mfgToneTestRunning = false;
static bool mfgStreamTestRunning = false;

static uint16_t  mfgTotalPacketCounter = 0;

// Add 1 for the length byte which is at the start of the buffer.
ALIGNMENT(4) // efr32xg22 parts and later need 32-bit alignment
static uint8_t   sendBuff[MAX_BUFFER_SIZE + 1];

#define PLUGIN_NAME "Mfglib"

#define MIN_CLI_MESSAGE_SIZE 3
#define MAX_CLI_MESSAGE_SIZE 16

#ifdef UC_BUILD
sl_zigbee_event_t emberAfPluginManufacturingLibraryCliCheckSendCompleteEvent;
#define checkSendCompleteEventControl (&emberAfPluginManufacturingLibraryCliCheckSendCompleteEvent)
void emberAfPluginManufacturingLibraryCliCheckSendCompleteEventHandler(SLXU_UC_EVENT);
#else // !UC_BUILD
EmberEventControl emberAfPluginManufacturingLibraryCliCheckSendCompleteEventControl;
#define checkSendCompleteEventControl emberAfPluginManufacturingLibraryCliCheckSendCompleteEventControl
#endif // UC_BUILD

static uint16_t savedPacketCount = 0;

#define CHECK_SEND_COMPLETE_DELAY_QS 2

// -----------------------------------------------------------------------------
// Forward Declarations

// -----------------------------------------------------------------------------
// External APIs
// Function to determine whether the manufacturing library functionality is
// running.  This is used by the network manager and bulb ui plugins to
// determine if it is safe to kick off joining behavoir.
bool emberAfMfglibRunning(void)
{
  return mfgLibRunning;
}

// Some joining behavoir kicks off before the device can receive a CLI command
// to start the manufacturing library.  Or in the case of devices that use
// UART for CLI access, they may be asleep.  In this case, we need to set a
// token that gives the manufacturing test a window of opportunity to enable
// the manufacturin library.  The idea is that fresh devices can more easily
// allow entry into the manufacturing test modes.  It is also intended to
// disable this token via CLI command at the end of manufacturing test so
// the end customer is not exposed to this functionality.
bool emberAfMfglibEnabled(void)
{
  uint8_t enabled;

#ifndef EMBER_TEST
  halCommonGetToken(&enabled, TOKEN_MFG_LIB_ENABLED);
#else
  return false;
#endif

  (void) emberSerialPrintf(APP_SERIAL,
                           "MFG_LIB Enabled %x\r\n", enabled);

  return enabled;
}

// -----------------------------------------------------------------------------

void emAfPluginManufacturingLibraryCliInitCallback(SLXU_INIT_ARG)
{
  SLXU_INIT_UNUSED_ARG;

  slxu_zigbee_event_init(checkSendCompleteEventControl,
                         emberAfPluginManufacturingLibraryCliCheckSendCompleteEventHandler);
  //MFG UPDATED CODE START -----------------------------------------------------------------------------------------------------------------
  slxu_zigbee_event_init(&packetSend, packetSendHandler);
  //MFG UPDATED CODE START -----------------------------------------------------------------------------------------------------------------
}

// This is unfortunate but there is no callback indicating when sending is complete
// for all packets.  So we must create a timer that checks whether the packet count
// has increased within the last second.

void emberAfPluginManufacturingLibraryCliCheckSendCompleteEventHandler(SLXU_UC_EVENT)
{
  slxu_zigbee_event_set_inactive(checkSendCompleteEventControl);
  if (!inReceivedStream) {
    return;
  }

  if (savedPacketCount == mfgTotalPacketCounter) {
    inReceivedStream = false;
    emberAfCorePrintln("%p Send Complete %d packets",
                       PLUGIN_NAME,
                       mfgCurrentPacketCounter);
    emberAfCorePrintln("First packet: lqi %d, rssi %d, len %d",
                       savedLinkQuality,
                       savedRssi,
                       savedPktLength);
    mfgCurrentPacketCounter = 0;
  } else {
    savedPacketCount = mfgTotalPacketCounter;
    slxu_zigbee_event_set_delay_qs(checkSendCompleteEventControl,
                                   CHECK_SEND_COMPLETE_DELAY_QS);
  }
}

static void fillBuffer(uint8_t* buff, uint8_t length, bool random)
{
  uint8_t i;
  // length byte does not include itself. If the user asks for 10
  // bytes of packet this means 1 byte length, 7 bytes, and 2 bytes CRC
  // this example will have a length byte of 9, but 10 bytes will show
  // up on the receive side
  buff[0] = length;

  for (i = 1; i < length; i += 2) {
    // Two buffer elements per iteration to use both random bytes.
    if (random) {
      uint16_t randomNumber = emberGetPseudoRandomNumber();
      buff[i] = (uint8_t)(randomNumber & 0xFF);
      buff[i + 1] = (uint8_t)((randomNumber >> 8)) & 0xFF;
    } else {
      // Test pattern is ascending integers starting from 1.
      buff[i] = i;
      buff[i + 1] = i + 1;
    }
  }
}

//MFG UPDATED CODE START------------------------------------------------------------------------------------------------------------
static void mfglibRxHandler(uint8_t *packet, uint8_t linkQuality, int8_t rssi) {

  // This increments the total packets for the whole mfglib session
  // this starts when mfglibStart is called and stops when mfglibEnd
  // is called.
  // additional code for emAfMfglibreceivePER

  mfgTotalPacketCounter++;
  mfgCurrentPacketCounter++;
  if (MODE1) {
    emberAfCorePrintln("Packet Received");
  }
  if (MODE2) {
    emberAfCorePrintln(
            "Received message: %s, link Quality: %u, RSSI: %d",
            packet + 5, linkQuality, rssi);

  }

  // If this is the first packet of a transmit group then save the information
  // of the current packet. Don't do this for every packet, just the first one.
    if (!inReceivedStream) {
      inReceivedStream = TRUE;
      mfgCurrentPacketCounter = 1;
      savedRssi = rssi;
      savedLinkQuality = linkQuality;
      savedPktLength = *packet;
      MEMMOVE(savedPkt, (packet + 1), savedPktLength);
  }
}
//MFG UPDATED CODE END------------------------------------------------------------------------------------------------------------


void emberAfMfglibRxStatistics(uint16_t* packetsReceived,
                               int8_t* savedRssiReturn,
                               uint8_t* savedLqiReturn)
{
  *packetsReceived = mfgTotalPacketCounter;
  *savedRssiReturn = savedRssi;
  *savedLqiReturn = savedLinkQuality;
}

void emberAfMfglibStart(bool wantCallback)
{
  EmberStatus status = mfglibStart(wantCallback ? mfglibRxHandler : NULL);
  emberAfCorePrintln("%p start, status 0x%X",
                     PLUGIN_NAME,
                     status);
  if (status == EMBER_SUCCESS) {
    mfgLibRunning = true;
    mfgTotalPacketCounter = 0;
  }
}

void emberAfMfglibStop(void)
{
  EmberStatus status = mfglibEnd();
  emberAfCorePrintln("%p end, status 0x%X",
                     PLUGIN_NAME,
                     status);
  emberAfCorePrintln("rx %d packets while in mfg mode", mfgTotalPacketCounter);
  if (status == EMBER_SUCCESS) {
    mfgLibRunning = false;
  }
}

#ifdef UC_BUILD

#include "app/util/serial/sl_zigbee_command_interpreter.h"

void emAfMfglibStartCommand(sl_cli_command_arg_t *arguments)
{
  bool wantCallback = sl_cli_get_argument_uint8(arguments, 0);

  emberAfMfglibStart(wantCallback);
}

void emAfMfglibStopCommand(sl_cli_command_arg_t *arguments)
{
  emberAfMfglibStop();
}

void emAfMfglibToneStartCommand(sl_cli_command_arg_t *arguments)
{
  EmberStatus status = mfglibStartTone();
  emberAfCorePrintln("%p start tone 0x%X", PLUGIN_NAME, status);
  if (status == EMBER_SUCCESS) {
    mfgToneTestRunning = true;
  }
}

void emAfMfglibToneStopCommand(sl_cli_command_arg_t *arguments)
{
  EmberStatus status = mfglibStopTone();
  emberAfCorePrintln("%p stop tone 0x%X", PLUGIN_NAME, status);
  if (status == EMBER_SUCCESS) {
    mfgToneTestRunning = false;
  }
}

void emAfMfglibStreamStartCommand(sl_cli_command_arg_t *arguments)
{
  EmberStatus status = mfglibStartStream();
  emberAfCorePrintln("%p start stream 0x%X", PLUGIN_NAME, status);
  if (status == EMBER_SUCCESS) {
    mfgStreamTestRunning = true;
  }
}

void emAfMfglibStreamStopCommand(sl_cli_command_arg_t *arguments)
{
  EmberStatus status = mfglibStopStream();
  emberAfCorePrintln("%p stop stream 0x%X", PLUGIN_NAME, status);
  if (status == EMBER_SUCCESS) {
    mfgStreamTestRunning = false;
  }
}

void emAfMfglibSendCommand(sl_cli_command_arg_t *arguments)
{
  bool random = memcmp(sl_cli_get_command_string(arguments, 3 /*arguments->arg_ofs - 1 */),
                       "random", strlen("random")) == 0;
  uint16_t numPackets = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t length = sl_cli_get_argument_uint8(arguments, 1);

  if (length > MAX_BUFFER_SIZE) {
    emberAfCorePrintln("Error: Length cannot be bigger than %d", MAX_BUFFER_SIZE);
    return;
  }

  if (numPackets == 0) {
    emberAfCorePrintln("Error: Number of packets cannot be 0.");
    return;
  }

  fillBuffer(sendBuff, length, random);

  // The second parameter to the mfglibSendPacket() is the
  // number of "repeats", therefore we decrement numPackets by 1.
  numPackets--;
  EmberStatus status = mfglibSendPacket(sendBuff, numPackets);
  emberAfCorePrintln("%p send packet, status 0x%X", PLUGIN_NAME, status);
}

void emAfMfglibSendMessageCommand(sl_cli_command_arg_t *arguments)
{
  size_t length = 0;
  uint8_t *message = sl_cli_get_argument_hex(arguments, 0, &length);
  uint16_t numPackets = sl_cli_get_argument_uint16(arguments, 1);

  if (length < MIN_CLI_MESSAGE_SIZE) {
    emberAfCorePrintln("Error: Minimum length is %d bytes.", MIN_CLI_MESSAGE_SIZE);
    return;
  }

  if (length > MAX_CLI_MESSAGE_SIZE) {
    emberAfCorePrintln("Error: Maximum length is %d bytes.", MAX_CLI_MESSAGE_SIZE);
    return;
  }

  if (numPackets == 0) {
    emberAfCorePrintln("Error: Number of packets cannot be 0.");
    return;
  }

  sendBuff[0] = length + 2; // message length plus 2-byte CRC
  MEMMOVE(sendBuff + 1, message, length);
  numPackets--;
  EmberStatus status = mfglibSendPacket(sendBuff, numPackets);
  emberAfCorePrintln("%p send message, status 0x%X", PLUGIN_NAME, status);
}

void emAfMfglibStatusCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t channel = mfglibGetChannel();
  int8_t power = mfglibGetPower();
  uint16_t powerMode = emberGetTxPowerMode();
  uint8_t options = mfglibGetOptions();
  emberAfCorePrintln("Channel: %d", channel);
  emberAfCorePrintln("Power: %d", power);
  emberAfCorePrintln("Power Mode: 0x%2X", powerMode);
  emberAfCorePrintln("Options: 0x%X", options);
  emberAfCorePrintln("%p running: %p", PLUGIN_NAME, (mfgLibRunning ? "yes" : "no"));
  emberAfCorePrintln("%p tone test running: %p", PLUGIN_NAME, (mfgToneTestRunning ? "yes" : "no"));
  emberAfCorePrintln("%p stream test running: %p", PLUGIN_NAME, (mfgStreamTestRunning ? "yes" : "no"));
  emberAfCorePrintln("Total %p packets received: %d", PLUGIN_NAME, mfgTotalPacketCounter);
}

void emAfMfglibSetChannelCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t channel = sl_cli_get_argument_uint8(arguments, 0);
  EmberStatus status = mfglibSetChannel(channel);
  emberAfCorePrintln("%p set channel, status 0x%X", PLUGIN_NAME, status);
}

void emAfMfglibSetPowerAndModeCommand(sl_cli_command_arg_t *arguments)
{
  int8_t power = sl_cli_get_argument_uint8(arguments, 0);
  uint16_t mode = sl_cli_get_argument_uint16(arguments, 1);
  EmberStatus status = mfglibSetPower(mode, power);
  emberAfCorePrintln("%p set power and mode, status 0x%X", PLUGIN_NAME, status);
}

void emAfMfglibSleepCommand(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  // In UC, Zigbee sits on top of the Legacy HAL component instead of the old
  // Ember HAL (previously located in platform/base). Whereas the Ember HAL
  // would allow applications to essentially force sleep, the Power Manager
  // component does not. We could try requesting it here (by calling
  // sl_power_manager_sleep), but there's no guarantee it would succeed (in
  // fact, it will likely fail under many circumstances). Furthermore, even
  // whether it succeeded requires additional code and increases complexity,
  // leaving aside how to handle the case where it fails. The best course of
  // action is to deprecate this command. I am leaving in place with an error
  // message for now to notify anyone who used it in the past.
  emberAfCorePrintln("Error: %p no longer supports forced sleep", PLUGIN_NAME);
}

// Function to program a custom EUI64 into the chip.
// Example:
// plugin mfglib programEui { 01 02 03 04 05 06 07 08 }
// Note:  this command is OTP.  It only works once.  To re-run, you
// must erase the chip.
void emAfMfglibProgramEuiCommand(sl_cli_command_arg_t *arguments)
{
  EmberEUI64 eui64;

  sl_zigbee_copy_eui64_arg(arguments, 0, eui64, true);

  // potentially verify first few bytes for customer OUI

#ifndef EMBER_TEST
  // OK, we verified the customer OUI.  Let's program it here.
  halInternalSetMfgTokenData(TOKEN_MFG_CUSTOM_EUI_64, (uint8_t *) &eui64, EUI64_SIZE);
#endif
}

void emAfMfglibEnableMfglib(sl_cli_command_arg_t *arguments)
{
#ifndef EMBER_TEST
  uint8_t enabled = sl_cli_get_argument_uint8(arguments, 0);

  halCommonSetToken(TOKEN_MFG_LIB_ENABLED, &enabled);
#endif
}

void emAfMfglibSetOptions(sl_cli_command_arg_t *arguments)
{
  uint8_t options = sl_cli_get_argument_uint8(arguments, 0);
  EmberStatus status = mfglibSetOptions(options);
  emberAfCorePrintln("%p set options, status 0x%X", PLUGIN_NAME, status);
}

#else // !UC_BUILD

// -----------------------------------------------------------------------------
// Forward Declarations

void emAfMfglibStartCommand(void)
{
  bool wantCallback = (bool)emberUnsignedCommandArgument(0);

  emberAfMfglibStart(wantCallback);
}

void emAfMfglibStopCommand(void)
{
  emberAfMfglibStop();
}

void emAfMfglibToneStartCommand(void)
{
  EmberStatus status = mfglibStartTone();
  emberAfCorePrintln("%p start tone 0x%X", PLUGIN_NAME, status);
  if (status == EMBER_SUCCESS) {
    mfgToneTestRunning = true;
  }
}

void emAfMfglibToneStopCommand(void)
{
  EmberStatus status = mfglibStopTone();
  emberAfCorePrintln("%p stop tone 0x%X", PLUGIN_NAME, status);
  if (status == EMBER_SUCCESS) {
    mfgToneTestRunning = false;
  }
}

void emAfMfglibStreamStartCommand(void)
{
  EmberStatus status = mfglibStartStream();
  emberAfCorePrintln("%p start stream 0x%X", PLUGIN_NAME, status);
  if (status == EMBER_SUCCESS) {
    mfgStreamTestRunning = true;
  }
}

void emAfMfglibStreamStopCommand(void)
{
  EmberStatus status = mfglibStopStream();
  emberAfCorePrintln("%p stop stream 0x%X", PLUGIN_NAME, status);
  if (status == EMBER_SUCCESS) {
    mfgStreamTestRunning = false;
  }
}

void emAfMfglibSendCommand(void)
{
  bool random = (emberCommandName()[0] == 'r');
  uint16_t numPackets = (uint16_t)emberUnsignedCommandArgument(0);
  uint8_t length = (uint16_t)emberUnsignedCommandArgument(1);

  if (length > MAX_BUFFER_SIZE) {
    emberAfCorePrintln("Error: Length cannot be bigger than %d", MAX_BUFFER_SIZE);
    return;
  }

  if (numPackets == 0) {
    emberAfCorePrintln("Error: Number of packets cannot be 0.");
    return;
  }

  fillBuffer(sendBuff, length, random);

  // The second parameter to the mfglibSendPacket() is the
  // number of "repeats", therefore we decrement numPackets by 1.
  numPackets--;
  EmberStatus status = mfglibSendPacket(sendBuff, numPackets);
  emberAfCorePrintln("%p send packet, status 0x%X", PLUGIN_NAME, status);
}

void emAfMfglibSendMessageCommand(void)
{
  uint8_t length = 0;
  uint8_t *message = emberStringCommandArgument(0, &length);
  uint16_t numPackets = (uint16_t)emberUnsignedCommandArgument(1);

  if (length < MIN_CLI_MESSAGE_SIZE) {
    emberAfCorePrintln("Error: Minimum length is %d bytes.", MIN_CLI_MESSAGE_SIZE);
    return;
  }

  if (length > MAX_CLI_MESSAGE_SIZE) {
    emberAfCorePrintln("Error: Maximum length is %d bytes.", MAX_CLI_MESSAGE_SIZE);
    return;
  }

  if (numPackets == 0) {
    emberAfCorePrintln("Error: Number of packets cannot be 0.");
    return;
  }

  sendBuff[0] = length + 2; // message length plus 2-byte CRC
  MEMMOVE(sendBuff + 1, message, length);
  numPackets--;
  EmberStatus status = mfglibSendPacket(sendBuff, numPackets);
  emberAfCorePrintln("%p send message, status 0x%X", PLUGIN_NAME, status);
}

void emAfMfglibStatusCommand(void)
{
  uint8_t channel = mfglibGetChannel();
  int8_t power = mfglibGetPower();
  uint16_t powerMode = emberGetTxPowerMode();
  uint8_t options = mfglibGetOptions();
  emberAfCorePrintln("Channel: %d", channel);
  emberAfCorePrintln("Power: %d", power);
  emberAfCorePrintln("Power Mode: 0x%2X", powerMode);
  emberAfCorePrintln("Options: 0x%X", options);
  emberAfCorePrintln("%p running: %p", PLUGIN_NAME, (mfgLibRunning ? "yes" : "no"));
  emberAfCorePrintln("%p tone test running: %p", PLUGIN_NAME, (mfgToneTestRunning ? "yes" : "no"));
  emberAfCorePrintln("%p stream test running: %p", PLUGIN_NAME, (mfgStreamTestRunning ? "yes" : "no"));
  emberAfCorePrintln("Total %p packets received: %d", PLUGIN_NAME, mfgTotalPacketCounter);
}

void emAfMfglibSetChannelCommand(void)
{
  uint8_t channel = (uint8_t)emberUnsignedCommandArgument(0);
  EmberStatus status = mfglibSetChannel(channel);
  emberAfCorePrintln("%p set channel, status 0x%X", PLUGIN_NAME, status);
}

void emAfMfglibSetPowerAndModeCommand(void)
{
  int8_t power = (int8_t)emberSignedCommandArgument(0);
  uint16_t mode = (uint16_t)emberUnsignedCommandArgument(1);
  EmberStatus status = mfglibSetPower(mode, power);
  emberAfCorePrintln("%p set power and mode, status 0x%X", PLUGIN_NAME, status);
}

void emAfMfglibSleepCommand(void)
{
  uint32_t sleepDurationMS = (uint32_t)emberUnsignedCommandArgument(0);

  // turn off the radio
  emberStackPowerDown();

  ATOMIC(
    // turn off board and peripherals
    halPowerDown();
    // turn micro to power save mode - wakes on external interrupt
    // or when the time specified expires
    halSleepForMilliseconds(&sleepDurationMS);
    // power up board and peripherals
    halPowerUp();
    );
  // power up radio
  emberStackPowerUp();

  emberAfEepromNoteInitializedStateCallback(false);

  // Allow the stack to time out any of its events and check on its
  // own network state.
  emberTick();
}

// Function to program a custom EUI64 into the chip.
// Example:
// plugin mfglib programEui { 01 02 03 04 05 06 07 08 }
// Note:  this command is OTP.  It only works once.  To re-run, you
// must erase the chip.
void emAfMfglibProgramEuiCommand(void)
{
  EmberEUI64 eui64;

  emberAfCopyBigEndianEui64Argument(0, eui64);

  // potentially verify first few bytes for customer OUI

#ifndef EMBER_TEST
  // OK, we verified the customer OUI.  Let's program it here.
  halInternalSetMfgTokenData(TOKEN_MFG_CUSTOM_EUI_64, (uint8_t *) &eui64, EUI64_SIZE);
#endif
}

void emAfMfglibEnableMfglib(void)
{
#ifndef EMBER_TEST
  uint8_t enabled = (uint8_t) emberSignedCommandArgument(0);

  halCommonSetToken(TOKEN_MFG_LIB_ENABLED, &enabled);
#endif
}

void emAfMfglibSetOptions(void)
{
  uint8_t options = (uint8_t)emberUnsignedCommandArgument(0);
  EmberStatus status = mfglibSetOptions(options);
  emberAfCorePrintln("%p set options, status 0x%X", PLUGIN_NAME, status);
}

#endif // UC_BUILD

//MFG UPDATED CODE START -----------------------------------------------------------------------------------------------------------------

//Send set number of packets(set from argument) with message indicating the number sent
void emAfMfglibPERTest(sl_cli_command_arg_t *arguments) {

  uint16_t numPackets = sl_cli_get_argument_uint16(arguments, 0);
  uint16_t interval = sl_cli_get_argument_uint16(arguments, 1);
  char str[5];
  char sig[15] = "test";
  EmberStatus status;
  emberAfCorePrintln("per test started.");

  for (int i = 1; i <= numPackets; i++) {
    //HALCommonDelayMS in api guide; give user option to change delay between sent packets
    halCommonDelayMilliseconds(interval);
    sprintf(str, "%d", i);
    strcat(sig, str);
    sendBuff[0] = strlen(sig) + 2;
    MEMMOVE(sendBuff + 1, sig, strlen(sig));
    status = mfglibSendPacket(sendBuff, 0);
    sig[4] = '\0';
    packetCounter++;
  }

  emberAfCorePrintln("%p send message, status 0x%X", PLUGIN_NAME, status);
}

void emAfMfglibReceiveStart(sl_cli_command_arg_t *arguments) {
  PERtest = TRUE;
  uint16_t expected = sl_cli_get_argument_uint16(arguments, 0);
  expectedPackets = expected;
  mfgTotalPacketCounter = 0;
  emberAfCorePrintln("Receive mode has been started, packets have been cleared.");
}

void emAfMfglibReceiveStop(void) {
  uint16_t lostPackets;
  //for calculating packets and handling errors
  if (expectedPackets < mfgTotalPacketCounter) {
    lostPackets = 0;
  } else {
    lostPackets = (expectedPackets - mfgTotalPacketCounter)*100; //multiply by 100 to get percentage
  }
  //PER print out
  if (PERtest == TRUE){
    emberAfCorePrintln("Packet counter %d",
        mfgTotalPacketCounter);
    emberAfCorePrintln("Expected packets %d",
        expectedPackets);
    emberAfCorePrintln("The Packet Error rate is %d.%d percent", lostPackets / expectedPackets, lostPackets % expectedPackets);
    expectedPackets = 0;
  } else {
    emberAfCorePrintln("Error: PER test has not been started yet");
  }
}

void emAfMfglibClearPackets(void) { // clear-packets
  mfgTotalPacketCounter = 0;
  emberAfCorePrintln("Packets cleared!");
}

void emAfMfglibSetPower(sl_cli_command_arg_t *arguments) { // set-power
  int32_t powerLevel = sl_cli_get_argument_int32(arguments, 0);
  int32_t actualPower = RAIL_SetTxPowerDbm(emPhyRailHandle, powerLevel);
  if (actualPower == 0){
      emberAfCorePrintln("The tx power has been set to %d.%d dBm.",
                         powerLevel / 10, powerLevel % 10);
  } else {
      emberAfCorePrintln("Set power failed, error 0x%X", actualPower);
  }
}

void emAfMfglibGetPower(sl_cli_command_arg_t *arguments) { // get-power
  int32_t currPower = RAIL_GetTxPowerDbm(emPhyRailHandle);
  emberAfCorePrintln("The tx power is %d.%d dBm.", currPower / 10,
      currPower % 10);
}

void emAfMfglibSetCcaThresholdReg(sl_cli_command_arg_t *arguments) { // set-cca
  tempThresh = sl_cli_get_argument_int8(arguments, 0);
  RAIL_SetCcaThreshold(emPhyRailHandle, tempThresh);
  emberAfCorePrintln("The temporary CCA threshold has been set to %d.",
      tempThresh);
}

void emAfMfglibGetCcaThresholdReg(void) { // get-cca
  if (tempThresh != 0) {
    emberAfCorePrintln("The temporary CCA threshold is %d.", tempThresh);
  } else {
    emberAfCorePrintln("Error: The temporary CCA threshold is NULL");
  }
}

void emAfMfglibGetCtuneValueReg(void) { // get-ctune-reg
  uint16_t val;
  val = halInternalGetCtune();
  emberAfCorePrintln("The temporary Ctune value is %d.", val);
}

void emAfMfglibSetCtuneValueReg(sl_cli_command_arg_t *arguments) { // set-ctune-reg
  uint16_t tune = sl_cli_get_argument_uint16(arguments, 0);
  halInternalSetCtune(tune);
  emberAfCorePrintln("The temporary Ctune value has been set.");
}

void emAfMfglibGetCcaThresholdTok(void) { //get-cca-tok
  tokTypeMfgCTune threshold = 0xFFFF;
  halCommonGetMfgToken(&threshold, TOKEN_MFG_CCA_THRESHOLD);
  if (threshold != 0xFFFF) {
    emberAfCorePrintln("The CCA threshold token is %d.", (int16_t)threshold);
  } else {
    emberAfCorePrintln("The CCA Threshold token is NULL and can be set.");
  }
}

void emAfMfglibSetCcaThresholdTok(sl_cli_command_arg_t *arguments) { // set-cca-tok
  tokTypeMfgCTune threshold_new = sl_cli_get_argument_uint16(arguments, 0);
  tokTypeMfgCTune threshold_old = 0xFFFF;
  halCommonGetMfgToken(&threshold_old, TOKEN_MFG_CCA_THRESHOLD);
  if (threshold_old != 0xFFFF) {
    emberAfCorePrintln(
        "The CCA threshold token had already been set previously and therefore not set again!");
  } else {
    halCommonSetMfgToken(TOKEN_MFG_CCA_THRESHOLD, &threshold_new);
    emberAfCorePrintln("The CCA threshold token has been set.");
  }
}

void emAfMfglibGetCtuneValueTok(void) { // get-ctune-tok
  tokTypeMfgCTune value = 0xFFFF;
  halCommonGetMfgToken(&value, TOKEN_MFG_CTUNE);
  if (value != 0xFFFF) {
    emberAfCorePrintln("The CTUNE value token is %d.", value);
  } else {
    emberAfCorePrintln("The CTUNE value token is NULL and can be set.");
  }
}

void emAfMfglibSetCtuneValueTok(sl_cli_command_arg_t *arguments) { // set-ctune-tok
  tokTypeMfgCTune value_new = sl_cli_get_argument_uint16(arguments, 0);
  tokTypeMfgCTune value_old = 0xFFFF;
  halCommonGetToken(&value_old, TOKEN_MFG_CTUNE);
  if (value_old != 0xFFFF) {
    emberAfCorePrintln(
        "The CTUNE value token had already been set previously and therefore not set again!");
  } else {
    halCommonSetToken(TOKEN_MFG_CTUNE, &value_new);
    emberAfCorePrintln("The CTUNE value token has been set.");
  }
}

void packetSendHandler(void)
{
  char str[5];
  char sig[15] = "test";
  EmberStatus status;

  sl_zigbee_event_set_inactive(&packetSend);

  if (contPacket) {

    packetCounter++;
    sprintf(str, "%d", packetCounter);
    strcat(sig, str);
    sendBuff[0] = strlen(sig) + 2;
    MEMMOVE(sendBuff + 1, sig, strlen(sig));
    status = mfglibSendPacket(sendBuff, 0);
    sig[4] = '\0';

    emberAfCorePrintln("%p send packet, status 0x%X", PLUGIN_NAME, status);
  }

  //Reschedule the event after a delay of 1 seconds
  sl_zigbee_event_set_delay_ms(&packetSend,  MY_DELAY_IN_MS);
}

//Sends packets continuously, set in milliseconds
void emAfMfglibContinuousPacket(void) {
  if (!mfgLibRunning) {
    emberAfCorePrintln("mfglib test not started!");
    return;
  } else if (contPacket) {
    emberAfCorePrintln("Continuous packet test already started!");
    return;
  }
  contPacket = TRUE;
  emberAfCorePrintln("Continuous packet test started!");
  packetCounter = 0;
  packetSendHandler();
}

//stops continuous packets
void emAfMfglibStopContinuous(void) {

  if (!contPacket) {
    emberAfCorePrintln("Continuous test is not in progress.");
    return;
  }
  contPacket = FALSE;
  emberAfCorePrintln("Continuous packet testing ended :(");
  emberAfCorePrintln("Packet Counter: %u", packetCounter);

}

void emAfMfglibClearPacketCounter(void) { //
  packetCounter = 0;
  emberAfCorePrintln("Packet Counter: %u", packetCounter);
}

void emAfMfglibGetPackets(void) { //
  emberAfCorePrintln("Packet Counter: %u", packetCounter);
}

void emAfMfglibReceiveMode(sl_cli_command_arg_t *arguments)
{
  int8_t mode = sl_cli_get_argument_int8(arguments, 0);
  if (mode == 0) {
    MODE1 = FALSE;
    MODE2 = FALSE;
    emberAfCorePrintln("No output for packets.");
  } else if (mode == 1) {
    MODE1 = TRUE;
    MODE2 = FALSE;
    emberAfCorePrintln("Confirmation Println for each packet.");
  } else if (mode == 2) {
    MODE1 = FALSE;
    MODE2 = TRUE;
    emberAfCorePrintln(
        "Prints packet number, Link Quality and RSSI for each packet");
  } else {
    emberAfCorePrintln("That is not a valid mode.");
  }
}

void emAfMfglibSetGpio(sl_cli_command_arg_t *arguments) {
  /*
   * AVAILABLE PORTS:
   * Port A = 0
   * Port B = 1
   * Port C = 2
   * Port D = 3
   * Port E = 4
   * Port F = 5
   * Port G = 6
   * Port H = 7
   * Port I = 8
   * Port J = 9
   * Port K = 10
   */
  GPIO_Port_TypeDef port = (GPIO_Port_TypeDef) sl_cli_get_argument_uint32(arguments, 0);
  /*
   * AVAILABLE PINS:
   * 0 - 15
   */
  uint32_t pin = sl_cli_get_argument_uint32(arguments,1);
  /*
   * PIN MODES:
   * gpioModeDisabled = 0
   *
   */
  GPIO_Mode_TypeDef mode = (GPIO_Mode_TypeDef) sl_cli_get_argument_uint32(arguments, 2);
  uint32_t out = sl_cli_get_argument_uint32(arguments, 3);
  GPIO_PinModeSet(port, pin, mode, out);
  emberAfCorePrintln("GPIO settings have been applied.");
}

void emAfMfglibGetGpio(sl_cli_command_arg_t *arguments) {
  /*
   * POSSIBLE PORTS:
   * Port A = 0
   * Port B = 1
   * Port C = 2
   * Port D = 3
   * Port E = 4
   * Port F = 5
   * Port G = 6
   * Port H = 7
   * Port I = 8
   * Port J = 9
   * Port K = 10
   */
  GPIO_Port_TypeDef port = (GPIO_Port_TypeDef) sl_cli_get_argument_uint32(arguments, 0);
  /*
   * AVAILABLE PINS:
   * 0 - 15
   */
  uint32_t pin = sl_cli_get_argument_uint32(arguments, 1);
  GPIO_Mode_TypeDef mode = GPIO_PinModeGet(port, pin);
  emberAfCorePrintln("The specified port is in mode %d.", (uint32_t )mode);
}

void emAfMfglibGpioHelp(void) {
  emberAfCorePrintln(
      "Possible ports:\nPort A = 0\nPort B = 1\nPort C = 2\nPort D = 3\nPort E = 4\nPort F = 5\nPort G = 6\nPort H = 7\nPort I = 8\nPort J = 9\nPort K = 10\n");
  emberAfCorePrintln("AVAILABLE PINS: 0 - 15\n");
  emberAfCorePrintln(
      "PIN MODES:\ngpioModeDisabled = 0\ngpioModeInput = 1\ngpioModeInputPull = 2\n"
          "gpioModeInputPullFilter = 3\ngpioModePushPull = 4\ngpioModePushPullAlternate = 5\ngpioModeWiredOr = 6\n"
          "gpioModeWiredOrPullDown = 7\ngpioModeWiredAnd = 8\ngpioModeWiredAndFilter = 9\ngpioModeWiredAndPullUp = 10\n"
          "gpioModeWiredAndPullUpFilter = 11\ngpioModeWiredAndAlternate = 12\ngpioModeWiredAndAlternateFilter = 13\n"
          "gpioModeWiredAndAlternatePullUp = 14\ngpioModeWiredAndAlternatePullUpFilter = 15\n");
  emberAfCorePrintln("DOUT PIN: Low = 0, High = 1\n");
}

void emAfMfglibTokDump(void) {
  //We define DEFINETOKENS so when token-stack.h is included below, only the
  //token definitions are operated on as opposed to the token types.
#define DEFINETOKENS
  //Treat the manufacturing tokens just like a normal token definition.
#define TOKEN_MFG TOKEN_DEF
  //Turn each token's definition into a block of code which performs
  //an internal GetTokenData call to read out that token and print it.
#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  {                                                                    \
  int8_t j;                                                          \
  /* Create an exact size array for the local copy of the data. */   \
  uint8_t i, dst[sizeof(type)];                                      \
  /* Tell me the creator code and name of the token. */              \
  emberAfCorePrint("\n{ [%2X] ", creator);    \
  emberAfCorePrint("{%s:", #name);                                     \
  /* If this is an indexed token, we're going to loop over each */   \
  /* index.  If this is not an indexed token, the array size    */   \
  /* should be 1 so we'll only perform this loop once.          */   \
  for (j = 0; j < arraysize; j++) {                                  \
    if (arraysize == 1) {                                            \
    /* We're trying to access a non-indexed token (denoted by  */  \
    /* arraysize = 1).  The index parameter is zero because as */  \
    /* a non-indexed token, we do not want to offset.          */  \
    halInternalGetTokenData(dst, TOKEN_##name, 0, sizeof(type));   \
    } else {                                                         \
    /* We're trying to access an indexed token.  The index */      \
    /* parameter is being looped over with the for loop.   */     \
      emberAfCorePrint("[index %d] ", j);       \
    halInternalGetTokenData(dst, TOKEN_##name, j, sizeof(type));   \
    }                                                                \
    /* Print out the token data we just obtained. */                 \
    for (i = 0; i < sizeof(type); i++) {                             \
      emberAfCorePrint("%X", dst[i]);                                  \
    }                                                                \
    emberAfCorePrintln("}}\r\n");   \
  }                                                                  \
  emberAfCorePrintln("}}\r\n");   \
  }

  //Now that we've defined the tokens to be a block of code, pull them in
#include "stack/config/token-stack.h"
  //Release the manufacturing token definition.
#undef TOKEN_MFG
  //Release the normal token definition.
#undef TOKEN_DEF
  //Release the token definition blocks.
#undef DEFINETOKENS
}

void emAfMfglibSleepTest(sl_cli_command_arg_t *arguments) {
  uint8_t mode = sl_cli_get_argument_uint8(arguments, 0);
  if (mode > 5) {
    emberAfCorePrintln("Invalid sleep mode.");
  } else {
    emberAfCorePrintln("Entering sleep mode.");
    halCommonDelayMicroseconds(1000);
    halSleep(mode);
  }
}

void halSleep(SleepModes sleepMode)
{
  halInternalSleep(sleepMode);
}

void halInternalSleep(SleepModes sleepMode)
{
  sl_power_manager_em_t em_power = SL_POWER_MANAGER_EM0;

  if (sleepMode == SLEEPMODE_IDLE) {
    em_power = SL_POWER_MANAGER_EM1;
  } else if (sleepMode == SLEEPMODE_WAKETIMER
             || sleepMode == SLEEPMODE_MAINTAINTIMER) {
    em_power = SL_POWER_MANAGER_EM2;
  } else if (sleepMode == SLEEPMODE_NOTIMER) {
    em_power = SL_POWER_MANAGER_EM3;
  } else if (sleepMode == SLEEPMODE_HIBERNATE) {
    // Only a power on reset or external reset pin can wake the device from EM4.
    EMU_EnterEM4();
  }

  //Add a requirement
  sl_power_manager_add_em_requirement(em_power);

  // The sleep functions will often be entered with interrupts turned off (via
  // BASEPRI). Our API documentation states that these functions will exit with
  // interupts on. Furthermore, Cortex-M processors will not wake up from an IRQ
  // if it's blocked by the current BASEPRI. However, we still want to run some
  // ode (including capturing the wake reasons) after waking but before
  // interrupts run. We therefore will enter sleep with PRIMASK set and BASEPRI
  // cleared.
  CORE_CriticalDisableIrq();
  INTERRUPTS_ON();

  //Go to sleep
  sl_power_manager_sleep();

  // Renable interrupts.
  CORE_CriticalEnableIrq();

  //Remove the previous requirement
  sl_power_manager_remove_em_requirement(em_power);
}

void emAfMfglibEnterBootloader() {

  halInternalSysReset(RESET_BOOTLOADER_BOOTLOAD);
}

//MFG UPDATED CODE END--------------------------------------------------------------------------------------------------------------------
