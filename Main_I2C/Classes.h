#ifndef CLASSES_H
#define CLASSES_H

#include <arduino.h>

#define signed_byte int8_t // equivalent to 'char' but more clear

class Stepper;
class LiquidCrystal_I2C;
class DS3231;

//--------------------------------------------------------------------
class Button
{
public:
	Button(byte pin) : m_pin(pin)
	{
		digitalWrite(m_pin, LOW);
		pinMode(m_pin, INPUT);
	}

	bool isPressed() const;

private:
	const byte m_pin;
};

//--------------------------------------------------------------------
#define STEPPER_DIRECTION -1 // Change to -1 to switch open/close directions
#define MAX_STEPS 65534 // Max steps to take before giving up (currently set to max value of an unsigned int)
#define RELAY_PIN 13

class Door
{
public:
	Door(byte switch_pin, int steps, Stepper* m, LiquidCrystal_I2C* d);
	bool isOpen() const {return m_open;}
	void open(bool override_open = false); // If override_open = true, it does not check whether door is already open
	void close();
	void calibrate();
	void openSteps(int steps);
	void closeSteps(int steps);
	void block() {m_blocked = true;}  // blocks door from opening/closing automatically (for calibration)
	void unBlock() {m_blocked = false;}

	void setStepsToClose(unsigned int steps) {m_stepsToClose = steps;}
	void setDoorState(bool state) {m_open = state;} // true = open, false = closed

private:
	const byte m_switch_pin;
	bool m_open;
	unsigned int m_stepsToClose;
	Stepper* m_motor;
	LiquidCrystal_I2C* m_display;
	bool m_blocked;
	void m_openRelay();
	void m_closeRelay();
};

//--------------------------------------------------------------------
class Clock
{
public:
	Clock(signed_byte tzone = 0);
	void setTimezone(signed_byte tzone);
	void setTime(byte hour, byte minute);
	void setDate(int year, byte month, byte day);
	void setOpenDelay(signed_byte delay);
	void setCloseDelay(signed_byte delay);
	byte getDay() const;
	byte getMonth() const;
	int getYear() const;
	byte getHour() const;
	byte getMin() const;
	String getTimeStr() const;
	String getDateStr() const;
	String getOpenTimeStr() const;
	String getCloseTimeStr() const;

	void printOpenTime(LiquidCrystal_I2C* lcd) const;
	void printCloseTime(LiquidCrystal_I2C* lcd) const;

	signed_byte getOpenDelay() const {return m_openDelay;}
	signed_byte getCloseDelay() const {return m_closeDelay;}
	signed_byte getTimezone() const {return m_timezone;}

	float getTemp() const;

	bool sunriseListener();
	bool sunsetListener();

	bool isDay();
	bool isNight();

private:
	DS3231* m_rtc;
	int m_getDayNum();
	bool m_lastReading_riseListener; // true for day, false for night
	bool m_lastReading_setListener;
	signed_byte m_timezone; // with respect to UTC
	signed_byte m_openDelay; // in minutes
	signed_byte m_closeDelay; // in minutes

	void m_addMinutes(byte &hour, byte &minute, int add);
};

//--------------------------------------------------------------------
class Display
{
public:
	Display(LiquidCrystal_I2C* lcd, Door* door, Clock* cl, Button* rb, Button* lb);
	void rightClick();
	void leftClick();
	void rightDoubleClick();
	void leftDoubleClick();
	void rightLongClick();
	void leftLongClick();
	void refresh() {m_display();}
	void turnOff();
	bool isOn() {return !(m_currentMenu == OFF);}
	unsigned long timeInactive() {return millis() - m_lastActive;}

private:
	enum Menu {OFF, DOOR_STATUS, TEMP_AND_DATE, DOOR_MODIFY, DOOR_MANUAL_MODIFY, DOOR_CALIBRATE, OPEN_DELAY_MODIFY, CLOSE_DELAY_MODIFY, CALIBRATION_WAIT, OPEN_DELAY_COUNTER, CLOSE_DELAY_COUNTER, TIMEZONE_MODIFY, TIMEZONE_COUNTER, TIME_MODIFY, DATE_MODIFY, HOURS_COUNTER, MINUTES_COUNTER, YEAR_COUNTER, MONTH_COUNTER, DAY_COUNTER};
	Menu m_currentMenu;

	LiquidCrystal_I2C* m_lcd;
	Door* m_door;
	Clock* m_clock;

	Button* m_rightButton;
	Button* m_leftButton;

	unsigned long m_lastActive;

	// menu display functions
	void m_display(); // displays current menu
	void m_display(Menu m);
};

#endif // CLASSES_H