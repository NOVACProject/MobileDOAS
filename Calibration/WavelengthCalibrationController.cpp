#include "StdAfx.h"

#undef max
#undef min

#include "WavelengthCalibrationController.h"
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/Calibration/WavelengthCalibration.h>
#include <SpectralEvaluation/Calibration/InstrumentLineShapeEstimation.h>
#include <SpectralEvaluation/Calibration/FraunhoferSpectrumGeneration.h>

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
    settings.estimateInstrumentLineShape = novac::InstrumentLineshapeEstimationOption::None; // So far...
    // So far no cross sections provided...

    novac::CSpectrum measuredSpectrum;
    if (!novac::ReadSpectrum(this->m_inputSpectrumFile, measuredSpectrum))
    {
        throw std::invalid_argument("Cannot read the provided input spectrum file");
    }

    if (this->m_initialLineShapeFile.size() > 0)
    {
        novac::CCrossSectionData measuredInstrumentLineShape;
        if (!novac::ReadCrossSectionFile(this->m_initialLineShapeFile, measuredInstrumentLineShape))
        {
            throw std::invalid_argument("Cannot read the provided instrument lineshape file");
        }
        settings.initialInstrumentLineShape = measuredInstrumentLineShape;
        settings.estimateInstrumentLineShape = novac::InstrumentLineshapeEstimationOption::None;
    }
    else
    {
        // The user has not supplied an instrument-line-shape, create a guess for one.
        std::vector<std::pair<std::string, double>> noCrossSections;
        novac::FraunhoferSpectrumGeneration fraunhoferSpectrumGen{ this->m_solarSpectrumFile, noCrossSections };

        novac::InstrumentLineShapeEstimation ilsEstimation{ settings.initialPixelToWavelengthMapping };

        double resultInstrumentFwhm;
        ilsEstimation.EstimateInstrumentLineShape(fraunhoferSpectrumGen, measuredSpectrum, settings.initialInstrumentLineShape, resultInstrumentFwhm);

        // no need to estimate further (or is it?)
        settings.estimateInstrumentLineShape = novac::InstrumentLineshapeEstimationOption::None;
    }

    novac::WavelengthCalibrationSetup setup{ settings };
    auto result = setup.DoWavelengthCalibration(measuredSpectrum);

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