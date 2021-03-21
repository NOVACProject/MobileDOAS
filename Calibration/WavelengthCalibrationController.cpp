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
    : m_calibrationDebug(0U)
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

    // Copy out the result
    this->m_resultingPixelToWavelengthMapping = result.pixelToWavelengthMapping;
    this->m_resultingPixelToWavelengthMappingCoefficients = result.pixelToWavelengthMappingCoefficients;

    // Also copy out some debug information, which makes it possible for the user to inspect the calibration
    {
        const auto& calibrationDebug = setup.GetLastCalibrationSetup();
        this->m_calibrationDebug = WavelengthCalibrationDebugState(calibrationDebug.allCorrespondences.size());
        this->m_calibrationDebug.initialPixelToWavelengthMapping = settings.initialPixelToWavelengthMapping;

        for (size_t correspondenceIdx = 0; correspondenceIdx < calibrationDebug.allCorrespondences.size(); ++correspondenceIdx)
        {
            const auto& corr = calibrationDebug.allCorrespondences[correspondenceIdx]; // easier syntax.

            if (calibrationDebug.correspondenceIsInlier[correspondenceIdx])
            {
                this->m_calibrationDebug.inlierCorrespondencePixels.push_back(corr.measuredValue);
                this->m_calibrationDebug.inlierCorrespondenceWavelengths.push_back(corr.theoreticalValue);
            }
            else
            {
                this->m_calibrationDebug.outlierCorrespondencePixels.push_back(corr.measuredValue);
                this->m_calibrationDebug.outlierCorrespondenceWavelengths.push_back(corr.theoreticalValue);
            }
        }
    }
}