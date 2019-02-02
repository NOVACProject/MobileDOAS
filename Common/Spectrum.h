#pragma once

#include "../Common.h"

class CSpectrum
{

public:
	CSpectrum();
	~CSpectrum();

	// -------- Copying CSpectrum objects --------
	CSpectrum(const CSpectrum& other);
	CSpectrum& operator=(CSpectrum other);
	friend void swap(CSpectrum& first, CSpectrum& second);

	double  I[MAX_SPECTRUM_LENGTH];	//< The spectral data itself.
	int     length;			// The length of the spectrum
	long    scans;			// The number of co-added spectra (exposures)
	long    exposureTime;	// The exposure-time in milliseconds
	double  lat;			// Latitude, in decimal degrees
	double  lon;			// Longitude, in decimal degrees
	double  altitude;		// Altitude, in meters above sea-level
	int     date[3];		// The date the spectrum acquisition started. date[0] is the year, date[1] is the month and date[2] is the day
	int     startTime[3];	// The local time-of-day when the spectrum acquisition started. [0] is hour, [1] is minute, [2] is seconds.
	int     stopTime[3];	// The local time-of-day when the spectrum acquisition stopped. [0] is hour, [1] is minute, [2] is seconds.
	bool    isDark;			// Set to true if this spectrum is dark.
	CString spectrometerSerial;	// Serial number of the spectrometer
	CString spectrometerModel;  // Model of the spectrometer
	CString name;			// name of the spectrum

	// additional added 1/30/2019 DLN
	double boardTemperature; 
	double detectorTemperature;
	int fitHigh;
	int fitLow;

	// statistics
	void    GetMinMax(double& minValue, double&maxValue) const;
	double  GetMax() const;
	double  GetAverage() const;
	double  GetAverage(int low, int high) const; // gets the average value between the indexes 'low' and 'high' (inclusive)
	double  GetSum() const;
	double  GetSum(int low, int high) const; // gets the sum of all value between the indexes 'low' and 'high' (inclusive)

	// spectrum math
	bool    Add(CSpectrum &spec2);
	bool    Div(CSpectrum &spec2);
	bool    Sub(CSpectrum &spec2);

	// scalar math
	bool    Add(double value);
	bool    Div(double value);
	bool    Sub(double value);
	bool    Mult(double value);

	// clearing out the information in the spectrum
	void    Clear();

	// date functions
	void SetDate(std::string startDate);
	void SetStartTime(long startTime);
	void SetStopTime(long stopTime);

	// read and write from STD files
	int readSTDFile(CString filename);
	bool WriteStdFile(const CString &fileName);

private:
	void GetHrMinSec(int time, int &hr, int &min, int &sec);
};
