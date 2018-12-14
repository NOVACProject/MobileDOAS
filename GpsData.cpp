#include "StdAfx.h"
#include "GpsData.h"


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
	this->nSatellites = other.nSatellites;
	this->date = other.date;
}

gpsData& gpsData::operator=(gpsData other)
{
	swap(*this, other);
	return *this;
}

void swap(gpsData & first, gpsData & second)
{
	using std::swap;
	swap(first.latitude, second.latitude);
	swap(first.longitude, second.longitude);
	swap(first.altitude, second.altitude);
	swap(first.time, second.time);
	swap(first.nSatellites, second.nSatellites);
	swap(first.date, second.date);
}

void ExtractTime(const gpsData& gpsData, int& hours, int& minutes, int& seconds)
{
	hours = gpsData.time / 10000;
	minutes = (gpsData.time - hours * 10000) / 100;
	seconds = gpsData.time % 100;
}

std::string GetDate(const gpsData& data)
{
	return std::to_string(data.date);
}

long GetTime(const gpsData& data)
{
	return data.time;
}

bool IsValidGpsData(const gpsData& data)
{
	if (data.nSatellites == 0)
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

/** Parse the read GPS-Information */
/** See http://www.gpsinformation.org/dale/nmea.htm */
bool Parse(char *gpsString, gpsData& data)
{
	const char sep[] = ",";   /* the separator */
	char* stopStr = "\0";

	char *token = strtok(gpsString, sep);  /* get first sentence identifier */

	int numberOfParsedSentences = 0;

	if (token == nullptr)
	{
		return false;
	}

	while (token != nullptr)
	{
		if (0 == strncmp(token, "$GPRMC", 6))
		{
			/* 1: the time */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			else {
				data.time = strtol(token, &stopStr, 10);
			}

			/* 2: the fix status */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			else {
				if (0 == strncmp(token, "A", 1)) {
					data.nSatellites = 3; /* we can see at least three satellites */
				}
				else {
					data.nSatellites = -1; /* void */
				}
			}

			/* 3: the latitude */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			else {
				double lat = ConvertToDecimalDegrees(strtod(token, &stopStr));
				if (lat >= -90.0 && lat <= 90.0) {
					data.latitude = lat;
				}
				else {
					return false;
				}
			}

			/* 4: north/south hemisphere */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			else {
				if (0 == strncmp(token, "S", 1)) {
					data.latitude = -data.latitude;
				}
				else if (0 == strncmp(token, "N", 1)) {
					// this is ok too
				}
				else {
					return false; // some issue here
				}
			}

			/* 5: the longitude  */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			else {
				double lon = ConvertToDecimalDegrees(strtod(token, &stopStr));
				if (lon >= -180.0 && lon <= 180.0) {
					data.longitude = lon;
				}
				else {
					return false;
				}
			}

			/* 6: east/west hemisphere */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			else {
				if (0 == strncmp(token, "W", 1)) {
					data.longitude = -data.longitude;
				}
				else if (0 == strncmp(token, "E", 1)) {
					// this is ok too
				}
				else {
					return false; // some issue here
				}
			}

			/* 7: the speed [knots] (ignore) */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			// else {
			// 	double speed = strtod(token, &stopStr); // not used
			// }

			/* 8: bearing [degrees] (ignore) */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			// else {
			// 	double bearing = strtod(token, &stopStr); // not used
			// }

			/* 9: date (mmddyy) */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			else {
				data.date = strtol(token, &stopStr, 10);
			}

			/* 10: magnetic variation(ignore) */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			// else {
			// 	double mv = strtod(token, &stopStr); // not used
			// }

			if (nullptr == (token = strtok(nullptr, "*"))) {
				return false;
			}
			// else {
			// 	char* mvd = token; // not used
			// }

			/* 11:checksum          (ignore) */

			++numberOfParsedSentences;
		}

		if (0 == strncmp(token, "$GPGGA", 6))
		{
			/* 1: the time */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			else {
				data.time = strtol(token, &stopStr, 10);
			}

			/* 2: the latitude */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			else {
				double lat = ConvertToDecimalDegrees(strtod(token, &stopStr));
				if (lat >= -90.0 && lat <= 90.0) {
					data.latitude = lat;
				}
				else {
					return false;
				}
			}

			/* 3: north/south hemisphere */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			else {
				if (0 == strncmp(token, "S", 1)) {
					data.latitude = -data.latitude;
				}
				else if (0 == strncmp(token, "N", 1)) {
					// this is ok too
				}
				else {
					return false; // some issue here
				}
			}

			/* 4: longitude */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			else {
				double lon = ConvertToDecimalDegrees(strtod(token, &stopStr));
				if (lon >= -180.0 && lon <= 180.0) {
					data.longitude = lon;
				}
				else {
					return false;
				}
			}

			/* 5: east/west hemisphere */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			else {
				if (0 == strncmp(token, "W", 1)) {
					data.longitude = -data.longitude;
				}
				else if (0 == strncmp(token, "E", 1)) {
					// this is ok too
				}
				else {
					return false; // some issue here
				}
			}

			/* 6: quality of fix (ignore) */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			// else {
			// 	int quality = strtol(token, &stopStr, 10);
			// }

			/* 7: number of satellites being used */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			else {
				data.nSatellites = strtol(token, &stopStr, 10);
			}

			/* 8: "horizontal dillution of precision" (ignore) */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			// else {
			// 	double hd = strtod(token, &stopStr);
			// }

			/* 9: Altitude */
			if (nullptr == (token = strtok(nullptr, sep))) {
				return false;
			}
			else {
				data.altitude = strtod(token, &stopStr);
			}

			// the remainder of stuff
			/*10: geoidal separation in meters (ignore) */
			/*11: age of the deferrential correction data (ignore) */
			/*12: deferential station's ID (ignore) */
			/*13: checksum for the sentence (ignore) */
			++numberOfParsedSentences;
		}

		token = strtok(nullptr, "\n"); // go to end of line
		if (token != nullptr) {
			token = strtok(nullptr, sep); // get next sentence identifier
		}
	}

	return (numberOfParsedSentences > 0);
}

double ConvertToDecimalDegrees(double rawData)
{
	const double minutes = fmod(rawData, 100.0);
	double integerDegrees = (int)(rawData / 100);

	return integerDegrees + minutes / 60.0;
}
