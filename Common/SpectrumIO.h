#pragma once

#include "Spectrum.h"

struct gpsData;

/** This is a simple, static, class for reading and writing spectra to/from file */
class CSpectrumIO
{
public:
	// ------------- Reading in spectra from file -----------------------

	static int readSTDFile(CString filename, CSpectrum *curSpec);
	static int readTextFile(CString filename, CSpectrum *curSpec);

	// ---------------- Writing spectra to file ---------------------
	static bool WriteStdFile(const CString &fileName, const CSpectrum& spectrum);
	static bool WriteStdFile(const CString &fileName, const double *spectrum, long specLength, const std::string& startdate, long starttime, long stoptime, const gpsData& position, long integrationTime, const CString &spectrometer, const CString &measName, long exposureNum);

private:
	CSpectrumIO();
	~CSpectrumIO();
};
