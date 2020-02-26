
#include "stdafx.h"
#include "mobileconfiguration.h"

extern CFormView *pView;

using namespace Configuration;

CMobileConfiguration::CMobileConfiguration(void)
{
	Clear();
}

CMobileConfiguration::CMobileConfiguration(const CString &fileName){
	Clear(); // <-- set the values to the default values
	ReadCfgXml(fileName);
}

CMobileConfiguration::~CMobileConfiguration(void)
{
}

void CMobileConfiguration::Clear(){
	// Spectrometer 
	m_spectrometerConnection = CONNECTION_USB;
	m_serialPort.Format("COM1");
	m_baudrate = 57600;
	m_nChannels = 1;
	m_setPointTemperature = -10.0;

	// Exposure-Time
	m_expTimeMode = EXPOSURETIME_AUTOMATIC;
	m_specCenter = 1144;	// approximate pixel with the highest intensity in standard spectrometers we use. user can change.
	m_percent = 70;
	m_timeResolution	= 5000;

	// saturation range
	m_saturationLow = 60;
	m_saturationHigh = 80;

	// GPS
	m_useGPS = 1;
	m_gpsPort.Format("COM4");
	m_gpsBaudrate = 4800;

	// Evaluation
	m_nFitWindows								= 0;
	
	m_fitWindow[0].fitLow				= 320;
	m_fitWindow[0].fitHigh			= 460;
	m_fitWindow[0].fitType			= Evaluation::FIT_HP_DIV;
	m_fitWindow[0].name.Format("NEW");
	m_fitWindow[0].nRef					= 0;
	m_fitWindow[0].ref[0].m_path.Format("");
	m_fitWindow[0].ref[0].m_specieName.Format("SO2");
	m_fitWindow[0].polyOrder		= 5;
	m_fitWindow[0].specLength		= MAX_SPECTRUM_LENGTH;

	// Misc
	m_maxColumn							= 200.0;
	m_offsetFrom						= 50;
	m_offsetTo							= 200;

	// Directory
	m_directoryMode = 0;
	m_directory.Format("");

}

/** Reading in a configuration file in .xml file format */
void CMobileConfiguration::ReadCfgXml(const CString &fileName){
	CFileException exceFile;
	CStdioFile file;

	// 1. Open the file
	if(!file.Open(fileName, CFile::modeRead | CFile::typeText, &exceFile)){
		return;
	}
	this->m_File = &file;

	// 2. Parse the file
	if(Parse()){
		file.Close();    // error in parsing
		return;
	}else{
		file.Close();    // parsing was ok
	//		CheckSettings();
		return;
	}
}

int CMobileConfiguration::Parse(){

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
		continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
		continue;
		}

		// The serial port to use
		if(Equals(szToken, "serialPort")){
			Parse_StringItem("/serialPort", m_serialPort);

			// If the serial-port begins with 'COM' then we're using the RS232-port
			if(Equals(m_serialPort, "COM", 3)){
				m_spectrometerConnection = CONNECTION_RS232;
			}
			continue;
		}

		// The baudrate to the spectrometer
		if(Equals(szToken, "serialBaudrate")){
			Parse_IntItem("/serialBaudrate", m_baudrate);
			continue;
		}

		// The time resolution of the measurements
		if(Equals(szToken, "timeResolution")){
			Parse_LongItem("/timeResolution", m_timeResolution);
			continue;
		}

		// Whether to use audio or not
		if (Equals(szToken, "useAudio")) {
			Parse_IntItem("/useAudio", m_useAudio);
			continue;
		}

		// The max-column, for scaling of the sound
		if(Equals(szToken, "maxColumn")){
			Parse_FloatItem("/maxColumn", m_maxColumn);
			continue;
		}

		// The number of channels to use...
		if(Equals(szToken, "nchannels")){
			Parse_IntItem("/nchannels", m_nChannels);
			continue;
		}

		// The set point for the CCD temperature
		if (Equals(szToken, "setPointTemperature")) {
			Parse_FloatItem("/setPointTemperature", m_setPointTemperature);
			continue;
		}

		// The GPS-Settings
		if(Equals(szToken, "GPS")){
			ParseGPS();
			continue;
		}

		// The Intensity Settings
		if(Equals(szToken, "Intensity")){
			ParseIntensity();
			continue;
		}

		// The Offset Settings
		if(Equals(szToken, "Offset")){
			ParseOffset();
			continue;
		}

		// Whether to skip dark measurement
		if (Equals(szToken, "noDark")) {
			Parse_IntItem("/noDark", m_noDark);
			continue;
		}

		// The Fit-window Settings
		if(Equals(szToken, "FitWindow")){
			ParseFitWindow();
			continue;
		}

		// Directory Mode
		if (Equals(szToken, "directoryMode")) {
			Parse_IntItem("/directoryMode", m_directoryMode);
			continue;
		}

		// Watch Directory
		if (Equals(szToken, "directory")) {
			Parse_StringItem("/directory", m_directory);
			continue;
		}
	}

	return 0;
}

/** Parses the settings for the GPS */
int CMobileConfiguration::ParseGPS(){

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of the GPS section
		if(Equals(szToken, "/GPS")){
			return 0;
		}

		// The serial-port
		if(Equals(szToken, "port")){
			Parse_StringItem(TEXT("/port"), m_gpsPort);
			continue;
		}

		// The baudrate
		if(Equals(szToken, "baudrate")){
			Parse_LongItem(TEXT("/baudrate"), m_gpsBaudrate);
			continue;
		}

		// Shall we use the GPS or not?
		if(Equals(szToken, "use")){
			Parse_IntItem(TEXT("/use"), m_useGPS);
			continue;
		}
	}
	return 0;
}

/** Parses the settings for the intensity */
int CMobileConfiguration::ParseIntensity(){

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3) {
			continue;
		}

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of the Intensity section
		if(Equals(szToken, "/Intensity")){
			return 0;
		}

		// Fixed exposure-time??
		if(Equals(szToken, "FixExpTime")){
			Parse_IntItem(TEXT("/FixExpTime"), m_fixExpTime);
			if(m_fixExpTime > 0)
				m_expTimeMode = EXPOSURETIME_FIXED;
			else if(m_fixExpTime < 0)
				m_expTimeMode = EXPOSURETIME_ADAPTIVE;
			else
				m_expTimeMode = EXPOSURETIME_AUTOMATIC;
		continue;
		}
		
		// The saturation-ratio
		if(Equals(szToken, "Percent")){
			Parse_LongItem(TEXT("/Percent"), m_percent);
		continue;
		}

		// The channel where to measure intensity
		if(Equals(szToken, "Channel")){
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
int CMobileConfiguration::ParseOffset(){

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 2) {
			continue;
		}

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of the Offset section
		if(Equals(szToken, "/Offset")){
			return 0;
		}

		// Measure offset from...
		if(Equals(szToken, "from")){
			Parse_IntItem(TEXT("/from"), m_offsetFrom);
			continue;
		}
		
		// Measure offset to...
		if(Equals(szToken, "to")){
			Parse_IntItem(TEXT("/to"), m_offsetTo);
			continue;
		}
	}
	return 0;
}

/** Parses a fit-window settings section */
int CMobileConfiguration::ParseFitWindow(){
	Evaluation::CFitWindow &curWindow = this->m_fitWindow[m_nFitWindows];

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3) {
			continue;
		}

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of the Fit-window section
		if(Equals(szToken, "/FitWindow")){
			return 0;
		}

		// The name of the fit-window
		if(Equals(szToken, "name")){
			Parse_StringItem(TEXT("/name"), curWindow.name);
			++m_nFitWindows;
			continue;
		}

		// The fit-range
		if(Equals(szToken, "fitLow")){
			Parse_IntItem(TEXT("/fitLow"), curWindow.fitLow);
			continue;
		}

		if(Equals(szToken, "fitHigh")){
			Parse_IntItem(TEXT("/fitHigh"), curWindow.fitHigh);
			continue;
		}

		// The channel that this window will be used for
		if(Equals(szToken, "spec_channel")){
			Parse_IntItem(TEXT("/spec_channel"), curWindow.channel);
			continue;
		}
		
		// The polynomial to use
		if(Equals(szToken, "polynomial")){
			Parse_IntItem(TEXT("/polynomial"), curWindow.polyOrder);
			continue;
		}

		// The References
		if(Equals(szToken, "Reference")){
			ParseReference(curWindow.ref[curWindow.nRef]);
			++curWindow.nRef;
			continue;
		}
	}

	return 0;
}

/** Parses a reference-file section */
int	CMobileConfiguration::ParseReference(Evaluation::CReferenceFile &reference){
	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3) {
			continue;
		}

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of the Reference section
		if(Equals(szToken, "/Reference")){
			return 0;
		}

		// The name of the specie
		if(Equals(szToken, "name")){
			Parse_StringItem(TEXT("/name"), reference.m_specieName);
			continue;
		}

		// The path of the specie
		if(Equals(szToken, "path")){
			Parse_StringItem(TEXT("/path"), reference.m_path);
			continue;
		}

		// The gas-factor of the specie
		if(Equals(szToken, "gasFactor")){
			this->Parse_FloatItem(TEXT("/gasFactor"), reference.m_gasFactor);
			continue;
		}

		// The shift to use
		if(Equals(szToken, "shift")){
			this->Parse_ShiftOrSqueeze(TEXT("/shift"), reference.m_shiftOption, reference.m_shiftValue);
			continue;
		}

		// The squeeze to use
		if(Equals(szToken, "squeeze")){
			this->Parse_ShiftOrSqueeze(TEXT("/squeeze"), reference.m_squeezeOption, reference.m_squeezeValue);
			continue;
		}
	}

	return 0;
}

/** Parses a shift or squeeze section */
int CMobileConfiguration::Parse_ShiftOrSqueeze(const CString &label, Evaluation::SHIFT_TYPE &option, double &lowValue/*, double &highValue*/){
	char *pt = nullptr;

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3) {
			continue;
		}

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of this section
		if(Equals(szToken, label)){
			return 0;
		}
		// convert the string to lowercase
		_strlwr(szToken);

		if(pt = strstr(szToken, "fix to")){
			sscanf(szToken, "fix to %lf", &lowValue);
			option = Evaluation::SHIFT_FIX;
		}else if(pt = strstr(szToken, "free")){
			option = Evaluation::SHIFT_FREE;
		//}else if(pt = strstr(szToken, "limit")){
		//	sscanf(szToken, "limit from %lf to %lf", &lowValue, &highValue);
		//	option = Evaluation::SHIFT_LIMIT;
		}else if(pt = strstr(szToken, "link")){
			sscanf(szToken, "link to %lf", &lowValue);
			option = Evaluation::SHIFT_LINK;
		}
	}
	return 0;
}
