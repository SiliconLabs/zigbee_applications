/***************************************************************************//**
 * @file custom_graphics.h
 * @brief Zigbee Large Network Testing example - Custom graphics header
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

#ifndef CUSTOM_GRAPHICS_H_
#define CUSTOM_GRAPHICS_H_

#include "dmd.h"
#include "glib.h"
#include "displayls013b7dh06.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/// Display-specific constant
#define GRAPHICS_MENU_DISP_SIZE        (12u)

/// Formatting defines
#define SL_CUSTOM_GRAPHICS_LINE_HEIGHT           10
#define SL_CUSTOM_GRAPHICS_RIGHT_PADDING          4
#define SL_CUSTOM_GRAPHICS_LEFT_PADDING           5
#define SL_CUSTOM_GRAPHICS_TEXT_AREA_UP_PADDING   2

/// Icon type
typedef enum {
	SL_ICON_NONE = 0x0,
	SL_ICON_CONNECT = 0x1,
	SL_ICON_LEAVE = 0x2,
	SL_ICON_OPEN = 0x3,
	SL_ICON_CLOSE = 0x04
} sl_icon_type_t;

/// Side of the display
typedef enum {
	SL_SIDE_LEFT = 0x0,
	SL_SIDE_RIGHT = 0x1
} sl_display_side_t;

/// Row of the menu (top or bottom)
typedef enum {
	SL_TOP_ROW = 0x0,
	SL_BOTTOM_ROW = 0x1
} sl_menu_row_t;

/// Text alignment
typedef enum {
  SL_ALIGN_RIGHT = 0,
  SL_ALIGN_LEFT = 1,
  SL_ALIGN_MIDDLE = 2
} sl_text_align_t;

/// Font style
typedef enum {
  SL_FONT_NORMAL = 0,
  SL_FONT_BOLD = 1
} sl_font_type_t;

/// Global graphic handle for the display.
extern GLIB_Context_t glibContext;

#ifdef __cplusplus
extern "C" {
#endif

/// Global function declarations
void GRAPHICS_Init(void);

/// App function declarations
void sl_graphics_draw_button_icon(sl_display_side_t side, 
                                  sl_icon_type_t type, 
                                  sl_menu_row_t row);
void sl_graphics_print_line(char *txt, 
                            uint8_t line, 
                            sl_text_align_t align, 
                            sl_font_type_t font_type);
void sl_graphics_print_block(char *txt, 
                             uint8_t line, 
                             sl_display_side_t side, 
                             sl_text_align_t align, 
                             sl_font_type_t font_type);
void sl_graphics_clear_line(uint8_t line);
void sl_graphics_clear_block(uint8_t line, sl_display_side_t side);
void sl_graphics_draw_counters_header();
void sl_graphics_draw_status_message(char *msg);
void sl_graphics_draw_title_text(char *title);
void sl_graphics_draw_short_id(uint16_t short_id);
void sl_graphics_draw_parent_id(uint16_t parent_id);
void sl_graphics_draw_route_errors(uint16_t route_error_count);
void sl_graphics_draw_cca_errors(uint16_t cca_error_count);
void sl_graphics_draw_mac_errors(uint16_t mac_rx, 
                                 uint16_t mac_tx_succ, 
                                 uint16_t mac_retry, 
                                 uint16_t mac_fail, 
                                 uint16_t mac_rx_bc, 
                                 uint16_t mac_tx_bc);
void sl_graphics_draw_aps_errors(uint16_t aps_rx, 
                                 uint16_t aps_tx_succ, 
                                 uint16_t apsRetry, 
                                 uint16_t aps_fail, 
                                 uint16_t aps_rx_bc, 
                                 uint16_t aps_tx_bc);

#ifdef __cplusplus
}
#endif

#endif /* CUSTOM_GRAPHICS_H_ */