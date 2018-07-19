// GPS.cpp: implementation of the CGPS class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math.h>
#include <time.h>

#include "GPS.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGPS::CGPS()
	: gpsInfo() {
	//m_gotContact = false;

	m_gpsThread = nullptr;
	fRun = false;
}

CGPS::CGPS(char* pCOMPort, long pBaudrate)
	: CGPS() {

	serial.baudrate = pBaudrate;
	strcpy(serial.serialPort, pCOMPort);

	if (!Connect()) {
		MessageBox(nullptr, "Could not communicate with GPS. No GPS-data can be retrieved!", "Error", MB_OK | MB_SYSTEMMODAL);
	}
}

CGPS::~CGPS(){

	m_gpsThread = nullptr;
	//serial.Close();
}

bool CGPS::Connect() {
	if (!serial.Init(this->serial.baudrate)) {
		return false;
	}
	return true;
}

/**Get position information - latitude and longitude
*
*/
void CGPS::Get(gpsData& dst) const
{
	dst = this->gpsInfo;
}

/** Parse the read GPS-Information */
/** See http://www.gpsinformation.org/dale/nmea.htm/ */
bool CGPS::Parse(char *string, gpsData& data){

	const char sep[]    = ",";   /* the separator */
	char* stopStr       = "\0";
	
	char *token = strtok(string, sep);  /* get first sentence identifier */

	if (token == nullptr) {
		return false;
	}

	while (token != nullptr) {
		
		if (0 == strncmp(token, "$GPRMC", 6)) {	// fisrt sentence should be GPRMC

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
		}

		if (0 == strncmp(token, "$GPGGA", 6)) {	// second sentence should be GPGGA
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
		}

		token = strtok(nullptr, "\n"); // go to end of line
		if (token != nullptr) {
			token = strtok(nullptr, sep); // get next sentence identifier
		}
	}

	return true;
}

double CGPS::ConvertToDecimalDegrees(double rawData) {
	const double minutes  = fmod(rawData, 100.0);
	double integerDegrees = (int)(rawData/100);
	
	return integerDegrees + minutes / 60.0;
}

void CGPS::Run(){
	m_gpsThread = AfxBeginThread(CollectGPSData, (LPVOID)this, THREAD_PRIORITY_NORMAL,0,0,nullptr);
}

void CGPS::Stop(){
	this->fRun = false;
}

bool CGPS::ReadGPS(){
	char gpstxt[256];

	gpstxt[0] = 0;

	do{
		long cnt = 0;
		serial.FlushSerialPort(10);
		if(serial.Check(550)){
			while(serial.Check(100) && cnt<256){ // Read GPRMC and GPGGA
				serial.Read(gpstxt+cnt,1);
				cnt++;
			}
			m_gotContact = true;
		}else{
			printf("timeout in getting gps\n");
			serial.FlushSerialPort(1);
			m_gotContact = false;
			return false;
		}
	}while(!Parse(gpstxt, this->gpsInfo));

	#ifdef _DEBUG
	m_logFile.Format("gps.log"); // for testing only
	if(strlen(m_logFile) > 0){
		FILE *f = fopen(g_exePath + m_logFile, "a+");
		fprintf(f, "%1d\t%ld\t", gpsInfo.date, gpsInfo.time);
		fprintf(f, "%lf\t%lf\t%lf\t", gpsInfo.latitude, gpsInfo.longitude, gpsInfo.altitude);
		fprintf(f, "%ld\n", gpsInfo.nSatellites);
		fclose(f);
	}
	#endif

	return SUCCESS;
}

void CGPS::CloseSerial(){
	this->serial.Close();
}

UINT CollectGPSData(LPVOID pParam){
	CGPS *gps = (CGPS *)pParam;
	gps->fRun = true;

	while(gps->fRun){
		bool ret=gps->ReadGPS();
		if (ret) {
			/* make a small pause */
			Sleep(100);
		}
		else {
			// Error reading GPS.  Sleep longer and try again.
			gps->CloseSerial();
			Sleep(1000);
			gps->Connect();
		}
	}

	gps->CloseSerial();
	gps->fRun = false;

	return 0;
}
