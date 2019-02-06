#pragma once

//////////////////////////////////////////////////////////////////////
// 
// DateTime.h - Collection of all Date and Time related functions
//
//////////////////////////////////////////////////////////////////////

#include <string>

struct gpsData;

#ifndef DATETIME_H
#define DATETIME_H

/** This storage class for a timestamp is commonly used throughout the program. */
typedef struct Time
{
	char hour   = 0;
	char minute = 0;
	char second = 0;
} Time;

/** Retrieves the current date, either from the system time.
	The date is a string formatted as: ddmmyy (6 characters) */
std::string GetCurrentDateFromComputerClock(char separatorCharacter);

/** Extracts the time from the provided gpsData and separates it into hour-minute-second */
void ExtractTime(const gpsData& gpsData, int& hours, int& minutes, int& seconds);

/** Extracts the date from the provided gpsData and separates it into day-month-year */
void ExtractDate(const gpsData& gpsData, int& day, int& month, int& year);

/** Reads out the data in the provided gpsData and formats it in the format 'ddmmyy' */
std::string GetDate(const gpsData& data);

/** Reads out the data in the provided gpsData and formats it as a string with 
	the given separator character.
	E.g. the values data.date=120514 (May 12th 2014) and separatorCharacter='.' will return the string "12.05.14" */
std::string GetDate(const gpsData& data, char separatorCharacter);

/** Reads out the timestamp in the provided gpsData */
long GetTime(const gpsData& data);

/** This function converts a time value given as an integer to three values (hour, minute second) */
void GetHrMinSec(int time, int &hr, int &min, int &sec);

/** Formats the provided timestamp (which should be in hhmmss format) as a string with 
	the given separator character. 
	E.g. the values hhmmss=121314 and separatorCharacter=':' will return the string "12:13:14" */
std::string FormatTime(long hhmmss, char separatorCharacter = ':');

#endif  // DATETIME_H