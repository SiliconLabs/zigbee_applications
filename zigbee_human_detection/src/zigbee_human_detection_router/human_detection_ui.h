/***************************************************************************//**
 * @file human_detection_ui.h
 * @brief human detection_ui.h display header file.
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
 * 1. The origin of this software must not be misrepresented{} you must not
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

#ifndef HUMAN_DETECTION_UI_H_
#define HUMAN_DETECTION_UI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/***************************************************************************//**
 * @brief
 *    OLED initialize.
 *
 ******************************************************************************/
void human_detection_ui_oled_init(void);

/***************************************************************************//**
 * @brief
 *    Application Display initialize.
 *
 ******************************************************************************/
void human_detection_ui_init(void);

/***************************************************************************//**
 * Show error when human_detection_recognition_init error
 ******************************************************************************/
void human_detection_ui_error(void);

/***************************************************************************//**
 * @brief
 *    Application Human Detection Display.
 *
 * @param[in] result
 *    Index of class ID.
 * @param[in] score
 *    Score of predict.
 *
 * @details
 *    This function displays the human detection in OLED.
 *
 ******************************************************************************/
void human_detection_ui_predict_update(uint8_t result, uint8_t score);

/***************************************************************************//**
 * @brief
 *    Update the Network status.
 *
 * @param[in] status
 *    Network status.
 *
 * @param[in] id
 *    Node ID.
 *
 * @details
 *    This function displays the human detection in OLED.
 *
 ******************************************************************************/
void human_detection_ui_network_status_update(uint8_t status, uint16_t id);

#ifdef __cplusplus
}
#endif

#endif /* HUMAN_DETECTION_UI_H_ */
