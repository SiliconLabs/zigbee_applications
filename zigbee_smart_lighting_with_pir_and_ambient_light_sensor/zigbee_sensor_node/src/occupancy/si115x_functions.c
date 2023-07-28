/***************************************************************************//**
 * @file
 * @brief Si115x driver functions
 * @version
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
#include "si115x_functions.h"

/*****************************************************************************/
/**************    Compile Switches   ****************************************/
/*****************************************************************************/

/***************************************************************************//**
 * @brief
 *   Waits until the Si115x is sleeping before proceeding
 ******************************************************************************/
static int16_t _waitUntilSleep(HANDLE si115x_handle)
{
  int8_t  retval = -1;
  uint8_t count = 0;
  // This loops until the Si115x is known to be in its sleep state
  // or if an i2c error occurs
  while (count < 5) {
    retval = Si11xxReadFromRegister(si115x_handle, REG_RESPONSE0);
    if ((retval & RSP0_CHIPSTAT_MASK) == RSP0_SLEEP) {
      break;
    }
    if (retval <  0) {
      return retval;
    }
    count++;
  }
  return 0;
}

/***************************************************************************//**
 * @brief
 *   Resets the Si115x/6x, clears any interrupts and initializes the HW_KEY
 *   register.
 * @param[in] si115x_handle
 *   The programmer's toolkit handle
 * @retval  0
 *   Success
 * @retval  <0
 *   Error
 ******************************************************************************/
int16_t Si115xReset(HANDLE si115x_handle)
{
  int32_t retval = 0;

  //
  // Do not access the Si115x earlier than 25 ms from power-up.
  // Uncomment the following lines if Si115xReset() is the first
  // instruction encountered, and if your system MCU boots up too
  // quickly.
  //
  delay_10ms();
  delay_10ms();
  delay_10ms();

  // Perform the Reset Command
  retval += Si11xxWriteToRegister(si115x_handle, REG_COMMAND, 1);

  // Delay for 10 ms. This delay is needed to allow the Si115x
  // to perform internal reset sequence.
  delay_10ms();

  return retval;
}

/***************************************************************************//**
 * @brief
 *   Helper function to send a command to the Si113x/4x
 ******************************************************************************/
static int16_t _sendCmd(HANDLE si115x_handle, uint8_t command)
{
//  int16_t  response;
  int8_t   retval;
//  uint8_t  count = 0;

//  // Get the response register contents
//  response = Si11xxReadFromRegister(si115x_handle, REG_RESPONSE0);
//  if(response < 0)
//    return response;
//
//  response = response & RSP0_COUNTER_MASK;
//
//  // Double-check the response register is consistent
//  while(count < 5)
//  {
//    if( (retval =_waitUntilSleep(si115x_handle)) != 0) return retval;
//
//    if(command==0) break; // Skip if the command is NOP
//
//    retval = Si11xxReadFromRegister( si115x_handle, REG_RESPONSE0 );
//
//    if( (retval&RSP0_COUNTER_MASK) == response ) break;
//
//    else if(retval<0) return retval;
//
//    else response = retval & RSP0_COUNTER_MASK;
//
//    count++;
//  }

  // Send the Command
  if ( (retval = Si11xxWriteToRegister(si115x_handle, REG_COMMAND, command)) != 0) {
    return retval;
  }

//  count = 0;
//  // Expect a change in the respownse register
//  while(count < 5)
//  {
//
//    if(command==0) break; // Skip if the command is NOP
//
//    retval= Si11xxReadFromRegister(si115x_handle, REG_RESPONSE0);
//    if( (retval & RSP0_COUNTER_MASK) != response) break;
//    else if(retval<0) return retval;
//    count++;
//  }
  return 0;
}

/***************************************************************************//**
 * @brief
 *   Sends a NOP command to the Si115x/6x
 * @param[in] si115x_handle
 *   The programmer's toolkit handle
 * @retval  0       Success
 * @retval  <0      Error
 ******************************************************************************/
int16_t Si115xNop(HANDLE si115x_handle)
{
  return _sendCmd(si115x_handle, 0x00);
}

/***************************************************************************//**
 * @brief
 *   Sends a FORCE command to the Si115x/6x
 * @param[in] si115x_handle
 *   The programmer's toolkit handle
 * @retval  0
 *   Success
 * @retval  <0
 *   Error
 ******************************************************************************/
int16_t Si115xForce(HANDLE si115x_handle)
{
  return _sendCmd(si115x_handle, 0x11);
}

/***************************************************************************//**
 * @brief
 *   Sends a PSALSAUTO command to the Si113x/4x
 * @param[in] si115x_handle
 *   The programmer's toolkit handle
 * @retval  0
 *   Success
 * @retval  <0
 *   Error
 ******************************************************************************/
int16_t Si115xStart(HANDLE si115x_handle)
{
  return _sendCmd(si115x_handle, 0x13);
}

/***************************************************************************//**
 * @brief
 *   Reads a Parameter from the Si115x/6x
 * @param[in] si115x_handle
 *   The programmer's toolkit handle
 * @param[in] address
 *   The address of the parameter.
 * @retval <0
 *   Error
 * @retval 0-255
 *   Parameter contents
 ******************************************************************************/
int16_t Si115xParamRead(HANDLE si115x_handle, uint8_t address)
{
  // returns Parameter[address]
  int16_t retval;
  uint8_t cmd = 0x40 + (address & 0x3F);

  retval = _sendCmd(si115x_handle, cmd);
  if ( retval != 0 ) {
    return retval;
  }

  retval = Si11xxReadFromRegister(si115x_handle, REG_RESPONSE1);
  return retval;
}

/***************************************************************************//**
 * @brief
 *   Writes a byte to an Si115x Parameter
 * @param[in] si115x_handle
 *   The programmer's toolkit handle
 * @param[in] address
 *   The parameter address
 * @param[in] value
 *   The byte value to be written to the parameter
 * @retval 0
 *   Success
 * @retval <0
 *   Error
 * @note This function ensures that the Si115x is idle and ready to
 * receive a command before writing the parameter. Furthermore,
 * command completion is checked. If setting parameter is not done
 * properly, no measurements will occur. This is the most common
 * error. It is highly recommended that host code make use of this
 * function.
 ******************************************************************************/
int16_t Si115xParamSet(HANDLE si115x_handle, uint8_t address, uint8_t value)
{
  int16_t retval;
  uint8_t buffer[2];
  int16_t response_stored;
  int16_t response;

  if ((retval = _waitUntilSleep(si115x_handle)) != 0) {
    return retval;
  }

  response_stored = RSP0_COUNTER_MASK & Si11xxReadFromRegister(si115x_handle, REG_RESPONSE0);

  buffer[0] = value;
  buffer[1] = 0x80 + (address & 0x3F);

  retval = Si11xxBlockWrite(si115x_handle, REG_HOSTIN0, 2, ( uint8_t* ) buffer);
  if (retval != 0) {
    return retval;
  }

  // Wait for command to finish
  response = Si11xxReadFromRegister(si115x_handle, REG_RESPONSE0);
  while ((response & RSP0_COUNTER_MASK) == response_stored ) {
    response = Si11xxReadFromRegister(si115x_handle, REG_RESPONSE0);
  }

  if (retval < 0) {
    return retval;
  } else {
    return 0;
  }
}

/***************************************************************************//**
 * @brief
 *   Pause measurement helper function
 ******************************************************************************/
static int16_t _Pause(HANDLE si115x_handle)
{
  return _sendCmd(si115x_handle, 0x12);
}

/***************************************************************************//**
 * @brief
 *   Pauses autonomous measurements
 * @param[in] si115x_handle
 *  The programmer's toolkit handle
 * @retval  0
 *   Success
 * @retval  <0
 *   Error
 ******************************************************************************/
int16_t Si115xPause(HANDLE si115x_handle)
{
  uint8_t countA, countB;
  int8_t  retval;

//    After a RESET, if the Si115x receives a command (including NOP) before the
//    Si115x has gone to sleep, the chip hangs. This first while loop avoids
//    this.  The reading of the REG_RESPONS0 does not disturb the internal MCU.

  retval = 0;  //initialize data so that we guarantee to enter the loop
  while ( (RSP0_CHIPSTAT_MASK & retval) != RSP0_SLEEP) {
    retval = Si11xxReadFromRegister(si115x_handle, REG_RESPONSE0);
  }

  countA = 0;
  while (countA < 5) {
    countB = 0;
    // Keep sending nops until the response is zero
    while (countB < 5) {
      retval = Si11xxReadFromRegister(si115x_handle, REG_RESPONSE0);
      if ( (retval & RSP0_COUNTER_MASK) == 0 ) {
        break;
      } else {
        // Send the NOP Command to clear any error...we cannot use Si115xNop()
        // because it first checks if REG_RESPONSE < 0 and if so it does not
        // perform the cmd. Since we have a saturation REG_RESPONSE will be <0
        Si11xxWriteToRegister(si115x_handle, REG_COMMAND, 0x00);
      }
      countB++;
    }

    // Pause the device
    _Pause(si115x_handle);

    countB = 0;
    // Wait for response
    while (countB < 5) {
      retval = Si11xxReadFromRegister(si115x_handle, REG_RESPONSE0);
      if ( (retval & RSP0_COUNTER_MASK) != 0 ) {
        break;
      }
      countB++;
    }

    // When the PsAlsPause() response is good, we expect it to be a '1'.
    retval = Si11xxReadFromRegister(si115x_handle, REG_RESPONSE0);
    if ( (retval & RSP0_COUNTER_MASK) == 1 ) {
      break;  // otherwise, start over.
    }
    countA++;
  }
  return 0;
}
