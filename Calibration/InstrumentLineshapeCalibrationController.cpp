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
#include <SpectralEvaluation/Interpolation.h>

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

    const double minimumWavelength = (hgSpectrum.m_wavelength.size() != 0) ? hgSpectrum.m_wavelength.front() : 280.0;
    const double maximumWavelength = (hgSpectrum.m_wavelength.size() != 0) ? hgSpectrum.m_wavelength.back() : 420.0;

    if (false) // if (hgSpectrum.m_wavelength.size() != 0) // TODO: Option from the user
    {
        this->m_inputSpectrumContainsWavelength = true;
        this->m_inputSpectrumWavelength = hgSpectrum.m_wavelength;

        // Find the peaks in the measured spectrum
        novac::FindEmissionLines(hgSpectrum, m_peaksFound);
    }
    else
    {
        this->m_inputSpectrumContainsWavelength = false;

        // If the measured spectrum does _not_ contain a pixel-to-wavelength calibration already
        // then make a guess for this from the distance of the peaks
        novac::SpectrometerCalibrationResult wavelengthCalibrationResult;
        novac::MercurySpectrumCalibrationState wavelengthCalibrationState;
        if (novac::MercuryCalibration(hgSpectrum, 3, minimumWavelength, maximumWavelength, wavelengthCalibrationResult, &wavelengthCalibrationState))
        {
            this->m_inputSpectrumWavelength = wavelengthCalibrationResult.pixelToWavelengthMapping;
            this->m_peaksFound = wavelengthCalibrationState.peaks;
        }
        else
        {
            this->m_inputSpectrumWavelength.resize(hgSpectrum.m_length);
            std::iota(begin(m_inputSpectrumWavelength), end(m_inputSpectrumWavelength), 0.0);
        }

        // Remap the points from the (possible) wavelength calibration of the measured spectrum to
        //  our own (home brewn) pixel-to-wavelength calibration.   
        for (auto& peak : m_peaksFound)
        {
            novac::LinearInterpolation(this->m_inputSpectrumWavelength, peak.pixel, peak.wavelength);
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
    const size_t spectrumStartIdx = static_cast<size_t>(std::max(0, static_cast<int>(std::round(this->m_peaksFound[peakIdx].pixel - 50))));
    const size_t spectralDataLength = std::min(this->m_inputSpectrum.size() - spectrumStartIdx, 100LLU);
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

    if (function == LineShapeFunction::Gaussian)
    {
        auto gaussian = new novac::GaussianLineShape();
        novac::FitInstrumentLineShape(*hgLine, *gaussian);

        this->m_fittedLineShape = std::pair<LineShapeFunction, void*>(function, gaussian);

        std::vector<double> highResPixelData(4 * hgLine->m_length); // super-sample with four points per pixel
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

        std::vector<double> highResPixelData(4 * hgLine->m_length); // super-sample with four points per pixel
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