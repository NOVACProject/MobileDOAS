#include <MobileDoasLib/GpsData.h>
#include <MobileDoasLib/Definitions.h>
#include <cmath>

//////////////////////////////////////////////////////////////////////
// GpsData
//////////////////////////////////////////////////////////////////////


namespace mobiledoas
{
    GpsData::GpsData() {}

    GpsData::GpsData(const GpsData& other)
        : latitude(other.latitude),
        longitude(other.longitude),
        altitude(other.altitude),
        time(other.time),
        date(other.date),
        nSatellitesTracked(other.nSatellitesTracked),
        nSatellitesSeen(other.nSatellitesSeen),
        fixQuality(other.fixQuality),
        speed(other.speed),
        course(other.course),
        status(other.status)
    {
    }

    GpsData& GpsData::operator=(GpsData other)
    {
        swap(*this, other);
        return *this;
    }

    void swap(GpsData& first, GpsData& second)
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

    bool IsValidGpsData(const GpsData& data)
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

    void SetTime(const std::string& curToken, GpsData& data)
    {
        if (curToken.length() < 6)
        {
            return; // invalid time format.
        }

        // the time should according to spec be an integer, but has been found to sometimes be a double value
        const double time = std::atof(curToken.c_str());
        if (time > 0.0)
        {
            data.time = (long)time;
        }
    }

    void SetDate(const std::string& curToken, GpsData& data)
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

    double ConvertToDecimalDegrees(double rawData)
    {
        const double minutes = fmod(rawData, 100.0);
        double integerDegrees = (int)(rawData / 100);

        return integerDegrees + minutes / 60.0;
    }

    void SetLatitude(const std::string& curToken, GpsData& data)
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

    void SetLongitude(const std::string& curToken, GpsData& data)
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

    void SetHemisphere(const std::string& curToken, GpsData& data)
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

    void SetNumberOfSatellitesTracked(const std::string& curToken, GpsData& data)
    {
        if (curToken.size() < 1 || curToken.size() > 2)
        {
            return; // not enough information
        }

        const long n = std::atol(curToken.c_str());

        data.nSatellitesTracked = n;
    }

    void SetNumberOfSatellitesSeen(const std::string& curToken, GpsData& data)
    {
        if (curToken.size() < 1 || curToken.size() > 2)
        {
            return; // not enough information
        }

        const long n = std::atol(curToken.c_str());

        data.nSatellitesSeen = n;
    }

    void SetFixQuality(const std::string& curToken, GpsData& data)
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


    void SetAltitude(const std::string& curToken, GpsData& data)
    {
        if (curToken.size() < 1)
        {
            return; // not enough information
        }

        const double altitude = std::atof(curToken.c_str());

        data.altitude = altitude;
    }

    void SetAltitudeUnit(const std::string& curToken, GpsData& /*data*/)
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
            // unknown case...
            // TODO: Error handling here
        }
    }

    void SetStatus(const std::string& curToken, GpsData& data) {
        if (curToken.length() == 1)
        {
            data.status = curToken;
        }
        else {
            return; // invalid status format.
        }

    }


    void SetSpeed(const std::string& curToken, GpsData& data) {
        if (curToken.length() == 5)
        {
            data.speed = std::atof(curToken.c_str()); // get speed in knots
            data.course /= 1.94384; // convert to m/s
        }
        else {
            return; // invalid speed format.
        }
    }

    void SetCourse(const std::string& curToken, GpsData& data) {
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


    // Parses the RMC string (starting with $GPRMC) and fills in the provided GpsData structure
    //	@return true if the parsing is successful
    bool ParseRMC(const char* gpsString, GpsData& data)
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

    // Parses the GGA string (starting with $GPGGA) and fills in the provided GpsData structure
    //	@return true if the parsing is successful
    bool ParseGGA(const char* gpsString, GpsData& data)
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

    // Parses the GSV string (starting with $GPGSV) and fills in the provided GpsData structure
    //	@return true if the parsing is successful
    bool ParseGSV(const char* gpsString, GpsData& data)
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
    bool Parse(const char* gpsString, GpsData& data)
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

    double GPSDistance(double lat1, double lon1, double lat2, double lon2) {
        const double R_Earth = 6367000;
        double distance, dlon, dlat, a, c;
        lat1 = lat1 * DEGREETORAD;
        lat2 = lat2 * DEGREETORAD;
        lon1 = lon1 * DEGREETORAD;
        lon2 = lon2 * DEGREETORAD;
        dlon = lon2 - lon1;
        dlat = lat2 - lat1;
        a = pow((sin(dlat / 2)), 2) + cos(lat1) * cos(lat2) * pow((sin(dlon / 2)), 2);
        c = 2 * std::asin(std::min(1.0, sqrt(a)));
        distance = R_Earth * c;

        return distance;
    }

    /** Calculate the bearing from point 1 to point 2.
      Bearing is here defined as the angle between the direction to point 2 (at point 1)
        and the direction to north (at point 1).
    *@lat1 - the latitude of beginning point,   [degree]
    *@lon1 - the longitude of beginning point,  [degree]
    *@lat2 - the latitude of ending point,      [degree]
    *@lon2 - the longitude of ending point,     [degree]
    */
    double GPSBearing(double lat1, double lon1, double lat2, double lon2) {
        double angle, dLat, dLon;

        lat1 = lat1 * DEGREETORAD;
        lat2 = lat2 * DEGREETORAD;
        lon1 = lon1 * DEGREETORAD;
        lon2 = lon2 * DEGREETORAD;

        dLat = lat1 - lat2;
        dLon = lon1 - lon2;

        if ((dLon == 0) && (dLat == 0))
            angle = 0;
        else
            angle = atan2(-sin(dLon) * cos(lat2),
                cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon));

        /*  	angle = atan2(lon1*cos(lat1)-lon2*cos(lat2), lat1-lat2); */

        if (angle < 0)
            angle = TWO_PI + angle;

        angle = RADTODEGREE * angle;
        return angle;
    }

    /** This function calculates the latitude and longitude for a point
            which is the distance 'dist' m and bearing 'az' degrees from
            the point defied by 'lat1' and 'lon1' */
    void CalculateDestination(double lat1, double lon1, double dist, double az, double& lat2, double& lon2) {
        const double R_Earth = 6367000; // radius of the earth

        double dR = dist / R_Earth;

        // convert to radians
        lat1 = lat1 * DEGREETORAD;
        lon1 = lon1 * DEGREETORAD;
        az = az * DEGREETORAD;

        // calculate the second point
        lat2 = asin(sin(lat1) * cos(dR) + cos(lat1) * sin(dR) * cos(az));

        lon2 = lon1 + atan2(sin(az) * sin(dR) * cos(lat1), cos(dR) - sin(lat1) * sin(lat2));

        // convert back to degrees
        lat2 = lat2 * RADTODEGREE;
        lon2 = lon2 * RADTODEGREE;
    }
}
