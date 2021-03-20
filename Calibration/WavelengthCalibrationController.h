#pragma once
#include <string>
#include <vector>

class WavelengthCalibrationController
{
public:
    WavelengthCalibrationController();

    /// <summary>
    /// The full path to the spectrum to calibrate
    /// </summary>
    std::string m_inputSpectrumFile;

    /// <summary>
    /// The full path to the dark spectrum of the spectrum to calibrate
    /// </summary>
    std::string m_darkSpectrumFile;

    /// <summary>
    /// The full path to the high resolved solar spectrum
    /// </summary>
    std::string m_solarSpectrumFile;

    /// <summary>
    /// The full path to a file which contains an initial wavelength calibration
    /// </summary>
    std::string m_initialWavelengthCalibrationFile;

    /// <summary>
    /// The full path to a file which contains an initial measured line shape
    /// </summary>
    std::string m_initialLineShapeFile;

    /// <summary>
    /// Output: the final estimate for the pixel to wavelength mapping.
    /// </summary>
    std::vector<double> m_resultingPixelToWavelengthMapping;

    /// <summary>
    /// Output: the coefficients of the pixel-to-wavelength mapping polynomial
    /// </summary>
    std::vector<double> m_resultingPixelToWavelengthMappingCoefficients;

    /// <summary>
    /// Performs the actual wavelength calibration
    /// </summary>
    void RunCalibration();

};