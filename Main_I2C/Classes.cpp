#include "Classes.h"
#include "Strings.h"
#include <Stepper.h>
#include <LiquidCrystal_I2C.h>
#include <DS3231.h>
#include <EEPROM.h>
#include "EEPROM_ADDRESSES.h"
#include "SunSchedule.h"

/****************************************************************/
/*						BUTTON									*/
/****************************************************************/
bool Button::isPressed() const
{
	return digitalRead(m_pin);
}


/****************************************************************/
/*						DOOR									*/
/****************************************************************/
Door::Door(byte switch_pin, int steps, Stepper* m, LiquidCrystal_I2C* d) : m_switch_pin(switch_pin), m_open(false), m_stepsToClose(steps), m_motor(m), m_display(d), m_blocked(false)
{
	pinMode(switch_pin, INPUT);
	pinMode(RELAY_PIN, OUTPUT);
	digitalWrite(RELAY_PIN, LOW);
}

void Door::open(bool override_open)
{
	if (m_blocked)
		return;

	if (m_open && !override_open)
		return;

	m_openRelay();

	int steps = 0;
	int switch_pos = digitalRead(m_switch_pin);

	printMessage(m_display, DOOR_OPENING_MSG);
	while (switch_pos == 0)
	{
		switch_pos = digitalRead(m_switch_pin);
		m_motor->step(STEPPER_DIRECTION);
		steps++;
		if (steps > MAX_STEPS)
			break;
	}
	m_open = true;

	EEPROM.put(DOOR_STATE_EEPROM_ADDR, m_open);

	m_closeRelay();
}

void Door::close()
{
	if (m_blocked)
		return;

	if (m_stepsToClose < 1 || !m_open)
		return;

	m_openRelay();

	printMessage(m_display, DOOR_CLOSING_MSG);
	m_motor->step(-m_stepsToClose * STEPPER_DIRECTION);
	m_open = false;

	EEPROM.put(DOOR_STATE_EEPROM_ADDR, m_open);

	m_closeRelay();
}

void Door::calibrate()
{
	m_openRelay();

	printMessage(m_display, DOOR_CALIBRATING_MSG);
	m_display->setCursor(0, 1);
	m_display->print(F("door..."));

	m_stepsToClose = 0;
	int switch_pos = digitalRead(m_switch_pin);
	while (switch_pos == 0)
	{
		switch_pos = digitalRead(m_switch_pin);
		m_motor->step(STEPPER_DIRECTION);
		m_stepsToClose++;
		if (m_stepsToClose > MAX_STEPS)
			break;
	}

	printMessage(m_display, DOOR_CALIBRATED_MSG);
	m_display->setCursor(0, 1);
	printMessage(m_display, COMPLETE_MSG, false);
	delay(2000);
	printMessage(m_display, DOOR_STEPS_MSG);
	m_display->setCursor(0, 1);
	m_display->print(m_stepsToClose);
	m_open = true;

	EEPROM.put(STEPS_TO_CLOSE_EEPROM_ADDR, m_stepsToClose);
	EEPROM.put(DOOR_STATE_EEPROM_ADDR, m_open);

	m_closeRelay();
}

void Door::openSteps(int steps)
{
	m_openRelay();
	m_motor->step(steps * STEPPER_DIRECTION);
	m_closeRelay();
}

void Door::closeSteps(int steps)
{
	m_openRelay();
	m_motor->step(steps * (-STEPPER_DIRECTION));
	m_closeRelay();
}

void Door::m_openRelay()
{
	digitalWrite(RELAY_PIN, HIGH);
}

void Door::m_closeRelay()
{
	digitalWrite(RELAY_PIN, LOW);
}


/****************************************************************/
/*						CLOCK									*/
/****************************************************************/
Clock::Clock(signed_byte tzone) : m_timezone(tzone)
{
	m_rtc = new DS3231(SDA, SCL);
	m_rtc->begin();
	m_lastReading_riseListener = isDay();
	m_lastReading_setListener = m_lastReading_riseListener;
}

void Clock::setTimezone(signed_byte tzone)
{
	m_timezone = tzone;
}

void Clock::setTime(byte hour, byte minute)
{
	Time currentTime = m_rtc->getTime();
	m_rtc->setTime(hour, minute, currentTime.sec);
}

void Clock::setDate(int year, byte month, byte day)
{
	m_rtc->setDate(day, month, year);
}

void Clock::setOpenDelay(signed_byte delay)
{
	m_openDelay = delay;
}
void Clock::setCloseDelay(signed_byte delay)
{
	m_closeDelay = delay;
}

byte Clock::getDay() const
{
	Time t = m_rtc->getTime();
	return t.date;
}

byte Clock::getMonth() const
{
	Time t = m_rtc->getTime();
	return t.mon;
}

int Clock::getYear() const
{
	Time t = m_rtc->getTime();
	return t.year;
}

byte Clock::getHour() const
{
	Time t = m_rtc->getTime();
	return t.hour;
}

byte Clock::getMin() const
{
	Time t = m_rtc->getTime();
	return t.min;
}

String Clock::getTimeStr() const
{
	return m_rtc->getTimeStr(FORMAT_SHORT);
}

String Clock::getDateStr() const
{
	return m_rtc->getDateStr(FORMAT_LONG, FORMAT_BIGENDIAN, '/');
}

String Clock::getOpenTimeStr() const
{
	int daynum = m_getDayNum();
	byte hour = getSunriseHour(daynum);
	byte min = getSunriseMinute(daynum);
	m_addMinutes(hour, min, m_timezone*60 + m_openDelay);

	char str[5];
	sprintf(str, "%02d:%02d", hour, min);
	return str;
}

String Clock::getCloseTimeStr() const
{
	int daynum = m_getDayNum();
	byte hour = getSunsetHour(daynum);
	byte min = getSunsetMinute(daynum);
	m_addMinutes(hour, min, m_timezone*60 + m_closeDelay);

	char str[5];
	sprintf(str, "%02d:%02d", hour, min);
	return str;
}

void Clock::printOpenTime(LiquidCrystal_I2C* lcd) const
{
	int daynum = m_getDayNum();
	byte hour = getSunriseHour(daynum);
	byte min = getSunriseMinute(daynum);
	m_addMinutes(hour, min, m_timezone*60 + m_openDelay);

	char str[5];
	sprintf(str, "%02d:%02d", hour, min);
	lcd->print(str);
}

void Clock::printCloseTime(LiquidCrystal_I2C* lcd) const
{
	int daynum = m_getDayNum();
	byte hour = getSunsetHour(daynum);
	byte min = getSunsetMinute(daynum);
	m_addMinutes(hour, min, m_timezone*60 + m_closeDelay);

	char str[5];
	sprintf(str, "%02d:%02d", hour, min);
	lcd->print(str);
}

// sunrise happens if it was night and now it is day
bool Clock::sunriseListener()
{
	// store current state to use after changing m_lastReading
	bool current = isDay();
	bool last = m_lastReading_riseListener;

	m_lastReading_riseListener = current;

	return (!last && current);
}

// sunset happens if it was day and now it is night
bool Clock::sunsetListener()
{
	// store current state to use after changing m_lastReading
	bool current = isDay();
	bool last = m_lastReading_setListener;

	m_lastReading_setListener = current;

	return (last && !current);
}

// Sunrise time is included in day, sunset time is included in night
bool Clock::isDay()
{
	int daynum = m_getDayNum();
	byte riseHour = getSunriseHour(daynum);
	byte riseMin = getSunriseMinute(daynum);
	byte setHour = getSunsetHour(daynum);
	byte setMin = getSunsetMinute(daynum);
	m_addMinutes(riseHour, riseMin, m_timezone*60 + m_openDelay);
	m_addMinutes(setHour, setMin, m_timezone*60 + m_closeDelay);

	Time currentTime = m_rtc->getTime();

	// compare hours first
	if (currentTime.hour > riseHour && currentTime.hour < setHour)
	{
		return true;
	}
	else if (currentTime.hour == riseHour)
	{
		return (currentTime.min >= riseMin);
	}
	else if (currentTime.hour == setHour)
	{
		return (currentTime.min < setMin);
	}
	
	return false;
}

bool Clock::isNight()
{
	return !isDay();
}

int Clock::m_getDayNum()
{
	byte day = getDay();
	byte month = getMonth();

	switch (month)
	{
		case 1:
			return day;
		case 2:
			return 31 + day;
		case 3:
			return 31 + 28 + day;
		case 4:
			return 31 + 28 + 31 + day;
		case 5:
			return 31 + 28 + 31 + 30 + day;
		case 6:
			return 31 + 28 + 31 + 30 + 31 + day;
		case 7:
			return 31 + 28 + 31 + 30 + 31 + 30 + day;
		case 8:
			return 31 + 28 + 31 + 30 + 31 + 30 + 31 + day;
		case 9:
			return 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + day;
		case 10:
			return 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + day;
		case 11:
			return 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + day;
		case 12:
			return 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + day;
		default:
			return 1;
	}
}

float Clock::getTemp() const
{
	return m_rtc->getTemp();
}

void Clock::m_addMinutes(byte &hour, byte &minute, int add)
{
	int minutes = hour*60 + minute + add;
	hour = minutes / 60;
	minute = minutes % 60;
}

/****************************************************************/
/*						DISPLAY									*/
/****************************************************************/
Display::Display(LiquidCrystal_I2C* lcd, Door* door, Clock* cl, Button* rb, Button* lb) : m_currentMenu(OFF), m_lcd(lcd), m_door(door),
m_clock(cl), m_rightButton(rb), m_leftButton(lb), m_lastActive(millis())
{}

// right click cycles through menus
void Display::rightClick()
{
	m_lastActive = millis();
	switch (m_currentMenu)
	{
		case OFF:
			m_lcd->display();
			m_lcd->backlight();
			m_currentMenu = DOOR_STATUS;
			break;

		case DOOR_STATUS:
			m_currentMenu = TEMP_AND_DATE;
			break;

		case TEMP_AND_DATE:
			m_currentMenu = DOOR_STATUS;
			break;

		case DOOR_MODIFY:
			m_currentMenu = DOOR_MANUAL_MODIFY;
			break;

		case DOOR_MANUAL_MODIFY:
			m_currentMenu = DOOR_CALIBRATE;
			break;

		case DOOR_CALIBRATE:
			m_currentMenu = OPEN_DELAY_MODIFY;
			break;

		case OPEN_DELAY_MODIFY:
			m_currentMenu = CLOSE_DELAY_MODIFY;
			break;

		case CLOSE_DELAY_MODIFY:
			m_currentMenu = TIMEZONE_MODIFY;
			break;

		case TIMEZONE_MODIFY:
			m_currentMenu = TIME_MODIFY;
			break;

		case TIME_MODIFY:
			m_currentMenu = DATE_MODIFY;
			break;

		case DATE_MODIFY:
			m_currentMenu = DOOR_MODIFY;

		case OPEN_DELAY_COUNTER:
			m_clock->setOpenDelay(m_clock->getOpenDelay() + 1);
			break;

		case CLOSE_DELAY_COUNTER:
			m_clock->setCloseDelay(m_clock->getCloseDelay() + 1);
			break;

		case TIMEZONE_COUNTER:
			m_clock->setTimezone(m_clock->getTimezone() + 1);
			break;

		case HOURS_COUNTER:
			m_clock->setTime(m_clock->getHour() + 1, m_clock->getMin());
			break;

		case MINUTES_COUNTER:
			m_clock->setTime(m_clock->getHour(), m_clock->getMin() + 1);
			break;

		case YEAR_COUNTER:
			m_clock->setDate(m_clock->getYear() + 1, m_clock->getMonth(), m_clock->getDay());
			break;

		case MONTH_COUNTER:
			m_clock->setDate(m_clock->getYear(), m_clock->getMonth() + 1, m_clock->getDay());
			break;

		case DAY_COUNTER:
			m_clock->setDate(m_clock->getYear(), m_clock->getMonth(), m_clock->getDay() + 1);
			break;

		default:
			break;
	}
	m_display();
}

// left click cycles through menus
void Display::leftClick()
{
	m_lastActive = millis();
	switch (m_currentMenu)
	{
		case OFF:
			m_lcd->display();
			m_lcd->backlight();
			m_currentMenu = DOOR_STATUS;
			break;

		case DOOR_STATUS:
			m_currentMenu = TEMP_AND_DATE;
			break;

		case TEMP_AND_DATE:
			m_currentMenu = DOOR_STATUS;
			break;

		case DOOR_MODIFY:
			m_currentMenu = DATE_MODIFY;
			break;

		case DOOR_MANUAL_MODIFY:
			m_currentMenu = DOOR_MODIFY;
			break;

		case DOOR_CALIBRATE:
			m_currentMenu = DOOR_MANUAL_MODIFY;
			break;

		case OPEN_DELAY_MODIFY:
			m_currentMenu = DOOR_CALIBRATE;
			break;

		case CLOSE_DELAY_MODIFY:
			m_currentMenu = OPEN_DELAY_MODIFY;
			break;

		case TIMEZONE_MODIFY:
			m_currentMenu = CLOSE_DELAY_MODIFY;
			break;

		case TIME_MODIFY:
			m_currentMenu = TIMEZONE_MODIFY;
			break;

		case DATE_MODIFY:
			m_currentMenu = TIME_MODIFY;
			break;

		case OPEN_DELAY_COUNTER:
			m_clock->setOpenDelay(m_clock->getOpenDelay() - 1);
			break;

		case CLOSE_DELAY_COUNTER:
			m_clock->setCloseDelay(m_clock->getCloseDelay() - 1);
			break;

		case TIMEZONE_COUNTER:
			m_clock->setTimezone(m_clock->getTimezone() - 1);
			break;

		case HOURS_COUNTER:
			m_clock->setTime(m_clock->getHour() - 1, m_clock->getMin());
			break;

		case MINUTES_COUNTER:
			m_clock->setTime(m_clock->getHour(), m_clock->getMin() - 1);
			break;

		case YEAR_COUNTER:
			m_clock->setDate(m_clock->getYear() - 1, m_clock->getMonth(), m_clock->getDay());
			break;

		case MONTH_COUNTER:
			m_clock->setDate(m_clock->getYear(), m_clock->getMonth() - 1, m_clock->getDay());
			break;

		case DAY_COUNTER:
			m_clock->setDate(m_clock->getYear(), m_clock->getMonth(), m_clock->getDay() - 1);
			break;

		default:
			break;
	}
	m_display();
}

// right double click enters a submenu
void Display::rightDoubleClick()
{
	m_lastActive = millis();
	switch (m_currentMenu)
	{
		case OFF:
			rightClick();
			break;

		case DOOR_STATUS:
			m_currentMenu = OPEN_DELAY_MODIFY;
			break;

		default:
			break;
	}
	m_display();
}

// left double click leaves the submenu
void Display::leftDoubleClick()
{
	m_lastActive = millis();
	switch (m_currentMenu)
	{
		case OFF:
			leftClick();
			break;

		case DOOR_STATUS:	// Fallthrough intentional
		case TEMP_AND_DATE:
			m_currentMenu = OFF;
			break;

		case DOOR_MODIFY:
		case DOOR_MANUAL_MODIFY:
		case DOOR_CALIBRATE:
		case OPEN_DELAY_MODIFY:
		case CLOSE_DELAY_MODIFY:
		case TIMEZONE_MODIFY:
		case TIME_MODIFY:
		case DATE_MODIFY:
			m_currentMenu = DOOR_STATUS;
			break;

		case CALIBRATION_WAIT:
			m_currentMenu = DOOR_CALIBRATE;
			break;

		case OPEN_DELAY_COUNTER:
			m_currentMenu = OPEN_DELAY_MODIFY;
			break;

		case CLOSE_DELAY_COUNTER:
			m_currentMenu = CLOSE_DELAY_MODIFY;
			break;

		case TIMEZONE_COUNTER:
			m_currentMenu = TIMEZONE_MODIFY;
			break;

		case HOURS_COUNTER:
			m_currentMenu = TIME_MODIFY;
			break;

		case MINUTES_COUNTER:
			m_currentMenu = HOURS_COUNTER;
			break;

		case YEAR_COUNTER:
			m_currentMenu = DATE_MODIFY;
			break;

		case MONTH_COUNTER:
			m_currentMenu = YEAR_COUNTER;
			break;

		case DAY_COUNTER:
			m_currentMenu = MONTH_COUNTER;
			break;

		default:
			m_currentMenu = DOOR_STATUS;
			break;
	}
	m_display();
}

// right long click opens/closes door; enters/confirms chicken count/light level change
void Display::rightLongClick()
{
	m_lastActive = millis();
	switch (m_currentMenu)
	{
		case OFF:
			turnOff();
			break;

		case DOOR_MODIFY:
			if (m_door->isOpen())
				m_door->close();
			else
				m_door->open();

			break;

		case DOOR_CALIBRATE:
			m_currentMenu = CALIBRATION_WAIT;
			m_door->block();
			break;

		case CALIBRATION_WAIT:
			m_door->calibrate();
			m_currentMenu = DOOR_STATUS;
			delay(2000);
			m_door->unBlock();
			break;

		case DOOR_MANUAL_MODIFY:
			while (m_rightButton->isPressed())
			{
				m_door->openSteps(1);
			}
			break;

		case OPEN_DELAY_MODIFY:
			m_currentMenu = OPEN_DELAY_COUNTER;
			break;

		case CLOSE_DELAY_MODIFY:
			m_currentMenu = CLOSE_DELAY_COUNTER;
			break;

		case OPEN_DELAY_COUNTER:
			EEPROM.put(OPEN_DELAY_EEPROM_ADDR, m_clock->getOpenDelay());
			m_currentMenu = OPEN_DELAY_MODIFY;
			break;

		case CLOSE_DELAY_COUNTER:
			EEPROM.put(CLOSE_DELAY_EEPROM_ADDR, m_clock->getCloseDelay());
			m_currentMenu = CLOSE_DELAY_MODIFY;
			break;

		case TIMEZONE_MODIFY:
			m_currentMenu = TIMEZONE_COUNTER;
			break;

		case TIMEZONE_COUNTER:
			EEPROM.put(TIMEZONE_EEPROM_ADDR, m_clock->getTimezone());
			m_currentMenu = TIMEZONE_MODIFY;
			break;

		case TIME_MODIFY:
			m_currentMenu = HOURS_COUNTER;
			break;

		case HOURS_COUNTER:
			m_currentMenu = MINUTES_COUNTER;
			break;

		case MINUTES_COUNTER:
			m_currentMenu = TIME_MODIFY;
			break;

		case DATE_MODIFY:
			m_currentMenu = YEAR_COUNTER;
			break;

		case YEAR_COUNTER:
			m_currentMenu = MONTH_COUNTER;
			break;

		case MONTH_COUNTER:
			m_currentMenu = DAY_COUNTER;
			break;

		case DAY_COUNTER:
			m_currentMenu = DATE_MODIFY;
			break;

		default:
			break;
	}
	m_display();
}

void Display::leftLongClick()
{
	m_lastActive = millis();
	switch (m_currentMenu)
	{
		case DOOR_MANUAL_MODIFY:
			while (m_leftButton->isPressed())
			{
				m_door->closeSteps(1);
			}
			break;

		case CALIBRATION_WAIT:
			while (m_leftButton->isPressed())
			{
				m_door->closeSteps(1);
			}

		default:
			break;
	}
	m_display();
}

void Display::m_display()
{
	m_display(m_currentMenu);
}

void Display::m_display(Menu m)
{
	switch (m)
	{
		case OFF:
			turnOff();
			break;

		case DOOR_STATUS:
			if (m_door->isOpen())
				printMessage(m_lcd, DOOR_OPEN_MSG);
			else
				printMessage(m_lcd, DOOR_CLOSED_MSG);

			m_lcd->setCursor(0, 1);

			if (m_door->isOpen())
			{
				m_lcd->print(F("Closes at "));
				m_lcd->print(m_clock->getCloseTimeStr());
			}
			else
			{
				m_lcd->print(F("Opens at "));
				m_lcd->print(m_clock->getOpenTimeStr());
			}

			break;

		case TEMP_AND_DATE:
			m_lcd->clear();
			m_lcd->print(F("   "));
			m_lcd->print(m_clock->getDateStr());
			m_lcd->setCursor(0, 1);
			m_lcd->print(m_clock->getTimeStr());
			m_lcd->print(F("   "));
			m_lcd->print(m_clock->getTemp());
			m_lcd->print(F(" "));
			m_lcd->write(223);
			m_lcd->print(F("C"));

			break;

		case DOOR_MODIFY:
			printMessage(m_lcd, HOLD_R_MSG);
			m_lcd->setCursor(0, 1);
			if (m_door->isOpen())
				printMessage(m_lcd, OPEN_DOOR_MSG, false);
			else
				printMessage(m_lcd, CLOSE_DOOR_MSG, false);

			break;

		case DOOR_MANUAL_MODIFY:
			m_lcd->clear();
			m_lcd->print(F("Hold R/L for ma-"));
			m_lcd->setCursor(0, 1);
			m_lcd->print(F("nual open/close."));
			break;

		case DOOR_CALIBRATE:
			printMessage(m_lcd, HOLD_R_MSG);
			m_lcd->setCursor(0, 1);
			m_lcd->print(F("calibrate door."));
			break;

		case CALIBRATION_WAIT:
			m_lcd->clear()	;
			m_lcd->print(F("Close door. Hold"));
			m_lcd->setCursor(0, 1);
			m_lcd->print(F("R to continue."));
			break;

		case OPEN_DELAY_MODIFY:
			printMessage(m_lcd, HOLD_R_MSG);
			m_lcd->print(F(" change"));
			m_lcd->setCursor(0, 1);
			m_lcd->print(F("open delay ("));
			m_lcd->print(m_clock->getOpenDelay());
			m_lcd->print(F(")"));
			break;

		case CLOSE_DELAY_MODIFY:
			printMessage(m_lcd, HOLD_R_MSG);
			m_lcd->print(F(" change"));
			m_lcd->setCursor(0, 1);
			m_lcd->print(F("close delay("));
			m_lcd->print(m_clock->getCloseDelay());
			m_lcd->print(F(")"));
			break;

		case OPEN_DELAY_COUNTER:
			m_lcd->clear();
			m_lcd->print(F("  <"));
			m_lcd->print(m_clock->getOpenDelay());
			m_lcd->print(F(">  "));
			m_clock->printOpenTime(m_lcd);
			m_lcd->setCursor(0, 1);
			printMessage(m_lcd, HOLD_R_SAVE_MSG, false);
			break;

		case CLOSE_DELAY_COUNTER:
			m_lcd->clear();
			m_lcd->print(F("  <"));
			m_lcd->print(m_clock->getCloseDelay());
			m_lcd->print(F(">  "));
			m_clock->printCloseTime(m_lcd);
			m_lcd->setCursor(0, 1);
			printMessage(m_lcd, HOLD_R_SAVE_MSG, false);
			break;

		case TIMEZONE_MODIFY:
			m_lcd->clear();
			m_lcd->print(F("Hold R to change"));
			m_lcd->setCursor(0, 1);
			m_lcd->print(F("timezone ("));
			m_lcd->print(m_clock->getTimezone());
			m_lcd->print(F(")"));
			break;

		case TIMEZONE_COUNTER:
			printMessage(m_lcd, LEFT_ARROW_MSG);
			m_lcd->print(m_clock->getTimezone());
			printMessage(m_lcd, RIGHT_ARROW_MSG, false);
			m_lcd->setCursor(0, 1);
			printMessage(m_lcd, HOLD_R_SAVE_MSG, false);
			break;

		case TIME_MODIFY:
			m_lcd->clear();
			m_lcd->print(F("Hold R to change"));
			m_lcd->setCursor(0, 1);
			m_lcd->print(F("time ("));
			m_lcd->print(m_clock->getTimeStr());
			m_lcd->print(F(")"));
			break;

		case HOURS_COUNTER:
			printMessage(m_lcd, LEFT_ARROW_MSG);
			m_lcd->print(m_clock->getHour());
			printMessage(m_lcd, RIGHT_ARROW_MSG, false);
			m_lcd->setCursor(0, 1);
			printMessage(m_lcd, HOLD_R_SAVE_MSG, false);
			break;

		case MINUTES_COUNTER:
			printMessage(m_lcd, LEFT_ARROW_MSG);
			m_lcd->print(m_clock->getMin());
			printMessage(m_lcd, RIGHT_ARROW_MSG, false);
			m_lcd->setCursor(0, 1);
			printMessage(m_lcd, HOLD_R_SAVE_MSG, false);
			break;

		case DATE_MODIFY:
			m_lcd->clear();
			m_lcd->print(F("Hold R to change"));
			m_lcd->setCursor(0, 1);
			m_lcd->print(F("date("));
			m_lcd->print(m_clock->getDateStr());
			m_lcd->print(F(")"));
			break;

		case YEAR_COUNTER:
			printMessage(m_lcd, LEFT_ARROW_MSG);
			m_lcd->print(m_clock->getYear());
			printMessage(m_lcd, RIGHT_ARROW_MSG, false);
			m_lcd->setCursor(0, 1);
			printMessage(m_lcd, HOLD_R_SAVE_MSG, false);
			break;

		case MONTH_COUNTER:
			printMessage(m_lcd, LEFT_ARROW_MSG);
			m_lcd->print(m_clock->getMonth());
			printMessage(m_lcd, RIGHT_ARROW_MSG, false);
			m_lcd->setCursor(0, 1);
			printMessage(m_lcd, HOLD_R_SAVE_MSG, false);
			break;

		case DAY_COUNTER:
			printMessage(m_lcd, LEFT_ARROW_MSG);
			m_lcd->print(m_clock->getDay());
			printMessage(m_lcd, RIGHT_ARROW_MSG, false);
			m_lcd->setCursor(0, 1);
			printMessage(m_lcd, HOLD_R_SAVE_MSG, false);
			break;

		default:
			m_lcd->clear();
			m_lcd->print(F("Menu not yet"));
			m_lcd->setCursor(0, 1);
			m_lcd->print(F("available."));
			break;

	}
}

void Display::turnOff()
{
	m_currentMenu = OFF;
	m_lcd->noDisplay();
	m_lcd->noBacklight();
}
