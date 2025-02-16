#pragma once

#include <string>

namespace Configuration
{
    class CMobileConfiguration;
}

namespace novac
{
    class CSpectrumInfo;
    class ILogger;
}

namespace Evaluation
{
    class CRealTimeCalibration
    {
    public:
        /** Performs an automatic instrument calibration for the supplied measured spectrum using the provided settings.
            If the AutomaticCalibrationSetting says that the references should be created and replace existing
                references in the real-time evaluation then the references will also be updated 'settings' will be written to file.
            @return true if the calibration succeeded and new references were created.
            @return false if the calibration succeeded but no new references were created.
            @throws std::exception if the calibration failed. */
        static bool RunInstrumentCalibration(
            const double* measuredSpectrum,
            const double* darkSpectrum,
            size_t spectrumLength,
            const novac::CSpectrumInfo& spectrumInfo,
            const std::string& outputDirectory,
            Configuration::CMobileConfiguration& settings,
            novac::ILogger& log,
            double spectrometerMaximumIntensityForSingleReadoutOverride = -1.0);

    };
}