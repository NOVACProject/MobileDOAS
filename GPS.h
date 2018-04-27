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

	/* try to parse the text read from the GPS. 
	    Returns 1 if all is ok, otherwise 0 */
	int Parse(char *string);

	/* Communication with other parts of the program */
	long    GetTime() const;
	double  GetAltitude() const;
	double  GetLatitude() const;
	double  GetLongitude() const;
	void    GetDate(std::string& dateStr) const;
	long    GetNumberOfSatellites() const;

	/* convert the raw GPS data into the format DD.DDDDDDD
			from being in the raw format DDMM.MMMMM */
	double  DoubleToAngle(double degreesAndMinutes);

	/* WriteGPSLog and WriteLog are currently not used */
	// void    WriteGPSLog(char *pFile,double *pPos,double pTime);
	// void    WriteLog(char *pFile,char* txt);

	/* Running the GPS collection */
	void    Run();
	void    Stop();
	void	CloseSerial();
	int     ReadGPS();
	bool    IsRunning(); // <-- returns true if the gps-collecting thread is running.

//variables 
private:

	/** true if the serial connection and the GPS seems to work */
	bool gotContact;

	/** The gps-logfile*/
	CString m_logFile;

	struct gpsInformation{
		struct    gpsPosition gpsPos;
		long      gpsTime;
		long      nSatellites;
		char      gpsDate[6];
	};

	/* The actual information */
	struct gpsInformation gpsInfo;

	/* Serial communication */
	CSerialConnection serial;

	/** The gps-reading thread. */
	CWinThread *m_gpsThread;
};

UINT CollectGPSData(LPVOID pParam);

#endif // !defined(AFX_GPS_H__EBAA5A71_5FAD_46B7_90C6_B5DBE3408F77__INCLUDED_)

