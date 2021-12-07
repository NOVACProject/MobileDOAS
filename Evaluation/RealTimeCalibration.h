#pragma once

#include <string>

namespace Configuration
{
    class CMobileConfiguration;
}

namespace novac
{
    class CDateTime;
}

namespace Evaluation
{
    class CRealTimeCalibration
    {
    public:
        /** Performs an automatic instrument calibration for the supplied measured spectrum using the provided settings.
            If the AutomaticCalibrationSetting says that the references should be created and replace existing
                references in the real-time evaluation then the references will also be updated 'settings' will be written to file.
            @return true if the calibration succeeded.*/
        static bool RunInstrumentCalibration(
            const double* measuredSpectrum,
            const double* darkSpectrum,
            size_t spectrumLength,
            const std::string& outputDirectory,
            Configuration::CMobileConfiguration& settings);

    };
}