// GPS.h: interface for the CGPS class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GPS_H__EBAA5A71_5FAD_46B7_90C6_B5DBE3408F77__INCLUDED_)
#define AFX_GPS_H__EBAA5A71_5FAD_46B7_90C6_B5DBE3408F77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SerialConnection.h"
#include "GpsData.h"
#include <string>
#include <mutex>

/** The CGPS manages the connection with the GPS through serial communication. */
class CGPS final
{
public:
	CGPS();

	/** Creates a GPS receiver instance which will communicate with the GPS on the provided COM-port 
		and using the provided baudrate. 
		@param pCOMPort The name of the COM-port to use. This must be on the form 'COMN' where 'N' is an integer.
		@param baudrate The baudrate to use in the communication. */
	CGPS(const char* pCOMPort, long baudrate);

	/** Creates a GPS receiver instance using the provided serial instance. This will take ownership
		of the serial port. NOTICE the provided instance of SerialConnection must not be used elsewhere. */
	CGPS(CSerialConnection&& serial);

	~CGPS();

	// --- This class manages a serial connction handle and is thus not copyable
	CGPS(const CGPS&) = delete;
	CGPS& operator=(const CGPS&) = delete;

	bool Connect();

	/** Set to true when the gps-collecting thread is running. */
	volatile bool fRun;

	/** true if the serial connection and the GPS seems to work */
	bool m_gotContact = false;

	/** Retrieving the read out data (this will not communicate with 
		the device, only copy out the last read piece of data. 
		This will temporarily lock the member 'm_gpsInfo' to avoid race conditions. */
	void Get(gpsData& dst);

	/* WriteGPSLog and WriteLog are currently not used */
	// void    WriteGPSLog(char *pFile,double *pPos,double pTime);
	// void    WriteLog(char *pFile,char* txt);

	/* Running the GPS collection */
	void    Run();
	void    Stop();
	void    CloseSerial();

	/** Reads data from the gps, this will use and block the serial port. 
		@return SUCCESS if the reading succeeded */
	bool ReadGPS();

private:

	/** The gps-logfile*/
	CString m_logFile;

	/* The actual information */
	struct gpsData m_gpsInfo;

	/** This mutex helps to protect the 'gpsInfo' struct such that no two threads 
		attempts to read/write it simultaneously */
	std::mutex m_gpsInfoMutex;

	/* Serial communication */
	CSerialConnection serial;

	/** The gps-reading thread. */
	CWinThread* m_gpsThread = nullptr;
};

UINT CollectGPSData(LPVOID pParam);

#endif // !defined(AFX_GPS_H__EBAA5A71_5FAD_46B7_90C6_B5DBE3408F77__INCLUDED_)

