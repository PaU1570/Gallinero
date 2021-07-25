#ifndef STRINGS_H
#define STRINGS_H

#include <arduino.h>

class LiquidCrystal_I2C;

// LCD Messages
const char str00[] PROGMEM = "   Welcome to   "; // This is the max size for on line (16 characters)
const char str01[] PROGMEM = "Opening door... ";
const char str02[] PROGMEM = "Closing door... ";
const char str03[] PROGMEM = "Calibrating";
const char str04[] PROGMEM = "Calibration";
const char str05[] PROGMEM = "Steps to open:";
const char str06[] PROGMEM = "Door: Open";
const char str07[] PROGMEM = "Door: Closed";
const char str08[] PROGMEM = "Chickens:     ";
const char str09[] PROGMEM = "Inside:       ";
const char str10[] PROGMEM = "Hold R to";
const char str11[] PROGMEM = "Hold L to";
const char str12[] PROGMEM = "open door.";
const char str13[] PROGMEM = "close door.";
const char str14[] PROGMEM = "Light: ";
const char str15[] PROGMEM = "Min light:   ";
const char str16[] PROGMEM = " Hold R to save ";
const char str17[] PROGMEM = "     < ";
const char str18[] PROGMEM = " >    ";
const char str19[] PROGMEM = "complete.       ";
const char str20[] PROGMEM = "                ";
const char str21[] PROGMEM = "                ";
const char str22[] PROGMEM = "                ";
const char str23[] PROGMEM = "                ";
const char str24[] PROGMEM = "                ";
#define WELCOME_MSG 0
#define DOOR_OPENING_MSG 1
#define DOOR_CLOSING_MSG 2
#define DOOR_CALIBRATING_MSG 3
#define DOOR_CALIBRATED_MSG 4
#define DOOR_STEPS_MSG 5
#define DOOR_OPEN_MSG 6
#define DOOR_CLOSED_MSG 7
#define CHICKENS_MSG 8
#define CHICKENS_INSIDE_MSG 9
#define HOLD_R_MSG 10
#define HOLD_L_MSG 11
#define CLOSE_DOOR_MSG 12
#define OPEN_DOOR_MSG 13
#define LIGHT_LVL_MSG 14
#define MIN_LIGHT_MSG 15
#define HOLD_R_SAVE_MSG 16
#define LEFT_ARROW_MSG 17
#define RIGHT_ARROW_MSG 18
#define COMPLETE_MSG 19

const char* const string_table[] PROGMEM = {str00, str01, str02, str03, str04, str05, str06, str07, str08, str09, str10, str11, str12, str13, str14, str15, str16, str17, str18, str19, str20, str21, str22, str23, str24};
void printMessage(LiquidCrystal_I2C* lcd, int message, bool clear = true);

#endif // STRINGS_H