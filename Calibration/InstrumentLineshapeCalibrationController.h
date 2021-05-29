#pragma once
#include <string>
#include <SpectralEvaluation/Spectra/SpectrumUtils.h>
#include <SpectralEvaluation/Spectra/Spectrum.h>

class InstrumentLineshapeCalibrationController
{
public:
    InstrumentLineshapeCalibrationController();

    enum class LineShapeFunction
    {
        None,
        Gaussian,
        SuperGauss
    };

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
    /// Output: The read in mercury spectrum data
    /// </summary>
    std::vector<double> m_inputSpectrum;

    /// <summary>
    /// Output: The read in mercury spectrum wavlengths
    /// </summary>
    std::vector<double> m_inputSpectrumWavelength;

    /// <summary>
    /// Output: This is set to true (when reading the input spectrum) 
    /// if the read in spectrum already contains a pixel-to-wavelength calibration.
    /// Otherwise false.
    /// </summary>
    bool m_inputSpectrumContainsWavelength = false;

    /// <summary>
    /// Output: The fitted line shape function.
    /// </summary>
    std::pair<LineShapeFunction, void*> m_fittedLineShape;

    /// <summary>
    /// Output: The fitted line shape function sampled over the selected peak.
    /// </summary>
    std::unique_ptr<novac::CSpectrum> m_sampledLineShapeFunction;

    /// <summary>
    /// Locates peaks in the spectrum.
    /// This updates m_inputSpectrum and m_peaksFound.
    /// </summary>
    void Update();

    /// <summary>
    /// Fits a function to the peak with the provided index.
    /// This updates m_fittedLineShape and m_sampledLineShapeFunction
    /// </summary>
    void FitFunctionToLineShape(size_t peakIdx, LineShapeFunction function);

    /// <summary>
    /// Extracts the peak with the provided index from the read in spectrum file.
    /// The peak will be normalized and have its baseline subtracted
    /// </summary>
    std::unique_ptr<novac::CSpectrum> GetMercuryPeak(size_t peakIdx, double* baseline = nullptr, size_t* startIdx = nullptr);

private:
    void ClearFittedLineShape();

    /// <summary>
    /// Subtracts the baseline from the provided spectrum. 
    /// </summary>
    /// <param name="spectrum">The spectrum to modify</param>
    /// <returns>The baseline which was subtracted.</returns>
    double SubtractBaseline(novac::CSpectrum& spectrum);
};