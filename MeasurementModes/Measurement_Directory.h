#pragma once

#include "../Spectrometer.h"

class CMeasurement_Directory :
    public CSpectrometer
{
public:
    CMeasurement_Directory(
        CView& mainForm,
        std::unique_ptr<mobiledoas::SpectrometerInterface> spectrometerInterface,
        std::unique_ptr<Configuration::CMobileConfiguration> conf);

    virtual ~CMeasurement_Directory();

    /**
        This is used to monitor directory for spectra collected and
        show the latest in the interface.
    */
    void Run();

private:

    bool ProcessSpectrum(CString latestSpectrum);
    bool ReadSky();
    bool ReadDark();
    bool ReadDarkcur();
    bool ReadOffset();
};

