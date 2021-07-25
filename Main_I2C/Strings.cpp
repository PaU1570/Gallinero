#include "Strings.h"
#include <LiquidCrystal_I2C.h>

char buffer[32];
void readIntoBuffer(int i)
{
	strcpy_P(buffer, (char *)pgm_read_word(&(string_table[i])));
}

void printMessage(LiquidCrystal_I2C* lcd, int messageNum, bool clear)
{
	if (clear)
		lcd->clear();
	readIntoBuffer(messageNum);
	lcd->print(buffer);
}
