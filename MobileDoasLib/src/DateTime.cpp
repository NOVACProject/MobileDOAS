#include <MobileDoasLib/DateTime.h>
#include <MobileDoasLib/GpsData.h>
#include <SpectralEvaluation/DateTime.h>

#include <assert.h>
#include <time.h>
#include <vector>

namespace mobiledoas {

    std::string GetCurrentDateFromComputerClock(char separatorCharacter)
    {
        char startDate[32];
        time_t t;
        time(&t);
        struct tm tim;
        localtime_s(&tim, &t);
        int mon = tim.tm_mon + 1;
        int day = tim.tm_mday;
        int year = tim.tm_year - 100; // only good for 21st century
        sprintf_s(startDate, "%02d%c%02d%c%02d", day, separatorCharacter, mon, separatorCharacter, year);

        return std::string(startDate, 8);
    }

    void GetCurrentDateFromComputerClock(novac::CDateTime& result)
    {
        time_t t;
        time(&t);
        struct tm tim;
        localtime_s(&tim, &t);
        result.month = static_cast<unsigned char>(tim.tm_mon + 1);
        result.day = static_cast<unsigned char>(tim.tm_mday);
        result.year = static_cast<unsigned char>(tim.tm_year - 100);
    }

    void ExtractTime(const mobiledoas::GpsData& gpsData, int& hours, int& minutes, int& seconds)
    {
        hours = gpsData.time / 10000;
        minutes = (gpsData.time - hours * 10000) / 100;
        seconds = gpsData.time % 100;
    }

    void ExtractDate(const mobiledoas::GpsData& gpsData, int& day, int& month, int& year)
    {
        day = gpsData.date / 10000;
        month = (gpsData.date - day * 10000) / 100;
        year = gpsData.date % 100;

        assert(day >= 1 && day <= 31);
        assert(month >= 1 && month <= 12);
    }

    void ExtractDateAndTime(const mobiledoas::GpsData& gpsData, novac::CDateTime& time)
    {
        time.day = static_cast<unsigned char>(gpsData.date / 10000);
        time.month = static_cast<unsigned char>((gpsData.date - time.day * 10000) / 100);
        time.year = gpsData.date % 100;

        time.hour = static_cast<unsigned char>(gpsData.time / 10000);
        time.minute = static_cast<unsigned char>((gpsData.time - time.hour * 10000) / 100);
        time.second = gpsData.time % 100;
        time.millisecond = 0;
    }

    std::string GetDate(const mobiledoas::GpsData& data)
    {
        char buffer[7];
        sprintf_s(buffer, "%06d", data.date);
        return std::string(buffer);
    }

    std::string GetDate(const mobiledoas::GpsData& data, char separatorCharacter)
    {
        int day, month, year;
        ExtractDate(data, day, month, year);

        char buffer[64];
        sprintf_s(buffer, "%02d%c%02d%c%02d", day, separatorCharacter, month, separatorCharacter, year);
        return std::string(buffer);
    }


    long GetTime(const mobiledoas::GpsData& data)
    {
        return data.time;
    }

    void GetHrMinSec(int time, int& hr, int& min, int& sec)
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
        sprintf_s(msgBuffer, "%02d%c%02d%c%02d", hr, separatorCharacter, min, separatorCharacter, sec);

        return std::string(msgBuffer);
    }


    void GetDateText(char* txt)
    {
        struct tm* tim;
        time_t t;

        txt[0] = 0;
        time(&t);
        tim = localtime(&t);
        sprintf(txt, "%04d.%02d.%02d", tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday);
    }

    void GetDateTimeText(char* txt)
    {
        struct tm* tim;
        time_t t;

        txt[0] = 0;
        time(&t);
        tim = localtime(&t);
        sprintf(txt, "%04d.%02d.%02d  %02d:%02d:%02d", tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday, tim->tm_hour, tim->tm_min, tim->tm_sec);
    }

    std::string GetDateTimeTextPlainFormat()
    {
        struct tm* tim;
        time_t t;

        char txt[64];
        txt[0] = 0;
        time(&t);
        tim = localtime(&t);
        sprintf(txt, "%04d.%02d.%02d_%02d%02d%02d", tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday, tim->tm_hour, tim->tm_min, tim->tm_sec);

        return std::string(txt);
    }

}