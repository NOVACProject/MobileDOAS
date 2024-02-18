#pragma once

#include "../Spectrometer.h"

/** The class <b>CMeasurement_Wind</b> is the implementation of the dual-beam
    wind-speed measurement used in MobileDOAS. It extends the functions found in CSpectrometer.
*/

class CMeasurement_Wind : public CSpectrometer
{
public:
    CMeasurement_Wind(
        CView& mainForm,
        std::unique_ptr<mobiledoas::SpectrometerInterface> spectrometerInterface,
        std::unique_ptr<Configuration::CMobileConfiguration> conf);

    virtual ~CMeasurement_Wind(void);

    /** This is used to make a dual-beam wind measurement, with each spectrum having the
        same exposure-time
    */
    void Run();

};
