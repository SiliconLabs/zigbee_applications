// File: custom-token.h
//
// Description: Custom token definitions used by the application.
//
// Copyright 2019 by Silicon Labs Corporation.  All rights reserved.

/**
* Custom Zigbee Application Tokens
*/
// Define token names here
#define NVM3KEY_DOOR_LOCK_PIN         (NVM3KEY_DOMAIN_USER | 0x0001)
#define NVM3KEY_DOOR_LOCK_PIN_IN_USE  (NVM3KEY_DOMAIN_USER | 0x0002)

#if defined(DEFINETYPES)

#define DOOR_LOCK_PIN_STRING_LENGTH 4
#define DOOR_LOCK_DEFAULT_PIN "XXXX"

// Define a type to store the PIN, we will use a fixed length buffer
typedef struct {
  int8u code[DOOR_LOCK_PIN_STRING_LENGTH];
  int length;
} doorLockPin_t;

#endif //DEFINETYPES


#ifdef DEFINETOKENS
// Define the actual token storage information here
DEFINE_BASIC_TOKEN(DOOR_LOCK_PIN,
                   doorLockPin_t,
                   {DOOR_LOCK_DEFAULT_PIN, DOOR_LOCK_PIN_STRING_LENGTH})

DEFINE_BASIC_TOKEN(DOOR_LOCK_PIN_IN_USE,
                   boolean,
                   FALSE)

#endif
