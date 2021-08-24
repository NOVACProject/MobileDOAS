#pragma once
#include <map>
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
    /// Input: The full path to the mercury spectrum to retreive the line shape from.
    /// </summary>
    std::string m_inputSpectrumPath;

    /// <summary>
    /// Input: The full path to the dark to use (if any)
    /// </summary>
    std::string m_darkSpectrumPath;

    /// <summary>
    /// Input: set to true if we should read the wavelength calibration
    /// from file (if available). Set to false to attempt to auto-determine
    /// the wavelength calibration from the mercury emission lines.
    /// </summary>
    bool m_readWavelengthCalibrationFromFile = true;

    /// <summary>
    /// Output: this is our list of found peaks in the m_inputSpectrum.
    /// </summary>
    std::vector<novac::SpectrumDataPoint> m_peaksFound;

    /// <summary>
    /// Output: this is our list of peaks which have been found in the m_inputSpectrum
    /// but have been rejected for some reason
    /// </summary>
    std::vector<novac::SpectrumDataPoint> m_rejectedPeaks;

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
    /// Output: this is set to true if we are to perform a wavelength calibration ourselves
    /// (i.e. if m_readWavelengthCalibrationFromFile==false or m_inputSpectrumContainsWavelength==false)
    /// and the wavelength calibration succeeded.
    /// </summary>
    bool m_wavelengthCalibrationSucceeded = false;

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
    /// The peak will be normalized and have its baseline subtracted.
    /// </summary>
    std::unique_ptr<novac::CSpectrum> GetMercuryPeak(size_t peakIdx, double* baseline = nullptr, size_t* startIdx = nullptr) const;

    /// <summary>
    /// Extracts the instrument line shape based on the peak with the provided index.
    /// If a line shape function has been fitted then this will be sampled and returned (needs a prior call to FitFunctionToLineShape),
    /// otherwise the spectrum itself will be returned.
    /// </summary>
    std::unique_ptr<novac::CSpectrum> GetInstrumentLineShape(size_t peakIdx) const;

    /// <summary>
    /// Returns a textual summary of parameters / properties of the last fitted function
    /// (i.e. the last call to FitFunctionToLineShape).
    /// Returns an empty set if no function has been fitted.
    /// </summary>
    std::vector<std::pair<std::string, std::string>> GetFittedFunctionDescription() const;

    /// <summary>
    /// Saves the resulting instrument line shape information as a .std file.
    /// </summary>
    void SaveResultAsStd(size_t peakIdx, const std::string& filename);

    /// <summary>
    /// Saves the resulting pixel-to-wavelength mapping information as a .clb file.
    /// </summary>
    void SaveResultAsClb(const std::string& filename);

    /// <summary>
    /// Saves the resulting instrument line shape information as a .slf file.
    /// </summary>
    void SaveResultAsSlf(size_t peakIdx, const std::string& filename);

private:
    void ClearFittedLineShape();

    /// <summary>
    /// Subtracts the baseline from the provided spectrum. 
    /// </summary>
    /// <param name="spectrum">The spectrum to modify</param>
    /// <returns>The baseline which was subtracted.</returns>
    static double SubtractBaseline(novac::CSpectrum& spectrum);

    /// <summary>
    /// The meta-data regarding the last read in mercury spectrum.
    /// </summary>
    novac::CSpectrumInfo m_inputspectrumInformation;

    /// <summary>
    /// Output: This is set if the wavelength calibration succeeded and will then
    /// contain the coefficients of the calibration. Empty if wavelength is read from file or
    /// the wavelength calibration failed.
    /// </summary>
    std::vector<double> m_wavelengthCalibrationCoefficients;

};