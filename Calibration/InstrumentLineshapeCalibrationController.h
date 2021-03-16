#pragma once
#include <string>
#include <SpectralEvaluation/Spectra/SpectrumUtils.h>

class InstrumentLineshapeCalibrationController
{
public:
    InstrumentLineshapeCalibrationController();

    /// <summary>
    /// The full path to the mercury spectrum to retreive the line shape from.
    /// </summary>
    std::string m_inputSpectrumPath;

    /// <summary>
    /// The full path to the dark to use (if any)
    /// </summary>
    std::string m_darkSpectrumPath;

    /// <summary>
    /// Output: this is our list of found peaks in the m_inputSpectrum.
    /// </summary>
    std::vector<novac::SpectrumDataPoint> m_peaksFound;

    /// <summary>
    /// The read in mercury spectrum data;
    /// </summary>
    std::vector<double> m_inputSpectrum;

    /// <summary>
    /// Locates peaks in the spectrum
    /// </summary>
    void Update();

};