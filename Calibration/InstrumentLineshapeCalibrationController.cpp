#include "StdAfx.h"

#undef min
#undef max

#include <algorithm>
#include <numeric>
#include "InstrumentLineshapeCalibrationController.h"
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/File/STDFile.h>
#include <SpectralEvaluation/Calibration/InstrumentLineShape.h>
#include <SpectralEvaluation/Calibration/WavelengthCalibration.h>
#include <SpectralEvaluation/VectorUtils.h>

InstrumentLineshapeCalibrationController::InstrumentLineshapeCalibrationController()
    : m_inputSpectrumPath(""), m_darkSpectrumPath(""), m_inputSpectrumContainsWavelength(false), m_fittedLineShape(LineShapeFunction::None, nullptr)
{
}

void InstrumentLineshapeCalibrationController::ClearFittedLineShape()
{
    if (m_fittedLineShape.second != nullptr)
    {
        delete m_fittedLineShape.second;
    }
    m_fittedLineShape = std::pair<LineShapeFunction, void*>(LineShapeFunction::None, nullptr);

    m_sampledLineShapeFunction.reset();
}

void InstrumentLineshapeCalibrationController::Update()
{
    ClearFittedLineShape();

    novac::CSpectrum hgSpectrum;
    if (!novac::ReadSpectrum(this->m_inputSpectrumPath, hgSpectrum))
    {
        throw std::invalid_argument("Cannot read a spectrum from the provided input file.");
    }
    this->m_inputspectrumInformation = hgSpectrum.m_info; //< remember the meta-data of the spectrum

    // The dark spectrum is optional
    novac::CSpectrum darkSpectrum;
    if (novac::ReadSpectrum(this->m_darkSpectrumPath, darkSpectrum))
    {
        hgSpectrum.Sub(darkSpectrum);
    }

    this->m_inputSpectrum = std::vector<double>(hgSpectrum.m_data, hgSpectrum.m_data + hgSpectrum.m_length);

    this->m_inputSpectrumContainsWavelength = hgSpectrum.m_wavelength.size() != 0;

    if (this->m_readWavelengthCalibrationFromFile)
    {
        // Find the peaks in the measured spectrum, rejecting double peaks.
        novac::FindEmissionLines(hgSpectrum, this->m_peaksFound, true);
        this->m_rejectedPeaks = novac::FilterByType(this->m_peaksFound, novac::SpectrumDataPointType::UnresolvedPeak);
        this->m_peaksFound = novac::FilterByType(this->m_peaksFound, novac::SpectrumDataPointType::Peak);

        this->m_wavelengthCalibrationCoefficients.clear();

        if (this->m_inputSpectrumContainsWavelength)
        {
            this->m_inputSpectrumWavelength = hgSpectrum.m_wavelength;
            this->m_wavelengthCalibrationSucceeded = true;
        }
        else
        {
            this->m_inputSpectrumWavelength.resize(hgSpectrum.m_length);
            std::iota(begin(m_inputSpectrumWavelength), end(m_inputSpectrumWavelength), 0.0);
            this->m_wavelengthCalibrationSucceeded = false;

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
            this->m_inputSpectrumWavelength = wavelengthCalibrationResult.pixelToWavelengthMapping;
            this->m_peaksFound = wavelengthCalibrationState.peaks;
            this->m_wavelengthCalibrationCoefficients = wavelengthCalibrationResult.pixelToWavelengthMappingCoefficients;
            this->m_rejectedPeaks = wavelengthCalibrationState.rejectedPeaks;
            this->m_wavelengthCalibrationSucceeded = true;
        }
        else
        {
            this->m_inputSpectrumWavelength.resize(hgSpectrum.m_length);
            std::iota(begin(m_inputSpectrumWavelength), end(m_inputSpectrumWavelength), 0.0);
            this->m_wavelengthCalibrationSucceeded = false;
            this->m_wavelengthCalibrationCoefficients.clear();
            this->m_rejectedPeaks.clear();
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

std::unique_ptr<novac::CSpectrum> InstrumentLineshapeCalibrationController::GetMercuryPeak(size_t peakIdx, double* baselinePtr, size_t* startIdx) const
{
    if (peakIdx >= this->m_peaksFound.size())
    {
        throw std::invalid_argument("Invalid index of mercury peak, please select a peak and try again");
    }

    // Extract the data surrounding the peak
    const novac::SpectrumDataPoint selectedPeak = this->m_peaksFound[peakIdx];
    const double leftWidth = selectedPeak.pixel - selectedPeak.leftPixel;
    const double rightWidth = selectedPeak.rightPixel - selectedPeak.pixel;

    const size_t spectrumStartIdx = static_cast<size_t>(std::max(0, static_cast<int>(std::round(selectedPeak.pixel - 3 * leftWidth))));
    const size_t spectralDataLength = std::min(this->m_inputSpectrum.size() - spectrumStartIdx, static_cast<size_t>(std::round(3 * (leftWidth + rightWidth))));
    const double* spectralData = this->m_inputSpectrum.data() + spectrumStartIdx;
    const double* wavelengthData = this->m_inputSpectrumWavelength.data() + spectrumStartIdx;

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

std::unique_ptr<novac::CSpectrum> InstrumentLineshapeCalibrationController::GetInstrumentLineShape(size_t peakIdx) const
{
    if (peakIdx >= this->m_peaksFound.size())
    {
        throw std::invalid_argument("Invalid index of mercury peak, please select a peak and try again");
    }

    if (this->m_fittedLineShape.first == LineShapeFunction::None)
    {
        // Get the measured peak itself
        double ignoredBaseline = 0.0;
        size_t startIdx = 0;
        auto hgLine = this->GetMercuryPeak(peakIdx, &ignoredBaseline, &startIdx);
        novac::Normalize(*hgLine);

        return hgLine;
    }
    else
    {
        return std::make_unique<novac::CSpectrum>(*m_sampledLineShapeFunction);
    }
}

std::string ToString(double value)
{
    char buffer[64];
    sprintf(buffer, "%0.3lf", value);
    return std::string(buffer);
}

std::vector<std::pair<std::string, std::string>> InstrumentLineshapeCalibrationController::GetFittedFunctionDescription() const
{
    std::vector<std::pair<std::string, std::string>> result;

    if (this->m_fittedLineShape.first == LineShapeFunction::Gaussian)
    {
        const auto func = static_cast<novac::GaussianLineShape*>(this->m_fittedLineShape.second);
        result.push_back(std::make_pair("center", ToString(func->center)));
        result.push_back(std::make_pair("sigma", ToString(func->sigma)));
        result.push_back(std::make_pair("fwhm", ToString(func->Fwhm())));
    }
    else if (this->m_fittedLineShape.first == LineShapeFunction::SuperGauss)
    {
        const auto func = static_cast<novac::SuperGaussianLineShape*>(this->m_fittedLineShape.second);
        result.push_back(std::make_pair("center", ToString(func->center)));
        result.push_back(std::make_pair("w", ToString(func->w)));
        result.push_back(std::make_pair("k", ToString(func->k)));
        result.push_back(std::make_pair("fwhm", ToString(func->Fwhm())));
    }

    return result;
}

void InstrumentLineshapeCalibrationController::FitFunctionToLineShape(size_t peakIdx, LineShapeFunction function)
{
    if (peakIdx >= this->m_peaksFound.size() || function == LineShapeFunction::None)
    {
        ClearFittedLineShape();
        return;
    }

    // Construct the spectrum we want to fit to
    double baseline = 0.0;
    size_t spectrumStartIdx = 0;
    auto hgLine = GetMercuryPeak(peakIdx, &baseline, &spectrumStartIdx);

    const size_t superSamplingRate = 1;
    if (function == LineShapeFunction::Gaussian)
    {
        auto gaussian = new novac::GaussianLineShape();
        novac::FitInstrumentLineShape(*hgLine, *gaussian);

        this->m_fittedLineShape = std::pair<LineShapeFunction, void*>(function, gaussian);

        std::vector<double> highResPixelData(superSamplingRate * hgLine->m_length); // super-sample with more points per pixel
        const double range = hgLine->m_wavelength.back() - hgLine->m_wavelength.front();
        for (size_t ii = 0; ii < highResPixelData.size(); ++ii)
        {
            highResPixelData[ii] = hgLine->m_wavelength[0] + ii * range / (double)(highResPixelData.size() - 1);
        }
        const std::vector<double> sampledLineShape = novac::SampleInstrumentLineShape(*gaussian, highResPixelData, m_peaksFound[peakIdx].wavelength, this->m_peaksFound[peakIdx].intensity - baseline, baseline);
        this->m_sampledLineShapeFunction = std::make_unique<novac::CSpectrum>(highResPixelData, sampledLineShape);
    }
    else if (function == LineShapeFunction::SuperGauss)
    {
        auto superGaussian = new novac::SuperGaussianLineShape();
        novac::FitInstrumentLineShape(*hgLine, *superGaussian);

        this->m_fittedLineShape = std::pair<LineShapeFunction, void*>(function, superGaussian);

        std::vector<double> highResPixelData(superSamplingRate * hgLine->m_length); // super-sample with more points per pixel
        const double range = hgLine->m_wavelength.back() - hgLine->m_wavelength.front();
        for (size_t ii = 0; ii < highResPixelData.size(); ++ii)
        {
            highResPixelData[ii] = hgLine->m_wavelength[0] + ii * range / (double)(highResPixelData.size() - 1);
        }
        const std::vector<double> sampledLineShape = novac::SampleInstrumentLineShape(*superGaussian, highResPixelData, m_peaksFound[peakIdx].wavelength, this->m_peaksFound[peakIdx].intensity - baseline, baseline);
        this->m_sampledLineShapeFunction = std::make_unique<novac::CSpectrum>(highResPixelData, sampledLineShape);
    }
    else if (function == LineShapeFunction::None)
    {
        this->m_fittedLineShape = std::pair<LineShapeFunction, void*>(function, nullptr);
        this->m_sampledLineShapeFunction.reset();
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
    if (peakIdx >= this->m_peaksFound.size())
    {
        throw std::invalid_argument("Invalid index of mercury peak, please select a peak and try again");
    }

    // Get the peak itself
    const novac::SpectrumDataPoint& selectedPeak = this->m_peaksFound[peakIdx];
    double ignoredBaseline = 0.0;
    size_t startIdx = 0;
    auto peak = this->GetMercuryPeak(peakIdx, &ignoredBaseline, &startIdx);
    novac::Normalize(*peak);

    // Additional information about the spectrum
    novac::CSTDFile::ExtendedFormatInformation extendedFileInfo;
    extendedFileInfo.MinChannel = static_cast<int>(startIdx);
    extendedFileInfo.MaxChannel = static_cast<int>(startIdx + peak->m_length);
    extendedFileInfo.MathLow = extendedFileInfo.MinChannel;
    extendedFileInfo.MathHigh = extendedFileInfo.MaxChannel;
    extendedFileInfo.Marker = selectedPeak.pixel;
    extendedFileInfo.calibrationPolynomial = this->m_wavelengthCalibrationCoefficients;

    // Get the full spectrum to save
    std::unique_ptr<novac::CSpectrum> spectrumToSave;
    if (m_sampledLineShapeFunction)
    {
        std::unique_ptr<novac::CSpectrum> hgLine = std::make_unique<novac::CSpectrum>(*m_sampledLineShapeFunction);
        novac::Normalize(*hgLine);

        // Extend this into a full spectrum
        std::vector<double> extendedPeak(m_inputSpectrum.size(), 0.0);
        std::copy(hgLine->m_data, hgLine->m_data + hgLine->m_length, extendedPeak.begin() + startIdx);

        spectrumToSave = std::make_unique<novac::CSpectrum>(m_inputSpectrumWavelength, extendedPeak);

        // Save the information we have on the function fit
        if (this->m_fittedLineShape.first == LineShapeFunction::Gaussian)
        {
            const auto& gaussian = static_cast<novac::GaussianLineShape*>(this->m_fittedLineShape.second);
            extendedFileInfo.additionalProperties.push_back(FormatProperty("GaussFitSigma", gaussian->sigma));
            extendedFileInfo.additionalProperties.push_back(FormatProperty("GaussFitCenter", gaussian->center));
        }
        else if (this->m_fittedLineShape.first == LineShapeFunction::SuperGauss)
        {
            const auto& superGaussian = static_cast<novac::SuperGaussianLineShape*>(this->m_fittedLineShape.second);
            extendedFileInfo.additionalProperties.push_back(FormatProperty("SuperGaussFitWidth", superGaussian->w));
            extendedFileInfo.additionalProperties.push_back(FormatProperty("SuperGaussFitPower", superGaussian->k));
            extendedFileInfo.additionalProperties.push_back(FormatProperty("SuperGaussFitCenter", superGaussian->center));
        }
    }
    else
    {
        // Extend this into a full spectrum
        std::vector<double> extendedPeak(m_inputSpectrum.size(), 0.0);
        std::copy(peak->m_data, peak->m_data + peak->m_length, extendedPeak.begin() + startIdx);

        spectrumToSave = std::make_unique<novac::CSpectrum>(m_inputSpectrumWavelength, extendedPeak);
    }

    spectrumToSave->m_info = this->m_inputspectrumInformation;

    novac::CSTDFile::WriteSpectrum(*spectrumToSave, filename, extendedFileInfo);
}

void InstrumentLineshapeCalibrationController::SaveResultAsClb(const std::string& filename)
{
    novac::SaveDataToFile(filename, this->m_inputSpectrumWavelength);
}

void InstrumentLineshapeCalibrationController::SaveResultAsSlf(size_t peakIdx, const std::string& filename)
{
    auto peak = this->GetInstrumentLineShape(peakIdx);

    // Differentiate the wavelength wrt the peak
    const double centerWavelength = this->m_peaksFound[peakIdx].wavelength;
    for (double& lambda : peak->m_wavelength)
    {
        lambda -= centerWavelength;
    }

    novac::SaveCrossSectionFile(filename, *peak);
}
