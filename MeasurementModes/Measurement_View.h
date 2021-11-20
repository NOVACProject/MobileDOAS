#pragma once

#include "../Spectrometer.h"

/** The class <b>CMeasurement_View</b> is the implementation of the viewing of
    spectra from the spectrometer. It extends the functions found in CSpectrometer
*/

class CMeasurement_View : public CSpectrometer
{
public:
    CMeasurement_View(void);
    ~CMeasurement_View(void);

    /** This is used to collect spectra from the spectrometer and show them in the interface
        without performing any data-analysis
    */
    void Run();

};
