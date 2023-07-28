/***************************************************************************//**
 * @file  ambient_light.h
 * @brief Header file for ambient light sensing demo for OCCUPANCY-EXP-EB.
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
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/

#ifndef SRC_AMBIENT_LIGHT_H_
#define SRC_AMBIENT_LIGHT_H_

#include <stdbool.h>
#include "em_i2c.h"

/*******************************************************************************
 ********************************   DEFINES  ***********************************
 ******************************************************************************/
#define SI1153_I2C_ADDR 0x53                                  /**< Si1153 default I2C address */

#define CHANNEL_OFFSET                        2               /**< Parameter table offset for channel configuration */
#define CHANNEL_LENGTH                        4               /**< Parameter table registers per channel */
#define ADCCONFIG_OFFSET                      0               /**< Parameter table offset within channel for ADCCONFIG_OFFSET */
#define ADCSENS_OFFSET                        1               /**< Parameter table offset within channel for ADCSENS_OFFSET */
#define ADCPOST_OFFSET                        2               /**< Parameter table offset within channel for ADCPOST_OFFSET */
#define MEASCONFIG_OFFSET                     3               /**< Parameter table offset within channel for MEASCONFIG_OFFSET */

/** Channel field shifts and masks. */
#define _SI1150_ADCPOST_POSTSHIFT_SHIFT       3               /**< Shift value for POSTSHIFT */
#define _SI1150_ADCPOST_POSTSHIFT_MASK        0x38            /**< Bit mask value for POSTSHIFT */
#define _SI1150_ADCPOST_RES24BIT_OUT_SHIFT    6               /**< Shift value for RES24BIT */
#define _SI1150_ADCPOST_RES24BIT_OUT_MASK     0x40            /**< Bit mask value for RES24BIT */
#define _SI1150_ADCONFIG_ADCMUX_SHIFT         0               /**< Shift value for ADCMUX */
#define _SI1150_ADCCONFIG_ADCMUX_MASK         0x1f            /**< Bit mask value for ADCMUX */
#define _SI1150_ADCCONFIG_DECIM_SHIFT         5               /**< Shift value for DECIM */
#define _SI1150_ADCCONFIG_DECIM_MASK          0x60            /**< Bit mask value for DECIM */
#define _SI1150_ADCSENS_SWGAIN_SHIFT          4               /**< Shift value for SWGAIN */
#define _SI1150_ADCSENS_SWGAIN_MASK           0x70            /**< Bit mask value for SWGAIN */
#define _SI1150_ADCSENS_HSIG_SHIFT            7               /**< Shift value for HSIG */
#define _SI1150_ADCSENS_HSIG_MASK             0x80            /**< Bit mask value for HSIG */
#define _SI1150_ADCSENS_HWGAIN_SHIFT          0               /**< Shift value for HWGAIN */
#define _SI1150_ADCSENS_HWGAIN_MASK           0x0f            /**< Bit mask value for HWGAIN */

/** Miscellaneous defines */
#define MAX_BLOCK_LENGTH 64

#define DELAY_10000_US           10000

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/** ADC MUX mode */
typedef enum {
  si1150AdcMuxSmallIr = 0,            /**< Measure small IR photo diode */
  si1150AdcMuxMediumIr = 1,           /**< Measure medium IR photo diode */
  si1150AdcMuxLargeIr = 2,            /**< Measure large IR photo diode */
  si1150AdcMuxSmallVisible = 0x0b,    /**< Measure small visible photo diode */
  si1150AdcMuxMediumVisible = 0x0d,   /**< Measure medium visible photo diode */
} Si1150_AdcMux_TypeDef;

/** ADC decimation selection */
typedef enum {
  si1150Decim1024 = 0,                  /**< ADC decimation rate 1024*/
  si1150Decim2048 = 1,                  /**< ADC decimation rate 2048*/
  si1150Decim4096 = 2,                  /**< ADC decimation rate 4096*/
  si1150Decim512 = 3                    /**< ADC decimation rate 512*/
} Si1150_Decim_TypeDef;

/** ADC software gain selection */
typedef enum {
  si1150SwGain1 = 0,                    /**< Software accumulation of 1 sample. */
  si1150SwGain2 = 1,                    /**< Software accumulation of 2 samples. */
  si1150SwGain4 = 2,                    /**< Software accumulation of 4 samples. */
  si1150SwGain8 = 3,                    /**< Software accumulation of 8 samples. */
  si1150SwGain16 = 4,                   /**< Software accumulation of 16 samples. */
  si1150SwGain32 = 5,                   /**< Software accumulation of 32 samples. */
  si1150SwGain64 = 6,                   /**< Software accumulation of 64 samples. */
  si1150SwGain128 = 7,                  /**< Software accumulation of 128 samples. */
} Si1150_SwGain_TypeDef;

/** ADC signal range selection */
typedef enum {
  si1150NormalRange = 0,                /**< Normal signal range. */
  si1150HighRange = 1                   /**< High signal range */
} Si1150_Hsig_TypeDef;

/** ADC hardware gain selection */
typedef enum {
  si1150HwGain1 = 0,                  /**< Hardware sensitivity 1x. */
  si1150HwGain2 = 1,                  /**< Hardware sensitivity 2x. */
  si1150HwGain4 = 2,                  /**< Hardware sensitivity 4x. */
  si1150HwGain8 = 3,                  /**< Hardware sensitivity 8x. */
  si1150HwGain16 = 4,                 /**< Hardware sensitivity 16x. */
  si1150HwGain32 = 5,                 /**< Hardware sensitivity 32x. */
  si1150HwGain64 = 6,                 /**< Hardware sensitivity 64x. */
  si1150HwGain128 = 7,                /**< Hardware sensitivity 128x. */
} Si1150_HwGain_TypeDef;

/** ADC sampling mode selection */
typedef enum {
  si1150ModeAuto = 0,                 /**< Use autonomous mode to interleave the burst measurements. */
  si1150ModeForce = 1                 /**< Use force mode to sequentially perform the measurements. */
} Si1150_Mode_TypeDef;

/** Channel initialization structure. */
typedef struct {
  Si1150_AdcMux_TypeDef adcMux;   /**< Photo diode selection. */
  Si1150_Decim_TypeDef decim;     /**< ADC decimation rate. */
  Si1150_HwGain_TypeDef hwGain;   /**< Hardware gain. Affects sensitivity and integration time. */
  Si1150_SwGain_TypeDef swGain;   /**< Software gain. Accumulates samples. */
  Si1150_Hsig_TypeDef hsig;       /**< Enable high signal range */
  uint8_t postShift;              /**< Bits to right shift after measurement. */
  bool res24bit;                  /**< Output resolution. */
} Si1150_InitChannel_TypeDef;


/** Default channel initialization matching register reset values. **/
#define SI1150_INITCHANNEL_DEFAULT \
  {                                \
    si1150AdcMuxSmallIr,           \
    si1150Decim1024,               \
    si1150HwGain1,                 \
    si1150SwGain1,                 \
    si1150NormalRange,             \
    1,                             \
    0,                             \
  }


/**
 * @brief callback for motion detection event
 */
typedef void (*get_lux_completed_callback_t)(uint16_t);

/*******************************************************************************
 ******************************  PROTOTYPES  ***********************************
 ******************************************************************************/
void als_init(get_lux_completed_callback_t callback_registration);

#endif /* SRC_AMBIENT_LIGHT_H_ */
