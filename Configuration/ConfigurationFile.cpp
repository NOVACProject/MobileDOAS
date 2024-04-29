#include "StdAfx.h"
#include "ConfigurationFile.h"
#include "MobileConfiguration.h"
#include <stdexcept>
#include <SpectralEvaluation/StringUtils.h>

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

using namespace Configuration;

void ConfigurationFile::Write(CMobileConfiguration& configuration)
{
    FILE* f = nullptr;
    CString fileName;

    // Get the filename (and path) of the configuration-file
    if (configuration.m_cfgFile.size() == 0)
    {
        fileName.Format("%s\\cfg.xml", (LPCTSTR)g_exePath);
    }
    else
    {
        const int p = Find(configuration.m_cfgFile, ".txt");
        if (p > 0)
        {
            fileName.Format("%s.xml", Left(configuration.m_cfgFile, p).c_str());
        }
        else
        {
            fileName.Format(configuration.m_cfgFile.c_str());
        }
    }

    // Try to open the file for writing
    f = fopen(fileName, "w");
    if (f == nullptr)
    {
        CString errMsg;
        errMsg.Format("Could not open file %s for writing.", (LPCTSTR)fileName);
        throw std::invalid_argument(errMsg);
    }

    // ---- Standard beginning ---
    fprintf(f, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
    fprintf(f, "<!-- This is the configuration file MobileDOAS -->\n\n");
    fprintf(f, "<Configuration>\n");

    // ------------ Spectrometer Settings ------------------
    if (configuration.m_spectrometerConnection == CMobileConfiguration::CONNECTION_RS232) {
        fprintf(f, "\t<serialPort>%s</serialPort>\n", configuration.m_serialPort.c_str());
        fprintf(f, "\t<serialBaudrate>%d</serialBaudrate>\n", configuration.m_baudrate);
    }
    else if (configuration.m_spectrometerConnection == CMobileConfiguration::CONNECTION_USB) {
        fprintf(f, "\t<serialPort>USB</serialPort>\n");
    }
    else if (configuration.m_spectrometerConnection == CMobileConfiguration::CONNECTION_DIRECTORY) {
        fprintf(f, "\t<serialPort>Directory</serialPort>\n");
    }
    fprintf(f, "\t<timeResolution>%ld</timeResolution>\n", configuration.m_timeResolution);
    fprintf(f, "\t<nchannels>%d</nchannels>\n", configuration.m_nChannels);

    fprintf(f, "\t<useAudio>%d</useAudio>\n", configuration.m_useAudio);
    fprintf(f, "\t<maxColumn>%.2lf</maxColumn>\n", configuration.m_maxColumn);
    fprintf(f, "\t<setPointTemperature>%.2lf</setPointTemperature>\n", configuration.m_setPointTemperature);

    // ------------ Settings for the Exposure-time -------------
    fprintf(f, "\t<Intensity>\n");
    fprintf(f, "\t\t<Percent>%ld</Percent>\n", configuration.m_percent);
    fprintf(f, "\t\t<Channel>%ld</Channel>\n", configuration.m_specCenter);
    if (configuration.m_expTimeMode == CMobileConfiguration::EXPOSURETIME_FIXED) {
        fprintf(f, "\t\t<FixExpTime>%d</FixExpTime>\n", configuration.m_fixExpTime);
    }
    else if (configuration.m_expTimeMode == CMobileConfiguration::EXPOSURETIME_ADAPTIVE) {
        fprintf(f, "\t\t<FixExpTime>-1</FixExpTime>\n");
    }
    else {
        fprintf(f, "\t\t<FixExpTime>0</FixExpTime>\n");
    }
    fprintf(f, "\t\t<saturationLow>%ld</saturationLow>\n", configuration.m_saturationLow);
    fprintf(f, "\t\t<saturationHigh>%ld</saturationHigh>\n", configuration.m_saturationHigh);
    fprintf(f, "\t</Intensity>\n");

    // ------------------- GPS-settings --------------------
    fprintf(f, "\t<GPS>\n");
    if (configuration.m_useGPS)
        fprintf(f, "\t\t<use>1</use>\n");
    else
        fprintf(f, "\t\t<use>0</use>\n");

    fprintf(f, "\t\t<baudrate>%ld</baudrate>\n", configuration.m_gpsBaudrate);
    fprintf(f, "\t\t<port>%s</port>\n", configuration.m_gpsPort.c_str());
    fprintf(f, "\t</GPS>\n");

    // ------------------- Offset-settings --------------------
    fprintf(f, "\t<Offset>\n");
    fprintf(f, "\t\t<from>%d</from>\n", configuration.m_offsetFrom);
    fprintf(f, "\t\t<to>%d</to>\n", configuration.m_offsetTo);
    fprintf(f, "\t</Offset>\n");
    fprintf(f, "\t<noDark>%d</noDark>\n", configuration.m_noDark);

    // ----------- Calibration ----------------

    fprintf(f, "\t<Calibration>\n");
    fprintf(f, "\t\t<Enable>%d</Enable>\n", configuration.m_calibration.m_enable);
    fprintf(f, "\t\t<GenerateReferences>%d</GenerateReferences>\n", configuration.m_calibration.m_generateReferences);
    fprintf(f, "\t\t<FilterReferences>%d</FilterReferences>\n", configuration.m_calibration.m_filterReferences);

    fprintf(f, "\t\t<SolarSpectrumFile>%s</SolarSpectrumFile>\n", configuration.m_calibration.m_solarSpectrumFile.GetBuffer());

    fprintf(f, "\t\t<InitialCalibrationFile>%s</InitialCalibrationFile>\n", configuration.m_calibration.m_initialCalibrationFile.GetBuffer());

    if (configuration.m_calibration.m_instrumentLineshapeFile.GetLength() > 0)
    {
        fprintf(f, "\t\t<InitialLineShapeFile>%s</InitialLineShapeFile>\n", configuration.m_calibration.m_instrumentLineshapeFile.GetBuffer());
    }
    fprintf(f, "\t\t<InitialCalibrationType>%d</InitialCalibrationType>\n", configuration.m_calibration.m_initialCalibrationType);

    fprintf(f, "\t\t<InstrumentLineShapeFitOption>%d</InstrumentLineShapeFitOption>\n", configuration.m_calibration.m_instrumentLineShapeFitOption);
    fprintf(f, "\t\t<instrumentLineShapeFitRegionLow>%lf</instrumentLineShapeFitRegionLow>\n", configuration.m_calibration.m_instrumentLineShapeFitRegion.low);
    fprintf(f, "\t\t<instrumentLineShapeFitRegionHigh>%lf</instrumentLineShapeFitRegionHigh>\n", configuration.m_calibration.m_instrumentLineShapeFitRegion.high);

    fprintf(f, "\t</Calibration>\n");

    // ----------- Evaluation ----------------
    for (int k = 0; k < configuration.m_nFitWindows; ++k) {
        fprintf(f, "\t<FitWindow>\n");
        fprintf(f, "\t\t<name>%s</name>\n", (LPCTSTR)configuration.m_fitWindow[k].name);
        fprintf(f, "\t\t<fitLow>%d</fitLow>\n", configuration.m_fitWindow[k].fitLow);
        fprintf(f, "\t\t<fitHigh>%d</fitHigh>\n", configuration.m_fitWindow[k].fitHigh);
        fprintf(f, "\t\t<spec_channel>%d</spec_channel>\n", configuration.m_fitWindow[k].channel);
        fprintf(f, "\t\t<polynomial>%d</polynomial>\n", configuration.m_fitWindow[k].polyOrder);

        for (int j = 0; j < configuration.m_fitWindow[k].nRef; ++j) {
            fprintf(f, "\t\t<Reference>\n");
            fprintf(f, "\t\t\t<name>%s</name>\n", configuration.m_fitWindow[k].ref[j].m_specieName.c_str());
            fprintf(f, "\t\t\t<path>%s</path>\n", configuration.m_fitWindow[k].ref[j].m_path.c_str());
            fprintf(f, "\t\t\t<gasFactor>%.2lf</gasFactor>\n", configuration.m_fitWindow[k].ref[j].m_gasFactor);

            // Shift
            if (configuration.m_fitWindow[k].ref[j].m_shiftOption == novac::SHIFT_TYPE::SHIFT_FIX) {
                fprintf(f, "\t\t\t<shift>fix to %.2lf</shift>\n", configuration.m_fitWindow[k].ref[j].m_shiftValue);
            }
            else if (configuration.m_fitWindow[k].ref[j].m_shiftOption == novac::SHIFT_TYPE::SHIFT_FREE) {
                fprintf(f, "\t\t\t<shift>free</shift>\n");
            }
            else if (configuration.m_fitWindow[k].ref[j].m_shiftOption == novac::SHIFT_TYPE::SHIFT_LINK) {
                fprintf(f, "\t\t\t<shift>link to %.0lf</shift>\n", configuration.m_fitWindow[k].ref[j].m_shiftValue);
            }

            // Squeeze
            if (configuration.m_fitWindow[k].ref[j].m_squeezeOption == novac::SHIFT_TYPE::SHIFT_FIX) {
                fprintf(f, "\t\t\t<squeeze>fix to %.2lf</squeeze>\n", configuration.m_fitWindow[k].ref[j].m_squeezeValue);
            }
            else if (configuration.m_fitWindow[k].ref[j].m_squeezeOption == novac::SHIFT_TYPE::SHIFT_FREE) {
                fprintf(f, "\t\t\t<squeeze>free</squeeze>\n");
            }
            else if (configuration.m_fitWindow[k].ref[j].m_squeezeOption == novac::SHIFT_TYPE::SHIFT_LINK) {
                fprintf(f, "\t\t\t<squeeze>link to %.0lf</squeeze>\n", configuration.m_fitWindow[k].ref[j].m_squeezeValue);
            }
            fprintf(f, "\t\t</Reference>\n");
        }
        fprintf(f, "\t</FitWindow>\n");
    }
    // ----------- Directory ----------------
    if (configuration.m_spectrometerConnection == CMobileConfiguration::CONNECTION_DIRECTORY) {
        fprintf(f, "\t<DirectoryMode>\n");
        fprintf(f, "\t\t<directory>%s</directory>\n", configuration.m_directory.GetBuffer());
        fprintf(f, "\t\t<spectrometerDynamicRange>%d</spectrometerDynamicRange>\n", configuration.m_spectrometerDynamicRange);
        fprintf(f, "\t\t<sleep>%d</sleep>\n", configuration.m_sleep);
        fprintf(f, "\t\t<defaultSkyFile>%s</defaultSkyFile>\n", configuration.m_defaultSkyFile.GetBuffer());
        fprintf(f, "\t\t<defaultDarkFile>%s</defaultDarkFile>\n", configuration.m_defaultDarkFile.GetBuffer());
        fprintf(f, "\t\t<defaultDarkcurFile>%s</defaultDarkcurFile>\n", configuration.m_defaultDarkcurFile.GetBuffer());
        fprintf(f, "\t\t<defaultOffsetFile>%s</defaultOffsetFile>\n", configuration.m_defaultOffsetFile.GetBuffer());
        fprintf(f, "\t</DirectoryMode>\n");
    }

    fprintf(f, "</Configuration>\n");
    fclose(f);
}
