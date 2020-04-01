/***************************************************************************//**
 * @file
 * @brief Main file for Main Bootloader.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "config/btl_config.h"
#include "api/btl_interface.h"

#include "core/btl_core.h"
#include "core/btl_reset.h"
#include "core/btl_parse.h"
#include "core/btl_bootload.h"
#include "core/btl_upgrade.h"

#include "plugin/debug/btl_debug.h"

#ifdef BTL_PLUGIN_GPIO_ACTIVATION
#include "plugin/gpio/gpio-activation/btl_gpio_activation.h"
#endif

#ifdef BTL_PLUGIN_EZSP_GPIO_ACTIVATION
#include "plugin/gpio/ezsp-gpio-activation/btl_ezsp_gpio_activation.h"
#endif

#ifdef BOOTLOADER_SUPPORT_STORAGE
#include "plugin/storage/btl_storage.h"
#include "plugin/storage/bootloadinfo/btl_storage_bootloadinfo.h"
#endif

#ifdef BOOTLOADER_SUPPORT_COMMUNICATION
#include "plugin/communication/btl_communication.h"
#endif

#include "em_device.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_chip.h"

#if defined(__GNUC__)
#define ROM_END_SIZE 0
extern const size_t __rom_end__;
#elif defined(__ICCARM__)
#define ROM_END_SIZE 4
const size_t __rom_end__ @ "ROM_SIZE";
#endif

// --------------------------------
// Local function declarations
__STATIC_INLINE bool enterBootloader(void);
SL_NORETURN static void bootToApp(uint32_t);

#if defined(BOOTLOADER_WRITE_DISABLE)
__STATIC_INLINE void lockBootloaderArea(void)
{
  // Disable write access to bootloader.
  // Prevents application from touching the bootloader.
#if defined(_MSC_PAGELOCK0_MASK)
#if defined(CRYPTOACC_PRESENT)
  CMU->CLKEN1_SET = CMU_CLKEN1_MSC;
#endif
  for (uint32_t i = (BTL_FIRST_STAGE_BASE / FLASH_PAGE_SIZE);
       i < ((BTL_MAIN_STAGE_MAX_SIZE + BTL_FIRST_STAGE_SIZE) / FLASH_PAGE_SIZE);
       i++) {
    MSC->PAGELOCK0_SET = (0x1 << i);
  }
#if defined(CRYPTOACC_PRESENT)
  CMU->CLKEN1_CLR = CMU_CLKEN1_MSC;
#endif
#elif defined(MSC_BOOTLOADERCTRL_BLWDIS)
  MSC->BOOTLOADERCTRL |= MSC_BOOTLOADERCTRL_BLWDIS;
#else
  // Do nothing
#endif
}
#endif

void HardFault_Handler(void)
{
  BTL_DEBUG_PRINTLN("Fault          ");
  reset_resetWithReason(BOOTLOADER_RESET_REASON_FATAL);
}

// Main Bootloader implementation

int main(void)
{
  int32_t ret = BOOTLOADER_ERROR_STORAGE_BOOTLOAD;

  CHIP_Init();

  // Enabling HFXO will add a hefty code size penalty (~1k)
  // CMU_HFXOInit_TypeDef hfxoInit = CMU_HFXOINIT_DEFAULT;
  // CMU_HFXOInit(&hfxoInit);
  // CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
  // CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);

  BTL_DEBUG_PRINTLN("BTL entry");

#if defined(EMU_CMD_EM01VSCALE2) && defined(EMU_STATUS_VSCALEBUSY)
  // Device supports voltage scaling, and the bootloader may have been entered
  // with a downscaled voltage. Scale voltage up to allow flash programming.
  EMU->CMD = EMU_CMD_EM01VSCALE2;
  while (EMU->STATUS & EMU_STATUS_VSCALEBUSY) {
    // Do nothing
  }
#endif

  btl_init();
  reset_invalidateResetReason();

#ifdef BOOTLOADER_SUPPORT_STORAGE
  // If the bootloader supports storage, first attempt to apply an existing
  // image from storage.
  ret = storage_main();

  if (ret == BOOTLOADER_OK) {
    // Firmware ugprade from storage successful, return to application
    reset_resetWithReason(BOOTLOADER_RESET_REASON_GO);
  } else {
    // Wait a short while (approx. 500 ms) before continuing.
    // This prevents the reset loop from being so tight that a debugger is
    // unable to reattach to flash a new app when neither the app nor the
    // contents of storage are valid.
    for (volatile int i = 800000; i > 0; i--) {
      // Do nothing
    }
  }
#endif

#ifdef BOOTLOADER_SUPPORT_COMMUNICATION
  communication_init();

  ret = communication_start();
  if (ret != BOOTLOADER_OK) {
    reset_resetWithReason(BOOTLOADER_RESET_REASON_FATAL);
  }

  ret = communication_main();
  BTL_DEBUG_PRINT("Protocol returned ");
  BTL_DEBUG_PRINT_WORD_HEX(ret);
  BTL_DEBUG_PRINT_LF();

  communication_shutdown();

  if ((ret == BOOTLOADER_OK)
      || (ret == BOOTLOADER_ERROR_COMMUNICATION_DONE)) {
    reset_resetWithReason(BOOTLOADER_RESET_REASON_GO);
  }
#endif // BOOTLOADER_SUPPORT_COMMUNICATION

  // An error occurred in storage or communication, and a firmware upgrade
  // was not performed
  if (0
#ifdef BOOTLOADER_SUPPORT_COMMUNICATION
      || (ret == BOOTLOADER_ERROR_COMMUNICATION_IMAGE_ERROR)
      || (ret == BOOTLOADER_ERROR_COMMUNICATION_TIMEOUT)
#endif
#ifdef BOOTLOADER_SUPPORT_STORAGE
      || (ret == BOOTLOADER_ERROR_STORAGE_BOOTLOAD)
#endif
      ) {
    reset_resetWithReason(BOOTLOADER_RESET_REASON_BADIMAGE);
  } else {
    reset_resetWithReason(BOOTLOADER_RESET_REASON_FATAL);
  }

  return 0;
}

#ifdef BOOTLOADER_SUPPORT_STORAGE
extern const BootloaderStorageFunctions_t storageFunctions;
#endif

/*const*/ MainBootloaderTable_t mainStageTable = {
  {
    .type = BOOTLOADER_MAGIC_MAIN,
    .layout = BOOTLOADER_HEADER_VERSION_MAIN,
    .version = BOOTLOADER_VERSION_MAIN
  },
  // Bootloader size is the relative address of the end variable plus 4 for the
  // CRC
  .size = ((uint32_t)&__rom_end__) - BTL_MAIN_STAGE_BASE + ROM_END_SIZE + 4,
  .startOfAppSpace = (BareBootTable_t *)(BTL_APPLICATION_BASE),
  .endOfAppSpace = (void *)(BTL_APPLICATION_BASE + BTL_APP_SPACE_SIZE),
  .capabilities = (0
#ifdef BOOTLOADER_ENFORCE_SIGNED_UPGRADE
                   | BOOTLOADER_CAPABILITY_ENFORCE_UPGRADE_SIGNATURE
#endif
#ifdef BOOTLOADER_ENFORCE_ENCRYPTED_UPGRADE
                   | BOOTLOADER_CAPABILITY_ENFORCE_UPGRADE_ENCRYPTION
#endif
#ifdef BOOTLOADER_ENFORCE_SECURE_BOOT
                   | BOOTLOADER_CAPABILITY_ENFORCE_SECURE_BOOT
#endif
                   | BOOTLOADER_CAPABILITY_BOOTLOADER_UPGRADE
                   | BOOTLOADER_CAPABILITY_EBL
                   | BOOTLOADER_CAPABILITY_EBL_SIGNATURE
#if !defined(BTL_PARSER_NO_SUPPORT_ENCRYPTION)
                   | BOOTLOADER_CAPABILITY_EBL_ENCRYPTION
#endif
#ifdef BOOTLOADER_SUPPORT_STORAGE
                   | BOOTLOADER_CAPABILITY_STORAGE
#endif
#ifdef BOOTLOADER_SUPPORT_COMMUNICATION
                   | BOOTLOADER_CAPABILITY_COMMUNICATION
#endif
                   ),
  .init = &btl_init,
  .deinit = &btl_deinit,
  .verifyApplication = &bootload_verifyApplication,
  .initParser = &core_initParser,
  .parseBuffer = &core_parseBuffer,
#ifdef BOOTLOADER_SUPPORT_STORAGE
  .storage = &storageFunctions
#else
  .storage = NULL
#endif
};

#if defined(BOOTLOADER_SUPPORT_CERTIFICATES)
const ApplicationCertificate_t sl_app_certificate = {
  .structVersion = APPLICATION_CERTIFICATE_VERSION,
  .flags = { 0U },
  .key = { 0U },
  .version = 0,
  .signature = { 0U },
};
#endif

const ApplicationProperties_t sl_app_properties = {
  .magic = APPLICATION_PROPERTIES_MAGIC,
  .structVersion = APPLICATION_PROPERTIES_VERSION,
  .signatureType = APPLICATION_SIGNATURE_NONE,
  .signatureLocation = ((uint32_t)&__rom_end__) - BTL_MAIN_STAGE_BASE + ROM_END_SIZE,
  .app = {
    .type = APPLICATION_TYPE_BOOTLOADER,
    .version = BOOTLOADER_VERSION_MAIN,
    .capabilities = 0UL,
    .productId = { 0U },
  },
#if defined(BOOTLOADER_SUPPORT_CERTIFICATES)
  // If certificate based boot chain is enabled, the bootloader binary will be provided with
  // an certificate that does not contain any key.
  // A valid certificate needs to be injected to the bootloader images using Simplicity Commander.
  // Simplicity Commander will replace this certificate.
  .cert = (ApplicationCertificate_t *)&sl_app_certificate,
#else
  .cert = NULL,
#endif
  .longTokenSectionAddress = NULL,
};

/**
 * This function gets executed before ANYTHING got initialized.
 * So, no using global variables here!
 */
void SystemInit2(void)
{
  // Initialize debug before first debug print
  BTL_DEBUG_INIT();

  // Assumption: We should enter the app
  bool enterApp = true;
  // Assumption: The app should be verified
  bool verifyApp = true;

  // Check if we came from EM4. If any other bit than the EM4 bit it set, we
  // can't know whether this was really an EM4 reset, and we need to do further
  // checking.
#if defined(RMU_RSTCAUSE_EM4RST) && defined(APPLICATION_VERIFICATION_SKIP_EM4_RST)
  if (RMU->RSTCAUSE == RMU_RSTCAUSE_EM4RST) {
    // We came from EM4, app doesn't need to be verified
    verifyApp = false;
  } else if (enterBootloader()) {
    // We want to enter the bootloader, app doesn't need to be verified
    enterApp = false;
    verifyApp = false;
  }
#else
  if (enterBootloader()) {
    // We want to enter the bootloader, app doesn't need to be verified
    enterApp = false;
    verifyApp = false;
  }
#endif

  //add by jim
  // *INDENT-OFF*
  #if defined(EMU_RSTCAUSE_SYSREQ)
    if (EMU->RSTCAUSE & EMU_RSTCAUSE_SYSREQ) {
  #else
    if (RMU->RSTCAUSE & RMU_RSTCAUSE_SYSREQRST) {
  #endif
      if (0xABCD == reset_classifyReset()) {
    	  mainStageTable.startOfAppSpace = (BareBootTable_t *)(0x80000);
      } else {
    	  mainStageTable.startOfAppSpace = (BareBootTable_t *)(0x0);
      }
    }
  //end add


  uint32_t startOfAppSpace = (uint32_t)mainStageTable.startOfAppSpace;

  // Sanity check application program counter
  uint32_t pc = *(uint32_t *)(startOfAppSpace + 4);
  if (pc == 0xFFFFFFFF) {
    // Sanity check failed; enter the bootloader
    reset_setResetReason(BOOTLOADER_RESET_REASON_BADAPP);
    enterApp = false;
    verifyApp = false;
  }

  // App should be verified
  if (verifyApp) {
    // If app verification fails, enter bootloader instead
    enterApp = bootload_verifyApplication(startOfAppSpace);
    if (!enterApp) {
      reset_setResetReason(BOOTLOADER_RESET_REASON_BADAPP);
    }
  }

  if (enterApp) {
    BTL_DEBUG_PRINTLN("Enter app");
    BTL_DEBUG_PRINT_LF();
#if defined(BOOTLOADER_WRITE_DISABLE)
    lockBootloaderArea();
#endif

#if defined(BOOTLOADER_ENFORCE_SECURE_BOOT) && defined(APPLICATION_WRITE_DISABLE)
    // The neccessary check of valid signature pointer for application at startOfAppSpace
    // is already done in bootload_verifyApplication.
    bootload_lockApplicationArea(startOfAppSpace, 0);
#endif

    // Set vector table to application's table
    SCB->VTOR = startOfAppSpace;

    bootToApp(startOfAppSpace);
  }
  // Enter bootloader
}

/**
 * Jump to app
 */
SL_NORETURN static void bootToApp(uint32_t startOfAppSpace)
{
  (void)startOfAppSpace;

  // Load SP and PC of application
  __ASM("mov r0, %0       \n" // Load address of SP into R0
        "ldr r1, [r0]     \n" // Load SP into R1
        "msr msp, r1      \n" // Set MSP
        "msr psp, r1      \n" // Set PSP
        "ldr r0, [r0, #4] \n" // Load PC into R0
        "mov pc, r0       \n" // Set PC
        :: "r" (startOfAppSpace) : "r0", "r1");

  while (1) {
    // Do nothing
  }
}

/**
 * Check whether we should enter the bootloader
 *
 * @return True if the bootloader should be entered
 */
__STATIC_INLINE bool enterBootloader(void)
{
// *INDENT-OFF*
#if defined(EMU_RSTCAUSE_SYSREQ)
  if (EMU->RSTCAUSE & EMU_RSTCAUSE_SYSREQ) {
#else
  if (RMU->RSTCAUSE & RMU_RSTCAUSE_SYSREQRST) {
#endif
    // Check if we were asked to run the bootloader...
    switch (reset_classifyReset()) {
      case BOOTLOADER_RESET_REASON_BOOTLOAD:
      case BOOTLOADER_RESET_REASON_FORCE:
      case BOOTLOADER_RESET_REASON_UPGRADE:
      case BOOTLOADER_RESET_REASON_BADAPP:
        // Asked to go into bootload mode
        return true;
      default:
        break;
    }
  }
// *INDENT-ON*

#ifdef BTL_PLUGIN_GPIO_ACTIVATION
  if (gpio_enterBootloader()) {
    // GPIO pin state signals bootloader entry
    return true;
  }
#endif

#ifdef BTL_PLUGIN_EZSP_GPIO_ACTIVATION
  if (ezsp_gpio_enterBootloader()) {
    // GPIO pin state signals bootloader entry
    return true;
  }
#endif

  return false;
}

bool bootloader_enforceSecureBoot(void)
{
#ifdef BOOTLOADER_ENFORCE_SECURE_BOOT
  return true;
#else
  return false;
#endif
}
