#define NVM3KEY_DOOR_LOCK_PIN         (NVM3KEY_DOMAIN_USER | 0x0001)
#define NVM3KEY_DOOR_LOCK_PIN_IN_USE  (NVM3KEY_DOMAIN_USER | 0x0002)

#if defined(DEFINETYPES)
#define DOOR_LOCK_PIN_STRING_MIN_LENGTH 1 // Set min pin length to 1
#define DOOR_LOCK_PIN_STRING_MAX_LENGTH 9 // Set max length to 9 since first String character is length
#define DOOR_LOCK_DEFAULT_PIN { 8, '0', '0', '0', '0', '0', '0', '0', '0' } // Set first character to '8' to specify String length of 8

typedef int8u doorLockPin_t[DOOR_LOCK_PIN_STRING_MAX_LENGTH];
#endif //DEFINETYPES

#ifdef DEFINETOKENS
// Define the actual token storage information here
DEFINE_BASIC_TOKEN(DOOR_LOCK_PIN,
                   doorLockPin_t,
                   DOOR_LOCK_DEFAULT_PIN)

DEFINE_BASIC_TOKEN(DOOR_LOCK_PIN_IN_USE,
                   boolean,
                   FALSE)

#endif