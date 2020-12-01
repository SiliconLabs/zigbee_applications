/***************************************************************************//**
 * @file  ambient_light.c
 * @brief Ambient light sensing demo for the OCCUPANCY-EXP-EB.
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

#include <stdbool.h>
#include <stdio.h>

#include "em_cmu.h"
#include "em_emu.h"

#include "em_gpio.h"
#include "gpiointerrupt.h"

#include "em_i2c.h"
#include "i2cspm.h"

#include "si115x_functions.h"
#include "ambient_light.h"
#include "occupancy_config.h"

#include "app/framework/include/af.h"

#define PERIOD_READ_LUX_MS            1000

static Si1150_Mode_TypeDef mode = si1150ModeForce;

HANDLE si115xHandle;                /** Si115x programmer's toolkit handle for I2C. */

EmberEventControl ambientLightSensorReadDataEventControl;
EmberEventControl ambientLightSensorInterruptEventControl;

/*******************************************************************************
 ******************************  Local Function  *******************************
 ******************************************************************************/
static int32_t initial(HANDLE si115xHandle, Si1150_Mode_TypeDef mode);
static uint32_t read_lux(HANDLE si115xHandle);
static void sensor_int_irq_handler(uint8_t pin);
static get_lux_completed_callback_t get_lux_completed_callback;

/*******************************************************************************
 ******************************  FUNCTIONS   ***********************************
 ******************************************************************************/

/** @brief handler for ambientLightSensorReadDataEventControl
 *
 */
void ambientLightSensorReadDataEventHandler()
{
  emberEventControlSetInactive(ambientLightSensorReadDataEventControl);
  Si11xxReadFromRegister(si115xHandle, REG_IRQ_STATUS);   // ensure INT is cleared
  if (mode == si1150ModeAuto) {
    Si115xStart(si115xHandle);
  } else {
    Si115xForce(si115xHandle);
  }
}

/** @brief handler for ambientLightSensorInterruptEventControl
 *
 */
void ambientLightSensorInterruptEventHandler()
{
  emberEventControlSetInactive(ambientLightSensorInterruptEventControl);
  uint32_t lux = read_lux(si115xHandle);

  get_lux_completed_callback(lux);

  /* repeat read lux value every 1s */
  emberEventControlSetDelayMS(ambientLightSensorReadDataEventControl, PERIOD_READ_LUX_MS);
}

/**
 * @brief init code for ambient light sensing.
 *
 * @param[in] lowPowerMode  Disables LCD for accurate current measurements when true.
 */
void als_init(get_lux_completed_callback_t callback_registration)
{
  /* Setup I2C and INT interfaces to Si1153. */
  I2CSPM_Init_TypeDef i2cSpmInit = I2CSPM_INIT_DEFAULT;
  i2cSpmInit.sclPort  = SENSOR_SCL_PORT;
  i2cSpmInit.sclPin   = SENSOR_SCL_PIN;
  i2cSpmInit.sdaPort  = SENSOR_SDA_PORT;
  i2cSpmInit.sdaPin   = SENSOR_SDA_PIN;
  I2CSPM_Init(&i2cSpmInit);

  NVIC_DisableIRQ(GPIO_ODD_IRQn);
  NVIC_DisableIRQ(GPIO_EVEN_IRQn);

  //gpio pinmode set
  GPIO_PinModeSet(SENSOR_INT_PORT, SENSOR_INT_PIN, gpioModeInput, 1);

  //register callback
  GPIOINT_CallbackRegister(SENSOR_INT_PIN, sensor_int_irq_handler);

  //interrupt config
  GPIO_IntConfig(SENSOR_INT_PORT, SENSOR_INT_PIN, false, true, true);

  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);

  /** Set MCU to lowest power modes. */
  EMU_EM01Init_TypeDef em01Init =  EMU_EM01INIT_DEFAULT;
  em01Init.vScaleEM01LowPowerVoltageEnable = true;
  EMU_EM01Init(&em01Init);

  EMU_EM23Init_TypeDef em23Init = EMU_EM23INIT_DEFAULT;
  em23Init.em23VregFullEn = 0;
  em23Init.vScaleEM23Voltage = emuVScaleEM23_LowPower;
  EMU_EM23Init(&em23Init);

  /* Configure Si1153 for ambient light sensing. */
  int32_t retval = initial(si115xHandle, mode);

  if (retval < 0) {
    /* Si1153 failed to initialize. */
    emberAfCorePrintln("Si1153 initialization failed.\n");

    /* Infinite loop here if error occurred. */
    while (1);
  }

  get_lux_completed_callback = callback_registration;
  emberEventControlSetDelayMS(ambientLightSensorReadDataEventControl, PERIOD_READ_LUX_MS);
}

/***************************************************************************//**
 * @brief
 *   Update with Si115x with a channel configuration.
 *
 * @param[in] si115xHandle  Si115x programmer's toolkit handle.
 * @param[in] ch            Channel number to configure [0-5]
 * @param[in] init          Channel configuration to load.
 ******************************************************************************/
void Si1150_InitChannel(HANDLE si115xHandle, uint8_t ch, Si1150_InitChannel_TypeDef *init)
{
  uint8_t adcconfig = (init->adcMux << _SI1150_ADCONFIG_ADCMUX_SHIFT)
                      | (init->decim << _SI1150_ADCCONFIG_DECIM_SHIFT);

  uint8_t adcsens = (init->hwGain << _SI1150_ADCSENS_HWGAIN_SHIFT)
                    | (init->swGain << _SI1150_ADCSENS_SWGAIN_SHIFT)
                    | (init->hsig << _SI1150_ADCSENS_HSIG_SHIFT);

  /* Does not include thresh_en, thresh_pol. */
  uint8_t adcpost = ((init->postShift << _SI1150_ADCPOST_POSTSHIFT_SHIFT) & _SI1150_ADCPOST_POSTSHIFT_MASK)
                    | (init->res24bit << _SI1150_ADCPOST_RES24BIT_OUT_SHIFT);

  /* Does not include measconfig. */
  uint8_t measconfig = 0;
  int16_t retval = 0;

  uint8_t adcconfig_addr = ch * CHANNEL_LENGTH + CHANNEL_OFFSET + ADCCONFIG_OFFSET;
  uint8_t adcsens_addr = ch * CHANNEL_LENGTH + CHANNEL_OFFSET + ADCSENS_OFFSET;
  uint8_t adcpost_addr = ch * CHANNEL_LENGTH + CHANNEL_OFFSET + ADCPOST_OFFSET;
  uint8_t measconfig_addr = ch * CHANNEL_LENGTH + CHANNEL_OFFSET + MEASCONFIG_OFFSET;

  retval += Si115xParamSet(si115xHandle, adcconfig_addr, adcconfig);
  retval += Si115xParamSet(si115xHandle, adcsens_addr, adcsens);
  retval += Si115xParamSet(si115xHandle, adcpost_addr, adcpost);
  retval += Si115xParamSet(si115xHandle, measconfig_addr, measconfig);
}

/***
 * @brief Initializes the Si115x for ALS measurements.
 * @param[in] mode  Selects autonomous or forced mode operation.
 */
int32_t initial(HANDLE si115xHandle, Si1150_Mode_TypeDef mode)
{
  int16_t retval;
  retval  = Si115xReset(si115xHandle);
  delay_10ms();

  Si1150_InitChannel_TypeDef ch0Visible = SI1150_INITCHANNEL_DEFAULT;
  ch0Visible.hwGain = si1150HwGain2;
  ch0Visible.swGain = si1150SwGain16;
  ch0Visible.hsig = si1150HighRange;
  ch0Visible.decim = si1150Decim512;
  ch0Visible.res24bit = true;
  ch0Visible.postShift = 0;
  ch0Visible.adcMux = si1150AdcMuxMediumVisible;
  Si1150_InitChannel(si115xHandle, 0, &ch0Visible);

  Si1150_InitChannel_TypeDef ch1Infrared = SI1150_INITCHANNEL_DEFAULT;
  ch1Infrared.hwGain = si1150HwGain2;
  ch1Infrared.swGain = si1150SwGain16;
  ch1Infrared.hsig = si1150HighRange;
  ch1Infrared.decim = si1150Decim512;
  ch1Infrared.res24bit = true;
  ch1Infrared.postShift = 2;
  ch1Infrared.adcMux = si1150AdcMuxMediumIr;
  Si1150_InitChannel(si115xHandle, 1, &ch1Infrared);

  /* Initialize global section. */
  retval += Si115xParamSet(si115xHandle, PARAM_CH_LIST, 0x03);
  if (mode == si1150ModeAuto) {
    retval += Si115xParamSet(si115xHandle, PARAM_MEASRATE_L, 0x02);
    retval += Si115xParamSet(si115xHandle, PARAM_MEASCOUNT0, 0x1);
    retval += Si115xParamSet(si115xHandle, PARAM_MEASCONFIG0, 0x80);
    retval += Si115xParamSet(si115xHandle, PARAM_MEASCONFIG1, 0x80);
  }

  retval += Si11xxWriteToRegister(si115xHandle, REG_IRQ_ENABLE, 0x03);
  return retval;
}

/**
 * @brief
 *  Calculates lux from Si115x data after an interrupt.
 *
 * @param[in] si115xHandle
 *  Si115x programmer's toolkit handle
 *
 * @return
 *  Measured illuminance in lux.
 */
uint32_t read_lux(HANDLE si115xHandle)
{
  uint8_t buffer[7];
  int32_t visiblePd, irPd;

  /* Clears IRQ_STATUS and reads the 24-bit outputs for CH0 and CH1. */
  Si11xxBlockRead(si115xHandle, REG_IRQ_STATUS, 7, buffer);
  visiblePd = (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
  irPd = (buffer[4] << 16) + (buffer[5] << 8) + buffer[6];

  /* Sign extend outputs to 24-bit words. */
  if (visiblePd & 0x800000) {
    visiblePd |= 0xFF000000;
  }

  if (irPd & 0x800000) {
    irPd |= 0xFF000000;
  }

  /* Convert IR + Visible measurements to lux based on model. */
  float irDivVisible = ((float) irPd) / visiblePd;
  int32_t lux;
  uint32_t scaleFactor = 4; /* Scale to the sw_avg=64 case. */
  if (irDivVisible > 1) {
    lux = (int32_t) scaleFactor * (0.0954 * visiblePd + -0.0137 * irPd);
  } else {
    if (irDivVisible > 0.7) {
      lux = (int32_t) scaleFactor  * (0.1838 * visiblePd + -0.0817 * irPd);
    } else {
      lux = (int32_t) scaleFactor * (-0.2153 * visiblePd + 0.5317 * irPd);
    }
  }

  return lux;
}

/**
 * @brief
 *  Writes a single byte over I2C to a host register for the Si11xx sensors.
 *
 * @param[in] si115x_handle
 *  Si115x programmer's toolkit handle.
 *
 * @param[in] address
 *  Register address
 *
 * @param[in] value
 *  Register value to write.
 *
 * @returns
 *  0 for success, < 0 for I2C failure.
 */
int16_t Si11xxWriteToRegister(HANDLE si115x_handle, uint8_t address, uint8_t value)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2cWriteData[2];
  uint8_t i2cReadData[1];

  uint16_t slaveAddr = SI1153_I2C_ADDR;
  seq.addr  = slaveAddr << 1;
  seq.flags = I2C_FLAG_WRITE;

  i2cWriteData[0] = address;
  i2cWriteData[1] = value;
  seq.buf[0].data   = i2cWriteData;
  seq.buf[0].len    = 2;
  seq.buf[1].data   = i2cReadData;
  seq.buf[1].len    = 0;

  ret = I2CSPM_Transfer(I2C0, &seq);

  if (ret != i2cTransferDone) {
    return -1;
  }
  return 0;
}

/**
 * @brief
 *  Reads a single byte over I2C for the Si11xx sensors.
 *
 * @param[in] si115x_handle
 *  Si115x programmer's toolkit handle.
 *
 * @param[in] address
 *  Register address
 *
 * @returns
 *  Register value. < 0 for I2C failure.
 */
int16_t Si11xxReadFromRegister(HANDLE si115x_handle, uint8_t address)
{
  (void) si115x_handle;
  I2C_TransferReturn_TypeDef ret;
  uint8_t data;
  uint16_t slaveAddr = SI1153_I2C_ADDR;
  I2C_TransferSeq_TypeDef seq;
  uint8_t i2cWriteData[1];

  seq.addr  = slaveAddr << 1;
  seq.flags = I2C_FLAG_WRITE_READ;

  i2cWriteData[0]   = address;
  seq.buf[0].data   = i2cWriteData;
  seq.buf[0].len    = 1;
  seq.buf[1].data   = &data;
  seq.buf[1].len    = 1;

  ret = I2CSPM_Transfer(I2C0, &seq);

  if (ret != i2cTransferDone) {
    data = 0xFF;
    return -1;
  }
  return data;
}

/**
 * @brief
 *  Reads a register block over I2C for Si11xx sensors.
 *
 * @param[in] si115x_handle
 *  Si115x programmer's toolkit handle.
 *
 * @param[in] address
 *  Register address
 *
 * @param[in] length
 *  Number of bytes to read.
 *
 * @param[out] values
 *  Pointer to array of uint8_t to hold output data.
 *
 * @returns
 *  0 for success, < 0 for I2C failure.
 */
int16_t Si11xxBlockRead(HANDLE si115x_handle, uint8_t  address, uint8_t  length, uint8_t* values)
{
  (void) si115x_handle;
  uint16_t slaveAddr = SI1153_I2C_ADDR;
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2cWriteData[1];

  seq.addr  = slaveAddr << 1;
  seq.flags = I2C_FLAG_WRITE_READ;

  i2cWriteData[0]   = address;
  seq.buf[0].data   = i2cWriteData;
  seq.buf[0].len    = 1;
  seq.buf[1].data   = values;
  seq.buf[1].len    = length;

  ret = I2CSPM_Transfer(I2C0, &seq);

  if (ret != i2cTransferDone) {
    for (uint8_t i = 0; i < length; i++) {
      values[i] = 0xFF;
    }
  }
  if (ret != i2cTransferDone) {
    return -1;
  }
  return 0;
}

/**
 * @brief
 *  Writes a register block over I2C for Si11xx sensors.
 *
 * @param[in] si115x_handle
 *  Si115x programmer's toolkit handle.
 *
 * @param[in] address
 *  Register address
 *
 * @param[in] length
 *  Number of bytes to read.
 *
 * @param[out] values
 *  Pointer to array of uint8_t of values to write.
 *
 * @returns
 *  0 for success, < 0 for I2C failure.
 */
int16_t Si11xxBlockWrite(HANDLE si115x_handle, uint8_t  address, uint8_t  length, uint8_t* values)
{
  (void) si115x_handle;
  uint16_t slaveAddr = SI1153_I2C_ADDR;
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  uint8_t i2cReadData[1];


  uint8_t i2cWriteData[MAX_BLOCK_LENGTH];
  if (length > MAX_BLOCK_LENGTH) {
    return i2cTransferUsageFault;
  }
  i2cWriteData[0] = address;
  for (uint8_t i = 0; i < length; i++) {
    i2cWriteData[i + 1] = values[i];
  }

  seq.addr  = slaveAddr << 1;
  seq.flags = I2C_FLAG_WRITE;

  seq.buf[0].data   = i2cWriteData;
  seq.buf[0].len    = length + 1;
  seq.buf[1].data   = i2cReadData;
  seq.buf[1].len    = 0;

  ret = I2CSPM_Transfer(I2C0, &seq);

  if (ret != i2cTransferDone) {
    return -1;
  }
  return 0;
}

/**
 * @brief
 *  Implements a delay function for Si115x startup and reset times.
 *
 * @note
 *  Does not account for clock scaling nor is an accurate delay.
 */
void delay_10ms(void)
{
  USTIMER_DelayIntSafe(DELAY_10000_US);
}

/***************************************************************************//**
 * @brief sensor_int_IRQHandler Si1115's data is ready to read
 ******************************************************************************/
void sensor_int_irq_handler(uint8_t pin)
{
  if (pin == SENSOR_INT_PIN) {
      emberEventControlSetActive(ambientLightSensorInterruptEventControl);
  }
}

