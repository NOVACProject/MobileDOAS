#include "StdAfx.h"
#include "InstrumentLineshapeCalibrationController.h"
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/File/File.h>

InstrumentLineshapeCalibrationController::InstrumentLineshapeCalibrationController()
    : m_inputSpectrumPath(""), m_darkSpectrumPath("")
{
}

void InstrumentLineshapeCalibrationController::Update()
{
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

    // Find the peaks in the measured spectrum
    double minimumIntensity = 1000; // TODO: Input
    novac::FindPeaks(hgSpectrum, minimumIntensity, m_peaksFound);
}