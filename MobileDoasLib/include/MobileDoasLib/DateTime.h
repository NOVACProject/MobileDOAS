#pragma once

//////////////////////////////////////////////////////////////////////
// 
// DateTime.h - Collection of all Date and Time related functions
//
//////////////////////////////////////////////////////////////////////

#include <string>

namespace novac
{
    class CDateTime;
}

#ifndef DATETIME_MOBILEDOAS_H
#define DATETIME_MOBILEDOAS_H


namespace mobiledoas
{
    struct GpsData;

    /** This storage class for a timestamp is commonly used throughout the program. */
    typedef struct Time
    {
        char hour = 0;
        char minute = 0;
        char second = 0;
    } Time;

    /** Retrieves the current date, either from the system time.
        The date is a string formatted as: ddmmyy (6 characters) */
    std::string GetCurrentDateFromComputerClock(char separatorCharacter);

    /** Retrieves and fills in the current year/month/date from the system time. */
    void GetCurrentDateFromComputerClock(novac::CDateTime& time);

    /** Extracts the time from the provided GpsData and separates it into hour-minute-second */
    void ExtractTime(const mobiledoas::GpsData& gpsData, int& hours, int& minutes, int& seconds);

    /** Extracts the date from the provided GpsData and separates it into day-month-year */
    void ExtractDate(const mobiledoas::GpsData& gpsData, int& day, int& month, int& year);

    /** Extracts the date and time from the provided GpsData */
    void ExtractDateAndTime(const mobiledoas::GpsData& gpsData, novac::CDateTime& time);

    /** Reads out the data in the provided GpsData and formats it in the format 'ddmmyy' */
    std::string GetDate(const mobiledoas::GpsData& data);

    /** Reads out the data in the provided GpsData and formats it as a string with
        the given separator character.
        E.g. the values data.date=120514 (May 12th 2014) and separatorCharacter='.' will return the string "12.05.14" */
    std::string GetDate(const mobiledoas::GpsData& data, char separatorCharacter);

    /** Reads out the timestamp in the provided GpsData */
    long GetTime(const mobiledoas::GpsData& data);

    /** This function converts a time value given as an integer to three values (hour, minute second) */
    void GetHrMinSec(int time, int& hr, int& min, int& sec);

    /** Formats the provided timestamp (which should be in hhmmss format) as a string with
        the given separator character.
        E.g. the values hhmmss=121314 and separatorCharacter=':' will return the string "12:13:14" */
    std::string FormatTime(long hhmmss, char separatorCharacter = ':');

    /** Formats the current date (from the computer clock) into a string using the YYYY.MM.DD format */
    void GetDateText(char* txt);

    /** Formats the current date and time (from the computer clock) into a string using the YYYY.MM.DD  HH.mm.SS format */
    void GetDateTimeText(char* txt);

    /** Formats the current date and time (from the computer clock) into a string using the YYYY.MM.DD_HHmmSS format */
    std::string GetDateTimeTextPlainFormat();

}

#endif  // DATETIME_MOBILEDOAS_H