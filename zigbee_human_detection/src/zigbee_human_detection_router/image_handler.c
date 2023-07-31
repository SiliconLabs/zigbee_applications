/***************************************************************************//**
 * @file image_handler.cc
 * @brief image handler file.
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
#include "sl_i2cspm_instances.h"

#include "sparkfun_mlx90640.h"
#include "image_handler.h"

static float mlx90640_image[SPARKFUN_MLX90640_NUM_OF_PIXELS];

/***************************************************************************//**
 * Accelerometer Setup
 ******************************************************************************/
sl_status_t mlx90640_setup(void)
{
  sl_status_t sc;
  uint16_t refrate = 0;

  sc = sparkfun_mlx90640_init(sl_i2cspm_inst0,
                              SPARKFUN_MLX90640_DEFAULT_I2C_ADDR);

  if (sc != SL_STATUS_OK) {
    return sc;
  }

  sc = sparkfun_mlx90640_set_refresh_rate(0x03);
  if (sc != SL_STATUS_OK) {
    return sc;
  }

  sc = sparkfun_mlx90640_get_refresh_rate(&refrate);

  return sc;
}

/***************************************************************************//**
 * Import accelerometer data to TensorFlow model
 ******************************************************************************/
sl_status_t mlx90640_read_image(float **image_data, int *image_size)
{
  sl_status_t sc;

  sc = sparkfun_mlx90640_get_image_array(mlx90640_image);
  if (sc != SL_STATUS_OK) {
    return sc;
  }

  *image_data = mlx90640_image;
  *image_size = SPARKFUN_MLX90640_NUM_OF_PIXELS;

  return SL_STATUS_OK;
}
