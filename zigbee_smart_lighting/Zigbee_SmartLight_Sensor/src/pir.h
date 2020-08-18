/***************************************************************************//**
 * @file
 * @brief Driver for PIR sensor
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
 ******************************************************************************/

#ifndef PIR_H
#define PIR_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup PIR
 * @{
 ******************************************************************************/

/// Configures whether to use the internal opamp or an external opamp.
typedef enum {
  pir_opamp_mode_internal = 0,
  pir_opamp_mode_external = 1,
} pir_opamp_mode_t;

/// Selects whether to use the dedicated main opamp ports or use the APORT selection.
typedef enum {
  pir_opamp_main_ports = 0,
  pir_opamp_aport      = 1,
} pir_opamp_port_t;

/// Selects whether to operate the ADC in single ended positive, negative or differential mode.
typedef enum {
  pir_adc_source_pos   = 0,
  pir_adc_source_neg   = 1,
  pir_adc_source_diff  = 2,
} pir_adc_source_t;

/***************************************************************************//**
 * @brief
 *   ADC IRQ callback function
 *
 * @note
 *   This function is called whenever an ADC interrupt is received. It's up to
 *   the application to decide how to handle the interrupt. However, in order to
 *   detect motion properly, pir_detect_motion function must be called within
 *   30ms after receiving the interrupt to process the latest ADC sample .
 ******************************************************************************/
typedef void (*pir_adc_irq_callback_t)();

/***************************************************************************//**
 * @brief
 *   Motion detection result callback function
 *
 * @param[in] motion_status
 *   True indicates motion on is detected
 *   False indicates motion off is detected
 *
 * @note
 *   This function is called whenever motion on/off status is detected in the
 *   algorithm. The application should check the motion_status and take actions
 *   accordingly.
 ******************************************************************************/
typedef void (*pir_motion_detection_callback_t)(bool motion_status);

/// Structure for PIR sample.
typedef struct {
  int32_t timestamp_ms;                     ///< A 16-bit timestamp from the 1024 Hz counter.
  int32_t adc_sample;                       ///< The ADC sample measuring the PIR voltage.
  int32_t win_base;                         ///< The center of the detector window.
  int32_t adc_upper_threshold;              ///< The upper threshold of the detector window.
  int32_t adc_lower_threshold;              ///< The lower threshold of the detector window.
  bool motion_status;                       ///< Whether motion was detected.
} pir_sample_t;

/// Initialization structure for PIR driver.
typedef struct {
  pir_opamp_mode_t opamp_mode;                                      ///< Use internal or external opamp.
  uint32_t         motion_on_time;                                  ///< The duration of time(in seconds) motion on being asserted after detected.
  uint32_t         win_size;                                        ///< The peak to peak window size for motion detection in lsb.
  pir_adc_irq_callback_t adc_irq_callback;                          ///< ADC IRQ callback.
  pir_motion_detection_callback_t motion_detection_callback;        ///< Motion detection result callback.
  uint16_t         sample_queue_size;                               ///< PIR sample queue size. Must be no smaller than 4.
  pir_sample_t     *sample_queue;                                   ///< A sample buffer supplied by the user.
  bool             use_timestamp;                                   ///< Whether to apply timestamp to PIR samples or not.
} pir_init_t;

/// Default initialization structure for PIR driver.
#define PIR_INIT_DEFAULT                   \
{                                          \
  .opamp_mode = pir_opamp_mode_external,   \
  .motion_on_time = 4,                     \
  .win_size = 1024,                        \
  .adc_irq_callback = NULL,                \
  .motion_detection_callback = NULL,       \
  .sample_queue_size = 4,                  \
  .sample_queue = NULL,                    \
  .use_timestamp = true,                   \
}

/***************************************************************************//**
 * @brief
 *   Initializes peripherals for PIR
 *
 * @param[in] pir_init
 *   A pointer to PIR initialization structure
 *
 * @param[in] adc_enter_em2
 *   Sets up ADC to run in EM2 when set to true
 ******************************************************************************/
void pir_init(pir_init_t *pir_init, bool adc_enter_em2);

/***************************************************************************//**
 * @brief
 *   Runs the motion detection algorithm
 *
 * @note
 *   This algorithms takes roughly 150us and must run on the latest ADC sample
 *   after receiving the ADC interrupt.
 ******************************************************************************/
void pir_detect_motion(void);

/***************************************************************************//**
 * @brief
 *   Starts PIR sensor sampling.
 ******************************************************************************/
void pir_start(void);

/***************************************************************************//**
 * @brief
 *   Stops PIR sensor sampling.
 ******************************************************************************/
void pir_stop(void);

/***************************************************************************//**
 * @brief
 *   Reads out a sample from the PIR sample queue.
 *
 * @param[out] pir_sample
 *   Pointer to the PIR sample
 ******************************************************************************/
void pir_read_queue(pir_sample_t *pir_sample);

/***************************************************************************//**
 * @brief
 *   Gets number of samples in the queue.
 *
 * @return
 *   The number of samples in the queue.
 ******************************************************************************/
uint16_t pir_get_queue_size(void);

/** @} (end addtogroup PIR) */

#ifdef __cplusplus
}
#endif

#endif // PIR_H
