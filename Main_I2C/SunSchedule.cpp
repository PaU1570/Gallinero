#include "SunSchedule.h"

SimpleTime timeBuffer;
void readTimeFromPROGMEM(int daynum, char type)
{
	if (type == 'r') // rise
	{
		memcpy_P(&timeBuffer, &sunriseTimes[daynum], sizeof(SimpleTime));
	}
	else if (type == 's') // set
	{
		memcpy_P(&timeBuffer, &sunsetTimes[daynum], sizeof(SimpleTime));
	}
}

byte getSunriseHour(int day)
{
	readTimeFromPROGMEM(day, 'r');
	return timeBuffer.hour;
}

byte getSunriseMinute(int day)
{
	readTimeFromPROGMEM(day, 'r');
	return timeBuffer.min;
}

byte getSunsetHour(int day)
{
	readTimeFromPROGMEM(day, 's');
	return timeBuffer.hour;
}

byte getSunsetMinute(int day)
{
	readTimeFromPROGMEM(day, 's');
	return timeBuffer.min;
}