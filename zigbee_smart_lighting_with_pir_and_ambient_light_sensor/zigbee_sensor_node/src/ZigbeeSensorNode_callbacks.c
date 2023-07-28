/***************************************************************************//**
 * @file ZigbeeSensorNode_callbacks.c
 * @brief Callback implementation for Zigbee smart lighting sensor node.
 * @version 1.0.1
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

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.
//#include "bspconfig.h"

#include "em_cmu.h"
#include "em_gpio.h"
#include "gpiointerrupt.h"

#include "pir.h"
#include "ambient_light.h"
#include "occupancy_config.h"

#include "app/framework/include/af.h"

#include EMBER_AF_API_NETWORK_STEERING
#include EMBER_AF_API_FIND_AND_BIND_INITIATOR


#define SENSOR_ENDPOINT                    (1)
#define TIME_DELAY_TURN_OFF_LIGHT_MS       5000

/* 200 is a threshold number between light and dark based on some reseaches
 * see Readme.md for more details */
#define LIGHT_THRESHOLD                    200

/*******************************************************************************
 ******************************  LOCAL FUNCTION  *******************************
 ******************************************************************************/
static void gpio_init(void);
static void motion_detection_callback(bool motion);
static void get_lux_completed_callback(uint16_t enter_lux_value);

static uint16_t convert_endian(uint16_t value_16bit);
static void button_irq_handler(uint8_t pin);

/*******************************************************************************
 ******************************  LOCAL VARIABLE  *******************************
 ******************************************************************************/
static bool appStart = false;
static uint8_t occupancy = 0;
static uint16_t lux_value = 0;
static EmberAfOccupancySensorType occupancy_sensor_type = EMBER_ZCL_OCCUPANCY_SENSOR_TYPE_PIR;

EmberEventControl appStartEventControl;
EmberEventControl motionDetectedEventControl;
EmberEventControl motionOffEventControl;
EmberEventControl initSensorEventControl;


/***************************************************************************//**
 * @brief convert from big endian to little endian for 16 bits variable
 * @param value: 16 bits variable need to convert
 ******************************************************************************/
uint16_t convert_endian(uint16_t value_16bit)
{
  uint16_t return_val;

  return_val = value_16bit & 0xff;
  return_val = return_val >> 8;
  return_val += (value_16bit << 8) & 0xff;

  return return_val;
}

/***************************************************************************//**
 * @brief Setup GPIO interrupt for pushbuttons.
 ******************************************************************************/
static void gpio_init(void)
{
  CMU_ClockEnable(cmuClock_GPIO, true);
  GPIO_PinModeSet(LDO_SHDN_B_PORT, LDO_SHDN_B_PIN, gpioModePushPull, 0);  /* Disable the LDO. */
  GPIO_PinModeSet(ADC_P_PORT, ADC_P_PIN, gpioModeDisabled, 0); // ADC_P
  GPIO_PinModeSet(ADC_N_PORT, ADC_N_PIN, gpioModeDisabled, 0); // ADC_N
  GPIO_PinModeSet(MOTION_B_PORT, MOTION_B_PIN, gpioModePushPull, 0);      /* Disable the EXP side LED. */

  GPIO_PinModeSet(SENSOR_SCL_PORT, SENSOR_SCL_PIN, gpioModeWiredAndPullUp, 1);
  GPIO_PinModeSet(SENSOR_SDA_PORT, SENSOR_SDA_PIN, gpioModeWiredAndPullUp, 1);
  GPIO_PinModeSet(SENSOR_INT_PORT, SENSOR_INT_PIN, gpioModeWiredAndPullUp, 1);


  NVIC_DisableIRQ(GPIO_ODD_IRQn);
  NVIC_DisableIRQ(GPIO_EVEN_IRQn);

  /* Configure PB0 as input and enable interrupt. */
  GPIO_PinModeSet(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, gpioModeInputPull, 1);

  /* set callback */
  GPIOINT_CallbackRegister(BSP_GPIO_PB0_PIN, button_irq_handler);

  /* interrupt config */
  GPIO_IntConfig(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, false, true, true);

  GPIOINT_Init();
}

/** @brief handler for appStartEventControl
 *  start steering network
 */
void appStartEventHandler()
{
  emberEventControlSetInactive(appStartEventControl);
  emberAfCorePrintln("Sensor node start \n");

  appStart = true;

  EmberStatus status = emberAfPluginNetworkSteeringStart();
  emberAfCorePrintln("Steering network: 0x%X \n", status);
}

/** @brief handler for initSensorEventControl
 * init pir sensor and ambient light sensor
 *
 */
void initSensorEventHandler()
{
  EmberAfStatus status;

  emberEventControlSetInactive(initSensorEventControl);
  emberAfCorePrintln("Init ambient light sensor \n");
  als_init(&get_lux_completed_callback);

  emberAfCorePrintln("Init pir sensor \n");
  pir_init(&motion_detection_callback);

  /* set occupancy sensor type to PIR sensor type */
  status = emberAfWriteServerAttribute(SENSOR_ENDPOINT,
                                       ZCL_OCCUPANCY_SENSING_CLUSTER_ID,
                                       ZCL_OCCUPANCY_SENSOR_TYPE_ATTRIBUTE_ID,
                                       &occupancy_sensor_type,
                                       ZCL_BITMAP8_ATTRIBUTE_TYPE);
}

/** @brief handler for motionDetectedEventControl
 *  run when sensor detected a motion
 *  update attribute and control the light
 */
void motionDetectedEventHandler(void)
{
  EmberAfStatus status;
  uint16_t illum_measured_value = 0;

  /* update motion status */
  occupancy = 1;

  emberEventControlSetInactive(motionDetectedEventControl);
  emberAfCorePrintln("Motion detected \n");

  /* update occupancy attribute */
  status = emberAfWriteServerAttribute(SENSOR_ENDPOINT,
                                       ZCL_OCCUPANCY_SENSING_CLUSTER_ID,
                                       ZCL_OCCUPANCY_ATTRIBUTE_ID,
                                       &occupancy,
                                       ZCL_BITMAP8_ATTRIBUTE_TYPE);
  emberAfCorePrintln( "Write attribute: %d \n", status);

  /* print lux value */
  emberAfCorePrintln("Lux: %d hex: %X \n", lux_value, lux_value);

  /* if there is motion and lux is less than LIGHT_THRESHOLD value, turn on the light */
  if(lux_value <= LIGHT_THRESHOLD){
      //turn on the light
      emberAfGetCommandApsFrame()->sourceEndpoint = SENSOR_ENDPOINT;
      status = emberAfFillCommandOnOffClusterOn();
      emberAfCorePrintln( "Turn on the light : %d \n", status);
      status = emberAfSendCommandUnicastToBindings();
      emberAfCorePrintln( "Send to bindings: %d \n", status);
  }
  else{
      //turn off the light
      emberAfGetCommandApsFrame()->sourceEndpoint = SENSOR_ENDPOINT;
      status = emberAfFillCommandOnOffClusterOff();
      emberAfCorePrintln( "Turn off the light : %d \n", status);
      status = emberAfSendCommandUnicastToBindings();
      emberAfCorePrintln( "Send to bindings: %d \n", status);
  }

  /* update illuminance measured value attribute */
  /* the  illuminance measured value attribute storage lux value in little-endian,
   * so that why the lux value need to convert endian before write to attribute */
  illum_measured_value = convert_endian(lux_value);
  status = emberAfWriteServerAttribute(SENSOR_ENDPOINT,
                                       ZCL_ILLUM_MEASUREMENT_CLUSTER_ID,
                                       ZCL_ILLUM_MEASURED_VALUE_ATTRIBUTE_ID,
                                       &illum_measured_value,
                                       ZCL_INT16U_ATTRIBUTE_TYPE);
  emberAfCorePrintln( "Write attribute: %d \n", status);

  /* turn off the light and update the motion attribute to off after 5s there is no motion */
  emberEventControlSetDelayMS(motionOffEventControl, TIME_DELAY_TURN_OFF_LIGHT_MS);
}

/** @brief handler for motionOffEventControl
 *  turn off the light and update motion status
 *
 */
void motionOffEventHandler(void)
{
  EmberAfStatus status;

  /* update motion status */
  occupancy = 0;

  emberEventControlSetInactive(motionOffEventControl);
  emberAfCorePrintln("Motion off \n");

  /* update occupancy attribute */
  status = emberAfWriteServerAttribute(SENSOR_ENDPOINT,
                                       ZCL_OCCUPANCY_SENSING_CLUSTER_ID,
                                       ZCL_OCCUPANCY_ATTRIBUTE_ID,
                                       &occupancy,
                                       ZCL_BITMAP8_ATTRIBUTE_TYPE);
  emberAfCorePrintln( "Write attribute: %d \n", status);

  /* turn off the light */
  emberAfGetCommandApsFrame()->sourceEndpoint = SENSOR_ENDPOINT;
  status = emberAfFillCommandOnOffClusterOff();
  emberAfCorePrintln( "Turn off the light : %d \n", status);
  status = emberAfSendCommandUnicastToBindings();
  emberAfCorePrintln( "Send to bindings: %d \n", status);
}


/** @brief motion_detection_callback
 *  Call by PIR driver after run detect motion algorithm
 *
 */
void motion_detection_callback(bool motion)
{
  if(motion){
      emberEventControlSetActive(motionDetectedEventControl);
  }
}

/** @brief get_lux_completed_callback
 *  Call by ALS driver after get lux value
 *
 */
void get_lux_completed_callback(uint16_t enter_lux_value)
{
  lux_value = enter_lux_value;
}

/** @brief Main Init
 *
 * This function is called from the application's main function. It gives the
 * application a chance to do any initialization required at system startup. Any
 * code that you would normally put into the top of the application's main()
 * routine should be put into this function. This is called before the clusters,
 * plugins, and the network are initialized so some functionality is not yet
 * available.
        Note: No callback in the Application Framework is
 * associated with resource cleanup. If you are implementing your application on
 * a Unix host where resource cleanup is a consideration, we expect that you
 * will use the standard Posix system calls, including the use of atexit() and
 * handlers for signals such as SIGTERM, SIGINT, SIGCHLD, SIGPIPE and so on. If
 * you use the signal() function to register your signal handler, please mind
 * the returned value which may be an Application Framework function. If the
 * return value is non-null, please make sure that you call the returned
 * function from your handler to avoid negating the resource cleanup of the
 * Application Framework itself.
 *
 */
void emberAfMainInitCallback(void)
{
  emberAfCorePrintln("GPIO init \n");
  gpio_init();
  emberAfCorePrintln("Press btn0 to start \n");
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
bool emberAfStackStatusCallback(EmberStatus status)
{
  emberAfCorePrintln("Stack status: 0x%X \n", status);
  // This value is ignored by the framework.
  return false;
}

/** @brief Complete
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to EMBER_SUCCESS to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt. Ver.: always
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID. Ver.: always
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network. Ver.: always
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete. Ver.: always
 */
void emberAfPluginNetworkSteeringCompleteCallback(EmberStatus status,
                                                  uint8_t totalBeacons,
                                                  uint8_t joinAttempts,
                                                  uint8_t finalState)
{
  emberAfCorePrintln("%p network %p: 0x%X", "Join", "complete", status);
  if(status == EMBER_SUCCESS){
      if(appStart){
          emberAfCorePrintln("Find and bind initiator start: 0x%X",
                             emberAfPluginFindAndBindInitiatorStart(SENSOR_ENDPOINT));
      }
  }
}

/** @brief Complete
 *
 * This callback is fired by the initiator when the Find and Bind process is
 * complete.
 */
void emberAfPluginFindAndBindInitiatorCompleteCallback(EmberStatus status)
{
  emberAfCorePrintln("Find and bind initiator %p: 0x%X", "complete", status);
  if(status == EMBER_SUCCESS){
      emberEventControlSetActive(initSensorEventControl);
  }
}

/***************************************************************************//**
 * @brief button_IRQHandler
 *        push PB0 to Start application.
 ******************************************************************************/
void button_irq_handler(uint8_t pin)
{
  /* if btn0 press */
  if (pin == BSP_GPIO_PB0_PIN) {
      if(!appStart){
          appStart = true;
          emberEventControlSetActive(appStartEventControl);
      }
  }
}

