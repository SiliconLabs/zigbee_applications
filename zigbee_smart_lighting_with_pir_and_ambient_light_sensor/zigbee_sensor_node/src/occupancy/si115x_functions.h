/***************************************************************************//**
 * @file
 * @brief Si115x driver functions
 * @version
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef SI115X_FUNCTIONS_H
#define SI115X_FUNCTIONS_H

/***************************************************************************//**
 * @addtogroup Drivers
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup Si115x
 * @{
 ******************************************************************************/

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef void *            HANDLE;
/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/*******************************************************************************
 ***************   Functions Needed by Si115x_functions.c   ********************
 ******************************************************************************/

int16_t Si11xxWriteToRegister(HANDLE si115x_handle,
                              uint8_t address,
                              uint8_t value);
int16_t Si11xxReadFromRegister(HANDLE si115x_handle, uint8_t address);
int16_t Si11xxBlockWrite(HANDLE si115x_handle,
                         uint8_t  address,
                         uint8_t  length,
                         uint8_t* values);
int16_t Si11xxBlockRead(HANDLE si115x_handle,
                        uint8_t  address,
                        uint8_t  length,
                        uint8_t* values);
void delay_10ms(void);

/*******************************************************************************
 ***************   Functions supplied by Si115x_functions.c   ******************
 ******************************************************************************/

int16_t  Si115xReset(HANDLE si115x_handle);
int16_t  Si115xNop(HANDLE si115x_handle);
int16_t  Si115xForce(HANDLE si115x_handle);
int16_t  Si115xPause(HANDLE si115x_handle);
int16_t  Si115xStart(HANDLE si115x_handle);
int16_t  Si115xParamSet(HANDLE si115x_handle, uint8_t address, uint8_t value);
int16_t  Si115xParamRead(HANDLE si115x_handle, uint8_t address);

/*******************************************************************************
 ************************** Si115x I2C Registers *******************************
 ******************************************************************************/
/// @cond DOXYGEN_SHOULD_SKIP_THIS

#define REG_PART_ID      0x00
#define REG_HW_ID        0x01
#define REG_REV_ID       0x02
#define REG_INFO0        0x03
#define REG_INFO1        0x04
#define REG_HOSTIN3      0x07
#define REG_HOSTIN2      0x08
#define REG_HOSTIN1      0x09
#define REG_HOSTIN0      0x0A
#define REG_COMMAND      0x0B
#define REG_IRQ_ENABLE   0x0F
#define REG_RESPONSE1    0x10
#define REG_RESPONSE0    0x11
#define REG_IRQ_STATUS   0x12
#define REG_HOSTOUT0     0x13
#define REG_HOSTOUT1     0x14
#define REG_HOSTOUT2     0x15
#define REG_HOSTOUT3     0x16
#define REG_HOSTOUT4     0x17
#define REG_HOSTOUT5     0x18
#define REG_HOSTOUT6     0x19
#define REG_HOSTOUT7     0x1A
#define REG_HOSTOUT8     0x1B
#define REG_HOSTOUT9     0x1C
#define REG_HOSTOUT10    0x1D
#define REG_HOSTOUT11    0x1E
#define REG_HOSTOUT12    0x1F
#define REG_HOSTOUT13    0x20
#define REG_HOSTOUT14    0x21
#define REG_HOSTOUT15    0x22
#define REG_HOSTOUT16    0x23
#define REG_HOSTOUT17    0x24
#define REG_HOSTOUT18    0x25
#define REG_HOSTOUT19    0x26
#define REG_HOSTOUT20    0x27
#define REG_HOSTOUT21    0x28
#define REG_HOSTOUT22    0x29
#define REG_HOSTOUT23    0x2A
#define REG_HOSTOUT24    0x2B
#define REG_HOSTOUT25    0x2C
#define REG_OTP_CONTROL  0x2F
#define REG_CHIP_STAT    0x30
//
/// @endcond

/*******************************************************************************
 ************************** Si115x I2C Parameter Offsets ***********************
 ******************************************************************************/
/// @cond DOXYGEN_SHOULD_SKIP_THIS
#define PARAM_I2C_ADDR      0x00
#define PARAM_CH_LIST       0x01
#define PARAM_ADCCONFIG0    0x02
#define PARAM_ADCSENS0      0x03
#define PARAM_ADCPOST0      0x04
#define PARAM_MEASCONFIG0   0x05
#define PARAM_ADCCONFIG1    0x06
#define PARAM_ADCSENS1      0x07
#define PARAM_ADCPOST1      0x08
#define PARAM_MEASCONFIG1   0x09
#define PARAM_ADCCONFIG2    0x0A
#define PARAM_ADCSENS2      0x0B
#define PARAM_ADCPOST2      0x0C
#define PARAM_MEASCONFIG2   0x0D
#define PARAM_ADCCONFIG3    0x0E
#define PARAM_ADCSENS3      0x0F
#define PARAM_ADCPOST3      0x10
#define PARAM_MEASCONFIG3   0x11
#define PARAM_ADCCONFIG4    0x12
#define PARAM_ADCSENS4      0x13
#define PARAM_ADCPOST4      0x14
#define PARAM_MEASCONFIG4   0x15
#define PARAM_ADCCONFIG5    0x16
#define PARAM_ADCSENS5      0x17
#define PARAM_ADCPOST5      0x18
#define PARAM_MEASCONFIG5   0x19
#define PARAM_MEASRATE_H    0x1A
#define PARAM_MEASRATE_L    0x1B
#define PARAM_MEASCOUNT0    0x1C
#define PARAM_MEASCOUNT1    0x1D
#define PARAM_MEASCOUNT2    0x1E
#define PARAM_LED1_A        0x1F
#define PARAM_LED1_B        0x20
#define PARAM_LED2_A        0x21
#define PARAM_LED2_B        0x22
#define PARAM_LED3_A        0x23
#define PARAM_LED3_B        0x24
#define PARAM_THRESHOLD0_H  0x25
#define PARAM_THRESHOLD0_L  0x26
#define PARAM_THRESHOLD1_H  0x27
#define PARAM_THRESHOLD1_L  0x28
#define PARAM_THRESHOLD2_H  0x29
#define PARAM_THRESHOLD2_L  0x2A
#define PARAM_BURST         0x2B
/// @endcond

/*******************************************************************************
 *******    Si115x Register and Parameter Bit Definitions  *********************
 ******************************************************************************/
/// @cond DOXYGEN_SHOULD_SKIP_THIS

#define RSP0_CHIPSTAT_MASK  0xe0
#define RSP0_COUNTER_MASK   0x1f
#define RSP0_SLEEP          0x20

/// @endcond

#ifdef __cplusplus
}
#endif

/** @} (end group Si115x) */
/** @} (end group Drivers) */

#endif // #define SI115X_FUNCTIONS_H
