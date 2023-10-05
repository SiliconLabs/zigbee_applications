/*
 * cpc_button_led.c
 *
 *  Created on: Sept 20, 2023
 *      Author: anbiro, orkevlar
 */

/*
 * TODO: There's a new feature coming in 23Q4, which allows us to get rid of
 * the initial handshake and add a callback instead. These parts are marked
 * with [23Q4+] or [23Q4-]
 */

#include "cpc_button_led.h"
#include "cpc_commands.h"
#include "sl_cpc.h"
#include <stdint.h>
#include "sl_simple_led.h"
#include "sl_simple_led_instances.h"
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

static sl_cpc_endpoint_handle_t test_endpoint_handle;
typedef enum {
  CPC_ENDPOINT_CLOSED,
  CPC_ENDPOINT_OPEN,
  CPC_ENDPOINT_OPEN_WAIT_FOR_CONNECTION,
  CPC_ENDPOINT_CONNECTED,
  CPC_ENDPOINT_DISCONNECTED,
} cpc_endpoint_status_t;

static cpc_endpoint_status_t endpoint_status = CPC_ENDPOINT_CLOSED;
static uint8_t notify_buffer;
static volatile bool send_button_pressed = false;


static void process_command(uint8_t *commandData){
  switch ( commandData[0] ){
    case CPC_COMMAND_LED_OFF:
      sl_led_turn_off(&sl_led_led0);
      break;
    case CPC_COMMAND_LED_ON:
      sl_led_turn_on(&sl_led_led0);
      break;
    default:
      break;
  }
}

static void cpc_write_complete(sl_cpc_user_endpoint_id_t endpoint_id, void *buffer, void *arg, sl_status_t status){
  (void)endpoint_id;
  (void)buffer;
  (void)arg;
  (void)status;
  //error handling would go here
}

static void cpc_read_command(uint8_t endpoint_id, void *arg)

{
  (void)endpoint_id;
  (void)arg;
  sl_status_t status;
  uint8_t *read_array;
  uint16_t size;

  //TODO [23Q4-]
  endpoint_status = CPC_ENDPOINT_CONNECTED;
  //end of [23Q4-]

  status = sl_cpc_read(&test_endpoint_handle,
                       (void **)&read_array,
                       &size,
                       0, // Timeout : relevent only when using a kernel with blocking
                       0); // flags : relevent only when using a kernel to specify a non-blocking operation (polling).

  if (status != SL_STATUS_OK) {
    //do something
  } else {
    process_command(read_array);
    sl_cpc_free_rx_buffer(read_array);
  }

}

static void cpc_error_cb(uint8_t endpoint_id, void *arg)
{
  (void)endpoint_id;
  (void)arg;
  uint8_t state = sl_cpc_get_endpoint_state(&test_endpoint_handle);
  if (state == SL_CPC_STATE_ERROR_DESTINATION_UNREACHABLE) {
    sl_status_t status = sl_cpc_close_endpoint(&test_endpoint_handle);
    EFM_ASSERT(status == SL_STATUS_OK);
    endpoint_status = CPC_ENDPOINT_DISCONNECTED;
  }
}

#if defined(SL_CATALOG_CPC_SECURITY_PRESENT)
uint64_t sl_cpc_security_on_unbind_request(bool is_link_encrypted)
{
  (void)is_link_encrypted;
  return SL_CPC_SECURITY_OK_TO_UNBIND;
}
#endif

//TODO [23Q4+]
//void cpc_connect_command(uint8_t endpoint_id, void *arg)
//{
//  endpoint_status = CPC_ENDPOINT_CONNECTED;
//}
//end of [23Q4+]

static cpc_endpoint_status_t connect(){
  sl_status_t status;
  uint8_t window_size = 1;
  uint8_t flags = 0;

  status = sl_cpc_open_user_endpoint(&test_endpoint_handle,
                                         SL_CPC_ENDPOINT_USER_ID_0,
                                         flags,
                                         window_size);

  if (status != SL_STATUS_OK && status != SL_STATUS_ALREADY_EXISTS )
       return CPC_ENDPOINT_CLOSED;

  status = sl_cpc_set_endpoint_option(&test_endpoint_handle,
                                      SL_CPC_ENDPOINT_ON_IFRAME_WRITE_COMPLETED,
                                      (void *)cpc_write_complete);
  if (status != SL_STATUS_OK)
     return CPC_ENDPOINT_CLOSED;

  status = sl_cpc_set_endpoint_option(&test_endpoint_handle,
                                      SL_CPC_ENDPOINT_ON_IFRAME_RECEIVE,
                                      (void *)cpc_read_command);

  if (status != SL_STATUS_OK)
       return CPC_ENDPOINT_CLOSED;

  status = sl_cpc_set_endpoint_option(&test_endpoint_handle,
                                      SL_CPC_ENDPOINT_ON_ERROR,
                                      (void*)cpc_error_cb);

  if (status != SL_STATUS_OK)
       return CPC_ENDPOINT_CLOSED;

//  //TODO [23Q4+]
//  status = sl_cpc_set_endpoint_option(&test_endpoint_handle,
//                                      SL_CPC_ENDPOINT_ON_CONNECT,
//                                      (void*)cpc_connect_command);
//
//  if (status != SL_STATUS_OK)
//         return CPC_ENDPOINT_CLOSED;
//  //end of [23Q4+]


  return CPC_ENDPOINT_OPEN;

}

// Check & Update endpoint status
void cpc_test_endpoint_status(){
  if ( endpoint_status == CPC_ENDPOINT_DISCONNECTED && sl_cpc_get_endpoint_state(&test_endpoint_handle) == SL_CPC_STATE_FREED){
      endpoint_status = CPC_ENDPOINT_CLOSED;
  }
  // If closed, open endpoint
  if ( endpoint_status == CPC_ENDPOINT_CLOSED ){
      endpoint_status = connect();
  }
}

void cpc_button_led_init(){
  // Check endpoint state and connect
  cpc_test_endpoint_status();

  // Any other cpc init tasks go here
}

void cpc_button_led_process_action(){
  // Verify that endpoint is connected
  cpc_test_endpoint_status();

  if ( send_button_pressed ){
      if ( endpoint_status == CPC_ENDPOINT_CONNECTED ){
          // Send '1' if LED is ON
          if(sl_led_get_state(&sl_led_led0) == SL_LED_CURRENT_STATE_ON){
              notify_buffer = CPC_NOTIFY_LED_ON;
          }
          // Send '0' if LED is OFF
          if(sl_led_get_state(&sl_led_led0) == SL_LED_CURRENT_STATE_OFF){
              notify_buffer = CPC_NOTIFY_LED_OFF;
          }
        // Write to endpoint
        sl_cpc_write(&test_endpoint_handle, &notify_buffer, CPC_NOTIFY_LEN,
                     0, NULL); //no flag, no write complete arg
        send_button_pressed = false;
      }
  }
}

void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (&sl_button_btn0 == handle) {
      sl_led_toggle(&sl_led_led0);
      send_button_pressed = true;
    }
  }
}
