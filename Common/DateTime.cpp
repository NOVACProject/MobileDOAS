#include "StdAfx.h"
#include "DateTime.h"
#include "../GpsData.h"

#include <assert.h>
#include <time.h>
#include <vector>

std::string GetCurrentDateFromComputerClock(char separatorCharacter)
{
	char startDate[32];
	time_t t;
	time(&t);
	struct tm *tim = localtime(&t);
	int mon = tim->tm_mon + 1;
	int day = tim->tm_mday;
	int year = tim->tm_year - 100; // only good for 21st century
	sprintf(startDate, "%02d%c%02d%c%02d", day, separatorCharacter, mon, separatorCharacter, year);

	return std::string(startDate, 8);
}

void ExtractTime(const gpsData& gpsData, int& hours, int& minutes, int& seconds)
{
	hours   = gpsData.time / 10000;
	minutes = (gpsData.time - hours * 10000) / 100;
	seconds = gpsData.time % 100;
}

void ExtractDate(const gpsData& gpsData, int& day, int& month, int& year)
{
	day   = gpsData.date / 10000;
	month = (gpsData.date - day * 10000) / 100;
	year  = gpsData.date % 100;

	assert(day >= 1 && day <= 31);
	assert(month >= 1 && month <= 12);
}

std::string GetDate(const gpsData& data)
{
	char buffer[7];
	sprintf(buffer, "%06d", data.date);
	return std::string(buffer);
}

std::string GetDate(const gpsData& data, char separatorCharacter)
{
	int day, month, year;
	ExtractDate(data, day, month, year);

	char buffer[64];
	sprintf(buffer, "%02d%c%02d%c%02d", day, separatorCharacter, month, separatorCharacter, year);
	return std::string(buffer);
}


long GetTime(const gpsData& data)
{
	return data.time;
}

void GetHrMinSec(int time, int &hr, int &min, int &sec)
{
	hr = time / 10000;
	min = (time - hr * 10000) / 100;
	sec = time % 100;

	// make sure that there's no numbers greather than or equal to 60 (or 24) !!!
	if (sec >= 60) {
		sec -= 60;
		min += 1;
	}
	if (min >= 60) {
		min -= 60;
		hr += 1;
	}
	hr = hr % 24;
}

std::string FormatTime(long hhmmss, char separatorCharacter)
{
	int hr, min, sec;
	GetHrMinSec(hhmmss, hr, min, sec);

	char msgBuffer[64];
	sprintf(msgBuffer, "%02d%c%02d%c%02d", hr, separatorCharacter, min, separatorCharacter, sec);

	return std::string(msgBuffer);
}