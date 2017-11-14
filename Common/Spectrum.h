#pragma once

#include "../Common.h"

class CSpectrum
{
public:
	CSpectrum(void);
	~CSpectrum(void);

	double  I[MAX_SPECTRUM_LENGTH];
	int     length;
	long    scans;
	long    intTime;	// The exposure-time in mili-seconds
	double  lat;
	double  lon;
	int     date[3];
	int     startTime[3];
	int     stopTime[3];
	bool    isDark;
	CString	spectrometer;

	// statistics
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

	// copying a spectrum
	bool    Copy(const CSpectrum &spec2);

	// clearing out the information in the spectrum
	void		Clear();
};
