#include "StdAfx.h"

#undef max
#undef min
#include <filesystem>
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


/// <summary>
/// Reads and parses m_initialLineShapeFile and saves the result to the provided settings
/// </summary>
void ReadInstrumentLineShape(const std::string& initialLineShapeFile, novac::WavelengthCalibrationSettings& settings)
{
    novac::CCrossSectionData measuredInstrumentLineShape;
    if (!novac::ReadCrossSectionFile(initialLineShapeFile, measuredInstrumentLineShape))
    {
        throw std::invalid_argument("Cannot read the provided instrument lineshape file");
    }
    settings.initialInstrumentLineShape = measuredInstrumentLineShape;
    settings.estimateInstrumentLineShape = novac::InstrumentLineshapeEstimationOption::None;
}

/// <summary>
/// Guesses for an instrment line shape from the measured spectrum. Useful if no measured instrument line shape exists
/// </summary>
void CreateGuessForInstrumentLineShape(const std::string& solarSpectrumFile, const novac::CSpectrum& measuredSpectrum, novac::WavelengthCalibrationSettings& settings)
{
    // The user has not supplied an instrument-line-shape, create a guess for one.
    std::vector<std::pair<std::string, double>> noCrossSections;
    novac::FraunhoferSpectrumGeneration fraunhoferSpectrumGen{ solarSpectrumFile, noCrossSections };

    novac::InstrumentLineShapeEstimation ilsEstimation{ settings.initialPixelToWavelengthMapping };

    double resultInstrumentFwhm;
    ilsEstimation.EstimateInstrumentLineShape(fraunhoferSpectrumGen, measuredSpectrum, settings.initialInstrumentLineShape, resultInstrumentFwhm);

    // no need to estimate further (or is it?)
    settings.estimateInstrumentLineShape = novac::InstrumentLineshapeEstimationOption::None;
}

void WavelengthCalibrationController::RunCalibration()
{
    novac::WavelengthCalibrationSettings settings;
    settings.highResSolarAtlas = this->m_solarSpectrumFile;

    novac::CSpectrum measuredSpectrum;
    if (!novac::ReadSpectrum(this->m_inputSpectrumFile, measuredSpectrum))
    {
        throw std::invalid_argument("Cannot read the provided input spectrum file");
    }

    if (novac::GetFileExtension(this->m_initialCalibrationFile).compare(".xml") == 0)
    {
        // TODO: Implement this properly
        novac::CSpectrum measuredInstrumentLineShapeSpectrum;
        if (!novac::ReadInstrumentCalibration(this->m_initialCalibrationFile, measuredInstrumentLineShapeSpectrum, settings.initialPixelToWavelengthMapping))
        {
            throw std::invalid_argument("Failed to read the instrument calibration file");
        }

        // Convert CSpectrum to CCrossSectionData
        novac::CCrossSectionData measuredInstrumentLineShape(measuredInstrumentLineShapeSpectrum);

        settings.initialInstrumentLineShape = measuredInstrumentLineShape;
        settings.estimateInstrumentLineShape = novac::InstrumentLineshapeEstimationOption::None;
    }
    else
    {
        settings.initialPixelToWavelengthMapping = novac::GetPixelToWavelengthMappingFromFile(this->m_initialCalibrationFile);

        if (this->m_initialLineShapeFile.size() > 0)
        {
            ReadInstrumentLineShape(m_initialLineShapeFile, settings);
        }
        else
        {
            CreateGuessForInstrumentLineShape(this->m_solarSpectrumFile, measuredSpectrum, settings);
        }
    }

    // So far no cross sections provided...

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