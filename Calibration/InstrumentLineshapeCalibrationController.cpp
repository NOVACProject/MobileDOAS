#include "StdAfx.h"

#undef min
#undef max

#include <algorithm>
#include <numeric>
#include "InstrumentLineshapeCalibrationController.h"
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/File/STDFile.h>
#include <SpectralEvaluation/Calibration/InstrumentCalibration.h>
#include <SpectralEvaluation/Calibration/InstrumentLineShape.h>
#include <SpectralEvaluation/Calibration/WavelengthCalibration.h>
#include <SpectralEvaluation/VectorUtils.h>

InstrumentLineshapeCalibrationController::InstrumentLineshapeCalibrationController()
    : m_inputSpectrumPath(""), m_darkSpectrumPath(""), m_inputSpectrumContainsWavelength(false)
{
    m_resultingCalibration = std::make_unique<novac::InstrumentCalibration>();
}

void InstrumentLineshapeCalibrationController::ClearFittedLineShape()
{
    if (m_resultingCalibration->instrumentLineShapeParameter != nullptr)
    {
        delete m_resultingCalibration->instrumentLineShapeParameter;
        m_resultingCalibration->instrumentLineShapeParameter = nullptr;
    }
    m_resultingCalibration->instrumentLineShape.clear();
    m_resultingCalibration->instrumentLineShapeGrid.clear();
}

void InstrumentLineshapeCalibrationController::Update()
{
    ClearFittedLineShape();

    novac::CSpectrum hgSpectrum;
    if (!novac::ReadSpectrum(m_inputSpectrumPath, hgSpectrum))
    {
        throw std::invalid_argument("Cannot read a spectrum from the provided input file.");
    }
    m_inputspectrumInformation = hgSpectrum.m_info; //< remember the meta-data of the spectrum

    // The dark spectrum is optional
    novac::CSpectrum darkSpectrum;
    if (novac::ReadSpectrum(m_darkSpectrumPath, darkSpectrum))
    {
        hgSpectrum.Sub(darkSpectrum);
    }

    m_inputSpectrum = std::vector<double>(hgSpectrum.m_data, hgSpectrum.m_data + hgSpectrum.m_length);

    m_inputSpectrumContainsWavelength = hgSpectrum.m_wavelength.size() != 0;

    if (m_readWavelengthCalibrationFromFile)
    {
        // Find the peaks in the measured spectrum, rejecting double peaks.
        novac::FindEmissionLines(hgSpectrum, m_peaksFound, true);
        m_rejectedPeaks = novac::FilterByType(m_peaksFound, novac::SpectrumDataPointType::UnresolvedPeak);
        m_peaksFound = novac::FilterByType(m_peaksFound, novac::SpectrumDataPointType::Peak);

        m_resultingCalibration->pixelToWavelengthPolynomial.clear();

        if (m_inputSpectrumContainsWavelength)
        {
            m_resultingCalibration->pixelToWavelengthMapping = hgSpectrum.m_wavelength;
            m_wavelengthCalibrationSucceeded = true;
        }
        else
        {
            m_resultingCalibration->pixelToWavelengthMapping.resize(hgSpectrum.m_length);
            std::iota(begin(m_resultingCalibration->pixelToWavelengthMapping), end(m_resultingCalibration->pixelToWavelengthMapping), 0.0);
            m_wavelengthCalibrationSucceeded = false;

            for (auto& peak : m_peaksFound)
            {
                peak.wavelength = peak.pixel;
            }
        }
    }
    else
    {
        // If the measured spectrum does _not_ contain a pixel-to-wavelength calibration already
        // then make a guess for this from the distance of the peaks
        novac::SpectrometerCalibrationResult wavelengthCalibrationResult;
        novac::MercurySpectrumCalibrationState wavelengthCalibrationState;
        std::vector<double> initialPixelToWavelengthMapping = (hgSpectrum.m_wavelength.size() != 0) ? hgSpectrum.m_wavelength : GenerateVector(280.0, 420.0, hgSpectrum.m_length);

        if (novac::MercuryCalibration(hgSpectrum, 3, initialPixelToWavelengthMapping, wavelengthCalibrationResult, &wavelengthCalibrationState))
        {
            m_resultingCalibration->pixelToWavelengthMapping = wavelengthCalibrationResult.pixelToWavelengthMapping;
            m_resultingCalibration->pixelToWavelengthPolynomial = wavelengthCalibrationResult.pixelToWavelengthMappingCoefficients;
            m_peaksFound = wavelengthCalibrationState.peaks;
            m_rejectedPeaks = wavelengthCalibrationState.rejectedPeaks;
            m_wavelengthCalibrationSucceeded = true;
        }
        else
        {
            m_resultingCalibration->pixelToWavelengthMapping.resize(hgSpectrum.m_length);
            std::iota(begin(m_resultingCalibration->pixelToWavelengthMapping), end(m_resultingCalibration->pixelToWavelengthMapping), 0.0);
            m_wavelengthCalibrationSucceeded = false;
            m_resultingCalibration->pixelToWavelengthPolynomial.clear();
            m_peaksFound.clear();
            m_rejectedPeaks.clear();
        }
    }
}

double InstrumentLineshapeCalibrationController::SubtractBaseline(novac::CSpectrum& spectrum)
{
    std::vector<double> data{ spectrum.m_data, spectrum.m_data + spectrum.m_length };
    std::vector<double> fiveLowestValues;
    FindNLowest(data, 5, fiveLowestValues);
    const double baseline = Average(fiveLowestValues);

    for (long ii = 0; ii < spectrum.m_length; ++ii)
    {
        spectrum.m_data[ii] -= baseline;
    }

    return baseline;
}

std::unique_ptr<novac::CSpectrum> InstrumentLineshapeCalibrationController::ExtractSelectedMercuryPeak(size_t peakIdx, double* baselinePtr, size_t* startIdx) const
{
    if (peakIdx >= m_peaksFound.size())
    {
        throw std::invalid_argument("Invalid index of mercury peak, please select a peak and try again");
    }

    // Extract the data surrounding the peak
    const novac::SpectrumDataPoint selectedPeak = m_peaksFound[peakIdx];
    const double leftWidth = selectedPeak.pixel - selectedPeak.leftPixel;
    const double rightWidth = selectedPeak.rightPixel - selectedPeak.pixel;

    const size_t spectrumStartIdx = static_cast<size_t>(std::max(0, static_cast<int>(std::round(selectedPeak.pixel - 3 * leftWidth))));
    const size_t spectralDataLength = std::min(m_inputSpectrum.size() - spectrumStartIdx, static_cast<size_t>(std::round(3 * (leftWidth + rightWidth))));
    const double* spectralData = m_inputSpectrum.data() + spectrumStartIdx;
    const double* wavelengthData = m_resultingCalibration->pixelToWavelengthMapping.data() + spectrumStartIdx;

    // Construct the spectrum we want to fit to
    std::unique_ptr<novac::CSpectrum> hgLine = std::make_unique<novac::CSpectrum>(wavelengthData, spectralData, spectralDataLength);
    const double baseline = SubtractBaseline(*hgLine);

    if (baselinePtr != nullptr)
    {
        *baselinePtr = baseline;
    }
    if (startIdx != nullptr)
    {
        *startIdx = spectrumStartIdx;
    }

    return hgLine;
}

std::string ToString(double value)
{
    char buffer[64];
    sprintf(buffer, "%0.3lf", value);
    return std::string(buffer);
}

std::vector<std::pair<std::string, std::string>> GetFunctionDescription(const novac::ParametricInstrumentLineShape* lineShapeFunction)
{
    std::vector<std::pair<std::string, std::string>> result;

    if (lineShapeFunction == nullptr)
    {
        return result;
    }
    else if (lineShapeFunction->Type() == novac::InstrumentLineShapeFunctionType::Gaussian)
    {
        const auto func = static_cast<const novac::GaussianLineShape*>(lineShapeFunction);
        result.push_back(std::make_pair("type", "Gaussian"));
        result.push_back(std::make_pair("sigma", ToString(func->sigma)));
        result.push_back(std::make_pair("fwhm", ToString(func->Fwhm())));
    }
    else if (lineShapeFunction->Type() == novac::InstrumentLineShapeFunctionType::SuperGaussian)
    {
        const auto func = static_cast<const novac::SuperGaussianLineShape*>(lineShapeFunction);
        result.push_back(std::make_pair("type", "Super Gaussian"));
        result.push_back(std::make_pair("w", ToString(func->w)));
        result.push_back(std::make_pair("k", ToString(func->k)));
        result.push_back(std::make_pair("fwhm", ToString(func->Fwhm())));
    }
    else
    {
        // unknown type...
    }

    return result;
}

std::vector<std::pair<std::string, std::string>> InstrumentLineshapeCalibrationController::GetFittedFunctionDescription() const
{
    return GetFunctionDescription(m_resultingCalibration->instrumentLineShapeParameter);
}

void InstrumentLineshapeCalibrationController::FitFunctionToLineShape(size_t peakIdx, LineShapeFunction function)
{
    ClearFittedLineShape();

    if (peakIdx >= m_peaksFound.size() || function == LineShapeFunction::None)
    {
        return;
    }

    // Construct the spectrum we want to fit to
    double baseline = 0.0;
    size_t spectrumStartIdx = 0;
    auto hgLine = ExtractSelectedMercuryPeak(peakIdx, &baseline, &spectrumStartIdx);

    const size_t superSamplingRate = 1;
    if (function == LineShapeFunction::Gaussian)
    {
        auto gaussian = new novac::GaussianLineShape();
        novac::FitInstrumentLineShape(*hgLine, *gaussian);

        m_resultingCalibration->instrumentLineShapeParameter = gaussian;

        std::vector<double> highResPixelData(superSamplingRate * hgLine->m_length); // super-sample with more points per pixel
        const double range = hgLine->m_wavelength.back() - hgLine->m_wavelength.front();
        for (size_t ii = 0; ii < highResPixelData.size(); ++ii)
        {
            highResPixelData[ii] = hgLine->m_wavelength[0] + ii * range / (double)(highResPixelData.size() - 1);
        }
        const std::vector<double> sampledLineShape = novac::SampleInstrumentLineShape(*gaussian, highResPixelData, m_peaksFound[peakIdx].wavelength, m_peaksFound[peakIdx].intensity - baseline, baseline);

        m_resultingCalibration->instrumentLineShapeGrid = highResPixelData;
        m_resultingCalibration->instrumentLineShape = sampledLineShape;
        m_resultingCalibration->instrumentLineShapeCenter = m_peaksFound[peakIdx].wavelength;
    }
    else if (function == LineShapeFunction::SuperGauss)
    {
        auto superGaussian = new novac::SuperGaussianLineShape();
        novac::FitInstrumentLineShape(*hgLine, *superGaussian);

        m_resultingCalibration->instrumentLineShapeParameter = superGaussian;

        std::vector<double> highResPixelData(superSamplingRate * hgLine->m_length); // super-sample with more points per pixel
        const double range = hgLine->m_wavelength.back() - hgLine->m_wavelength.front();
        for (size_t ii = 0; ii < highResPixelData.size(); ++ii)
        {
            highResPixelData[ii] = hgLine->m_wavelength[0] + ii * range / (double)(highResPixelData.size() - 1);
        }
        const std::vector<double> sampledLineShape = novac::SampleInstrumentLineShape(*superGaussian, highResPixelData, m_peaksFound[peakIdx].wavelength, m_peaksFound[peakIdx].intensity - baseline, baseline);

        m_resultingCalibration->instrumentLineShapeGrid = highResPixelData;
        m_resultingCalibration->instrumentLineShape = sampledLineShape;
        m_resultingCalibration->instrumentLineShapeCenter = m_peaksFound[peakIdx].wavelength;
    }
    else if (function == LineShapeFunction::None)
    {
        m_resultingCalibration->instrumentLineShapeGrid = hgLine->m_wavelength;
        m_resultingCalibration->instrumentLineShape = std::vector<double>(hgLine->m_data, hgLine->m_data + hgLine->m_length);
        m_resultingCalibration->instrumentLineShapeCenter = m_peaksFound[peakIdx].wavelength;
    }
    else
    {
        throw std::invalid_argument("Invalid type of function selected");
    }
}

std::pair<std::string, std::string> FormatProperty(const char* name, double value)
{
    char formattedValue[128];
    sprintf_s(formattedValue, sizeof(formattedValue), "%.9g", value);

    return std::make_pair(std::string(name), std::string(formattedValue));
}

void InstrumentLineshapeCalibrationController::SaveResultAsStd(size_t peakIdx, const std::string& filename)
{
    // Extract the spectrum in the format we want to use
    if (peakIdx >= m_peaksFound.size())
    {
        throw std::invalid_argument("Invalid index of mercury peak, please select a peak and try again");
    }

    // Create an instrument calibration with a normalized intensity of the instrument line shape
    novac::InstrumentCalibration calibrationWithNormalizedInstrumentLineShape;
    calibrationWithNormalizedInstrumentLineShape.pixelToWavelengthMapping = m_resultingCalibration->pixelToWavelengthMapping;
    calibrationWithNormalizedInstrumentLineShape.pixelToWavelengthPolynomial = m_resultingCalibration->pixelToWavelengthPolynomial;

    calibrationWithNormalizedInstrumentLineShape.instrumentLineShape = m_resultingCalibration->instrumentLineShape;
    calibrationWithNormalizedInstrumentLineShape.instrumentLineShapeGrid = m_resultingCalibration->instrumentLineShapeGrid;
    calibrationWithNormalizedInstrumentLineShape.instrumentLineShapeCenter = m_resultingCalibration->instrumentLineShapeCenter;

    if (m_resultingCalibration->instrumentLineShapeParameter != nullptr)
    {
        calibrationWithNormalizedInstrumentLineShape.instrumentLineShapeParameter = m_resultingCalibration->instrumentLineShapeParameter->Clone();
    }

    Normalize(calibrationWithNormalizedInstrumentLineShape.instrumentLineShape);

    if (!novac::SaveInstrumentCalibration(filename, calibrationWithNormalizedInstrumentLineShape))
    {
        throw std::invalid_argument("Failed to save the resulting instrument calibration file");
    }
}

void InstrumentLineshapeCalibrationController::SaveResultAsClb(const std::string& filename)
{
    novac::SaveDataToFile(filename, m_resultingCalibration->pixelToWavelengthMapping);
}

void InstrumentLineshapeCalibrationController::SaveResultAsSlf(size_t peakIdx, const std::string& filename)
{
    novac::CCrossSectionData instrumentLineShape;
    instrumentLineShape.m_crossSection = m_resultingCalibration->instrumentLineShape;
    instrumentLineShape.m_waveLength = m_resultingCalibration->instrumentLineShapeGrid;

    novac::SaveCrossSectionFile(filename, instrumentLineShape);
}
