#pragma once

#include "../Spectrometer.h"

/** The class <b>CMeasurement_View</b> is the implementation of the viewing of
    spectra from the spectrometer. It extends the functions found in CSpectrometer
*/

class CMeasurement_View : public CSpectrometer
{
public:
    CMeasurement_View(
        CView& mainForm,
        std::unique_ptr<mobiledoas::SpectrometerInterface> spectrometerInterface,
        std::unique_ptr<Configuration::CMobileConfiguration> conf);

    virtual ~CMeasurement_View();

    /** This is used to collect spectra from the spectrometer and show them in the interface
        without performing any data-analysis
    */
    void Run();
};
