/***************************************************************************//**
 * @file
 * @brief PIR Config
 * @version 1.0.2
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef PIR_CONFIG_H_
#define PIR_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EFM32PG12B500F1024GL125 /* BRD2501: SLSTK3402A: PG12 STK */

// PIR Occupancy Sensor Analog Pins
#define ADC_P_PORT        gpioPortC
#define ADC_P_PIN         9
#define ADC_P_APORT       adcPosSelAPORT2XCH9

#define ADC_N_PORT        gpioPortA
#define ADC_N_PIN         6
#define ADC_N_APORT       adcNegSelAPORT4YCH14

#define MOTION_B_PORT     gpioPortD
#define MOTION_B_PIN      10

#define LDO_SHDN_B_PORT   gpioPortD
#define LDO_SHDN_B_PIN    8

#elif defined(EFR32MG12P332F1024GL125) /* BRD4001A (WSTK) + BRD4162A (MG12) */

// PIR Occupancy Sensor Analog Pins
#define ADC_P_PORT        gpioPortD
#define ADC_P_PIN         8
#define ADC_P_APORT       adcPosSelAPORT3XCH0

#define ADC_N_PORT        gpioPortA
#define ADC_N_PIN         6
#define ADC_N_APORT       adcNegSelAPORT4YCH14

#define MOTION_B_PORT     gpioPortB
#define MOTION_B_PIN      6

#define LDO_SHDN_B_PORT   gpioPortC
#define LDO_SHDN_B_PIN    9

#elif defined(ZGM130S037HGN) /* BRD4001A (WSTK) + BRD4202A (ZGM130S) */

// PIR Occupancy Sensor Analog Pins
#define ADC_P_PORT        gpioPortA
#define ADC_P_PIN         2
#define ADC_P_APORT       adcPosSelAPORT3XCH10

#define ADC_N_PORT        gpioPortC
#define ADC_N_PIN         6
#define ADC_N_APORT       adcNegSelAPORT2YCH6

// Use LED0 on WSTK
#define MOTION_B_PORT     gpioPortF
#define MOTION_B_PIN      4

#define LDO_SHDN_B_PORT   gpioPortF
#define LDO_SHDN_B_PIN    3

#endif

#ifdef __cplusplus
}
#endif

#endif // PIR_CONFIG_H
