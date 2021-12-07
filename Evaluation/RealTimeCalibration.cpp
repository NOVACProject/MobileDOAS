#include "StdAfx.h"
#include "RealTimeCalibration.h"
#include "../Common.h"
#include "../Configuration/MobileConfiguration.h"
#include "../Configuration/ConfigurationFile.h"
#include <SpectralEvaluation/Evaluation/ReferenceFile.h>
#include <SpectralEvaluation/DialogControllers/ReferenceCreationController.h>
#include <SpectralEvaluation/DialogControllers/WavelengthCalibrationController.h>
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/Calibration/StandardCrossSectionSetup.h>

#include <sstream>
#include <afxstr.h>

using namespace Evaluation;

// ------------- Free functions used to help out with the calibration -------------
/*

std::string GetLogFileName()
{
    std::stringstream debugFile;
    debugFile << GetOutputDirectory() << "Debug_InstrumentCalibration.txt";
    return debugFile.str();
}
*/

std::string FormatDateAndTimeOfSpectrum(const novac::CSpectrumInfo& spectrumInformation)
{
    CString dateAndTime;

    dateAndTime.Format(
        "%02d%02d%02d_%02d%02d",
        spectrumInformation.m_startTime.year % 1000,
        spectrumInformation.m_startTime.month,
        spectrumInformation.m_startTime.day,
        spectrumInformation.m_startTime.hour,
        spectrumInformation.m_startTime.minute);

    return std::string(dateAndTime);
}

std::string GetCalibrationFileName(const novac::CSpectrumInfo& spectrumInformation)
{
    return "Calibration_" + spectrumInformation.m_device + "_" + FormatDateAndTimeOfSpectrum(spectrumInformation) + ".std";
}

void RunCalibration(
    MobileDoasWavelengthCalibrationController& calibrationController,
    const double* measuredSpectrum,
    const double* darkSpectrum,
    size_t spectrumLength, 
    const Configuration::CMobileConfiguration::AutomaticCalibration& autoCalibrationSettings)
{
    // calibrationController.m_inputSpectrumFile = scanFile;
    // calibrationController.m_darkSpectrumFile = darkSpectrum;
    calibrationController.m_solarSpectrumFile = autoCalibrationSettings.m_solarSpectrumFile;
    calibrationController.m_initialCalibrationFile = autoCalibrationSettings.m_initialCalibrationFile;
    calibrationController.m_initialLineShapeFile = autoCalibrationSettings.m_instrumentLineshapeFile;
    calibrationController.m_instrumentLineShapeFitOption = (WavelengthCalibrationController::InstrumentLineShapeFitOption)autoCalibrationSettings.m_instrumentLineShapeFitOption;
    calibrationController.m_instrumentLineShapeFitRegion = autoCalibrationSettings.m_instrumentLineShapeFitRegion;

    // Does the actual calibration. Throws a std::exception if the calibration fails.
    calibrationController.RunCalibration();
}

std::vector<novac::CReferenceFile> CreateStandardReferences(
    const novac::CSpectrumInfo& spectrumInformation,
    const std::unique_ptr<novac::InstrumentCalibration>& calibration,
    const std::string& directoryName,
    Configuration::CMobileConfiguration& settings)
{
    Common common;
    common.GetExePath();
    std::string exePath = common.m_exePath;
    novac::StandardCrossSectionSetup standardCrossSections{ exePath };

    ReferenceCreationController referenceController;
    std::vector<novac::CReferenceFile> referencesCreated;

    for (size_t ii = 0; ii < standardCrossSections.NumberOfReferences(); ++ii)
    {
        referenceController.m_highPassFilter = settings.m_calibration.m_filterReferences;
        referenceController.m_convertToAir = standardCrossSections.IsReferenceInVacuum(ii);
        referenceController.m_highResolutionCrossSection = standardCrossSections.ReferenceFileName(ii);
        referenceController.ConvolveReference(*calibration);

        // Save the result
        const std::string filteringStr = (settings.m_calibration.m_filterReferences) ?
            "_HP500_PPMM" :
            "";
        const std::string dstFileName =
            directoryName +
            spectrumInformation.m_device +
            "_" +
            standardCrossSections.ReferenceSpecieName(ii) +
            filteringStr +
            "_" +
            FormatDateAndTimeOfSpectrum(spectrumInformation) +
            ".txt";
        novac::SaveCrossSectionFile(dstFileName, *(referenceController.m_resultingCrossSection));

        novac::CReferenceFile newReference;
        newReference.m_specieName = standardCrossSections.ReferenceSpecieName(ii);
        newReference.m_path = dstFileName;
        newReference.m_isFiltered = settings.m_calibration.m_filterReferences;
        referencesCreated.push_back(newReference);
    }

    return referencesCreated;
}

void ReplaceReferences(std::vector<novac::CReferenceFile>& newReferences, Configuration::CMobileConfiguration& settings)
{
    // First make sure that all the cross sections could be read in before attempting to replace anything
    for (size_t idx = 0; idx < newReferences.size(); ++idx)
    {
        if (newReferences[idx].ReadCrossSectionDataFromFile())
        {
            std::stringstream message;
            message << "Failed to read cross section data from file: " << newReferences[idx].m_path << std::endl;
            throw std::invalid_argument(message.str());
        }

        if (newReferences[idx].m_data == nullptr ||
            newReferences[idx].m_data->m_crossSection.size() == 0)
        {
            std::stringstream message;
            message << "The cross section was read from file but contained no data. File: " << newReferences[idx].m_path << std::endl;
            throw std::invalid_argument(message.str());
        }
    }

    // Now replace the references. Notice that this only supports replacing the references of the first channel.
    auto& fitWindow = settings.m_fitWindow[0];
    fitWindow.nRef = static_cast<int>(newReferences.size());
    for (size_t idx = 0; idx < newReferences.size(); ++idx)
    {
        fitWindow.ref[idx] = newReferences[idx];
    }
}

bool CRealTimeCalibration::RunInstrumentCalibration(
    const double* measuredSpectrum,
    const double* darkSpectrum,
    size_t spectrumLength,
    const std::string& outputDirectory,
    Configuration::CMobileConfiguration& settings)
{
    try
    {
        // Use the WavelengthCalibrationController, which is also used when the 
        //  user performs the instrument calibrations using the CCalibratePixelToWavelengthDialog.
        // This makes sure we get the same behavior in the dialog and here.
        MobileDoasWavelengthCalibrationController calibrationController;
        RunCalibration(calibrationController, measuredSpectrum, darkSpectrum, spectrumLength, settings.m_calibration);

        // Save new instrument calibration.
        const std::string calibrationFileName = outputDirectory + GetCalibrationFileName(calibrationController.m_calibrationDebug.spectrumInfo);
        calibrationController.SaveResultAsStd(calibrationFileName);

        // Create the standard references.
        const auto finalCalibration = calibrationController.GetFinalCalibration();
        auto referencesCreated = CreateStandardReferences(
            calibrationController.m_calibrationDebug.spectrumInfo,
            finalCalibration,
            outputDirectory,
            settings);

        // All references have successfully been created, replace the references used by the evaluation with the new references.
        if (settings.m_calibration.m_generateReferences && referencesCreated.size() > 0)
        {
            // Update the settings.
            ReplaceReferences(referencesCreated, settings);

            // Save the updated settings to file
            Configuration::ConfigurationFile::Write(settings);
        }

        return true; 
    }
    catch (std::exception& e)
    {
        // AppendMessageToLog(spectrometer, e.what());
    }
    return false;
}
