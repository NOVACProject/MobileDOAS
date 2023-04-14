
#include "stdafx.h"
#include "mobileconfiguration.h"
#include <SpectralEvaluation/StringUtils.h>

using namespace Configuration;

CMobileConfiguration::CMobileConfiguration(void)
{
    Clear();
}

CMobileConfiguration::CMobileConfiguration(const CString& fileName) {
    Clear(); // <-- set the values to the default values
    ReadCfgXml(fileName);
}

CMobileConfiguration::~CMobileConfiguration(void)
{
}

void CMobileConfiguration::Clear() {
    // Spectrometer 
    m_spectrometerConnection = CONNECTION_USB;
    m_serialPort = "COM1";
    m_baudrate = 57600;
    m_nChannels = 1;
    m_setPointTemperature = -10.0;

    // Exposure-Time
    m_expTimeMode = EXPOSURETIME_AUTOMATIC;
    m_specCenter = 1144;	// approximate pixel with the highest intensity in standard spectrometers we use. user can change.
    m_percent = 70;
    m_timeResolution = 5000;

    // saturation range
    m_saturationLow = 60;
    m_saturationHigh = 80;

    // GPS
    m_useGPS = 1;
    m_gpsPort = "COM4";
    m_gpsBaudrate = 4800;

    // Evaluation
    m_nFitWindows = 0;

    m_fitWindow[0].fitLow = 320;
    m_fitWindow[0].fitHigh = 460;
    m_fitWindow[0].fitType = Evaluation::FIT_HP_DIV;
    m_fitWindow[0].name.Format("NEW");
    m_fitWindow[0].nRef = 0;
    m_fitWindow[0].ref[0].m_path = "";
    m_fitWindow[0].ref[0].m_specieName = "SO2";
    m_fitWindow[0].polyOrder = 5;
    m_fitWindow[0].specLength = MAX_SPECTRUM_LENGTH;

    // Misc
    m_maxColumn = 200.0;
    m_offsetFrom = 50;
    m_offsetTo = 200;

    // Directory
    m_directory.Format("");
    m_sleep = 500;
    m_spectrometerDynamicRange = 4096;
    m_defaultSkyFile.Format("");
    m_defaultDarkFile.Format("");
    m_defaultDarkcurFile.Format("");
    m_defaultOffsetFile.Format("");
}

/** Reading in a configuration file in .xml file format */
void CMobileConfiguration::ReadCfgXml(const CString& fileName) {
    CFileException exceFile;
    CStdioFile file;

    // 1. Open the file
    if (!file.Open(fileName, CFile::modeRead | CFile::typeText, &exceFile)) {
        return;
    }
    this->m_File = &file;

    // 2. Parse the file
    if (Parse()) {
        file.Close();    // error in parsing
        return;
    }
    else {
        file.Close();    // parsing was ok
        //		CheckSettings();
        return;
    }
}

int CMobileConfiguration::Parse() {

    // the actual reading loop
    while (szToken = NextToken()) {

        // no use to parse empty lines
        if (strlen(szToken) < 3)
            continue;

        // ignore comments
        if (Equals(szToken, "!--", 3)) {
            continue;
        }

        // The serial port to use
        if (Equals(szToken, "serialPort")) {
            Parse_StringItem("/serialPort", m_serialPort);

            // If the serial-port begins with 'COM' then we're using the RS232-port
            if (EqualsIgnoringCase(m_serialPort, "COM", 3)) {
                m_spectrometerConnection = CONNECTION_RS232;
            }
            // Directory mode enabled
            if (EqualsIgnoringCase(m_serialPort, "Directory", 3)) {
                m_spectrometerConnection = CONNECTION_DIRECTORY;
            }
            continue;
        }

        // The baudrate to the spectrometer
        if (Equals(szToken, "serialBaudrate")) {
            Parse_IntItem("/serialBaudrate", m_baudrate);
            continue;
        }

        // The time resolution of the measurements
        if (Equals(szToken, "timeResolution")) {
            Parse_LongItem("/timeResolution", m_timeResolution);
            continue;
        }

        // Whether to use audio or not
        if (Equals(szToken, "useAudio")) {
            Parse_IntItem("/useAudio", m_useAudio);
            continue;
        }

        // The max-column, for scaling of the sound
        if (Equals(szToken, "maxColumn")) {
            Parse_FloatItem("/maxColumn", m_maxColumn);
            continue;
        }

        // The number of channels to use...
        if (Equals(szToken, "nchannels")) {
            Parse_IntItem("/nchannels", m_nChannels);
            continue;
        }

        // The set point for the CCD temperature
        if (Equals(szToken, "setPointTemperature")) {
            Parse_FloatItem("/setPointTemperature", m_setPointTemperature);
            continue;
        }

        // The GPS-Settings
        if (Equals(szToken, "GPS")) {
            ParseGPS();
            continue;
        }

        // The Intensity Settings
        if (Equals(szToken, "Intensity")) {
            ParseIntensity();
            continue;
        }

        // The Offset Settings
        if (Equals(szToken, "Offset")) {
            ParseOffset();
            continue;
        }

        // Whether to skip dark measurement
        if (Equals(szToken, "noDark")) {
            Parse_IntItem("/noDark", m_noDark);
            continue;
        }

        // The Fit-window Settings
        if (Equals(szToken, "FitWindow")) {
            ParseFitWindow();
            continue;
        }

        // The Directory Mode Settings
        if (Equals(szToken, "DirectoryMode")) {
            ParseDirectoryMode();
            continue;
        }

        // The Calibration Settings
        if (Equals(szToken, "Calibration")) {
            ParseCalibrationSettings();
            continue;
        }
    }

    return 0;
}

/** Parses the settings for the GPS */
int CMobileConfiguration::ParseGPS() {

    // the actual reading loop
    while (szToken = NextToken()) {

        // no use to parse empty lines
        if (strlen(szToken) < 3)
            continue;

        // ignore comments
        if (Equals(szToken, "!--", 3)) {
            continue;
        }

        // the end of the GPS section
        if (Equals(szToken, "/GPS")) {
            return 0;
        }

        // The serial-port
        if (Equals(szToken, "port")) {
            Parse_StringItem("/port", m_gpsPort);
            continue;
        }

        // The baudrate
        if (Equals(szToken, "baudrate")) {
            Parse_LongItem(TEXT("/baudrate"), m_gpsBaudrate);
            continue;
        }

        // Shall we use the GPS or not?
        if (Equals(szToken, "use")) {
            Parse_IntItem(TEXT("/use"), m_useGPS);
            continue;
        }
    }
    return 0;
}

/** Parses the settings for the intensity */
int CMobileConfiguration::ParseIntensity() {

    // the actual reading loop
    while (szToken = NextToken()) {

        // no use to parse empty lines
        if (strlen(szToken) < 3) {
            continue;
        }

        // ignore comments
        if (Equals(szToken, "!--", 3)) {
            continue;
        }

        // the end of the Intensity section
        if (Equals(szToken, "/Intensity")) {
            return 0;
        }

        // Fixed exposure-time??
        if (Equals(szToken, "FixExpTime")) {
            Parse_IntItem(TEXT("/FixExpTime"), m_fixExpTime);
            if (m_fixExpTime > 0)
                m_expTimeMode = EXPOSURETIME_FIXED;
            else if (m_fixExpTime < 0)
                m_expTimeMode = EXPOSURETIME_ADAPTIVE;
            else
                m_expTimeMode = EXPOSURETIME_AUTOMATIC;
            continue;
        }

        // The saturation-ratio
        if (Equals(szToken, "Percent")) {
            Parse_LongItem(TEXT("/Percent"), m_percent);
            continue;
        }

        // The channel where to measure intensity
        if (Equals(szToken, "Channel")) {
            Parse_LongItem(TEXT("/Channel"), m_specCenter);
            continue;
        }

        // Saturation range for adaptive mode
        if (Equals(szToken, "saturationLow")) {
            Parse_IntItem("/saturationLow", m_saturationLow);
            continue;
        }
        if (Equals(szToken, "saturationHigh")) {
            Parse_IntItem("/saturationHigh", m_saturationHigh);
            continue;
        }
    }
    return 0;
}

/** Parses a section on how to subtract the offset */
int CMobileConfiguration::ParseOffset() {

    // the actual reading loop
    while (szToken = NextToken()) {

        // no use to parse empty lines
        if (strlen(szToken) < 2) {
            continue;
        }

        // ignore comments
        if (Equals(szToken, "!--", 3)) {
            continue;
        }

        // the end of the Offset section
        if (Equals(szToken, "/Offset")) {
            return 0;
        }

        // Measure offset from...
        if (Equals(szToken, "from")) {
            Parse_IntItem(TEXT("/from"), m_offsetFrom);
            continue;
        }

        // Measure offset to...
        if (Equals(szToken, "to")) {
            Parse_IntItem(TEXT("/to"), m_offsetTo);
            continue;
        }
    }
    return 0;
}

/** Parses a fit-window settings section */
int CMobileConfiguration::ParseFitWindow() {
    Evaluation::CFitWindow& curWindow = this->m_fitWindow[m_nFitWindows];

    // the actual reading loop
    while (szToken = NextToken()) {

        // no use to parse empty lines
        if (strlen(szToken) < 3) {
            continue;
        }

        // ignore comments
        if (Equals(szToken, "!--", 3)) {
            continue;
        }

        // the end of the Fit-window section
        if (Equals(szToken, "/FitWindow")) {
            return 0;
        }

        // The name of the fit-window
        if (Equals(szToken, "name")) {
            Parse_StringItem(TEXT("/name"), curWindow.name);
            ++m_nFitWindows;
            continue;
        }

        // The fit-range
        if (Equals(szToken, "fitLow")) {
            Parse_IntItem(TEXT("/fitLow"), curWindow.fitLow);
            continue;
        }

        if (Equals(szToken, "fitHigh")) {
            Parse_IntItem(TEXT("/fitHigh"), curWindow.fitHigh);
            continue;
        }

        // The channel that this window will be used for
        if (Equals(szToken, "spec_channel")) {
            Parse_IntItem(TEXT("/spec_channel"), curWindow.channel);
            continue;
        }

        // The polynomial to use
        if (Equals(szToken, "polynomial")) {
            Parse_IntItem(TEXT("/polynomial"), curWindow.polyOrder);
            continue;
        }

        // The References
        if (Equals(szToken, "Reference")) {
            ParseReference(curWindow.ref[curWindow.nRef]);
            ++curWindow.nRef;
            continue;
        }
    }

    return 0;
}

int CMobileConfiguration::ParseCalibrationSettings()
{
    while (szToken = NextToken()) {

        // no use to parse empty lines
        if (strlen(szToken) < 3) {
            continue;
        }

        // the end of the directory mode section
        if (Equals(szToken, "/Calibration")) {
            return 0;
        }

        if (Equals(szToken, "Enable")) {
            int tempInt = 0;
            Parse_IntItem("/Enable", tempInt);
            m_calibration.m_enable = (tempInt != 0);
            continue;
        }

        if (Equals(szToken, "GenerateReferences")) {
            Parse_IntItem("/GenerateReferences", m_calibration.m_generateReferences);
            continue;
        }

        if (Equals(szToken, "FilterReferences")) {
            Parse_IntItem("/FilterReferences", m_calibration.m_filterReferences);
            continue;
        }

        if (Equals(szToken, "SolarSpectrumFile")) {
            Parse_StringItem("/SolarSpectrumFile", m_calibration.m_solarSpectrumFile);
            continue;
        }

        if (Equals(szToken, "InitialCalibrationFile")) {
            Parse_StringItem("/InitialCalibrationFile", m_calibration.m_initialCalibrationFile);
            continue;
        }

        if (Equals(szToken, "InitialLineShapeFile")) {
            Parse_StringItem("/InitialLineShapeFile", m_calibration.m_instrumentLineshapeFile);
            continue;
        }

        if (Equals(szToken, "InitialCalibrationType")) {
            Parse_IntItem("/InitialCalibrationType", m_calibration.m_initialCalibrationType);
            continue;
        }

        if (Equals(szToken, "InstrumentLineShapeFitOption")) {
            Parse_IntItem("/InstrumentLineShapeFitOption", m_calibration.m_instrumentLineShapeFitOption);
            continue;
        }

        if (Equals(szToken, "instrumentLineShapeFitRegionLow")) {
            Parse_FloatItem("/instrumentLineShapeFitRegionLow", m_calibration.m_instrumentLineShapeFitRegion.low);
            continue;
        }

        if (Equals(szToken, "instrumentLineShapeFitRegionHigh")) {
            Parse_FloatItem("/instrumentLineShapeFitRegionHigh", m_calibration.m_instrumentLineShapeFitRegion.high);
            continue;
        }

    }

    return 0;
}

int	CMobileConfiguration::ParseReference(novac::CReferenceFile& reference) {
    // the actual reading loop
    while (szToken = NextToken()) {

        // no use to parse empty lines
        if (strlen(szToken) < 3) {
            continue;
        }

        // ignore comments
        if (Equals(szToken, "!--", 3)) {
            continue;
        }

        // the end of the Reference section
        if (Equals(szToken, "/Reference")) {
            return 0;
        }

        // The name of the specie
        if (Equals(szToken, "name")) {
            CString tmpStr;
            Parse_StringItem(TEXT("/name"), tmpStr);
            reference.m_specieName = std::string(tmpStr);
            continue;
        }

        // The path of the specie
        if (Equals(szToken, "path")) {
            CString tmpStr;
            Parse_StringItem(TEXT("/path"), tmpStr);
            reference.m_path = std::string(tmpStr);
            continue;
        }

        // The gas-factor of the specie
        if (Equals(szToken, "gasFactor")) {
            this->Parse_FloatItem(TEXT("/gasFactor"), reference.m_gasFactor);
            continue;
        }

        // The shift to use
        if (Equals(szToken, "shift")) {
            this->Parse_ShiftOrSqueeze(TEXT("/shift"), reference.m_shiftOption, reference.m_shiftValue);
            continue;
        }

        // The squeeze to use
        if (Equals(szToken, "squeeze")) {
            this->Parse_ShiftOrSqueeze(TEXT("/squeeze"), reference.m_squeezeOption, reference.m_squeezeValue);
            continue;
        }
    }

    return 0;
}

int CMobileConfiguration::Parse_ShiftOrSqueeze(const CString& label, novac::SHIFT_TYPE& option, double& lowValue/*, double &highValue*/) {
    char* pt = nullptr;

    // the actual reading loop
    while (szToken = NextToken()) {

        // no use to parse empty lines
        if (strlen(szToken) < 3) {
            continue;
        }

        // ignore comments
        if (Equals(szToken, "!--", 3)) {
            continue;
        }

        // the end of this section
        if (Equals(szToken, label)) {
            return 0;
        }
        // convert the string to lowercase
        _strlwr(szToken);

        if (pt = strstr(szToken, "fix to")) {
            sscanf(szToken, "fix to %lf", &lowValue);
            option = novac::SHIFT_TYPE::SHIFT_FIX;
        }
        else if (pt = strstr(szToken, "free")) {
            option = novac::SHIFT_TYPE::SHIFT_FREE;
            //}else if(pt = strstr(szToken, "limit")){
            //	sscanf(szToken, "limit from %lf to %lf", &lowValue, &highValue);
            //	option = Evaluation::SHIFT_LIMIT;
        }
        else if (pt = strstr(szToken, "link")) {
            sscanf(szToken, "link to %lf", &lowValue);
            option = novac::SHIFT_TYPE::SHIFT_LINK;
        }
    }
    return 0;
}


/** Parses the settings for the DirectoryMode */
int CMobileConfiguration::ParseDirectoryMode() {

    // the actual reading loop
    while (szToken = NextToken()) {

        // no use to parse empty lines
        if (strlen(szToken) < 3) {
            continue;
        }

        // the end of the directory mode section
        if (Equals(szToken, "/DirectoryMode")) {
            return 0;
        }

        // Directory to watch for STD files in
        if (Equals(szToken, "directory")) {
            Parse_StringItem("/directory", m_directory);
            continue;
        }

        // Spectrometer range
        if (Equals(szToken, "spectrometerDynamicRange")) {
            Parse_LongItem("/spectrometerDynamicRange", m_spectrometerDynamicRange);
            continue;
        }

        // Sleep time in ms between directory check
        if (Equals(szToken, "sleep")) {
            Parse_IntItem("/sleep", m_sleep);
            continue;
        }

        // Default sky file
        if (Equals(szToken, "defaultSkyFile")) {
            Parse_StringItem("/defaultSkyFile", m_defaultSkyFile);
            continue;
        }

        // Default dark file
        if (Equals(szToken, "defaultDarkFile")) {
            Parse_StringItem("/defaultDarkFile", m_defaultDarkFile);
            continue;
        }

        // Default darkcur file
        if (Equals(szToken, "defaultDarkcurFile")) {
            Parse_StringItem("/defaultDarkcurFile", m_defaultDarkcurFile);
            continue;
        }

        // Default sky file
        if (Equals(szToken, "defaultOffsetFile")) {
            Parse_StringItem("/defaultOffsetFile", m_defaultOffsetFile);
            continue;
        }
    }

    return 0;
}
