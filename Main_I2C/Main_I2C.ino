// Gallinero
// Creado por Paul Uriarte
// Junio 2020
// Última actualización: 14 de Agosto 2020

#include <Stepper.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include "EventHandler.h"
#include "Classes.h"
#include "Strings.h"
#include "FreeMemory.h"
#include "EEPROM_ADDRESSES.h"

// Motor
#define STEPS_PER_REV 200
#define MOTOR_SPEED 60 // in rpm
#define STEPS_TO_CLOSE_DOOR 200
#define IN1 11
#define IN2 10
#define IN3 9
#define IN4 8

// Limit switch reads HIGH when not activated (door not open)
#define LIMIT_SWITCH 7
#define DOOR_CHECK_INTERVAL 1800000 // (30 minutes) How often we check to see if door is really open (i.e. limit switch is activated)

// Buttons
#define RIGHT_BUTTON 4
#define LEFT_BUTTON 5
//#define UP_BUTTON 4
//#define DOWN_BUTTON 3
#define DEBOUNCE_TIME 50 // minimum time a button has to be pressed for it to register as a click (in milliseconds)
#define LONG_CLICK_TIME 600 // minimum time to hold for a long click
#define DOUBLE_CLICK_SEPARATION 300 // maximum separation between two clicks for a double click

// LCD
#define DISPLAY_TIMEOUT_TIME 120000 // Time after which display will turn off if inactive

// Listeners
bool dayListener();
bool nightListener();
bool clickListener();
bool rightClickListener();
bool leftClickListener();
bool rightDoubleClickListener();
bool leftDoubleClickListener();
bool rightLongClickListener();
bool leftLongClickListener();
bool limitSwitchListener();
bool displayTimeoutListener();
bool doorCheckListener(); // returns true if door says open but limit switch is not activated
bool displayUpdateListener();
bool displayChanged = false;
//bool upClickListener();
//bool downClickListener();

// Callback functions
void onDay();
void onNight();
void onClick() {} // this is never called
void onRightClick();
void onLeftClick();
void onRightDoubleClick();
void onLeftDoubleClick();
void onRightLongClick();
void onLeftLongClick();
void onLimitSwitch();
void onDisplayTimeout();
void onDoorCheck();
void onDisplayUpdate();
//void onUpClick();
//void onDownClick();


Stepper motor(STEPS_PER_REV, IN1, IN2, IN3, IN4);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Clock myclock;

Door door(LIMIT_SWITCH, STEPS_TO_CLOSE_DOOR, &motor, &lcd);

Button rightButton(RIGHT_BUTTON);
Button leftButton(LEFT_BUTTON);
//Button upButton(UP_BUTTON);
//Button downButton(DOWN_BUTTON);

Display display(&lcd, &door, &myclock, &rightButton, &leftButton);

EventHandler eventHdl;

bool serial = true;

void setup()
{
	if (serial)
	{
		Serial.begin(9600);
		Serial.println(F("Setting up..."));
		Serial.print(F("Free memory: "));
		Serial.println(freeMemory()); // for some reason this needs to be here for the program to work on the off-brand UNO
	}

	// Read data from EEPROM
	myclock.setTimezone(EEPROM.read(TIMEZONE_EEPROM_ADDR));
	myclock.setOpenDelay(EEPROM.read(OPEN_DELAY_EEPROM_ADDR));
	myclock.setCloseDelay(EEPROM.read(CLOSE_DELAY_EEPROM_ADDR));

	unsigned int stepsToClose;
	EEPROM.get(STEPS_TO_CLOSE_EEPROM_ADDR, stepsToClose);
	door.setStepsToClose(stepsToClose);
	door.setDoorState(EEPROM.read(DOOR_STATE_EEPROM_ADDR));

	// Initialize display
	lcd.init();
	lcd.backlight();
	lcd.clear();
	printMessage(&lcd, WELCOME_MSG);
	lcd.setCursor(0, 1);
	lcd.print(F("  Ardugallino   "));
	delay(1500);
	
	// Set motor speed
	motor.setSpeed(MOTOR_SPEED);

	// Add listeners.
	eventHdl.addListener(&dayListener, &onDay); // 0
	eventHdl.addListener(&nightListener, &onNight); // 1
	eventHdl.addListener(&clickListener, &onClick); // 2 this has to be added before the other click listeners
	eventHdl.addListener(&rightClickListener, &onRightClick); // 3
	eventHdl.addListener(&leftClickListener, &onLeftClick); // 4
	eventHdl.addListener(&rightDoubleClickListener, &onRightDoubleClick); // 5
	eventHdl.addListener(&leftDoubleClickListener, &onLeftDoubleClick); // 6
	eventHdl.addListener(&rightLongClickListener, &onRightLongClick); // 7
	eventHdl.addListener(&leftLongClickListener, &onLeftLongClick); // 8
	eventHdl.addListener(&limitSwitchListener, &onLimitSwitch); // 9
	eventHdl.addListener(&displayTimeoutListener, &onDisplayTimeout); // 10
	//eventHdl.addListener(&doorCheckListener, &onDoorCheck); // 11
	eventHdl.addListener(&displayUpdateListener, &onDisplayUpdate); // 12
	//eventHdl.addListener(&upClickListener, &onUpClick); // 13
	//eventHdl.addListener(&downClickListener, &onDownClick); // 14

	if (serial)
	{
		Serial.println(F("Setup complete."));
		Serial.print(F("Free memory: "));
		Serial.println(freeMemory());
	}

	display.rightClick();
	display.refresh();
}

unsigned long lastRefresh = millis();
void loop()
{
	eventHdl.listen();
	eventHdl.processEvent();
	if (serial)
	{
		Serial.print(F("Free memory: "));
		Serial.println(freeMemory());
	}
}

bool dayListener()
{
	return myclock.sunriseListener();
}

bool nightListener()
{
	return myclock.sunsetListener();;
}

bool clickArray[] = {false, false, false, false, false, false}; // {left click, left double click, left long click, right click, right double click, right long click}
bool clickListener() // always returns false
{
	// Determine if we are working with a right or left click, or neither (in that case, do nothing and return false)
	byte shift;
	if (rightButton.isPressed())
		shift = 3;
	else if (leftButton.isPressed())
		shift = 0;
	else
		return false;

	// Measure how long the button was pressed (for a maximum of timeout)
	unsigned long initial_time = millis();
	if (shift == 0)
	{
		while (leftButton.isPressed() && (millis() - initial_time < LONG_CLICK_TIME))
		{}
	}
	else
	{
		while (rightButton.isPressed() && (millis() - initial_time < LONG_CLICK_TIME))
		{}
	}
	unsigned long click_duration = millis() - initial_time;

	if (click_duration < DEBOUNCE_TIME) // no click detected
		return false;

	if (click_duration >= LONG_CLICK_TIME) // long click detected
	{
		clickArray[2 + shift] = true;
		return false;
	}

	// If not long click, wait to see if there is a double click
	while (millis() - (initial_time + click_duration) < DOUBLE_CLICK_SEPARATION)
	{
		click_duration = 0; // repurpose click_duration
		// If button is pressed again, check if it stays pressed for the debounce time
		if (shift == 0 && leftButton.isPressed())
		{
			// repurpose initial_time and click_duration
			initial_time = millis();
			while (leftButton.isPressed() && (millis() - initial_time < DEBOUNCE_TIME))
			{}
			click_duration = millis() - initial_time;
		}
		else if (shift == 3 && rightButton.isPressed())
		{
			// repurpose initial_time and click_duration
			initial_time = millis();
			while (rightButton.isPressed() && (millis() - initial_time < DEBOUNCE_TIME))
			{}
			click_duration = millis() - initial_time;
		}

		if (click_duration >= DEBOUNCE_TIME) // double click detected
		{
			clickArray[1 + shift] = true;
			return false;
		}

	}

	// If not double click, it was a normal click
	clickArray[0 + shift] = true;

	return false;
}

bool leftClickListener()
{
	if (clickArray[0])
	{
		clickArray[0] = false;
		return true;
	}
	return false;
}

bool leftDoubleClickListener()
{
	if (clickArray[1])
	{
		clickArray[1] = false;
		return true;
	}
	return false;
}

bool leftLongClickListener()
{
	if (clickArray[2])
	{
		clickArray[2] = false;
		return true;
	}
	return false;
}

bool rightClickListener()
{
	if (clickArray[3])
	{
		clickArray[3] = false;
		return true;
	}
	return false;
}

bool rightDoubleClickListener()
{
	if (clickArray[4])
	{
		clickArray[4] = false;
		return true;
	}
	return false;
}

bool rightLongClickListener()
{
	if (clickArray[5])
	{
		clickArray[5] = false;
		return true;
	}
	return false;
}

bool limitSwitchListener()
{
	return false;
}

bool displayTimeoutListener()
{
	if (display.isOn())
		return (abs(display.timeInactive()) > DISPLAY_TIMEOUT_TIME);

	return false;
}

unsigned long lastCheckedDoor = millis();
bool doorCheckListener()
{
	if (millis() - lastCheckedDoor > DOOR_CHECK_INTERVAL)
	{
		lastCheckedDoor = millis();
		return (door.isOpen() && !digitalRead(LIMIT_SWITCH));
	}
	return false;
}

byte lastMinute = myclock.getMin();
float lastTemp = myclock.getTemp();
bool displayUpdateListener()
{
	if (myclock.getMin() != lastMinute)
	{
		lastMinute = myclock.getMin();
		return true;
	}
	if (myclock.getTemp() != lastTemp)
	{
		lastTemp = myclock.getTemp();
		return true;
	}
	return displayChanged;
}

/*
bool upClickListener()
{
	unsigned long initial_time = millis();
	while (upButton.isPressed() && (millis() - initial_time < LONG_CLICK_TIME)) // use long click time as maximum wait time
	{}
	
	unsigned long click_duration = millis() - initial_time;
	return (click_duration >= DEBOUNCE_TIME);
}

bool downClickListener()
{
	unsigned long initial_time = millis();
	while (downButton.isPressed() && (millis() - initial_time < LONG_CLICK_TIME)) // use long click time as maximum wait time
	{}
	
	unsigned long click_duration = millis() - initial_time;
	return (click_duration >= DEBOUNCE_TIME);
}
*/

void onDay()
{
	//Serial.println(F("Day!"));
	door.open();
	display.refresh();
}

void onNight()
{
	//Serial.println(F("Night!"));
	door.close();
	display.refresh();
}

void onRightClick()
{
	displayChanged = true;
	//Serial.println(F("Right click!"));
	display.rightClick();
}

void onLeftClick()
{
	displayChanged = true;
	//Serial.println(F("Left click!"));
	display.leftClick();
}

void onRightDoubleClick()
{
	displayChanged = true;
	//Serial.println(F("Right double click!"));
	display.rightDoubleClick();
	// wait until button is lifted to continue
	while (rightButton.isPressed())
	{
	}
}

void onLeftDoubleClick()
{
	displayChanged = true;
	//Serial.println(F("Left double click!"));
	display.leftDoubleClick();
	// wait until button is lifted to continue
	while (leftButton.isPressed())
	{
	}
}

void onRightLongClick()
{
	displayChanged = true;
	//Serial.println(F("Right long click!"));
	display.rightLongClick();
	// wait until button is lifted to continue
	while (rightButton.isPressed())
	{
	}
}

void onLeftLongClick()
{
	displayChanged = true;
	//Serial.println(F("Left long click!"));
	display.leftLongClick();
	// wait until button is lifted to continue
	while (leftButton.isPressed())
	{
	}
}

void onLimitSwitch()
{
}

void onDisplayTimeout()
{
	displayChanged = true;
	display.turnOff();
}

void onDoorCheck()
{
	door.open(true);
	displayChanged = true;
}

void onDisplayUpdate()
{
	display.refresh();
	displayChanged = false;
}

/*
void onUpClick()
{
	displayChanged = true;
	door.open();
	// wait until button is lifted
	while (upButton.isPressed())
	{}
}

void onDownClick()
{
	displayChanged = true;
	door.close();
	// wait until button is lifted
	while (downButton.isPressed())
	{}
}
*/

