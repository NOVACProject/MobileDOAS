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
    /// The full path to a file which contains an initial calibration
    /// This can be either a full calibration file, as saved from another novac program,
    /// or just the pixel-to-wavelength mapping file.
    /// </summary>
    std::string m_initialCalibrationFile;

    /// <summary>
    /// The full path to a file which contains an initial measured line shape.
    /// This may be left out if m_initialCalibrationFile does contain an instrument line shape.
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

    struct WavelengthCalibrationDebugState
    {
        WavelengthCalibrationDebugState(size_t estimatedSize)
        {
            inlierCorrespondencePixels.reserve(estimatedSize);
            inlierCorrespondenceWavelengths.reserve(estimatedSize);
            outlierCorrespondencePixels.reserve(estimatedSize);
            outlierCorrespondenceWavelengths.reserve(estimatedSize);
        }

        std::vector<double> initialPixelToWavelengthMapping;

        std::vector<double> inlierCorrespondencePixels;
        std::vector<double> inlierCorrespondenceWavelengths;

        std::vector<double> outlierCorrespondencePixels;
        std::vector<double> outlierCorrespondenceWavelengths;

        std::vector<double> measuredSpectrum;

        std::vector<double> measuredSpectrumKeypointPixels;
        std::vector<double> measuredSpectrumKeypointIntensities;

        std::vector<double> fraunhoferSpectrum;
        std::vector<double> fraunhoferSpectrumKeypointPixels;
        std::vector<double> fraunhoferSpectrumKeypointIntensities;
    };

    WavelengthCalibrationDebugState m_calibrationDebug;

    /// <summary>
    /// Performs the actual wavelength calibration
    /// </summary>
    void RunCalibration();

};