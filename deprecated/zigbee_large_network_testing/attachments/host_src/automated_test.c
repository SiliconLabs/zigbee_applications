/***************************************************************************//**
 * @file automated_test.c
 * @brief Zigbee Large Network Testing example - Automated test
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
 * DEPRECATION NOTICE
 * This code has been deprecated. It has been provided for historical reference
 * only and should not be used. This code will not be maintained.
 * This code is subject to the quality disclaimer at the point in time prior
 * to deprecation and superseded by this deprecation notice.
******************************************************************************/

#include "automated_test.h"

/// Event control for the custom traffic event
EmberEventControl sl_custom_traffic_event;

/// Local device database holder
uint16_t devices[SL_NODES_INVENTORY_MAX_SIZE] = {0};
/// Number of devices that can be stored in local database
uint8_t sl_devices_num = 0;

/**************************************************************************//**
 * @brief Callback to handle joining and leaving devices
 * 
 * @param new_node_id Node ID
 * @param status Status of the node. Indicates whether node joined or left
*****************************************************************************/
void sl_large_network_testing_trust_center_join_callback(EmberNodeId new_node_id, 
                                                       EmberDeviceUpdate status)
{
  if (status == EMBER_DEVICE_LEFT ) {
    // device left, clear node from file
    if (sl_is_in_inventory(new_node_id)) {
      printf("%s: 0x%04X left the network, removing from inventory\n", 
             SL_LARGE_NETWORK_TESTING_DEBUG,
             new_node_id);

      sl_remove_node(new_node_id);
      sl_write_nodes_to_file();
    }
  } else {
    // device joined, add device to file
    if (!sl_is_in_inventory(new_node_id)) {
      printf("%s: 0x%04X joined the network, adding to inventory\n",
             SL_LARGE_NETWORK_TESTING_DEBUG, 
             new_node_id);

      sl_add_node(new_node_id);
      sl_write_nodes_to_file();
    }
  }
}

/**************************************************************************//**
 * @brief Read saved inventory upon startup 
 * 
*****************************************************************************/
void sl_large_network_testing_main_init_callback(void)
{
  sl_get_devices();
}

/**************************************************************************//**
 * @brief Remove a node from the inventory
 * 
 * @param node_id Node to remove
 * @return uint8_t 1 on success, 0 on failure
*****************************************************************************/
uint8_t sl_remove_node(EmberNodeId node_id)
{
  uint8_t i = 0;
  bool move = false;
  while (i < SL_NODES_INVENTORY_MAX_SIZE) {
    if (!move && (devices[i] == node_id)) {
      move = true;
    }
    if (move) {
      if (i == SL_NODES_INVENTORY_MAX_SIZE - 1) {
        devices[i] = 0;
        return 1;
      }
      devices[i] = devices[i+1];
    }
    i++;
  }
  if (!move) {
    return 0;
  }
  return 1;
}

/**************************************************************************//**
 * @brief Add a node to the inventory
 * 
 * @param new_node_id Node to add
 * @return uint8_t 1 on success, 0 on failure
*****************************************************************************/
uint8_t sl_add_node(EmberNodeId new_node_id)
{
  uint8_t i = 0;
  while (i < SL_NODES_INVENTORY_MAX_SIZE) {
    if (devices[i] == 0) {
      devices[i] = new_node_id;
      return 1;
    }
    i++;
  }
  return 0;
}

/**************************************************************************//**
 * @brief Check if a device is already in inventory
 * 
 * @param node_id Node to check
 * @return uint8_t 1 if device is already in inventory, 0 otherwise
*****************************************************************************/
uint8_t sl_is_in_inventory(EmberNodeId node_id)
{
  uint8_t i = 0;
  while (i < SL_NODES_INVENTORY_MAX_SIZE) {
    if (devices[i] == node_id) {
      return 1;
    }
    i++;
  }
  return 0;
}

/**************************************************************************//**
 * @brief Write all nodes to a file
 * @note Filename can be defined through SL_NODES_FILE constant
 * 
 * @return uint8_t 0 on success, -1 on failure
*****************************************************************************/
uint8_t sl_write_nodes_to_file(void)
{
  FILE *fp;
  char buff[8];
  uint16_t i;
  fp = fopen(SL_NODES_FILE, "w");
  if (fp == NULL) {
    printf("%s: Failed to open %s: %s\n", 
           SL_LARGE_NETWORK_TESTING_ERROR, 
           SL_NODES_FILE, 
           strerror(errno));
    return -1;
  }

  for(i = 0; i < SL_NODES_INVENTORY_MAX_SIZE; i++) {
    if (devices[i] == 0) {
      break;
    }
    sprintf(buff, "0x%04X\n", devices[i]);
    if (fputs(buff, fp) == EOF) {
      printf("%s: Failed to write to %s: %s\n", 
             SL_LARGE_NETWORK_TESTING_ERROR,
             SL_NODES_FILE, 
             strerror(errno));
      return -1;
    }
  }
  fclose(fp);

  return 0;
}

/**************************************************************************//**
 * @brief Read devices from node inventory and store in memory
 * @note Filename can be defined through SL_NODES_FILE constant
 * 
 * @return uint8_t Number of found devices, -1 on failure
*****************************************************************************/
uint8_t sl_get_devices(void)
{
  printf("%s: Reading device library from %s\n", 
         SL_LARGE_NETWORK_TESTING_DEBUG, 
         SL_NODES_FILE);

  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  uint8_t index = 0;
  uint16_t dest_node_id;

  fp = fopen(SL_NODES_FILE, "r");
  if (fp == NULL) {
    printf("%s: Failed to open %s: %s\n", 
           SL_LARGE_NETWORK_TESTING_ERROR, 
           SL_NODES_FILE, 
           strerror(errno));

    return -1;
  }

  while ((read = getline(&line, &len, fp)) != -1) {
    // we can use dest_node_id as a buffer here
    dest_node_id = (uint16_t)strtol(line, NULL, 0);
    printf("%s: Read 0x%4X from inventory, storing at index %d\n",
           SL_LARGE_NETWORK_TESTING_DEBUG, 
           dest_node_id, index);

    devices[index++] = dest_node_id;
  }

  fclose(fp);
  if (line) {
    free(line);
  }

  sl_devices_num = index;

  return sl_devices_num;
}

/**************************************************************************//**
 * @brief Handle a successful transmission
 * 
 * @param type Type of EmberOutgoingMessageType
 * @param indexOrDestination Destintation short address
 * @param apsFrame APS frame of the message
 * @param msgLen Length of the message
 * @param message Message buffer
 * @param status Status indicating success or failure
*****************************************************************************/
void sl_toggle_sent_callback(EmberOutgoingMessageType type, 
                             uint16_t indexOrDestination, 
                             EmberApsFrame *apsFrame, 
                             uint16_t msgLen, 
                             uint8_t *message, 
                             EmberStatus status)
{
  if (status == EMBER_SUCCESS) {
    printf("%s APS SENT: Successful transmission for 0x%X (status: 0x%X)\n", 
           SL_LARGE_NETWORK_TESTING_DEBUG, 
           indexOrDestination, 
           status);

  } else {
    printf("%s APS SENT: Failed transmission for 0x%X (status: 0x%X)\n", 
           SL_LARGE_NETWORK_TESTING_ERROR, 
           indexOrDestination, 
           status);
  }
}

/**************************************************************************//**
 * @brief Command handler for starting on/off transmissions
 * 
*****************************************************************************/
void sl_start_command(void)
{
  printf("%s: Activating traffic event\n", SL_LARGE_NETWORK_TESTING_DEBUG);
  emberEventControlSetActive(sl_custom_traffic_event);
}

/**************************************************************************//**
 * @brief Command handler for stopping on/off transmissions
 * 
*****************************************************************************/
void sl_stop_command(void)
{
  printf("%s: Deactivating traffic event\n", SL_LARGE_NETWORK_TESTING_DEBUG);
  emberEventControlSetInactive(sl_custom_traffic_event);
}

/**************************************************************************//**
 * @brief Custom implementation of a random function. To achieve a relatively 
 * good randomness, it host utilizes the `/dev/urandom` file, that achieves a 
 * sufficient entropy. For this to work, this function must be called on an 
 * Unix-like system.
 * This function can be substituted with any other random generators.
 * 
 * @return uint8_t Generated random number (1 byte from /dev/urandom)
*****************************************************************************/
uint8_t sl_custom_rand(void)
{
  FILE *urandom_fp = fopen("/dev/urandom", "r");
    
  if (!urandom_fp){
    printf("%s: Could't open urandom: %s\n",
           SL_LARGE_NETWORK_TESTING_ERROR, 
           strerror(errno));

    exit(1);
  }

  uint8_t rand_got = fgetc(urandom_fp);
  fclose(urandom_fp);
  return rand_got;
}

/**************************************************************************//**
 * @brief Handler for the On/Off traffic generator timing event
 * 
*****************************************************************************/
void sl_custom_traffic_event_handler(void)
{
  emberEventControlSetInactive(sl_custom_traffic_event);
  uint16_t dest_node_id;

  // get a random device to send message to
  uint8_t rand_num = sl_custom_rand();
  uint8_t rand_index = rand_num % sl_devices_num;
  dest_node_id = devices[rand_index];

  printf("%s: Sending On/Off toggle to 0x%X\n", 
         SL_LARGE_NETWORK_TESTING_DEBUG, 
         dest_node_id);

  emberAfFillCommandOnOffClusterToggle()
  emberAfGetCommandApsFrame()->profileId = emberAfPrimaryProfileId();
  emberAfGetCommandApsFrame()->sourceEndpoint = emberAfPrimaryEndpoint();
  emberAfGetCommandApsFrame()->destinationEndpoint = 0x1;
  
  // enable aps terty and route discovery
  emberAfGetCommandApsFrame()->options |= \
      (EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY | EMBER_APS_OPTION_RETRY);

  EmberStatus status;
  status = emberAfSendCommandUnicastWithCallback(EMBER_OUTGOING_DIRECT, 
                                                 dest_node_id, 
                                                 sl_toggle_sent_callback);
  if (status != EMBER_SUCCESS) {
    printf("%s: Couldn't send APS command to 0x%X, status: 0x%X\n", 
           SL_LARGE_NETWORK_TESTING_ERROR, 
           dest_node_id, status);
  }

  emberEventControlSetDelayMS(sl_custom_traffic_event, SL_TRAFFIC_DT_MS);
}

/**************************************************************************//**
 * @brief Handler for the inventory printing CLI command
 * 
*****************************************************************************/
void sl_inventory_command(void)
{
  uint16_t i;
  printf("Devices in inventory:\n");
  for (i = 0; i < SL_NODES_INVENTORY_MAX_SIZE; i++) {
    if (devices[i] == 0) {
      return;
    }
    printf("0x%04X\n", devices[i]);
  }
}