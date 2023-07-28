/***************************************************************************//**
 * @file gui.c
 * @brief Zigbee Large Network Testing example - GUI
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

#include "gui.h"

/// GLIB context to use
extern GLIB_Context_t glibContext;
/// Counters from the stack
extern uint16_t emberCounters[EMBER_COUNTER_TYPE_COUNT];

/// App events
EmberEventControl sl_large_network_testing_gui_button_check;
EmberEventControl sl_large_network_testing_gui_button_pressed;
EmberEventControl sl_large_network_testing_gui_counters_print;

/// Variable to strore if a button check is already in progress
static uint8_t sl_button_check_in_progress = 0;
/// Variable to store last pressed button
static sl_pressed_button_t sl_last_pressed_button;
/// Status message
static char sl_status_msg[32];


/// Static functions declarations
static void sl_button_pressed(sl_pressed_button_t pressed_button);

/**************************************************************************//**
 * @brief Initialization of the GUI
 * 
 *****************************************************************************/
void sl_large_network_testing_gui_init(void)
{
  GRAPHICS_Init();
  GLIB_clear(&glibContext);

  sl_graphics_draw_button_icon(SL_SIDE_LEFT, SL_ICON_CONNECT, SL_BOTTOM_ROW);
  sl_graphics_draw_button_icon(SL_SIDE_RIGHT, SL_ICON_LEAVE, SL_BOTTOM_ROW);
  sl_graphics_draw_button_icon(SL_SIDE_LEFT, SL_ICON_OPEN, SL_TOP_ROW);
  sl_graphics_draw_button_icon(SL_SIDE_RIGHT, SL_ICON_CLOSE, SL_TOP_ROW);
  sl_graphics_draw_counters_header();
  sl_graphics_draw_title_text("test router");

  sl_graphics_draw_short_id(emberAfGetNodeId());
  sl_graphics_draw_parent_id(emberGetParentId());

  sl_graphics_draw_status_message("Initialized");

  emberEventControlSetDelayMS(sl_large_network_testing_gui_counters_print,
                              SL_COUNTERS_QUERY_PERIOD_MS);
}

/**************************************************************************//**
 * @brief Callback to be called upon network steering completion
 * 
 * @param status Status indicating whether joining was successful or not
 *****************************************************************************/
void sl_large_network_testing_gui_network_steering_complete_callback(EmberStatus status)
{
	if (status == EMBER_SUCCESS) {
    return;
  }
	if (status == EMBER_NO_BEACONS) {
		sprintf(sl_status_msg, "No NWK found");
	} else if (status != EMBER_SUCCESS) {
    sprintf(sl_status_msg, "Failed: 0x%X", status);
  }

	emberAfCorePrintln(sl_status_msg);
	sl_graphics_draw_status_message(sl_status_msg);
}

/**************************************************************************//**
 * @brief Stack status callback to present updates on the GUI
 * 
 * @param status EmberStatus indicating updated network status
 *****************************************************************************/
void sl_large_network_testing_gui_stack_status_callback(EmberStatus status)
{
	switch (status) {
    case EMBER_NETWORK_DOWN:
      sl_graphics_draw_short_id(emberGetNodeId());
      sl_graphics_draw_parent_id(emberGetParentId());
      sprintf(sl_status_msg, "Network down");
      break;
    case EMBER_NETWORK_UP:
      sl_graphics_draw_short_id(emberGetNodeId());
      sl_graphics_draw_parent_id(emberGetParentId());
      sprintf(sl_status_msg, "Joined 0x%X", emberAfGetPanId());
      break;
    case EMBER_NETWORK_OPENED:
      sprintf(sl_status_msg, "Network open");
      break;
    case EMBER_NETWORK_CLOSED:
      sprintf(sl_status_msg, "Network closed");
      break;
    default:
      sprintf(sl_status_msg, "Status: 0x%X", status);
      break;
	}

	emberAfCorePrintln(sl_status_msg);
	sl_graphics_draw_status_message(sl_status_msg);

  return;
}

/**************************************************************************//**
 * @brief Handler of the counter printing event
 * 
 *****************************************************************************/
void sl_large_network_testing_gui_counters_print_handler(void)
{
	emberEventControlSetInactive(sl_large_network_testing_gui_counters_print);

	sl_graphics_draw_route_errors(emberCounters[EMBER_COUNTER_ROUTE_DISCOVERY_INITIATED]);
	sl_graphics_draw_mac_errors(emberCounters[EMBER_COUNTER_MAC_RX_UNICAST],
                         emberCounters[EMBER_COUNTER_MAC_TX_UNICAST_SUCCESS],
                         emberCounters[EMBER_COUNTER_MAC_TX_UNICAST_RETRY],
                         emberCounters[EMBER_COUNTER_MAC_TX_UNICAST_FAILED],
                         emberCounters[EMBER_COUNTER_MAC_RX_BROADCAST],
                         emberCounters[EMBER_COUNTER_MAC_TX_BROADCAST]);
	
	sl_graphics_draw_aps_errors(emberCounters[EMBER_COUNTER_APS_DATA_RX_UNICAST],
                       emberCounters[EMBER_COUNTER_APS_DATA_TX_UNICAST_SUCCESS],
                       emberCounters[EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY],
                       emberCounters[EMBER_COUNTER_APS_DATA_TX_UNICAST_FAILED],
                       emberCounters[EMBER_COUNTER_APS_DATA_RX_BROADCAST],
                       emberCounters[EMBER_COUNTER_APS_DATA_TX_BROADCAST]);

	emberEventControlSetDelayMS(sl_large_network_testing_gui_counters_print,
                              SL_COUNTERS_QUERY_PERIOD_MS);
}

/**************************************************************************//**
 * @brief Handler for the press of the connect button
 * @note This should be called out of interrupt context
 * 
 *****************************************************************************/
void sl_large_network_testing_gui_connect_button_handler(void)
{
	EmberStatus status;
	if (emberAfNetworkState() != EMBER_JOINED_NETWORK) {
		status = emberAfPluginNetworkSteeringStart();
		if (status == EMBER_SUCCESS) {
			sprintf(sl_status_msg, "Connecting...");
		} else {
			sprintf(sl_status_msg, "Failed: 0x%X", status);
		}
	} else if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
		sprintf(sl_status_msg, "Already joined");
	} else {
		sprintf(sl_status_msg, "Fatal error");
	}
	
	emberAfCorePrintln(sl_status_msg);
	sl_graphics_draw_status_message(sl_status_msg);
}

/**************************************************************************//**
 * @brief Handler for the press of the open button
 * @note This should be called out of interrupt context
 * 
 *****************************************************************************/
void sl_large_network_testing_gui_open_button_handler(void)
{
	if (emberAfGetNodeId() == 0xFFFE) {
		sprintf(sl_status_msg, "Not joined");
	} else {
		EmberStatus status = emberAfPermitJoin(120, false);
		if (status == EMBER_SUCCESS) {
			sprintf(sl_status_msg, "Network open");
		} else {
			sprintf(sl_status_msg, "Failed: 0x%X", status);
		}
	}

	emberAfCorePrintln(sl_status_msg);
	sl_graphics_draw_status_message(sl_status_msg);
}

/**************************************************************************//**
 * @brief Handler for press of the close button
 * @note This should be called out of interrupt context
 * 
 *****************************************************************************/
void sl_large_network_testing_gui_close_button_handler(void)
{
	if (emberAfGetNodeId() == 0xFFFE) {
		sprintf(sl_status_msg, "Not joined");
	} else {
		EmberStatus status = emberAfPermitJoin(0, false);
		if (status == EMBER_SUCCESS) {
			sprintf(sl_status_msg, "Network closed");
		} else {
			sprintf(sl_status_msg, "Failed: 0x%X", status);
		}
	}

	emberAfCorePrintln(sl_status_msg);
	sl_graphics_draw_status_message(sl_status_msg);
}

/**************************************************************************//**
 * @brief Handler for the press of the leave button
 * 
 *****************************************************************************/
void sl_large_network_testing_gui_leave_button_handler(void)
{
	EmberStatus status;
	status = emberLeaveNetwork();
	if (status == EMBER_SUCCESS) {
		sprintf(sl_status_msg, "Left network");
	} else {
		sprintf(sl_status_msg, "Failed: 0x%X", status);
	}

	emberAfCorePrintln(sl_status_msg);
	sl_graphics_draw_status_message(sl_status_msg);
}

/**************************************************************************//**
 * @brief General handler for a button press that will be invoked when a button 
 * is released. This will call sl_button_pressed that handles the logic of 
 * pressed buttons. At the time this is called, the button state machine should 
 * be set. An event logic is necessary so that button logic is not called from 
 * ISR context.
 * 
 *****************************************************************************/
void sl_large_network_testing_gui_button_pressed_handler(void)
{
	emberEventControlSetInactive(sl_large_network_testing_gui_button_pressed);
	sl_button_pressed(sl_last_pressed_button);
}

/**************************************************************************//**
 * @brief Handler for a button press. Called from the expired event, this 
 * handles button press logic based on the state machine which is set by the 
 * time this gets called. This function calls the appropriate handlers that will
 * initiate different stack operations.
 * 
 * @param pressed_button 
 *****************************************************************************/
static void sl_button_pressed(sl_pressed_button_t pressed_button)
{
	if (pressed_button.button == BUTTON1 
     && pressed_button.type == SL_LONG_PRESS) {
    // connect
		sl_large_network_testing_gui_connect_button_handler();
	} else if (pressed_button.button == BUTTON1 
          && pressed_button.type == SL_SHORT_PRESS) {
    // open network
		sl_large_network_testing_gui_open_button_handler();
	} else if (pressed_button.button == BUTTON0 
          && pressed_button.type == SL_LONG_PRESS) {
    // leave
		sl_large_network_testing_gui_leave_button_handler();
	}	else if (pressed_button.button == BUTTON0 
          && pressed_button.type == SL_SHORT_PRESS) {
    // close
		sl_large_network_testing_gui_close_button_handler();
	}
}

/**************************************************************************//**
 * @brief ISR handler for a button press. This will start/stop the event 
 * responsible for checking press duration (based on the button press state 
 * machine).
 * 
 * @param button 
 * @param state 
 *****************************************************************************/
void sl_large_network_testing_gui_button_isr_callback(uint8_t button, 
                                                      uint8_t state)
{
  if (state == BUTTON_PRESSED) {
		emberEventControlSetInactive(sl_large_network_testing_gui_button_check);
		sl_button_check_in_progress = 1;
		emberEventControlSetDelayMS(sl_large_network_testing_gui_button_check,
                                SL_BUTTON_SL_LONG_PRESS_TRESHOLD_MS);
	}
	if (state == BUTTON_RELEASED) {
		emberEventControlSetInactive(sl_large_network_testing_gui_button_check);

		sl_last_pressed_button.button = button;
		sl_last_pressed_button.type = ((sl_button_check_in_progress == 1) ? 
                                    SL_SHORT_PRESS : SL_LONG_PRESS);

		emberEventControlSetActive(sl_large_network_testing_gui_button_pressed);
	}
}

/**************************************************************************//**
 * @brief Handler for an expired button press length checker event. Based on 
 * when this gets called, the button state machine will be adjusted if the press
 * is a long or short press.
 * 
 *****************************************************************************/
void sl_large_network_testing_gui_button_check_handler(void)
{
  emberEventControlSetInactive(sl_large_network_testing_gui_button_check);
  sl_button_check_in_progress = 0;
}
