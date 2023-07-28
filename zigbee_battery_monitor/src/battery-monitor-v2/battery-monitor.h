 /***************************************************************************//**
* @file battery-monitor.h
* @brief API for monitoring the voltage level on a battery.
* @version v0.01
*******************************************************************************
* # License
* <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef __BATTERY_MONITOR_H__
#define __BATTERY_MONITOR_H__

/** @brief Get the temperature in millidegrees Celsius
 *
 * This function will perform whatever hardware interaction is necessary to
 * read a temperature value from the temperature sensor and return it in units
 * of millidegrees Celsius.
 *
 * @return The temperature in millidegrees Celsius
 */
uint16_t halGetBatteryVoltageMilliV(void);

/** @brief Initializes the battery monitor hardware.  The application
 * framework will generally initialize this plugin automatically.  Customers who
 * do not use the framework must ensure the plugin is initialized by calling
 * this function.
 */
void halBatteryMonitorInitialize(void);

/** @brief Callback generated when new measured battery voltage data is
 * available.
 *
 * @note This callback will be called when the battery monitor plugin has
 * collected a new measurement of the current battery voltage level.
 *
 * @param voltageMilliV  The battery level measured, in milli volts.
 */
void emberAfPluginBatteryMonitorDataReadyCallback(uint16_t batteryVoltageMilliV);

#endif // __BATTERY_MONITOR_H__
