#pragma once

#include "../Spectrometer.h"

/** The class <b>CMeasurement_Traverse</b> is the implementation of the standard
    traverse used in MobileDOAS. It extends the functions found in CSpectrometer.
*/

class CMeasurement_Traverse : public CSpectrometer
{
public:
    CMeasurement_Traverse(std::unique_ptr<mobiledoas::SpectrometerInterface> spectrometerInterface);
    virtual ~CMeasurement_Traverse();

    /** This is used to make a standard traverse, with each spectrum having the
        same exposure-time
    */
    void Run();

    /** This is used to make a traverse with varying exposure-time along the
        the measurement route
    */
    void Run_Adaptive();
};
