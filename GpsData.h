#pragma once

#include <string>

// --------------- This file collects data structures and routines related to GPS reading --------------- 

enum GpsFixQuality
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

/** Common structure for stroring data read out from the GPS */
struct gpsData {
	gpsData();
	gpsData(const gpsData& other);

	gpsData& operator=(gpsData other);

	friend void swap(gpsData& first, gpsData& second);

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
	std::string status;

	/* Speed over ground in m/s */
	double speed;

	/* Track angle in degrees */
	double course;

	/* The quality of the GPS-fix */
	GpsFixQuality fixQuality = GpsFixQuality::INVALID;
};

/** @return true if the provided gpsData contains a valid GPS readout
	This checks that any satelite was seen and that the lat/long aren't zero. */
bool IsValidGpsData(const gpsData& data);

/* Tries to parse the text read from the GPS.
	The parsed information will be filled into the provided 'data.
	@return true if the parsing suceeded, otherwise false. */
bool Parse(char* gpsString, gpsData& data);

/* Convert an angle from the raw format of the GPS data DDMM.MMMMM
into the format DD.DDDDDDD */
double ConvertToDecimalDegrees(double degreesAndMinutes);

