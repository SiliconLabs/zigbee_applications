/***************************************************************************//**
 * @file manufacturing-library-cli-soc.c
 * @brief Commands for executing manufacturing related tests
 * @version 1.0.1
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
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
//MFG UPDATED CODE START-------------------------------------------------------
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

static int8_t tempThresh = NULL;
static uint16_t expectedPackets = 0;
//MFG UPDATED CODE END-------------------------------------------------------
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
ALIGNMENT(4) // em3xx needs 16-bit alignment; efr32xg22 needs 32-bit
static uint8_t   sendBuff[MAX_BUFFER_SIZE + 1];

#define PLUGIN_NAME "Mfglib"

#define MIN_CLI_MESSAGE_SIZE 3
#define MAX_CLI_MESSAGE_SIZE 16

EmberEventControl emberAfPluginManufacturingLibraryCliCheckSendCompleteEventControl;
#define checkSendCompleteEventControl emberAfPluginManufacturingLibraryCliCheckSendCompleteEventControl

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

  emberSerialPrintf(APP_SERIAL,
                    "MFG_LIB Enabled %x\r\n", enabled);

  return enabled;
}

// -----------------------------------------------------------------------------

// This is unfortunate but there is no callback indicating when sending is complete
// for all packets.  So we must create a timer that checks whether the packet count
// has increased within the last second.
void emberAfPluginManufacturingLibraryCliCheckSendCompleteEventHandler(void)
{
  emberEventControlSetInactive(checkSendCompleteEventControl);
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
    emberEventControlSetDelayQS(checkSendCompleteEventControl,
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

	uint8_t packetNumStr[10];

	// This increments the total packets for the whole mfglib session
	// this starts when mfglibStart is called and stops when mfglibEnd
	// is called.
	//additional code for emAfMfglibreceivePER
	uint8_t *sig = "test";
	uint8_t *str = packet + 4;


	mfgTotalPacketCounter++;
	mfgCurrentPacketCounter++;
	if (MODE1) {
		emberAfCorePrintln("Packet Received");
	}
	if (MODE2) {
		emberAfCorePrintln(
						"Received message: %s, link Quality: %u, RSSI: %d, sig: %d",
						packet + 4, linkQuality, rssi, strncmp(sig, packet, 4));

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
//MFG UPDATED CODE END-------------------------------------------------------

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

void emAfMfglibStartCommand(void)
{
  bool wantCallback = (bool)emberUnsignedCommandArgument(0);

  emberAfMfglibStart(wantCallback);
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


//MFG UPDATED CODE START------------------------------------------------------------------------------------

//Send set number of packets(set from argument) with message indicating the number sent
void emAfMfglibPERTest(void) {
	uint16_t numPackets = (uint16_t) emberUnsignedCommandArgument(0);
	uint16_t interval = (uint16_t) emberUnsignedCommandArgument(1);
	uint8_t str[5];
	uint8_t sig[15] = "test";
	EmberStatus status;
	emberAfCorePrintln("per test started.");
	for (int i = 1; i <= numPackets; i++) {
		//HALCommonDelayMS in api guide; give user option to change delay between sent packets
		halCommonDelayMilliseconds(interval);
		sprintf(str, "%d", i);
		strcat(sig, str);
		status = mfglibSendPacket(sig, 0);
		sig[4] = '\0';
		packetCounter++;
	}

	emberAfCorePrintln("%p send message, status 0x%X", PLUGIN_NAME, status);
}

void emAfMfglibReceiveStart(void) {
	PERtest = TRUE;
	uint16_t expected = (uint16_t) emberUnsignedCommandArgument(0);
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

void emAfMfglibSetPower(void) { // set-power
	int32_t powerLevel = (int32_t) emberUnsignedCommandArgument(0);
	int32_t actualPower = RAIL_SetTxPowerDbm(emPhyRailHandle, powerLevel);
	emberAfCorePrintln("The tx power has been set to %d.%d dBm.",
			actualPower / 10, actualPower % 10);
}

void emAfMfglibGetPower(void) { // get-power
	int32_t currPower = RAIL_GetTxPowerDbm(emPhyRailHandle);
	emberAfCorePrintln("The tx power is %d.%d dBm.", currPower / 10,
			currPower % 10);
}

void emAfMfglibSetCcaThresholdReg(void) { // set-cca
	tempThresh = (int8_t) emberSignedCommandArgument(0);
	RAIL_SetCcaThreshold(emPhyRailHandle, tempThresh);
	emberAfCorePrintln("The temporary CCA threshold has been set to %d.",
			tempThresh);
}

void emAfMfglibGetCcaThresholdReg(void) { // get-cca
	if (tempThresh != NULL) {
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

void emAfMfglibSetCtuneValueReg(void) { // set-ctune-reg
	uint16_t tune = (uint16_t) emberUnsignedCommandArgument(0);
	halInternalSetCtune(tune);
	emberAfCorePrintln("The temporary Ctune value has been set.");
}

void emAfMfglibGetCcaThresholdTok(void) { //get-cca-tok
	tokTypeMfgCTune threshold = 0xFFFF;
	halCommonGetMfgToken(&threshold, TOKEN_MFG_CCA_THRESHOLD);
	if (threshold != 0xFFFF) {
		emberAfCorePrintln("The CCA threshold token is %d.", threshold);
	} else {
		emberAfCorePrintln("The CCA Threshold token is NULL and can be set.");
	}
}

void emAfMfglibSetCcaThresholdTok(void) { // set-cca-tok
	tokTypeMfgCTune threshold_new = (uint16_t) emberUnsignedCommandArgument(0);
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

void emAfMfglibSetCtuneValueTok(void) { // set-ctune-tok
	tokTypeMfgCTune value_new = (uint16_t) emberUnsignedCommandArgument(0);
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

EmberEventControl packetSend;


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


void packetSendHandler(void)
{
		uint8_t str[5];
		uint8_t sig[15] = "test";
		EmberStatus status;

		emberEventControlSetInactive(packetSend);
		if (contPacket) {

			sprintf(str, "%d", packetCounter);
			strcat(sig, str);
			status = mfglibSendPacket(sig, 0);
			sig[4] = '\0';

			packetCounter++;
			emberAfCorePrintln("%p send packet, status 0x%X", PLUGIN_NAME, status);
		}

	  //Reschedule the event after a delay of 1 seconds
	  emberEventControlSetDelayMS(packetSend,  MY_DELAY_IN_MS);

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

void emAfMfglibReceiveMode(void)
{
	int8_t mode = (int8_t) emberUnsignedCommandArgument(0);
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
				"Prints Link Quality, RSSI, and packet length for each packet");
	} else {
		emberAfCorePrintln("That is not a valid mode.");
	}
}

void emAfMfglibSetGpio(void) {
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
	GPIO_Port_TypeDef port = (GPIO_Port_TypeDef) emberUnsignedCommandArgument(
			0);
	/*
	 * AVAILABLE PINS:
	 * 0 - 15
	 */
	uint32_t pin = (uint32_t) emberUnsignedCommandArgument(1);
	/*
	 * PIN MODES:
	 * gpioModeDisabled = 0
	 *
	 */
	GPIO_Mode_TypeDef mode = (GPIO_Mode_TypeDef) emberUnsignedCommandArgument(
			2);
	uint32_t out = (uint32_t) emberUnsignedCommandArgument(3);
	GPIO_PinModeSet(port, pin, mode, out);
	emberAfCorePrintln("GPIO settings have been applied.");
}

void emAfMfglibGetGpio(void) {
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
	GPIO_Port_TypeDef port = (GPIO_Port_TypeDef) emberUnsignedCommandArgument(
			0);
	/*
	 * AVAILABLE PINS:
	 * 0 - 15
	 */
	uint32_t pin = (uint32_t) emberUnsignedCommandArgument(1);
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
		/* parameter is being looped over with the for loop.   */   	\
	    emberAfCorePrint("[index %d] ", j);				\
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

void emAfMfglibSleepTest(void) {
	uint8_t mode = (uint8_t) emberUnsignedCommandArgument(0);
	if (mode > 5) {
		emberAfCorePrintln("Invalid sleep mode.");
	} else {
		emberAfCorePrintln("Entering sleep mode.");
		halCommonDelayMicroseconds(1000);
		halSleep(mode);
	}
}

void emAfMfglibEnterBootloader() {

	halInternalSysReset(RESET_BOOTLOADER_BOOTLOAD);
}
