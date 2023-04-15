#include "catch.hpp"
#include <MobileDoasLib/GpsData.h>

using namespace mobiledoas;

TEST_CASE("GpsData - Copy constructor copies all data", "[GpsData]")
{
    // Setup an original with some rather unusual values.
    GpsData original;
    original.latitude = 3.141592;
    original.longitude = -24.231;
    original.altitude = 282.123;
    original.time = 231232;
    original.nSatellitesSeen = 88;
    original.nSatellitesTracked = 9878;
    original.date = 9823;
    original.speed = .9772;
    original.course = -28.132;
    original.fixQuality = GpsFixQuality::RTK_FIX;

    // Act
    GpsData copy = original;

    // Assert
    REQUIRE(original.latitude == Approx(copy.latitude));
    REQUIRE(original.longitude == Approx(copy.longitude));
    REQUIRE(original.altitude == Approx(copy.altitude));
    REQUIRE(original.time == copy.time);
    REQUIRE(original.nSatellitesSeen == copy.nSatellitesSeen);
    REQUIRE(original.nSatellitesTracked == copy.nSatellitesTracked);
    REQUIRE(original.date == copy.date);
    REQUIRE(original.status == copy.status);
    REQUIRE(original.speed == Approx(copy.speed));
    REQUIRE(original.course == Approx(copy.course));
    REQUIRE(original.fixQuality == copy.fixQuality);
}

TEST_CASE("GpsData - Copy assignment operator copies all data", "[GpsData]")
{
    // Setup an original with some rather unusual values.
    GpsData original;
    original.latitude = 3.141592;
    original.longitude = -24.231;
    original.altitude = 282.123;
    original.time = 231232;
    original.nSatellitesSeen = 88;
    original.nSatellitesTracked = 9878;
    original.date = 9823;
    original.speed = .9772;
    original.course = -28.132;
    original.fixQuality = GpsFixQuality::RTK_FIX;
    GpsData copy;

    // Act
    copy = original;

    // Assert
    REQUIRE(original.latitude == Approx(copy.latitude));
    REQUIRE(original.longitude == Approx(copy.longitude));
    REQUIRE(original.altitude == Approx(copy.altitude));
    REQUIRE(original.time == copy.time);
    REQUIRE(original.nSatellitesSeen == copy.nSatellitesSeen);
    REQUIRE(original.nSatellitesTracked == copy.nSatellitesTracked);
    REQUIRE(original.date == copy.date);
    REQUIRE(original.status == copy.status);
    REQUIRE(original.speed == Approx(copy.speed));
    REQUIRE(original.course == Approx(copy.course));
    REQUIRE(original.fixQuality == copy.fixQuality);
}

TEST_CASE("Parse returns expected result", "[GpsData]")
{
    SECTION("GPRMC")
    {
        const char* data = "$GPRMC,061924.006,A,5741.9759,N,01154.5998,E,0.08,89.51,140423,,,A*54\r\n";
        GpsData result;

        // Act
        const bool success = Parse(data, result);

        // Assert
        REQUIRE(true == success);
        REQUIRE(61924 == result.time); // i.e. 06:19:24
        REQUIRE(140423 == result.date); // i.e. 2023-04-14
        REQUIRE(57.699598== Approx(result.latitude));
        REQUIRE(11.909997 == Approx(result.longitude));
    }

    SECTION("$GPGGA")
    {
        const char* data = "$GPGGA,061925.006,5741.9759,N,01154.5999,E,1,10,0.7,12.9,M,40.2,M,,0000*61\r\n";
        GpsData result;

        // Act
        const bool success = Parse(data, result);

        // Assert
        REQUIRE(true == success);
        REQUIRE(61925 == result.time); // i.e. 06:19:24
        REQUIRE(57.699598 == Approx(result.latitude));
        REQUIRE(11.909997 == Approx(result.longitude));
        REQUIRE(GpsFixQuality::GPS_FIXED == result.fixQuality);
        REQUIRE(10 == result.nSatellitesTracked);
        REQUIRE(12.9 == Approx(result.altitude));
    }

    SECTION("$GPGSV")
    {
        const char* data = "$GPGSV,3,1,12,24,79,232,21,19,45,102,30,17,38,062,36,15,25,186,41*7C\r\n";
        GpsData result;

        // Act
        const bool success = Parse(data, result);

        // Assert
        REQUIRE(true == success);
        REQUIRE(12 == result.nSatellitesSeen);
    }
}
