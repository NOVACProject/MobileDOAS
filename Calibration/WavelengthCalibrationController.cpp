#include "StdAfx.h"

#undef max
#undef min

#include "WavelengthCalibrationController.h"
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/Calibration/WavelengthCalibration.h>

// TODO: It should be possible to remove this header...
#include <SpectralEvaluation/Calibration/WavelengthCalibrationByRansac.h>

WavelengthCalibrationController::WavelengthCalibrationController()
{

}

void WavelengthCalibrationController::RunCalibration()
{
    novac::WavelengthCalibrationSettings settings;
    settings.highResSolarAtlas = this->m_solarSpectrum;
    settings.initialPixelToWavelengthMapping = novac::GetPixelToWavelengthMappingFromFile(this->m_initialWavelengthCalibration);

    novac::WavelengthCalibrationSetup setup{ settings };

}