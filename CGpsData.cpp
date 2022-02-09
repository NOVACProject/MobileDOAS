#include "StdAfx.h"
#include "CGpsData.h"
#include <cmath>

//////////////////////////////////////////////////////////////////////
// gpsData
//////////////////////////////////////////////////////////////////////

gpsData::gpsData() {}

gpsData::gpsData(const gpsData& other)
{
    this->latitude = other.latitude;
    this->longitude = other.longitude;
    this->altitude = other.altitude;
    this->time = other.time;
    this->nSatellitesTracked = other.nSatellitesTracked;
    this->nSatellitesSeen = other.nSatellitesSeen;
    this->date = other.date;
    this->fixQuality = other.fixQuality;
    this->status = other.status;
    this->speed = other.speed;
    this->course = other.course;
}

gpsData& gpsData::operator=(gpsData other)
{
    swap(*this, other);
    return *this;
}

void swap(gpsData& first, gpsData& second)
{
    using std::swap;
    swap(first.latitude, second.latitude);
    swap(first.longitude, second.longitude);
    swap(first.altitude, second.altitude);
    swap(first.time, second.time);
    swap(first.nSatellitesTracked, second.nSatellitesTracked);
    swap(first.nSatellitesSeen, second.nSatellitesSeen);
    swap(first.date, second.date);
    swap(first.fixQuality, second.fixQuality);
    swap(first.status, second.status);
    swap(first.speed, second.speed);
    swap(first.course, second.course);
}

bool IsValidGpsData(const gpsData& data)
{
    if (data.nSatellitesTracked == 0 || data.fixQuality == GpsFixQuality::INVALID)
    {
        return false;
    }
    else if (std::abs(data.latitude) < std::numeric_limits<double>::epsilon() && std::abs(data.longitude) < std::numeric_limits<double>::epsilon())
    {
        // Invalid data, no latitude / longitude could be retrieved
        return false;
    }
    else
    {
        // all seems to be ok
        return true;
    }
}

unsigned char CalculateMessageChecksum(const std::string& message)
{
    if (message.size() == 0)
    {
        return 0; // invalid
    }

    unsigned char checksum = 0;

    for (const char e : message)
    {
        checksum = checksum ^ e;
    }

    return checksum;
}

bool ChecksumIsCorrect(const std::string& message)
{
    auto startOfChecksum = message.find("*");
    if (std::string::npos == startOfChecksum)
    {
        return false;
    }

    const unsigned char expectedChecksum = CalculateMessageChecksum(message.substr(1, startOfChecksum - 1));
    unsigned int actualChecksum = 0;
    if (!sscanf_s(message.c_str() + startOfChecksum + 1, "%x", &actualChecksum))
    {
        return false;
    }

    return (actualChecksum == expectedChecksum);
}

void SetTime(const std::string& curToken, gpsData& data)
{
    if (curToken.length() < 6)
    {
        return; // invalid time format.
    }

    // the time should according to spec be an integer, but has been found to sometimes be a double value
    double time = std::atof(curToken.c_str());
    if (time > 0.0)
    {
        data.time = (long)time;
    }
}
void SetDate(const std::string& curToken, gpsData& data)
{
    if (curToken.length() != 6)
    {
        return; // invalid date format.
    }

    long date = std::atol(curToken.c_str());
    if (0 != date)
    {
        data.date = date;
    }
}

void SetLatitude(const std::string& curToken, gpsData& data)
{
    if (curToken.length() < 3)
    {
        return; // invalid format.
    }

    const double latInDegreesAndMinutes = std::atof(curToken.c_str());
    if (std::abs(latInDegreesAndMinutes) < std::numeric_limits<double>::epsilon() || !std::isfinite(latInDegreesAndMinutes))
    {
        return;
    }

    const double lat = ConvertToDecimalDegrees(latInDegreesAndMinutes);
    if (std::abs(lat) <= 90.0)
    {
        data.latitude = lat;
    }
}

void SetLongitude(const std::string& curToken, gpsData& data)
{
    if (curToken.length() < 3)
    {
        return; // invalid format.
    }

    const double lonInDegreesAndMinutes = std::atof(curToken.c_str());
    if (std::abs(lonInDegreesAndMinutes) < std::numeric_limits<double>::epsilon() || !std::isfinite(lonInDegreesAndMinutes))
    {
        return;
    }

    const double lon = ConvertToDecimalDegrees(lonInDegreesAndMinutes);
    if (std::abs(lon) <= 180.0)
    {
        data.longitude = lon;
    }
}

void SetHemisphere(const std::string& curToken, gpsData& data)
{
    if (curToken.size() != 1)
    {
        return; // not enough information
    }

    switch (curToken.at(0))
    {
    case 'N':
        data.latitude = std::abs(data.latitude);
        return;
    case 'S':
        data.latitude = -std::abs(data.latitude);
        return;
    case 'E':
        data.longitude = std::abs(data.longitude);
        return;
    case 'W':
        data.longitude = -std::abs(data.longitude);
        return;
    default:
        return;
    }
}

void SetNumberOfSatellitesTracked(const std::string& curToken, gpsData& data)
{
    if (curToken.size() < 1 || curToken.size() > 2)
    {
        return; // not enough information
    }

    const long n = std::atol(curToken.c_str());

    data.nSatellitesTracked = n;
}

void SetNumberOfSatellitesSeen(const std::string& curToken, gpsData& data)
{
    if (curToken.size() < 1 || curToken.size() > 2)
    {
        return; // not enough information
    }

    const long n = std::atol(curToken.c_str());

    data.nSatellitesSeen = n;
}

void SetFixQuality(const std::string& curToken, gpsData& data)
{
    if (curToken.size() != 1 || curToken.at(0) < '0' || curToken.at(0) > '9')
    {
        return; // invalid data
    }

    const long n = std::atol(curToken.c_str());

    if (n >= 0 && n < 9)
    {
        data.fixQuality = (GpsFixQuality)n;
    }
}


void SetAltitude(const std::string& curToken, gpsData& data)
{
    if (curToken.size() < 1)
    {
        return; // not enough information
    }

    const double altitude = std::atof(curToken.c_str());

    data.altitude = altitude;
}

void SetAltitudeUnit(const std::string& curToken, gpsData& data)
{
    if (curToken.size() != 1)
    {
        return; // not enough information
    }

    if (curToken.at(0) == 'M')
    {
        // unit is meters, all is ok.
        return;
    }
    else
    {
        // unknown case...s
    }

}

void SetStatus(const std::string& curToken, gpsData& data) {
    if (curToken.length() == 1)
    {
        data.status = curToken;
    }
    else {
        return; // invalid status format.
    }

}


void SetSpeed(const std::string& curToken, gpsData& data) {
    if (curToken.length() == 5)
    {
        data.speed = std::atof(curToken.c_str()); // get speed in knots
        data.course /= 1.94384; // convert to m/s
    }
    else {
        return; // invalid speed format.
    }
}

void SetCourse(const std::string& curToken, gpsData& data) {
    if (curToken.length() == 5)
    {
        data.course = std::atof(curToken.c_str());
    }
    else {
        return; // invalid course format.
    }
}

bool ExtractMessage(const char* gpsString, const char* messageType, std::string& result)
{
    const char* pt = strstr(gpsString, messageType);
    if (nullptr == pt)
    {
        return false;
    }

    // The message starts with $GPRMC and ends with a carriage return / line feed
    const char* endOfMessage = strstr(pt, "\r");
    if (nullptr == endOfMessage)
    {
        endOfMessage = strstr(pt, "\n");
    }
    if (nullptr == endOfMessage)
    {
        return false;
    }

    // Convert to a C++ string
    result = std::string(pt, size_t(endOfMessage - pt));

    return true;
}


// Parses the RMC string (starting with $GPRMC) and fills in the provided gpsData structure
//	@return true if the parsing is successful
bool ParseRMC(char* gpsString, gpsData& data)
{
    const std::string delimiter = ",";

    // Convert to a C++ string
    std::string message;
    if (!ExtractMessage(gpsString, "$GPRMC", message))
    {
        return false;
    }

    if (!ChecksumIsCorrect(message))
    {
        return false;
    }

    // Parse the elements in the string...
    size_t pos = 0;
    int fieldIdx = 0;
    while ((pos = message.find(delimiter)) != std::string::npos)
    {
        std::string curToken = message.substr(0, pos);

        switch (fieldIdx)
        {
        case 1:
            SetTime(curToken, data); break;
        case 2:
            SetStatus(curToken, data); break;
        case 3:
            SetLatitude(curToken, data); break;
        case 4:
            SetHemisphere(curToken, data); break;
        case 5:
            SetLongitude(curToken, data); break;
        case 6:
            SetHemisphere(curToken, data); break;
        case 7:
            SetSpeed(curToken, data); break;
        case 8:
            SetCourse(curToken, data); break;
        case 9:
            SetDate(curToken, data); break;
        case 10:
            /* magnetic variation */ break;
        }

        ++fieldIdx;
        message.erase(0, pos + delimiter.length());
    }

    return true;
}

// Parses the GGA string (starting with $GPGGA) and fills in the provided gpsData structure
//	@return true if the parsing is successful
bool ParseGGA(char* gpsString, gpsData& data)
{
    const std::string delimiter = ",";

    // Convert to a C++ string
    std::string message;
    if (!ExtractMessage(gpsString, "$GPGGA", message))
    {
        return false;
    }

    if (!ChecksumIsCorrect(message))
    {
        return false;
    }

    // Parse the elements in the string...
    size_t pos = 0;
    int fieldIdx = 0;
    while ((pos = message.find(delimiter)) != std::string::npos)
    {
        std::string curToken = message.substr(0, pos);

        switch (fieldIdx)
        {
        case 1:
            SetTime(curToken, data); break;
        case 2:
            SetLatitude(curToken, data); break;
        case 3:
            SetHemisphere(curToken, data); break;
        case 4:
            SetLongitude(curToken, data); break;
        case 5:
            SetHemisphere(curToken, data); break;
        case 6:
            SetFixQuality(curToken, data); break;
        case 7:
            SetNumberOfSatellitesTracked(curToken, data); break;
        case 8:
            /* horizontal dillution of precision */; break;
        case 9:
            SetAltitude(curToken, data); break;
        case 10:
            SetAltitudeUnit(curToken, data); break;
        case 11:
            /* Height of geoid, above WGS84 */; break;
        case 12:
            /* Unit of Height of geoid, above WGS84 */; break;
        case 13:
            /* Time since last DGPS update */; break;
        case 14:
            /* DGPS station ID. */; break;
        }

        ++fieldIdx;
        message.erase(0, pos + delimiter.length());
    }

    return true;
}

// Parses the GSV string (starting with $GPGSV) and fills in the provided gpsData structure
//	@return true if the parsing is successful
bool ParseGSV(char* gpsString, gpsData& data)
{
    const std::string delimiter = ",";

    // Convert to a C++ string
    std::string message;
    if (!ExtractMessage(gpsString, "$GPGSV", message))
    {
        return false;
    }

    if (!ChecksumIsCorrect(message))
    {
        return false;
    }

    // Parse the elements in the string...
    size_t pos = 0;
    int fieldIdx = 0;
    while ((pos = message.find(delimiter)) != std::string::npos)
    {
        std::string curToken = message.substr(0, pos);

        switch (fieldIdx)
        {
        case 1:
            /* Number of sentences sent for full GSV data */
            break;
        case 2:
            /* Sentence identifier. */
            break;
        case 3:
            SetNumberOfSatellitesSeen(curToken, data);
            break;
        default:
            /* The remainder of this sentence is not parsed. */
            break;
        }

        ++fieldIdx;
        message.erase(0, pos + delimiter.length());
    }

    return true;
}

/** Parse the read GPS-Information */
/** See http://www.gpsinformation.org/dale/nmea.htm */
bool Parse(char* gpsString, gpsData& data)
{
    if (nullptr == gpsString || strlen(gpsString) < 40)
    {
        // TODO: Determine the minimum string length here.
        return false;
    }

    int numberOfParsedSentences = 0;

    // Find parseable GPS messages
    if (ParseRMC(gpsString, data))
    {
        ++numberOfParsedSentences;
    }

    if (ParseGGA(gpsString, data))
    {
        ++numberOfParsedSentences;
    }

    if (ParseGSV(gpsString, data))
    {
        ++numberOfParsedSentences;
    }

    return (numberOfParsedSentences > 0);
}

double ConvertToDecimalDegrees(double rawData)
{
    const double minutes = fmod(rawData, 100.0);
    double integerDegrees = (int)(rawData / 100);

    return integerDegrees + minutes / 60.0;
}
