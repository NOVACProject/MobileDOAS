#pragma once

#include "CSpectrum.h"

/** This is a simple, static, class for reading and writing spectra to/from file */
class CSpectrumIO
{
public:
    // ------------- Reading in spectra from file -----------------------

    static int readSTDFile(CString filename, CSpectrum* curSpec);
    [[deprecated]]
    static int readTextFile(CString filename, CSpectrum* curSpec);

    // ---------------- Writing spectra to file ---------------------
    static bool WriteStdFile(const CString& fileName, const CSpectrum& spectrum);

private:
    CSpectrumIO();
    ~CSpectrumIO();
};
