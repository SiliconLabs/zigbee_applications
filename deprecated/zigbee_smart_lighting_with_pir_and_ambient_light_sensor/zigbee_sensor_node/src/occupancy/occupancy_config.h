/***************************************************************************//**
 * @file
 * @brief Header file for demo of OCCUPANCY-EXP-EB
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

#ifndef SRC_OCCUPANCY_EXP_MAIN_H_
#define SRC_OCCUPANCY_EXP_MAIN_H_

#include "em_adc.h"
#include "em_opamp.h"
#include "em_gpio.h"

#ifdef EFM32PG12B500F1024GL125
/* BRD2501: SLSTK3402A: PG12 STK*/

/* PIR Occupancy Sensor Analog Pins */
/* If PIR_SettleCapacitor is used on a part that has errata
 * VDAC_E201, this should be placed on a APORT X bus. */
  #define ADC_P_PORT        gpioPortC
  #define ADC_P_PIN         9
  #define ADC_P_APORT       adcPosSelAPORT2XCH9
  #define ADC_P_OPA_APORT   opaPosSelAPORT2XCH9

/* If PIR_SettleCapacitor is used on a part that has errata
 * VDAC_E201, this should be placed on a APORT Y bus. */
  #define ADC_N_PORT        gpioPortA
  #define ADC_N_PIN         6
  #define ADC_N_APORT       adcNegSelAPORT4YCH14
  #define ADC_N_OPA_OUTMODE opaOutModeAPORT4YCH14

  #define LED_PORT          BSP_GPIO_LED0_PORT
  #define LED_PIN           BSP_GPIO_LED0_PIN

  #define MOTION_B_PORT     gpioPortD
  #define MOTION_B_PIN      10

  #define LDO_SHDN_B_PORT   gpioPortD
  #define LDO_SHDN_B_PIN    8

/* Si1153 Ambient Light Sensor Pins */
  #define SENSOR_SCL_PORT   gpioPortC
  #define SENSOR_SCL_PIN    11

  #define SENSOR_SDA_PORT   gpioPortC
  #define SENSOR_SDA_PIN    10

  #define SENSOR_INT_PORT   gpioPortD
  #define SENSOR_INT_PIN    11

#elif defined(EFR32MG12P332F1024GL125)
/* BRD4001A (WSTK) + BRD4162A (MG12) */
/* PIR Occupancy Sensor Analog Pins */
/* If PIR_SettleCapacitor is used on a part that has errata
 * VDAC_E201, this should be placed on a APORT X bus. */
  #define ADC_P_PORT        gpioPortD
  #define ADC_P_PIN         8
  #define ADC_P_APORT       adcPosSelAPORT3XCH0
  #define ADC_P_OPA_APORT   opaPosSelAPORT3XCH0

/* If PIR_SettleCapacitor is used on a part that has errata
 * VDAC_E201, this should be placed on a APORT Y bus. */
  #define ADC_N_PORT        gpioPortA
  #define ADC_N_PIN         6
  #define ADC_N_APORT       adcNegSelAPORT4YCH14
  #define ADC_N_OPA_OUTMODE opaOutModeAPORT4YCH14

  #define LED_PORT          BSP_GPIO_LED0_PORT
  #define LED_PIN           BSP_GPIO_LED0_PIN

  #define MOTION_B_PORT     gpioPortB
  #define MOTION_B_PIN      6

  #define LDO_SHDN_B_PORT   gpioPortC
  #define LDO_SHDN_B_PIN    9

/* Si1153 Ambient Light Sensor Pins */
  #define SENSOR_SCL_PORT   gpioPortC
  #define SENSOR_SCL_PIN    10

  #define SENSOR_SDA_PORT   gpioPortC
  #define SENSOR_SDA_PIN    11

  #define SENSOR_INT_PORT   gpioPortB
  #define SENSOR_INT_PIN    7

#endif

/**
 * @brief Enum for the possible demo modes.
 */
enum {
  demoModeOccupancy=0,
  demoModeAmbientLight=1,
  demoModeOccupancyLowPower=2,
  demoModeAmbientLightLowPower=3,
} typedef Demo_Mode_TypeDef;

#endif /* SRC_OCCUPANCY_EXP_MAIN_H_ */
