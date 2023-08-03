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
#include "sl_string.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "rail.h"

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "stack/include/mfglib.h"
#include \
  "app/framework/plugin/manufacturing-library-cli/manufacturing-library-cli-plugin.h"
// -----------------------------------------------------------------------------
// Globals

#define MY_DELAY_IN_MS 1000   // 1000 ms = 1 second
static int packetCounter = 0;   // used for sent packets
static boolean contPacket = FALSE;
static boolean MODE1 = FALSE;
static boolean MODE2 = FALSE;
static boolean PERtest = FALSE;

static int8_t tempThresh = 0;
static uint16_t expectedPackets = 0;

typedef uint8_t SleepModes;
enum
{
  SLEEPMODE_RUNNING = 0U,
  SLEEPMODE_IDLE = 1U,
  SLEEPMODE_WAKETIMER = 2U,
  SLEEPMODE_MAINTAINTIMER = 3U,
  SLEEPMODE_NOTIMER = 4U,
  SLEEPMODE_HIBERNATE = 5U,
  SLEEPMODE_RESERVED = 6U,
  SLEEPMODE_POWERDOWN = 7U,
  SLEEPMODE_POWERSAVE = 8U,
};

extern RAIL_Handle_t emPhyRailHandle;

void halInternalSleep(SleepModes sleepMode);
void packetSendHandler(void);

sl_zigbee_event_t packetSend;

// The max packet size for 802.15.4 is 128, minus 1 byte for the length, and 2
//   bytes for the CRC.
#define MAX_BUFFER_SIZE 125

static uint16_t  mfgTotalPacketCounter = 0;

// Add 1 for the length byte which is at the start of the buffer.
ALIGNMENT(4) // efr32xg22 parts and later need 32-bit alignment
static uint8_t   sendBuff[MAX_BUFFER_SIZE + 1];

#define PLUGIN_NAME          "Mfglib"

#define MIN_CLI_MESSAGE_SIZE 3
#define MAX_CLI_MESSAGE_SIZE 16

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

// Send set number of packets(set from argument) with message indicating the
//   number sent
void emAfMfglibPERTest(sl_cli_command_arg_t *arguments)
{
  uint16_t numPackets = sl_cli_get_argument_uint16(arguments, 0);
  uint16_t interval = sl_cli_get_argument_uint16(arguments, 1);
  char str[11];
  char sig[15] = "test";
  EmberStatus status = EMBER_SUCCESS;
  emberAfCorePrintln("per test started.");

  for (int i = 1; i <= numPackets; i++) {
    // HALCommonDelayMS in api guide; give user option to change delay between
    //   sent packets
    halCommonDelayMilliseconds(interval);
    snprintf(str, sizeof(str), "%d", i);
    sl_strcat_s(sig, sizeof(sig), str);
    // length byte does not include itself, it includes data bytes to send,
    // 2 bytes CRC and a NULL terminator.
    sendBuff[0] = sl_strlen(sig) + 1 + 2;
    MEMMOVE(sendBuff + 1, sig, sl_strlen(sig) + 1);
    status = mfglibSendPacket(sendBuff, 0);
    sig[4] = '\0';
    packetCounter++;
  }

  emberAfCorePrintln("%p send message, status 0x%X", PLUGIN_NAME, status);
}

void emAfMfglibReceiveStart(sl_cli_command_arg_t *arguments)
{
  PERtest = TRUE;
  uint16_t expected = sl_cli_get_argument_uint16(arguments, 0);
  expectedPackets = expected;
  mfgTotalPacketCounter = 0;
  emberAfCorePrintln("Receive mode has been started, packets have been cleared.");
}

void emAfMfglibReceiveStop(void)
{
  uint16_t lostPackets;
  // for calculating packets and handling errors
  if (expectedPackets < mfgTotalPacketCounter) {
    lostPackets = 0;
  } else {
    lostPackets = expectedPackets - mfgTotalPacketCounter;
  }
  // PER print out
  if (PERtest == TRUE) {
    emberAfCorePrintln("Packet counter %d",
                       mfgTotalPacketCounter);
    emberAfCorePrintln("Expected packets %d",
                       expectedPackets);
    emberAfCorePrintln("The Packet Error rate is %d.%d percent",
                       // multiply to get percentage
                       lostPackets * 100 / expectedPackets,
                       // multiply to get percentage
                       (lostPackets * 1000 / expectedPackets) % 10);
    expectedPackets = 0;
  } else {
    emberAfCorePrintln("Error: PER test has not been started yet");
  }
}

void emAfMfglibClearPackets(void)  // clear-packets
{
  mfgTotalPacketCounter = 0;
  emberAfCorePrintln("Packets cleared!");
}

void emAfMfglibSetPower(sl_cli_command_arg_t *arguments)  // set-power
{
  int32_t powerLevel = sl_cli_get_argument_int32(arguments, 0);
  RAIL_Status_t status = RAIL_SetTxPowerDbm(emPhyRailHandle, powerLevel);
  if (status == RAIL_STATUS_NO_ERROR) {
    emberAfCorePrintln("The tx power has been set to %d.%d dBm.",
                       powerLevel / 10, powerLevel % 10);
  } else {
    emberAfCorePrintln("Set power failed, error 0x%X", status);
  }
}

void emAfMfglibGetPower(sl_cli_command_arg_t *arguments)  // get-power
{
  RAIL_TxPower_t currPower = RAIL_GetTxPowerDbm(emPhyRailHandle);
  emberAfCorePrintln("The tx power is %d.%d dBm.", currPower / 10,
                     currPower % 10);
}

void emAfMfglibSetCcaThresholdReg(sl_cli_command_arg_t *arguments)  // set-cca
{
  tempThresh = sl_cli_get_argument_int8(arguments, 0);
  RAIL_SetCcaThreshold(emPhyRailHandle, tempThresh);
  emberAfCorePrintln("The temporary CCA threshold has been set to %d.",
                     tempThresh);
}

void emAfMfglibGetCcaThresholdReg(void)  // get-cca
{
  if (tempThresh != 0) {
    emberAfCorePrintln("The temporary CCA threshold is %d.", tempThresh);
  } else {
    emberAfCorePrintln("Error: The temporary CCA threshold is NULL");
  }
}

void emAfMfglibGetCtuneValueReg(void)  // get-ctune-reg
{
  uint16_t val;
  val = halInternalGetCtune();
  emberAfCorePrintln("The temporary Ctune value is %d.", val);
}

void emAfMfglibSetCtuneValueReg(sl_cli_command_arg_t *arguments)  // set-ctune-reg
{
  uint16_t tune = sl_cli_get_argument_uint16(arguments, 0);
  halInternalSetCtune(tune);
  emberAfCorePrintln("The temporary Ctune value has been set.");
}

void emAfMfglibGetCcaThresholdTok(void)  // get-cca-tok
{
  tokTypeMfgCTune threshold = 0xFFFF;
  halCommonGetMfgToken(&threshold, TOKEN_MFG_CCA_THRESHOLD);
  if (threshold != 0xFFFF) {
    emberAfCorePrintln("The CCA threshold token is %d.", (int16_t)threshold);
  } else {
    emberAfCorePrintln("The CCA Threshold token is NULL and can be set.");
  }
}

void emAfMfglibSetCcaThresholdTok(sl_cli_command_arg_t *arguments)  // set-cca-tok
{
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

void emAfMfglibGetCtuneValueTok(void)  // get-ctune-tok
{
  tokTypeMfgCTune value = 0xFFFF;
  halCommonGetMfgToken(&value, TOKEN_MFG_CTUNE);
  if (value != 0xFFFF) {
    emberAfCorePrintln("The CTUNE value token is %d.", value);
  } else {
    emberAfCorePrintln("The CTUNE value token is NULL and can be set.");
  }
}

void emAfMfglibSetCtuneValueTok(sl_cli_command_arg_t *arguments)  // set-ctune-tok
{
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
  char str[12];
  char sig[15] = "test";
  EmberStatus status;

  sl_zigbee_event_set_inactive(&packetSend);

  if (contPacket) {
    packetCounter++;
    snprintf(str, sizeof(str), "%d", packetCounter);
    sl_strcat_s(sig, sizeof(sig), str);
    // length byte does not include itself, it includes data bytes to send and 2
    //   bytes CRC
    sendBuff[0] = sl_strlen(sig) + 1 + 2;
    MEMMOVE(sendBuff + 1, sig, sl_strlen(sig) + 1);
    status = mfglibSendPacket(sendBuff, 0);
    emberAfCorePrintln("%p send packet, status 0x%X", PLUGIN_NAME, status);
  }

  // Reschedule the event after a delay of 1 seconds
  sl_zigbee_event_set_delay_ms(&packetSend, MY_DELAY_IN_MS);
}

// Sends packets continuously, set in milliseconds
void emAfMfglibContinuousPacket(void)
{
  if (!emberAfMfglibRunning()) {
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

// stops continuous packets
void emAfMfglibStopContinuous(void)
{
  if (!contPacket) {
    emberAfCorePrintln("Continuous test is not in progress.");
    return;
  }
  contPacket = FALSE;
  emberAfCorePrintln("Continuous packet testing ended :(");
  emberAfCorePrintln("Packet Counter: %u", packetCounter);
}

void emAfMfglibClearPacketCounter(void)
{
  packetCounter = 0;
  emberAfCorePrintln("Packet Counter: %u", packetCounter);
}

void emAfMfglibGetPackets(void)
{
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

void emAfMfglibModeSetGpio(sl_cli_command_arg_t *arguments)
{
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
  GPIO_Port_TypeDef port = (GPIO_Port_TypeDef) sl_cli_get_argument_uint32(
    arguments,
    0);

  /*
   * AVAILABLE PINS:
   * 0 - 15
   */
  uint32_t pin = sl_cli_get_argument_uint32(arguments, 1);

  /*
   * PIN MODES:
   * gpioModeDisabled = 0
   *
   */
  GPIO_Mode_TypeDef mode = (GPIO_Mode_TypeDef) sl_cli_get_argument_uint32(
    arguments,
    2);
  uint32_t out = sl_cli_get_argument_uint32(arguments, 3);
  GPIO_PinModeSet(port, pin, mode, out);
  emberAfCorePrintln("GPIO settings have been applied.");
}

void emAfMfglibModeGetGpio(sl_cli_command_arg_t *arguments)
{
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
  GPIO_Port_TypeDef port = (GPIO_Port_TypeDef) sl_cli_get_argument_uint32(
    arguments,
    0);

  /*
   * AVAILABLE PINS:
   * 0 - 15
   */
  uint32_t pin = sl_cli_get_argument_uint32(arguments, 1);
  GPIO_Mode_TypeDef mode = GPIO_PinModeGet(port, pin);
  emberAfCorePrintln("The specified port is in mode %d.", (uint32_t )mode);
}

void emAfMfglibSetGpio(sl_cli_command_arg_t *arguments)
{
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
  GPIO_Port_TypeDef port = (GPIO_Port_TypeDef) sl_cli_get_argument_uint32(
    arguments,
    0);

  /*
   * AVAILABLE PINS:
   * 0 - 15
   */
  uint32_t pin = sl_cli_get_argument_uint32(arguments, 1);

  uint32_t out = sl_cli_get_argument_uint32(arguments, 2);
  if (out == 0) {
    GPIO_PinOutClear(port, pin);
    emberAfCorePrintln("GPIO output have been set");
  } else {
    GPIO_PinOutSet(port, pin);
    emberAfCorePrintln("GPIO output have been cleared");
  }
}

void emAfMfglibGetGpio(sl_cli_command_arg_t *arguments)
{
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
  GPIO_Port_TypeDef port = (GPIO_Port_TypeDef) sl_cli_get_argument_uint32(
    arguments,
    0);

  /*
   * AVAILABLE PINS:
   * 0 - 15
   */
  uint32_t pin = sl_cli_get_argument_uint32(arguments, 1);
  uint8_t value = (GPIO_PortInGet(port) & (1 << pin)) ? 1 : 0;
  emberAfCorePrintln("The specified GPIO input value: %d.", value);
}

void emAfMfglibGpioHelp(void)
{
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

void emAfMfglibTokDump(void)
{
  // We define DEFINETOKENS so when token-stack.h is included below, only the
  // token definitions are operated on as opposed to the token types.
#define DEFINETOKENS
  // Treat the manufacturing tokens just like a normal token definition.
#define TOKEN_MFG TOKEN_DEF
  // Turn each token's definition into a block of code which performs
  // an internal GetTokenData call to read out that token and print it.
#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...)   \
  {                                                                    \
    int8_t j;                                                          \
    /* Create an exact size array for the local copy of the data. */   \
    uint8_t i, dst[sizeof(type)];                                      \
    /* Tell me the creator code and name of the token. */              \
    emberAfCorePrint("\n{ [%2X] ", creator);                           \
    emberAfCorePrint("{%s:", #name);                                   \
    /* If this is an indexed token, we're going to loop over each */   \
    /* index.  If this is not an indexed token, the array size    */   \
    /* should be 1 so we'll only perform this loop once.          */   \
    for (j = 0; j < arraysize; j++) {                                  \
      if (arraysize == 1) {                                            \
        /* We're trying to access a non-indexed token (denoted by  */  \
        /* arraysize = 1).  The index parameter is zero because as */  \
        /* a non-indexed token, we do not want to offset.          */  \
        halInternalGetTokenData(dst, TOKEN_ ## name, 0, sizeof(type)); \
      } else {                                                         \
        /* We're trying to access an indexed token.  The index */      \
        /* parameter is being looped over with the for loop.   */      \
        emberAfCorePrint("[index %d] ", j);                            \
        halInternalGetTokenData(dst, TOKEN_ ## name, j, sizeof(type)); \
      }                                                                \
      /* Print out the token data we just obtained. */                 \
      for (i = 0; i < sizeof(type); i++) {                             \
        emberAfCorePrint("%X", dst[i]);                                \
      }                                                                \
      emberAfCorePrintln("}}\r\n");                                    \
    }                                                                  \
    emberAfCorePrintln("}}\r\n");                                      \
  }

  // Now that we've defined the tokens to be a block of code, pull them in
#include "stack/config/token-stack.h"
  // Release the manufacturing token definition.
#undef TOKEN_MFG
  // Release the normal token definition.
#undef TOKEN_DEF
  // Release the token definition blocks.
#undef DEFINETOKENS
}

void emAfMfglibSleepTest(sl_cli_command_arg_t *arguments)
{
  uint8_t mode = sl_cli_get_argument_uint8(arguments, 0);
  if (mode > 5) {
    emberAfCorePrintln("Invalid sleep mode.");
  } else {
    emberAfCorePrintln("Entering sleep mode.");
    halCommonDelayMicroseconds(1000);
    halInternalSleep(mode);
  }
}

void halInternalSleep(SleepModes sleepMode)
{
  sl_power_manager_em_t em_power = SL_POWER_MANAGER_EM0;

  if (sleepMode == SLEEPMODE_IDLE) {
    em_power = SL_POWER_MANAGER_EM1;
  } else if ((sleepMode == SLEEPMODE_WAKETIMER)
             || (sleepMode == SLEEPMODE_MAINTAINTIMER)) {
    em_power = SL_POWER_MANAGER_EM2;
  } else if (sleepMode == SLEEPMODE_NOTIMER) {
    em_power = SL_POWER_MANAGER_EM3;
  } else if (sleepMode == SLEEPMODE_HIBERNATE) {
    // Only a power on reset or external reset pin can wake the device from EM4.
    EMU_EnterEM4();
  }

  // Add a requirement
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

  // Go to sleep
  sl_power_manager_sleep();

  // Renable interrupts.
  CORE_CriticalEnableIrq();

  // Remove the previous requirement
  sl_power_manager_remove_em_requirement(em_power);
}

void emAfMfglibEnterBootloader()
{
  halInternalSysReset(RESET_BOOTLOADER_BOOTLOAD);
}
