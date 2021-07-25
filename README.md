# Gallinero
Code for an automatic chicken coop door using an Arduino and a stepper motor.
<p>
The door opens and closes according to sunrise/sunset times. Sunrise/sunset time data for each day of the year is stored in <code>SunSchedule.h</code>.
This data can be obtained from the <a href="https://gml.noaa.gov/grad/solcalc/">NOAA Solar Calculator</a>.
</p>
<p>
An Arduino with an RTC module and a stepper motor driver is required (I used a DS3231 and an L298N board).
The interface is controlled with two buttons (Left and Right). Each button can register three types of clicks: normal, double click, and long click.
These are used to navigate the interface and change settings. Settings are saved into EEPROM, so they are not lost after loss of power.
