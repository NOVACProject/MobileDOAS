#include "StdAfx.h"
#include "ReferenceCreationController.h"
#include <SpectralEvaluation/Evaluation/BasicMath.h>
#include <SpectralEvaluation/Evaluation/CrossSectionData.h>
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/Calibration/ReferenceSpectrumConvolution.h>

ReferenceCreationController::ReferenceCreationController()
{

}

void ReferenceCreationController::ConvolveReference()
{
    auto conversion = (m_convertToAir) ? novac::WavelengthConversion::VacuumToAir : novac::WavelengthConversion::None;

    // Read the calibration
    novac::CSpectrum instrumentLineShapeSpectrum;
    std::vector<double> pixelToWavelengthMapping;
    if (!novac::ReadInstrumentCalibration(this->m_calibrationFile, instrumentLineShapeSpectrum, pixelToWavelengthMapping))
    {
        throw std::invalid_argument("Failed to read the instrument calibration file");
    }

    // Red the reference
    novac::CCrossSectionData highResReference;
    if (!novac::ReadCrossSectionFile(this->m_highResolutionCrossSection, highResReference))
    {
        throw std::exception("Failed to read the reference file");
    }

    // Do the convolution
    novac::CCrossSectionData instrumentLineShape(instrumentLineShapeSpectrum);
    std::vector<double> convolutionResult;
    if (!novac::ConvolveReference(pixelToWavelengthMapping, instrumentLineShape, highResReference, convolutionResult, conversion))
    {
        throw std::exception("Failed to convolve the references");
    }

    // Combine the results into the final output cross section data 
    this->m_resultingCrossSection = std::make_unique<novac::CCrossSectionData>();
    this->m_resultingCrossSection->m_crossSection = convolutionResult;
    this->m_resultingCrossSection->m_waveLength = pixelToWavelengthMapping;

    if (this->m_highPassFilter)
    {
        CBasicMath math;

        const int length = (int)this->m_resultingCrossSection->m_crossSection.size();

        math.Mul(this->m_resultingCrossSection->m_crossSection.data(), length, -2.5e15);
        math.Delog(this->m_resultingCrossSection->m_crossSection.data(), length);
        math.HighPassBinomial(this->m_resultingCrossSection->m_crossSection.data(), length, 500);
        math.Log(this->m_resultingCrossSection->m_crossSection.data(), length);
    }

}