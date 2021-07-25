#ifndef EEPROM_ADDRESSES_H
#define EEPROM_ADDRESSES_H

#include <EEPROM.h>

#define TIMEZONE_EEPROM_ADDR 0 			// byte
#define OPEN_DELAY_EEPROM_ADDR 1		// byte
#define CLOSE_DELAY_EEPROM_ADDR 2		// byte
#define STEPS_TO_CLOSE_EEPROM_ADDR 3	// (unsigned) int
#define DOOR_STATE_EEPROM_ADDR 5		// bool (one byte)


#endif // EEPROM_ADDRESSES_H