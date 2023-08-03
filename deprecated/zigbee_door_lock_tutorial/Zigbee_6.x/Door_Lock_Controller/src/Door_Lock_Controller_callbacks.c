// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

/* ...
*
* DEPRECATION NOTICE
* This code has been deprecated. It has been provided for historical reference
* only and should not be used. This code will not be maintained.
* This code is subject to the quality disclaimer at the point in time prior
* to deprecation and superseded by this deprecation notice.
*
... */

#include "app/framework/include/af.h"
#include "hal/hal.h"
#include EMBER_AF_API_NETWORK_STEERING




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
boolean emberAfStackStatusCallback(EmberStatus status)
{
  return false;
}

/** @brief Door Lock Cluster Lock Door Response
 *
 * 
 *
 * @param status   Ver.: always
 */
boolean emberAfDoorLockClusterLockDoorResponseCallback(int8u status)
{
  return TRUE;
}

/** @brief Door Lock Cluster Unlock Door Response
 *
 * 
 *
 * @param status   Ver.: always
 */
boolean emberAfDoorLockClusterUnlockDoorResponseCallback(int8u status)
{
  return TRUE;
}

/** @brief Door Lock Cluster Get Pin Response
 *
 * 
 *
 * @param userId   Ver.: always
 * @param userStatus   Ver.: always
 * @param userType   Ver.: always
 * @param pin   Ver.: always
 */
boolean emberAfDoorLockClusterGetPinResponseCallback(int16u userId,
                                                     int8u userStatus,
                                                     int8u userType,
                                                     int8u* pin)
{
  return TRUE;
}

/** @brief Door Lock Cluster Clear Pin Response
 *
 * 
 *
 * @param status   Ver.: always
 */
boolean emberAfDoorLockClusterClearPinResponseCallback(int8u status)
{
  return TRUE;
}

/** @brief Door Lock Cluster Set Pin Response
 *
 * 
 *
 * @param status   Ver.: always
 */
boolean emberAfDoorLockClusterSetPinResponseCallback(int8u status)
{
  return TRUE;
}


