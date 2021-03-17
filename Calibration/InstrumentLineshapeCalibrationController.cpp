#include "StdAfx.h"

#undef min
#undef max

#include <algorithm>
#include <numeric>
#include "InstrumentLineshapeCalibrationController.h"
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/Calibration/InstrumentLineShape.h>

InstrumentLineshapeCalibrationController::InstrumentLineshapeCalibrationController()
    : m_inputSpectrumPath(""), m_darkSpectrumPath(""), m_fittedLineShape(LineShapeFunction::None, nullptr)
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

    // TODO: Don't we also need to make sure that the spectrum contains wavelength data ? 
    this->m_inputSpectrum = std::vector<double>(hgSpectrum.m_data, hgSpectrum.m_data + hgSpectrum.m_length);

    // Find the peaks in the measured spectrum
    double minimumIntensity = 1000; // TODO: Input
    novac::FindPeaks(hgSpectrum, minimumIntensity, m_peaksFound);
}

void InstrumentLineshapeCalibrationController::FitFunctionToLineShape(size_t peakIdx, LineShapeFunction function)
{
    if (peakIdx >= this->m_peaksFound.size())
    {
        ClearFittedLineShape();
        return;
    }

    // Extract the data surrounding the peak
    const size_t spectrumStartIdx = static_cast<size_t>(std::max(0, static_cast<int>(std::round(this->m_peaksFound[peakIdx].pixel - 50))));
    const size_t spectralDataLength = std::min(this->m_inputSpectrum.size() - spectrumStartIdx, 100LLU);
    const double* spectralData = this->m_inputSpectrum.data() + spectrumStartIdx;
    std::vector<double> pixelData(spectralDataLength);
    std::iota(begin(pixelData), end(pixelData), (double)spectrumStartIdx);

    if (function == LineShapeFunction::None)
    {
        ClearFittedLineShape();
        return;
    }
    else if (function == LineShapeFunction::Gaussian)
    {
        novac::CSpectrum hgLine{ pixelData.data(), spectralData, spectralDataLength };

        auto gaussian = new novac::GaussianLineShape();
        novac::FitInstrumentLineShape(hgLine, *gaussian);

        this->m_fittedLineShape = std::pair<LineShapeFunction, void*>(function, gaussian);

        std::vector<double> highResPixelData(4 * spectralDataLength); // super-sample with four points per pixel
        for (size_t ii = 0; ii < highResPixelData.size(); ++ii)
        {
            highResPixelData[ii] = (double)spectrumStartIdx + 0.25 * ii;
        }
        const std::vector<double> sampledLineShape = novac::SampleInstrumentLineShape(*gaussian, highResPixelData, m_peaksFound[peakIdx].pixel, this->m_peaksFound[peakIdx].intensity);
        this->m_sampledLineShapeFunction = std::make_unique<novac::CSpectrum>(highResPixelData, sampledLineShape);
    }
    else if (function == LineShapeFunction::SuperGauss)
    {
        novac::CSpectrum hgLine{ pixelData.data(), spectralData, spectralDataLength };

        auto superGaussian = new novac::SuperGaussianLineShape();
        novac::FitInstrumentLineShape(hgLine, *superGaussian);

        this->m_fittedLineShape = std::pair<LineShapeFunction, void*>(function, superGaussian);

        std::vector<double> highResPixelData(4 * spectralDataLength); // super-sample with four points per pixel
        for (size_t ii = 0; ii < highResPixelData.size(); ++ii)
        {
            highResPixelData[ii] = (double)spectrumStartIdx + 0.25 * ii;
        }
        const std::vector<double> sampledLineShape = novac::SampleInstrumentLineShape(*superGaussian, highResPixelData, m_peaksFound[peakIdx].pixel, this->m_peaksFound[peakIdx].intensity);
        this->m_sampledLineShapeFunction = std::make_unique<novac::CSpectrum>(highResPixelData, sampledLineShape);
    }
    else
    {
        throw std::invalid_argument("Invalid type of function selected");
    }
}