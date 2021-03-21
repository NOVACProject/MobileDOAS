#include "StdAfx.h"
#include "ReferenceCreationController.h"
#include <SpectralEvaluation/Evaluation/BasicMath.h>
#include <SpectralEvaluation/Evaluation/CrossSectionData.h>
#include <SpectralEvaluation/Calibration/ReferenceSpectrumConvolution.h>

ReferenceCreationController::ReferenceCreationController()
{

}

void ReferenceCreationController::ConvolveReference()
{
    this->m_resultingCrossSection = std::make_unique<novac::CCrossSectionData>();

    auto conversion = (m_convertToAir) ? novac::WavelengthConversion::VacuumToAir : novac::WavelengthConversion::None;

    if (!novac::ConvolveReference(this->m_wavelengthCalibrationFile, this->m_instrumentLineshapeFile, this->m_highResolutionCrossSection, *(this->m_resultingCrossSection), conversion))
    {
        // TODO: Get a reason why here
        throw std::exception("Failed to convolve the references");
    }

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