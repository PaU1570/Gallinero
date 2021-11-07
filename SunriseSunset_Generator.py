import csv
import re

filename = 'sunrisesunset.csv'

dayexpr = re.compile(r"(\d+)/(\d+)/(\d+)")
hourexpr = re.compile(r"(\d+):(\d+):(\d+)")

option1 = input("Sunrise (r) or Sunset (s) times? ")
option2 = input("Weeks only (w) or all days (d)? ")

alldays = True

with open(filename) as file:
	csv_reader = csv.reader(file, delimiter=',')

	first_line = True
	day = 1
	week = 1
	for row in csv_reader:
		if first_line:
			first_line = False
		else:
			if day % 7 == 1 or option2 == 'd':
				date = dayexpr.findall(row[3])
				date = date[0]
				sunrise = hourexpr.findall(row[24])
				sunrise = sunrise[0]
				sunset = hourexpr.findall(row[25])
				sunset = sunset[0]

				#print("{2}/{1}/{0} => Sunrise: {3}:{4}; Sunset: {5}:{6}".format(date[0], date[1], date[2], sunrise[0], sunrise[1], sunset[0], sunset[1]))
				
				if option1 == 'r':
					# get rid of leading 0s
					minutes = sunrise[1]
					if minutes[0] == '0':
						minutes = minutes[1]

					print("{{{0}, {1}}},".format(sunrise[0], minutes))

				if option1 == 's':
					# get rid of leading 0s
					minutes = sunset[1]
					if minutes[0] == '0':
						minutes = minutes[1]
					print("{{{0}, {1}}},".format(sunset[0], minutes))

				week += 1
			day += 1
