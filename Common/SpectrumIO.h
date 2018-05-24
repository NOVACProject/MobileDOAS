#pragma once
#include <string>
#include "Spectrum.h"

class CSpectrumIO
{
public:
	CSpectrumIO(void);
	~CSpectrumIO(void);

	// ------------- Reading in spectra from file -----------------------

	static int readSTDFile(CString filename, CSpectrum *curSpec);
	static int readTextFile(CString filename, CSpectrum *curSpec);

	// ---------------- Writing spectra to file ---------------------
	//	static int WriteStdFile(const CString &fileName, const CSpectrum &spectrum);
	static bool WriteStdFile(const CString &fileName, const double *spectrum, long specLength, const std::string& startdate, long starttime, long stoptime, double lat, double lon, double alt, long integrationTime, const CString &spectrometer, const CString &measName, long exposureNum);
};
