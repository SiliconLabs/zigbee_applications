/***************************************************************************//**
 * @file gui.h
 * @brief Zigbee Large Network Testing example - GUI header
 * @version 1.0.0
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided \'as-is\', without any express or implied
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

#ifndef GUI_H_
#define GUI_H_

#include "app/framework/include/af.h"
#include "app/framework/plugin/counters/counters.h"
#include "custom_graphics.h"

/// GUI title string
#define SL_LARGE_NETWORK_TESTING_GUI_TITLE_STRING "test node"

/// Threshold of a long press in milliseconds
#define SL_BUTTON_SL_LONG_PRESS_TRESHOLD_MS 400

/// Timeperiod in which the counters will be queried
#define SL_COUNTERS_QUERY_PERIOD_MS 1000

/// Button press type, short or long press
typedef enum {
	SL_SHORT_PRESS = 0x0,
	SL_LONG_PRESS = 0x1,
} sl_button_press_type_t;

/// Structure to hold pressed button number and whether it's short or long press.
typedef struct {
  sl_button_press_type_t type;
  uint8_t button;
} sl_pressed_button_t;

#ifdef __cplusplus
extern "C" {
#endif

/// App function declarations
void sl_large_network_testing_gui_init(void);
void sl_large_network_testing_gui_network_steering_complete_callback(EmberStatus status);
void sl_large_network_testing_gui_stack_status_callback(EmberStatus status);
void sl_large_network_testing_gui_counters_print_handler(void);
void sl_large_network_testing_gui_connect_button_handler(void);
void sl_large_network_testing_gui_open_button_handler(void);
void sl_large_network_testing_gui_close_button_handler(void);
void sl_large_network_testing_gui_leave_button_handler(void);
void sl_large_network_testing_gui_button_pressed_handler(void);
void sl_large_network_testing_gui_button_isr_callback(uint8_t button, 
                                                      uint8_t state);
void sl_large_network_testing_gui_button_check_handler(void);

#ifdef __cplusplus
}
#endif

#endif //GUI_H_