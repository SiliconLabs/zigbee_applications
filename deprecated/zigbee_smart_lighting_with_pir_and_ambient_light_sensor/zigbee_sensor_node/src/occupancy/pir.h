/***************************************************************************//**
 * @file
 * @brief Header file for PIR demonstration code.
 * @version 1.0.2
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
 * DEPRECATION NOTICE
 * This code has been deprecated. It has been provided for historical reference
 * only and should not be used. This code will not be maintained.
 * This code is subject to the quality disclaimer at the point in time prior
 * to deprecation and superseded by this deprecation notice.
 ******************************************************************************/

#ifndef _PIR_H_
#define _PIR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "bspconfig.h"

/**
 * @brief Configures whether to use the internal op-amp or an external op-amp.
 */
typedef enum {
  pirOpampModeInternal = 0,
  pirOpampModeExternal = 1,
} PIR_OpampMode_TypeDef;

/**
 * @brief Selects whether to use the dedicated main op-amp ports or use the APORT selection.
 */
typedef enum {
  pirOpampMainPorts = 0,
  pirOpampAport = 1,
} PIR_Opamp_Port_t;

/**
 * @brief Selects whether to operate the ADC in single ended positive, negative or differential mode.
 */
typedef enum {
  adcSourceAdcP = 0,
  adcSourceAdcN = 1,
  adcSourceAdcDiff = 2,
} AdcSource_Typedef_t;

/**
 * @brief Initialization struct for PIR operation.
 */
typedef struct {
  PIR_OpampMode_TypeDef opampMode;    /** opampMode: Use internal or external opamp. */
  uint32_t motionOnTime;              /** motionOnTime: The duration of time which motion is asserted after a motion event. */
  uint32_t winSize;                   /** winSize: The peak to peak window size for motion detection in lsb. */
} PIR_Init_TypeDef;

/**
 * @brief Default initialization struct for PIR.
 */
#define PIR_INIT_DEFAULT                                                                                  \
  {                                                                                                       \
    .opampMode = pirOpampModeExternal,  /**< opampMode: Select whether to initialize the internal opamp. */ \
    .motionOnTime = 128,                /**< motionOnTime: 4 seconds @ 32 Hz = 128 */                       \
    .winSize = 1024,                    /**< winSize: peak to peak size of window (lsb). */                 \
  }

/**
 * @brief callback for motion detection event
 */
typedef void (*motion_detection_callback_t)(bool motion);


/**
 * @brief init function for PIR occupancy sensing.
 *
 * @param[in] register_callback register a callback from application
 *
 */
void pir_init(motion_detection_callback_t callback_registration);

/**
 * @brief
 *  Charges up the slow charging capacitor by bypassing the opamp feedback network.
 *
 *  Not used in the OCCUPANCY-EXP-EVB as the R_f * C_g settle time is short enough.
 *  May be used if R_f * C_g is increased further.
 *
 * @param[in] pirSettleTimeMs
 *  The time in milliseconds to short the feedback network with a unity gain buffer.
 */
void pir_settle_capacitor(uint32_t pir_settle_time_ms);

/**
 * @brief
 *   Begins running the motion detection sampling and algorithm.
 */
void pir_start(void);

/**
 * @brief
 *   stop running the motion detection sampling and algorithm.
 */
void pir_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* _PIR_H_ */
