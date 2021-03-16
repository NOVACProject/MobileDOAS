#pragma once
#include <string>

class WavelengthCalibrationController
{
public:
    WavelengthCalibrationController();

    /// <summary>
    /// The full path to the spectrum to calibrate
    /// </summary>
    std::string m_inputSpectrum;

    /// <summary>
    /// The full path to the high resolved solar spectrum
    /// </summary>
    std::string m_solarSpectrum;

    /// <summary>
    /// The full path to a file which contains an initial wavelength calibration
    /// </summary>
    std::string m_initialWavelengthCalibration;

    /// <summary>
    /// Performs the actual wavelength calibration
    /// </summary>
    void RunCalibration();

};