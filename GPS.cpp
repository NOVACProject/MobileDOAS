// GPS.cpp: implementation of the CGPS class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <iostream>
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
	: m_gpsInfo(), fRun(false)
{
}

CGPS::CGPS(const char* pCOMPort, long pBaudrate)
	: m_gpsInfo(), fRun(false)
{
	serial.SetBaudrate(pBaudrate);
	serial.SetPort(pCOMPort);

	if (!Connect())
	{
		MessageBox(nullptr, "Could not communicate with GPS. No GPS-data can be retrieved!", "Error", MB_OK | MB_SYSTEMMODAL);
	}
}

CGPS::CGPS(CSerialConnection&& serial)
	: m_gpsInfo(), fRun(false)
{
	// Take ownership of the serial connection.
	this->serial = std::move(serial);
}

CGPS::CGPS(CGPS&& other)
{
	this->serial		= std::move(other.serial);
	this->fRun			= other.fRun;
	this->m_gotContact	= other.m_gotContact;
	this->m_gpsInfo		= other.m_gpsInfo;
	this->m_logFile		= other.m_logFile;
}

CGPS& CGPS::operator=(CGPS&& other)
{
	this->serial		= std::move(other.serial);
	this->fRun			= other.fRun;
	this->m_gotContact	= other.m_gotContact;
	this->m_gpsInfo		= other.m_gpsInfo;
	this->m_logFile		= other.m_logFile;
	return *this;
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
				sprintf(data.date, "%s", token);
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
    }
  }
}

CGPS::~CGPS()
{
	serial.Close();
}

bool CGPS::Connect()
{
	if (!serial.Init())
	{
		return false;
	}
	return true;
}

void CGPS::Get(gpsData& dst)
{
	std::lock_guard<std::mutex> guard(this->m_gpsInfoMutex); // lock the access to the 'm_gpsInfo' while we're copying out the data
	dst = this->m_gpsInfo;
}


bool CGPS::ReadGPS()
{
	const long bytesToRead = 512;
	char gpstxt[512];

	gpstxt[0] = 0;

	// copy the old data into the temp structure (not all sentences provide all data...)
	gpsData localGpsInfo = this->m_gpsInfo;

	do{
		long cnt = 0;
		// serial.FlushSerialPort(10);
		if(serial.Check(2000))
		{
			while(serial.Check(100) && cnt < bytesToRead)
			{
				// Read GPRMC and GPGGA
				serial.Read(gpstxt + cnt, 1);
				if (gpstxt[cnt] == '\n')
				{
 					break; // each sentence ends with newline
				}
				cnt++;
			}
			m_gotContact = true;
		}
		else
		{
			std::cerr << "timeout in getting gps." << std::endl;
			serial.FlushSerialPort(1);
			m_gotContact = false;
			return false;
		}
	}while(!Parse(gpstxt, localGpsInfo));

	// Copy the parsed data to our member structure in a controlled fashion
	{
		std::lock_guard<std::mutex> guard(this->m_gpsInfoMutex); // lock the access to the 'm_gpsInfo' while we're copying out the data
		this->m_gpsInfo = localGpsInfo;
	}

	#ifdef _DEBUG
	m_logFile.Format("gps.log"); // for testing only
	if(strlen(m_logFile) > 0){
		FILE *f = fopen(g_exePath + m_logFile, "a+");
		fprintf(f, "%1d\t%ld\t", localGpsInfo.date, localGpsInfo.time);
		fprintf(f, "%lf\t%lf\t%lf\t", localGpsInfo.latitude, localGpsInfo.longitude, localGpsInfo.altitude);
		fprintf(f, "%ld\t", localGpsInfo.nSatellitesTracked);
		fprintf(f, "%ld\n", localGpsInfo.nSatellitesSeen);
		fclose(f);
	}
	#endif

	return true;
}

void CGPS::CloseSerial()
{
	this->serial.Close();
}

/** The async GPS data collection thread. This will take ownership of the CGPS which is passed in as reference
	and delete it when we're done. */
UINT CollectGPSData(LPVOID pParam)
{
	CGPS *gps = (CGPS *)pParam;
	gps->fRun = true;

	while(gps->fRun)
	{
		if (gps->ReadGPS())
		{
			/* make a small pause */
			Sleep(10);
		}
		else
		{
			// Error reading GPS.  Sleep longer and try again.
			gps->CloseSerial();
			Sleep(1000);
			gps->Connect();
			Sleep(1000);
		}
	}

	gps->fRun = false;

	delete gps;

	return 0;
}

GpsAsyncReader::GpsAsyncReader(const char* pCOMPort, long baudrate)
{
	m_gps = new CGPS(pCOMPort, baudrate);
	m_gpsThread = AfxBeginThread(CollectGPSData, (LPVOID)m_gps, THREAD_PRIORITY_NORMAL, 0, 0, nullptr);
}

GpsAsyncReader::GpsAsyncReader(CGPS&& gps)
{
	m_gps = new CGPS(std::move(gps));
	m_gpsThread = AfxBeginThread(CollectGPSData, (LPVOID)m_gps, THREAD_PRIORITY_NORMAL, 0, 0, nullptr);
}


GpsAsyncReader::~GpsAsyncReader()
{
	m_gps = nullptr; // we don't get to delete the m_gps, its up to the background thread to do so.
}

void GpsAsyncReader::Stop()
{
	if(nullptr != m_gps && m_gps->fRun == true)
	{
		m_gps->fRun = false;
	}
}

void GpsAsyncReader::Get(gpsData& data)
{
	m_gps->Get(data);
}

bool GpsAsyncReader::GotContact() const
{
	return m_gps->m_gotContact;
}