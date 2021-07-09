#include "StdAfx.h"

#undef min
#undef max

#include <algorithm>
#include <numeric>
#include "InstrumentLineshapeCalibrationController.h"
#include <SpectralEvaluation/File/File.h>
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
        // Find the peaks in the measured spectrum
        novac::FindEmissionLines(hgSpectrum, this->m_peaksFound);
        this->m_rejectedPeaks.clear();

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
            // this->m_rejectedPeaks = wavelengthCalibrationState.unknownPeaks;
            this->m_wavelengthCalibrationSucceeded = true;
        }
        else
        {
            this->m_inputSpectrumWavelength.resize(hgSpectrum.m_length);
            std::iota(begin(m_inputSpectrumWavelength), end(m_inputSpectrumWavelength), 0.0);
            this->m_wavelengthCalibrationSucceeded = false;
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

std::unique_ptr<novac::CSpectrum> InstrumentLineshapeCalibrationController::GetMercuryPeak(size_t peakIdx, double* baselinePtr, size_t* startIdx)
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

std::unique_ptr<novac::CSpectrum> InstrumentLineshapeCalibrationController::GetInstrumentLineShape(size_t peakIdx)
{
    if (peakIdx >= this->m_peaksFound.size())
    {
        throw std::invalid_argument("Invalid index of mercury peak, please select a peak and try again");
    }

    if (m_sampledLineShapeFunction)
    {
        std::unique_ptr<novac::CSpectrum> hgLine = std::make_unique<novac::CSpectrum>(*m_sampledLineShapeFunction);
        novac::Normalize(*hgLine);

        return hgLine;
    }
    else
    {
        return this->GetMercuryPeak(peakIdx);
    }
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

    const size_t superSamplingRate = 4;
    if (function == LineShapeFunction::Gaussian)
    {
        auto gaussian = new novac::GaussianLineShape();
        novac::FitInstrumentLineShape(*hgLine, *gaussian);

        this->m_fittedLineShape = std::pair<LineShapeFunction, void*>(function, gaussian);

        std::vector<double> highResPixelData(superSamplingRate * hgLine->m_length); // super-sample with more points per pixel
        const double range = hgLine->m_wavelength[hgLine->m_wavelength.size() - 1] - hgLine->m_wavelength[0];
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
        const double range = hgLine->m_wavelength[hgLine->m_wavelength.size() - 1] - hgLine->m_wavelength[0];
        for (size_t ii = 0; ii < highResPixelData.size(); ++ii)
        {
            highResPixelData[ii] = hgLine->m_wavelength[0] + ii * range / (double)(highResPixelData.size() - 1);
        }
        const std::vector<double> sampledLineShape = novac::SampleInstrumentLineShape(*superGaussian, highResPixelData, m_peaksFound[peakIdx].wavelength, this->m_peaksFound[peakIdx].intensity - baseline, baseline);
        this->m_sampledLineShapeFunction = std::make_unique<novac::CSpectrum>(highResPixelData, sampledLineShape);
    }
    else
    {
        throw std::invalid_argument("Invalid type of function selected");
    }
}

void InstrumentLineshapeCalibrationController::FilterPeaks(
    const std::vector<novac::SpectrumDataPoint>& peaks,
    std::vector<novac::SpectrumDataPoint>& good,
    std::vector<novac::SpectrumDataPoint>& rejected)
{
    good.clear();
    good.reserve(peaks.size());
    rejected.clear();
    rejected.reserve(peaks.size());

    for (const auto& peak : peaks)
    {
        if (peak.flatTop)
        {
            // This peak is saturated and is hence not a good ILS candidate
            rejected.push_back(peak);
            continue;
        }

        // Check the data around the peak to assess wether the peaks is isolated or part of a double-peak
        bool isIsolated = true;
        const size_t leftWidth = static_cast<size_t>(peak.pixel - peak.leftPixel);
        const size_t rightWidth = static_cast<size_t>(peak.rightPixel - peak.pixel);
        const size_t peakIdx = static_cast<size_t>(peak.pixel);
        for (size_t ii = peakIdx; ii < peakIdx + 2 * rightWidth; ++ii)
        {
            if (m_inputSpectrum[ii] - m_inputSpectrum[ii + 1] < -0.05 * peak.intensity)
            {
                // The intensity starts to go up again, may be secondary peak
                isIsolated = false;
                break;
            }
        }

        if (!isIsolated)
        {
            rejected.push_back(peak);
            continue;
        }

        for (size_t ii = peakIdx - 2 * leftWidth; ii < peakIdx; ++ii)
        {
            if (m_inputSpectrum[ii + 1] - m_inputSpectrum[ii] < -0.05 * peak.intensity)
            {
                // The intensity starts to go down again, may be secondary peak
                isIsolated = false;
                break;
            }
        }

        if (!isIsolated)
        {
            rejected.push_back(peak);
            continue;
        }

        // All seems good
        good.push_back(peak);
    }
}

