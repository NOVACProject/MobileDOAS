#pragma once

#include <string>

// --------------- This file collects data structures and routines related to GPS reading --------------- 

/** Common structure for stroring data read out from the GPS */
struct gpsData {
	gpsData();
	gpsData(const gpsData& other);

	gpsData& operator=(gpsData other);

	friend void swap(gpsData& first, gpsData& second);

	/* Latitude in (decimal) degrees. */
	double latitude = 0.0;

	/* Longitude in (decimal) degrees. */
	double longitude = 0.0;

	/* Altitude above sea level in meters. */
	double altitude = 0.0;

	/* The time stamp from the Gps */
	long time = 0;

	/* Number of satellites seen by the receiver. */
	long nSatellites = 0;

	/* Date */
	int date = 0;
};

/** Extracts the time from the provided gpsData and separates it into hour-minute-second */
void ExtractTime(const gpsData& gpsData, int& hours, int& minutes, int& seconds);

/** Reads out the data in the provided gpsData and formats it in the format 'mmddyy' */
std::string GetDate(const gpsData& data);

/** Reads out the timestamp in the provided gpsData */
long GetTime(const gpsData& data);

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

