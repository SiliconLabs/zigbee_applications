/***************************************************************************//**
 * @file custom_graphics.c
 * @brief Zigbee Large Network Testing example - Custom graphics
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
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
******************************************************************************/

#include "custom_graphics.h"

/// GLIB context to use
extern GLIB_Context_t glibContext;
/// GLIC rectangle to use
GLIB_Rectangle_t sl_glib_rect;

/// Buffer for sprintf calls
static char sl_print_buff[64];
/// Variables to store x and y values
static uint16_t sl_glob_x, sl_glob_y;

/**************************************************************************//**
 * @brief Draw a button icon
 * 
 * @param side Side to draw, left or right
 * @param type Icon type to draw
 * @param row Which row to draw
 *****************************************************************************/
void sl_graphics_draw_button_icon(sl_display_side_t side,
                                  sl_icon_type_t type,
                                  sl_menu_row_t row)
{
  GLIB_setFont(&glibContext, (GLIB_Font_t *) &GLIB_FontNormal8x8);
  sl_glob_x = glibContext.font.fontWidth;
  sl_glob_y = glibContext.font.fontHeight;

  switch (type) {
    case SL_ICON_NONE:
      sl_print_buff[0u] = '\0';
      break;

    case SL_ICON_CONNECT:
      sprintf(sl_print_buff, "conn");
      break;

    case SL_ICON_LEAVE:
      sprintf(sl_print_buff, "leave");
      break;

    case SL_ICON_OPEN:
      sprintf(sl_print_buff, "open");
      break;

    case SL_ICON_CLOSE:
      sprintf(sl_print_buff, "close");
      break;

    default:
      sprintf(sl_print_buff, "none");
      break;
  }

  sl_glob_x *= strlen(sl_print_buff);

  // Right side
  if (side) {
    sl_glob_x = (((3 * glibContext.pDisplayGeometry->xSize) / 2u) \
                 - sl_glob_x) / 2u;
  // Left side
  } else {
    sl_glob_x = ((glibContext.pDisplayGeometry->xSize / 2u) - sl_glob_x) / 2u;
  }

  switch (row) {
    case SL_BOTTOM_ROW:
      sl_glob_y = glibContext.pDisplayGeometry->ySize \
                  - glibContext.font.fontHeight - 1u;
      break;
    case SL_TOP_ROW:
      GLIB_setFont(&glibContext, (GLIB_Font_t *) &GLIB_FontNarrow6x8);
      sl_glob_y = glibContext.pDisplayGeometry->ySize \
                  - 2*glibContext.font.fontHeight - 4u;
      break;
    default:
      break;
  }

  GLIB_drawString(&glibContext, 
                  sl_print_buff, 
                  strlen(sl_print_buff), 
                  sl_glob_x, 
                  sl_glob_y, 
                  false);

  // Set font back
  GLIB_setFont(&glibContext, (GLIB_Font_t *) &GLIB_FontNarrow6x8);

  // Vertical line that separates the buttons
  GLIB_drawLineV(&glibContext,
    (glibContext.pDisplayGeometry->xSize / 2u),
    (glibContext.pDisplayGeometry->ySize - 2*glibContext.font.fontHeight - 4u),
    glibContext.pDisplayGeometry->ySize);

  // Horizontal lines
  // Bottom line
  GLIB_drawLineH(&glibContext,
    0u,
    (glibContext.pDisplayGeometry->ySize - glibContext.font.fontHeight - 2u-1u),
    glibContext.pDisplayGeometry->xSize);

  // top line
  GLIB_drawLineH(&glibContext,
    0u,
    (glibContext.pDisplayGeometry->ySize - 2*glibContext.font.fontHeight-5u-1u),
    glibContext.pDisplayGeometry->xSize);

  DMD_updateDisplay();
}

/**************************************************************************//**
 * @brief Print a text to a line with alignment and font type
 * 
 * @param txt Text tp print
 * @param line Which line to print to
 * @param align Alignment of the text
 * @param font_type
 *****************************************************************************/
void sl_graphics_print_line(char *txt,
                            uint8_t line,
                            sl_text_align_t align,
                            sl_font_type_t font_type)
{
  switch (font_type) {
    case SL_FONT_NORMAL:
      GLIB_setFont(&glibContext, (GLIB_Font_t *) &GLIB_FontNarrow6x8);
      break;
    case SL_FONT_BOLD:
      GLIB_setFont(&glibContext, (GLIB_Font_t *) &GLIB_FontNormal8x8);
      break;
    default:
      break;
  }

  switch (align) {
    case SL_ALIGN_LEFT:
      sl_glob_x = SL_CUSTOM_GRAPHICS_LEFT_PADDING;
      break;
    case SL_ALIGN_RIGHT:
      sl_glob_x = glibContext.pDisplayGeometry->xSize \
                   - (glibContext.font.fontWidth * strlen(txt)) \
                   - SL_CUSTOM_GRAPHICS_RIGHT_PADDING;
      break;
    case SL_ALIGN_MIDDLE:
      sl_glob_x = (glibContext.pDisplayGeometry->xSize \
                   - (glibContext.font.fontWidth * strlen(txt))) / 2;
      break;
    default:
      sl_glob_x = SL_CUSTOM_GRAPHICS_LEFT_PADDING;
      break;
  }

  sl_glob_y = SL_CUSTOM_GRAPHICS_LINE_HEIGHT * line \
              + SL_CUSTOM_GRAPHICS_TEXT_AREA_UP_PADDING;

  GLIB_drawString(&glibContext, txt, strlen(txt), sl_glob_x, sl_glob_y, false);

  DMD_updateDisplay();
}

/**************************************************************************//**
 * @brief Print a text to a block
 * 
 * @param txt Text to print
 * @param line Which line to print to
 * @param side Which side to print to
 * @param align Alignment
 * @param font_type 
 *****************************************************************************/
void sl_graphics_print_block(char *txt,
                             uint8_t line,
                             sl_display_side_t side, 
                             sl_text_align_t align, 
                             sl_font_type_t font_type)
{
  switch (font_type) {
    case SL_FONT_NORMAL:
      GLIB_setFont(&glibContext, (GLIB_Font_t *) &GLIB_FontNarrow6x8);
      break;
    case SL_FONT_BOLD:
      GLIB_setFont(&glibContext, (GLIB_Font_t *) &GLIB_FontNormal8x8);
      break;
    default:
      break;
  }

  uint8_t x_offset;

  switch (side) {
    case SL_SIDE_LEFT:
      x_offset = 0;
      break;
    case SL_SIDE_RIGHT:
      x_offset = glibContext.pDisplayGeometry->xSize / 2;
      break;
    default:
      x_offset = 0;
      break;
  }

  switch (align) {
    case SL_ALIGN_LEFT:
      sl_glob_x = x_offset + SL_CUSTOM_GRAPHICS_LEFT_PADDING;
      break;
    case SL_ALIGN_RIGHT:
      sl_glob_x = glibContext.pDisplayGeometry->xSize \
                   - (glibContext.font.fontWidth * strlen(txt)) \
                   - SL_CUSTOM_GRAPHICS_RIGHT_PADDING;
      break;
    case SL_ALIGN_MIDDLE:
      sl_glob_x = (glibContext.pDisplayGeometry->xSize / 2 \
                   - SL_CUSTOM_GRAPHICS_RIGHT_PADDING \
                   - SL_CUSTOM_GRAPHICS_LEFT_PADDING) / 2 \
                   - (glibContext.font.fontWidth * strlen(txt)) / 2;
      break;
    default:
      break;
  }

  if (sl_glob_x < x_offset) {
    sl_glob_x = x_offset + SL_CUSTOM_GRAPHICS_LEFT_PADDING;
  }

  sl_glob_y = SL_CUSTOM_GRAPHICS_LINE_HEIGHT * line \
               + SL_CUSTOM_GRAPHICS_TEXT_AREA_UP_PADDING;

  GLIB_drawString(&glibContext, txt, strlen(txt), sl_glob_x, sl_glob_y, false);

  DMD_updateDisplay();
}

/**************************************************************************//**
 * @brief Clear a line
 * 
 * @param line Which line to clear
 *****************************************************************************/
void sl_graphics_clear_line(uint8_t line)
{
  sl_glob_x = 0;
  sl_glob_y = SL_CUSTOM_GRAPHICS_LINE_HEIGHT * line \
               + SL_CUSTOM_GRAPHICS_TEXT_AREA_UP_PADDING;

  sl_glib_rect.xMin = sl_glob_x;
  sl_glib_rect.yMin = sl_glob_y;
  sl_glib_rect.xMax = glibContext.pDisplayGeometry->xSize;
  sl_glib_rect.yMax = sl_glob_y + glibContext.font.fontHeight;
  glibContext.foregroundColor = White;
  GLIB_drawRectFilled(&glibContext, &sl_glib_rect);
  glibContext.foregroundColor = Black;
  DMD_updateDisplay();
}

/**************************************************************************//**
 * @brief Clear a block
 * 
 * @param line Line of block to clear
 * @param side Side of block to clear
 *****************************************************************************/
void sl_graphics_clear_block(uint8_t line, sl_display_side_t side)
{
  switch (side) {
    case SL_SIDE_LEFT:
      sl_glob_x = 0;
      break;
    case SL_SIDE_RIGHT:
      sl_glob_x = glibContext.pDisplayGeometry->xSize / 2;
      break;
    default:
      sl_glob_x = 0;
      break;
  }

  sl_glob_y = SL_CUSTOM_GRAPHICS_LINE_HEIGHT * line \
               + SL_CUSTOM_GRAPHICS_TEXT_AREA_UP_PADDING;

  sl_glib_rect.xMin = sl_glob_x;
  sl_glib_rect.yMin = sl_glob_y;
  sl_glib_rect.xMax = sl_glob_x + glibContext.pDisplayGeometry->xSize / 2;
  sl_glib_rect.yMax = sl_glob_y + glibContext.font.fontHeight;
  glibContext.foregroundColor = White;
  GLIB_drawRectFilled(&glibContext, &sl_glib_rect);
  glibContext.foregroundColor = Black;

  DMD_updateDisplay();
}

/**************************************************************************//**
 * @brief Draw the header for counters section
 * 
 *****************************************************************************/
void sl_graphics_draw_counters_header(void)
{
  sl_graphics_print_line("Ru/Tu  r/f Rb/Tb", 2, SL_ALIGN_LEFT, SL_FONT_NORMAL);
  DMD_updateDisplay();
}

// -------------------------------
// Wrappers for printing different information

/**************************************************************************//**
 * @brief Draw status message to the screen
 * 
 * @param msg Message to draw
 *****************************************************************************/
void sl_graphics_draw_status_message(char *msg)
{
  uint8_t line = 9;
  sl_graphics_clear_line(line);
  uint8_t margin_up = 2;
  GLIB_setFont(&glibContext, (GLIB_Font_t *) &GLIB_FontNormal8x8);
  sl_glob_x = (glibContext.pDisplayGeometry->xSize \
               - ((glibContext.font.fontWidth + glibContext.font.charSpacing) \
               * strlen(msg))) / 2;
  sl_glob_y = SL_CUSTOM_GRAPHICS_LINE_HEIGHT * line \
               + SL_CUSTOM_GRAPHICS_TEXT_AREA_UP_PADDING + margin_up;
	
  GLIB_drawString(&glibContext, msg, strlen(msg), sl_glob_x, sl_glob_y, false);

  DMD_updateDisplay();
}

/**************************************************************************//**
 * @brief Draw title to the screen
 * 
 * @param title 
 *****************************************************************************/
void sl_graphics_draw_title_text(char *title)
{
  sl_graphics_print_block(title, 
                          0,
                          SL_SIDE_LEFT, 
                          SL_ALIGN_LEFT, 
                          SL_FONT_NORMAL);
  DMD_updateDisplay();
}

/**************************************************************************//**
 * @brief Draw short_id of the node to the screen
 * 
 * @param short_id 
 *****************************************************************************/
void sl_graphics_draw_short_id(uint16_t short_id)
{
  sprintf(sl_print_buff, "0x%X", short_id);
  sl_graphics_clear_block(0, SL_SIDE_RIGHT);
  sl_graphics_print_block(sl_print_buff,
                          0, 
                          SL_SIDE_RIGHT, 
                          SL_ALIGN_RIGHT, 
                          SL_FONT_BOLD);
  DMD_updateDisplay();
}

/**************************************************************************//**
 * @brief Draw the parent's ID to the screen
 * 
 * @param parent_id 
 *****************************************************************************/
void sl_graphics_draw_parent_id(uint16_t parent_id)
{
  sprintf(sl_print_buff, "0x%X", parent_id);

  sl_graphics_clear_line(1);
  sl_graphics_print_block("parent:",
                          1, 
                          SL_SIDE_LEFT, 
                          SL_ALIGN_LEFT, 
                          SL_FONT_NORMAL);
  
  sl_graphics_print_block(sl_print_buff,
                          1, 
                          SL_SIDE_RIGHT, 
                          SL_ALIGN_RIGHT, 
                          SL_FONT_NORMAL);

  DMD_updateDisplay();
}

/**************************************************************************//**
 * @brief Draw route errors to the screen
 * 
 * @param route_error_count 
 *****************************************************************************/
void sl_graphics_draw_route_errors(uint16_t route_error_count)
{
  sl_graphics_clear_line(7);
  sl_graphics_print_block("route errors:",
                          7, 
                          SL_SIDE_LEFT, 
                          SL_ALIGN_LEFT,
                          SL_FONT_NORMAL);

  sprintf(sl_print_buff, "%d", route_error_count);
  sl_graphics_print_block(sl_print_buff,
                          7,
                          SL_SIDE_RIGHT,
                          SL_ALIGN_RIGHT,
                          SL_FONT_NORMAL);

  DMD_updateDisplay();
}

/**************************************************************************//**
 * @brief Draw CCA errors to the screen
 * 
 * @param cca_error_count 
 *****************************************************************************/
void sl_graphics_draw_cca_errors(uint16_t cca_error_count)
{
  sl_graphics_clear_line(8);
  sl_graphics_print_block("CCA errors:",
                          8, 
                          SL_SIDE_LEFT, 
                          SL_ALIGN_LEFT, 
                          SL_FONT_NORMAL);

  sprintf(sl_print_buff, "%d", cca_error_count);
  sl_graphics_print_block(sl_print_buff,
                          8, 
                          SL_SIDE_RIGHT, 
                          SL_ALIGN_RIGHT, 
                          SL_FONT_NORMAL);
  DMD_updateDisplay();
}

/**************************************************************************//**
 * @brief Draw MAC statistics to the screen
 * 
 * @param mac_rx MAC Rx error count
 * @param mac_tx_succ MAC Tx success count
 * @param mac_retry MAC retry count
 * @param mac_fail MAC failure count
 * @param mac_rx_bc MAC broadcast Rx count
 * @param mac_tx_bc MAC broadcast Tx count
 *****************************************************************************/
void sl_graphics_draw_mac_errors(uint16_t mac_rx, 
                                 uint16_t mac_tx_succ, 
                                 uint16_t mac_retry, 
                                 uint16_t mac_fail, 
                                 uint16_t mac_rx_bc, 
                                 uint16_t mac_tx_bc)
{
  sl_graphics_clear_line(3);
  sl_graphics_clear_line(4);
  sl_graphics_print_block("MAC:",
                          3, 
                          SL_SIDE_LEFT, 
                          SL_ALIGN_LEFT, 
                          SL_FONT_NORMAL);

  sprintf(sl_print_buff, "%d/%d", mac_rx, mac_tx_succ);
  sl_graphics_print_block(sl_print_buff,
                          3,
                          SL_SIDE_RIGHT,
                          SL_ALIGN_RIGHT,
                          SL_FONT_NORMAL);

  sprintf(sl_print_buff,
          "%d/%d   %d/%d",
          mac_retry,
          mac_fail,
          mac_rx_bc,
          mac_tx_bc);

  sl_graphics_print_line(sl_print_buff, 4, SL_ALIGN_RIGHT, SL_FONT_NORMAL);
  DMD_updateDisplay();
}

/**************************************************************************//**
 * @brief Draw APS statistics to the screen
 * 
 * @param aps_rx APS Rx count
 * @param aps_tx_succ APS Tx success count
 * @param aps_retry APS retry count
 * @param aps_fail APS failure count
 * @param aps_rx_bc APS broadcast Rx count
 * @param aps_tx_bc APS broadcast Tx count
 *****************************************************************************/
void sl_graphics_draw_aps_errors(uint16_t aps_rx, 
                                 uint16_t aps_tx_succ, 
                                 uint16_t aps_retry, 
                                 uint16_t aps_fail, 
                                 uint16_t aps_rx_bc, 
                                 uint16_t aps_tx_bc)
{
  sl_graphics_clear_line(5);
  sl_graphics_clear_line(6);
  sl_graphics_print_block("APS:",
                          5, 
                          SL_SIDE_LEFT, 
                          SL_ALIGN_LEFT, 
                          SL_FONT_NORMAL);

  sprintf(sl_print_buff, "%d/%d", aps_rx, aps_tx_succ);
  sl_graphics_print_block(sl_print_buff,
                          5,
                          SL_SIDE_RIGHT, 
                          SL_ALIGN_RIGHT, 
                          SL_FONT_NORMAL);

  sprintf(sl_print_buff,
          "%d/%d   %d/%d",
          aps_retry, 
          aps_fail,
          aps_rx_bc,
          aps_tx_bc);
  sl_graphics_print_line(sl_print_buff, 6, SL_ALIGN_RIGHT, SL_FONT_NORMAL);
  DMD_updateDisplay();
}