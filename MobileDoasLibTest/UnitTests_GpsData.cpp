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
