#include "StdAfx.h"

#undef max
#undef min
#include <filesystem>
#include <sstream>
#include "WavelengthCalibrationController.h"
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/File/STDFile.h>
#include <SpectralEvaluation/Calibration/WavelengthCalibration.h>
#include <SpectralEvaluation/Calibration/InstrumentLineShapeEstimation.h>
#include <SpectralEvaluation/Calibration/FraunhoferSpectrumGeneration.h>
#include <SpectralEvaluation/Interpolation.h>

// TODO: It should be possible to remove this header...
#include <SpectralEvaluation/Calibration/WavelengthCalibrationByRansac.h>

// From InstrumentLineShapeCalibrationController...
std::vector<std::pair<std::string, std::string>> GetFunctionDescription(const novac::ParametricInstrumentLineShape* lineShapeFunction);

WavelengthCalibrationController::WavelengthCalibrationController()
    : m_calibrationDebug(0U),
    m_instrumentLineShapeFitOption(InstrumentLineShapeFitOption::None)
{
}

WavelengthCalibrationController::~WavelengthCalibrationController()
{
    if (m_measuredInstrumentLineShape != nullptr)
    {
        delete m_measuredInstrumentLineShape;
        m_measuredInstrumentLineShape = nullptr;
    }
    if (m_resultingInstrumentLineShape != nullptr)
    {
        delete m_resultingInstrumentLineShape;
        m_resultingInstrumentLineShape = nullptr;
    }
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

    novac::InstrumentLineShapeEstimationFromKeypointDistance ilsEstimation{ settings.initialPixelToWavelengthMapping };

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

    // subtract the dark-spectrum (if this has been provided)
    novac::CSpectrum darkSpectrum;
    if (novac::ReadSpectrum(this->m_darkSpectrumFile, darkSpectrum))
    {
        measuredSpectrum.Sub(darkSpectrum);
    }

    if (novac::GetFileExtension(this->m_initialCalibrationFile).compare(".std") == 0)
    {
        if (m_measuredInstrumentLineShape != nullptr)
        {
            delete m_measuredInstrumentLineShape;
        }

        std::unique_ptr<novac::CSpectrum> instrumentLineShapeSpec = std::make_unique<novac::CSpectrum>();
        if (!novac::ReadInstrumentCalibration(this->m_initialCalibrationFile, *instrumentLineShapeSpec, settings.initialPixelToWavelengthMapping))
        {
            throw std::invalid_argument("Failed to read the instrument calibration file");
        }

        // Convert CSpectrum to CCrossSectionData
        this->m_measuredInstrumentLineShape = new novac::CCrossSectionData(*instrumentLineShapeSpec);

        settings.initialInstrumentLineShape = *m_measuredInstrumentLineShape;
        settings.estimateInstrumentLineShape = novac::InstrumentLineshapeEstimationOption::SuperGaussian;
    }
    else
    {
        settings.initialPixelToWavelengthMapping = novac::GetPixelToWavelengthMappingFromFile(this->m_initialCalibrationFile);

        if (this->m_initialLineShapeFile.size() > 0)
        {
            ReadInstrumentLineShape(m_initialLineShapeFile, settings);
            this->m_measuredInstrumentLineShape = new novac::CCrossSectionData(settings.initialInstrumentLineShape);
        }
        else
        {
            CreateGuessForInstrumentLineShape(this->m_solarSpectrumFile, measuredSpectrum, settings);
            this->m_measuredInstrumentLineShape = new novac::CCrossSectionData(settings.initialInstrumentLineShape);
        }
    }
    if (this->m_instrumentLineShapeFitOption == WavelengthCalibrationController::InstrumentLineShapeFitOption::SuperGaussian)
    {
        if (this->m_instrumentLineShapeFitRegion.first > this->m_instrumentLineShapeFitRegion.second)
        {
            std::stringstream msg;
            msg << "Invalid region for fitting the instrument line shape ";
            msg << "(" << this->m_instrumentLineShapeFitRegion.first << ", " << this->m_instrumentLineShapeFitRegion.second << ") [nm]. ";
            msg << "From must be smaller than To";
            throw std::invalid_argument(msg.str());
        }
        if (this->m_instrumentLineShapeFitRegion.first < settings.initialPixelToWavelengthMapping.front() ||
            this->m_instrumentLineShapeFitRegion.second > settings.initialPixelToWavelengthMapping.back())
        {
            std::stringstream msg;
            msg << "Invalid region for fitting the instrument line shape ";
            msg << "(" << this->m_instrumentLineShapeFitRegion.first << ", " << this->m_instrumentLineShapeFitRegion.second << ") [nm]. ";
            msg << "Region does not overlap initial pixel to wavelength calibration: ";
            msg << "(" << settings.initialPixelToWavelengthMapping.front() << ", " << settings.initialPixelToWavelengthMapping.back() << ") [nm]. ";
            throw std::invalid_argument(msg.str());
        }

        settings.estimateInstrumentLineShape = novac::InstrumentLineshapeEstimationOption::SuperGaussian;

        // Convert the region in nm to pixels.
        settings.estimateInstrumentLineShapePixelRegion.first = novac::GetFractionalIndex(settings.initialPixelToWavelengthMapping, this->m_instrumentLineShapeFitRegion.first);
        settings.estimateInstrumentLineShapePixelRegion.second = novac::GetFractionalIndex(settings.initialPixelToWavelengthMapping, this->m_instrumentLineShapeFitRegion.second);
    }
    else
    {
        settings.estimateInstrumentLineShape = novac::InstrumentLineshapeEstimationOption::None;
    }


    // So far no cross sections provided...

    novac::WavelengthCalibrationSetup setup{ settings };
    auto result = setup.DoWavelengthCalibration(measuredSpectrum);

    // Copy out the result
    this->m_resultingPixelToWavelengthMapping = result.pixelToWavelengthMapping;
    this->m_resultingPixelToWavelengthMappingCoefficients = result.pixelToWavelengthMappingCoefficients;
    if (result.estimatedInstrumentLineShape.GetSize() > 0)
    {
        this->m_resultingInstrumentLineShape = new novac::CCrossSectionData(result.estimatedInstrumentLineShape);
    }
    if (result.estimatedInstrumentLineShapeParameters != nullptr)
    {
        this->m_instrumentLineShapeParameterDescriptions = GetFunctionDescription(&(*result.estimatedInstrumentLineShapeParameters));
    }

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
                this->m_calibrationDebug.inlierCorrespondenceMeasuredIntensity.push_back(calibrationDebug.measuredKeypoints[corr.measuredIdx].intensity);
                this->m_calibrationDebug.inlierCorrespondenceFraunhoferIntensity.push_back(calibrationDebug.fraunhoferKeypoints[corr.theoreticalIdx].intensity);

                this->m_calibrationDebug.measuredSpectrumInlierKeypointPixels.push_back(calibrationDebug.measuredKeypoints[corr.measuredIdx].pixel);
                this->m_calibrationDebug.measuredSpectrumInlierKeypointIntensities.push_back(calibrationDebug.measuredKeypoints[corr.measuredIdx].intensity);

                this->m_calibrationDebug.fraunhoferSpectrumInlierKeypointWavelength.push_back(calibrationDebug.fraunhoferKeypoints[corr.theoreticalIdx].wavelength);
                this->m_calibrationDebug.fraunhoferSpectrumInlierKeypointIntensities.push_back(calibrationDebug.fraunhoferKeypoints[corr.theoreticalIdx].intensity);
            }
            else
            {
                this->m_calibrationDebug.outlierCorrespondencePixels.push_back(corr.measuredValue);
                this->m_calibrationDebug.outlierCorrespondenceWavelengths.push_back(corr.theoreticalValue);
            }
        }

        this->m_calibrationDebug.measuredSpectrum = std::vector<double>(calibrationDebug.measuredSpectrum->m_data, calibrationDebug.measuredSpectrum->m_data + calibrationDebug.measuredSpectrum->m_length);
        this->m_calibrationDebug.fraunhoferSpectrum = std::vector<double>(calibrationDebug.fraunhoferSpectrum->m_data, calibrationDebug.fraunhoferSpectrum->m_data + calibrationDebug.fraunhoferSpectrum->m_length);

        for (const auto& pt : calibrationDebug.measuredKeypoints)
        {
            this->m_calibrationDebug.measuredSpectrumKeypointPixels.push_back(pt.pixel);
            this->m_calibrationDebug.measuredSpectrumKeypointIntensities.push_back(pt.intensity);
        }

        for (const auto& pt : calibrationDebug.fraunhoferKeypoints)
        {
            this->m_calibrationDebug.fraunhoferSpectrumKeypointWavelength.push_back(pt.wavelength);
            this->m_calibrationDebug.fraunhoferSpectrumKeypointIntensities.push_back(pt.intensity);
        }
    }
}


std::pair<std::string, std::string> FormatProperty(const char* name, double value);

void WavelengthCalibrationController::SaveResultAsStd(const std::string& filename)
{
    // Get the peak itself
    size_t startIdx = 0; // TODO: Set this value to something more reasonable

    // Additional information about the spectrum
    // TODO: SAVE THE FITTED INSTRUMENT LINE SHAPE INSTEAD!
    novac::CSTDFile::ExtendedFormatInformation extendedFileInfo;
    extendedFileInfo.MinChannel = static_cast<int>(startIdx);
    extendedFileInfo.MaxChannel = static_cast<int>(startIdx + m_measuredInstrumentLineShape->GetSize());
    extendedFileInfo.MathLow = extendedFileInfo.MinChannel;
    extendedFileInfo.MathHigh = extendedFileInfo.MaxChannel;
    novac::LinearInterpolation(m_resultingPixelToWavelengthMapping, 0.5 * m_measuredInstrumentLineShape->GetSize(), extendedFileInfo.Marker);
    extendedFileInfo.calibrationPolynomial = this->m_resultingPixelToWavelengthMappingCoefficients;

    // Create the spectrum to save from the instrument line-shape + the 
    std::unique_ptr<novac::CSpectrum> spectrumToSave;
    {
        // Extend the measured spectrum line shape into a full spectrum
        std::vector<double> extendedPeak(m_resultingPixelToWavelengthMapping.size(), 0.0);
        std::copy(m_measuredInstrumentLineShape->m_crossSection.data(), m_measuredInstrumentLineShape->m_crossSection.data() + m_measuredInstrumentLineShape->GetSize(), extendedPeak.begin() + startIdx);

        spectrumToSave = std::make_unique<novac::CSpectrum>(m_resultingPixelToWavelengthMapping, extendedPeak);
    }

    // spectrumToSave->m_info = this->m_inputspectrumInformation;

    novac::CSTDFile::WriteSpectrum(*spectrumToSave, filename, extendedFileInfo);
}

void WavelengthCalibrationController::SaveResultAsClb(const std::string& filename)
{
    novac::SaveDataToFile(filename, this->m_resultingPixelToWavelengthMapping);
}

void WavelengthCalibrationController::SaveResultAsSlf(const std::string& filename)
{
    novac::SaveCrossSectionFile(filename, *m_measuredInstrumentLineShape);
}
