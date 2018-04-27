// GPS.h: interface for the CGPS class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GPS_H__EBAA5A71_5FAD_46B7_90C6_B5DBE3408F77__INCLUDED_)
#define AFX_GPS_H__EBAA5A71_5FAD_46B7_90C6_B5DBE3408F77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SerialConnection.h"
#include "Common.h"
#include <string>

class CGPS {
public:
	CGPS();
	CGPS(char* pCOMPort, long pBaudrate);
	virtual ~CGPS();

	/** Set to true when the gps-collecting thread is running. */
	volatile bool fRun;

	/* Tries to parse the text read from the GPS.
		The parsed information will be filled into the provided 'data.
		@return true if the parsing suceeded, otherwise false. */
	static bool Parse(char* string, gpsData& data);

	/* Convert an angle from the raw format of the GPS data DDMM.MMMMM
		into the format DD.DDDDDDD */
	static double ConvertToDecimalDegrees(double degreesAndMinutes);

	/** Retrieving the read out data (this will not communicate with 
		the device, only copy out the last read piece of data. */
	void    Get(gpsData& dst) const;

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

	/** true if the serial connection and the GPS seems to work */
	bool m_gotContact = false;

	/** The gps-logfile*/
	CString m_logFile;

	/* The actual information */
	struct gpsData gpsInfo;

	/* Serial communication */
	CSerialConnection serial;

	/** The gps-reading thread. */
	CWinThread *m_gpsThread;
};

UINT CollectGPSData(LPVOID pParam);

#endif // !defined(AFX_GPS_H__EBAA5A71_5FAD_46B7_90C6_B5DBE3408F77__INCLUDED_)

