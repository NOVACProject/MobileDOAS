#pragma once

#include <string>

// --------------- This file collects data structures and routines related to GPS reading --------------- 

namespace mobiledoas
{

    enum class GpsFixQuality
    {
        INVALID = 0,
        GPS_FIXED = 1,
        DGPS_FIXED = 2,
        PPS_FIX = 3,
        RTK_FIX = 4,
        FRTK_FIX = 5,
        MANUAL = 7,
        SIMULATED = 8
    };

    /** GpsData is a basic structure for storing data read out from the GPS */
    struct GpsData {
        GpsData();
        GpsData(const GpsData& other);

        GpsData& operator=(GpsData other);

        friend void swap(GpsData& first, GpsData& second);

        /* Latitude in (decimal) degrees. Positive values corresponds to northern hemisphere. */
        double latitude = 0.0;

        /* Longitude in (decimal) degrees. Positive values corresponds to eastern hemisphere. */
        double longitude = 0.0;

        /* Altitude above sea level in meters. */
        double altitude = 0.0;

        /* The time stamp from the Gps (in the format hhmmss) */
        long time = 0;

        /* Number of satellites seen by the receiver (not all of these needs to be used) */
        long nSatellitesSeen = 0;

        /* Number of satellites tracked by the receiver */
        long nSatellitesTracked = 0;

        /* Date (in the format ddmmyy) */
        int date = 0;

        /* GPS status. A=active, V=void, NA=not available (i.e. not connected) */
        std::string status = "NA";

        /* Speed over ground in m/s */
        double speed = 0.0;

        /* Track angle in degrees */
        double course = 0.0;

        /* The quality of the GPS-fix */
        GpsFixQuality fixQuality = GpsFixQuality::INVALID;
    };

    /** @return true if the provided GpsData contains a valid GPS readout
        This checks that any satelite was seen and that the lat/long aren't zero. */
    bool IsValidGpsData(const GpsData& data);

    /* Tries to parse the text read from the GPS.
        The parsed information will be filled into the provided 'data, remaining fields will remain as is.
        @return true if the parsing suceeded, otherwise false. */
    bool Parse(const char* gpsString, GpsData& data);

    /* GPSDistance returns the distance in meters between the two points defined
          by (lat1,lon1) and (lat2, lon2). All angles must be in degrees */
    double GPSDistance(double lat1, double lon1, double lat2, double lon2);

    /* GPSBearing returns the initial bearing (degrees) when travelling from
          the point defined by (lat1, lon1) to the point (lat2, lon2).
          All angles must be in degrees */
    double GPSBearing(double lat1, double lon1, double lat2, double lon2);

    /** CalculateDestination calculates the latitude and longitude for point
            which is the distance 'dist' (in meters) and bearing 'az' degrees from
            the point defied by 'lat1' and 'lon1' */
    void CalculateDestination(double lat1, double lon1, double dist, double az, double& lat2, double& lon2);

}
