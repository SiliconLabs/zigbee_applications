/***************************************************************************//**
 * @file buttonMulticast.c
 * @brief File to create a group and send Multicast Message.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

/***************************************************************************//**
 * DESCRIPTION OF THE FILE
 *  -
 *  -
 * HOW TO USE :
 *
 ******************************************************************************/

#include "buttonMulticast.h"

/***************************************************************************//**
 * Variable declarations / Includes.
 ******************************************************************************/

static uint8_t currentBtnState = 0;

/***************************************************************************//**
 * Functions & events.
 ******************************************************************************/
/** @brief
 *
 * Application Callback when push-button pressed
 */
void sl_button_on_change(const sl_button_t *handle)
{
  EmberStatus status;

  if(SL_SIMPLE_BUTTON_INSTANCE(BUTTON0) == handle)
  {
    if(sl_simple_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED)
      {
        emberAfCorePrintln("Button0 is pressed");
        if(currentBtnState == 0) // ON Command
        {
            emberAfCorePrintln("Command is zcl on-off ON");
            emberAfFillCommandOnOffClusterOn();
            currentBtnState = 1;
        }
        else // OFF command
        {
            emberAfCorePrintln("Command is zcl on-off OFF");
            emberAfFillCommandOnOffClusterOff();
            currentBtnState = 0;
        }
        emberAfSetCommandEndpoints(1,1);
        status = emberAfSendCommandMulticast(GROUP_ID);

        if(status == EMBER_SUCCESS){
          emberAfCorePrintln("Command is successfully sent");
        }else{
          emberAfCorePrintln("Failed to send");
          emberAfCorePrintln("Status code: 0x%x",status);
        }
      }
  }
}
