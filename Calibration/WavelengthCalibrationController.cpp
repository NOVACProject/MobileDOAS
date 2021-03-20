#include "StdAfx.h"

#undef max
#undef min

#include "WavelengthCalibrationController.h"
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/Calibration/WavelengthCalibration.h>

// TODO: It should be possible to remove this header...
#include <SpectralEvaluation/Calibration/WavelengthCalibrationByRansac.h>

WavelengthCalibrationController::WavelengthCalibrationController()
{
}

void WavelengthCalibrationController::RunCalibration()
{
    novac::WavelengthCalibrationSettings settings;
    settings.highResSolarAtlas = this->m_solarSpectrumFile;
    settings.initialPixelToWavelengthMapping = novac::GetPixelToWavelengthMappingFromFile(this->m_initialWavelengthCalibrationFile);
    // So far no cross sections provided...

    novac::CSpectrum measuredSpectrum;
    if (!novac::ReadSpectrum(this->m_inputSpectrumFile, measuredSpectrum))
    {
        throw std::invalid_argument("Cannot read the provided input spectrum file");
    }

    novac::CCrossSectionData measuredInstrumentLineShape;
    if (!novac::ReadCrossSectionFile(this->m_initialLineShapeFile, measuredInstrumentLineShape))
    {
        throw std::invalid_argument("Cannot read the provided input spectrum file");
    }

    novac::WavelengthCalibrationSetup setup{ settings };
    auto result = setup.DoWavelengthCalibration(measuredSpectrum, measuredInstrumentLineShape);

    this->m_resultingPixelToWavelengthMapping = result.pixelToWavelengthMapping;
    this->m_resultingPixelToWavelengthMappingCoefficients = result.pixelToWavelengthMappingCoefficients;

    // TODO: We need to be able to display something regarding the setup here, such as the used keypoints ...
}