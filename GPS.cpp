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
	char gpstxt[256];

	gpstxt[0] = 0;

	gpsData localGpsInfo;

	do{
		long cnt = 0;
		serial.FlushSerialPort(10);
		if(serial.Check(550))
		{
			while(serial.Check(100) && cnt<256){ // Read GPRMC and GPGGA
				serial.Read(gpstxt+cnt,1);
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
		fprintf(f, "%ld\n", localGpsInfo.nSatellites);
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
			Sleep(100);
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