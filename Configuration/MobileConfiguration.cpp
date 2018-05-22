
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

	if(-1 != fileName.Find(".txt"))
		ReadCfgTxt(fileName);
	else
		ReadCfgXml(fileName);
}

/** Reading in a configuration file in .txt file format */
void CMobileConfiguration::ReadCfgTxt(const CString &fileName){
	char *pt;
	FILE *fil;
	char txt[256];
	char nl[2]={ 0x0a, 0 };
	char lf[2]={ 0x0d, 0 };

	CString msg,str;

	char serialPort[11], gpsPort[10], refFile[500];
	fil = fopen(fileName, "r");
	if(fil<(FILE *)1)
	{
		MessageBox(pView->m_hWnd, "No configuration file found. Configure software and click 'Save' to create cfg.xml.",TEXT("Warning"),MB_OK);
		return;
	}

	while(fgets(txt,sizeof(txt)-1,fil) )
	{
		if(strlen(txt)>4 && txt[0]!='%')
		{		  
			pt=txt;
			if(pt=strstr(txt,nl)) 
				pt[0]=0;
			pt=txt;
			if(pt=strstr(txt,lf)) 
				pt[0]=0;

			// ----------- The spectrometer  -------------------

			if(pt=strstr(txt,"SPEC_BAUD="))
			{
				pt=strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_baudrate);
			}
			if(pt=strstr(txt,"SERIALPORT="))
			{
				pt=strstr(txt,"=");
				sscanf(pt+1,"%10s",serialPort);
				m_serialPort.Format(serialPort);

				// If the serial-port begins with 'COM' then we're using the RS232-port
				if(Equals(serialPort, "COM", 3)){
					m_spectrometerConnection = CONNECTION_RS232;
				}
			}

			if(pt=strstr(txt,"NCHANNELS=")){
				pt=strstr(txt,"=");
				sscanf(&pt[1],"%d", &m_nChannels);
			}

			// ------------ The exposure time -------------------
			if(pt=strstr(txt,"FIXEXPTIME="))
			{
				int fixExpTime;
				pt=strstr(txt,"=");
				sscanf(&pt[1],"%d",&fixExpTime);
				if(fixExpTime > 0){
					m_expTimeMode = EXPOSURETIME_FIXED;
					m_fixExpTime	= fixExpTime;
				}else{
					m_expTimeMode = EXPOSURETIME_AUTOMATIC;
					m_fixExpTime	= 0;
				}
			}	
			if(pt=strstr(txt,"SPECCENTER="))
			{
				pt=strstr(txt,"=");
				sscanf(&pt[1],"%ld",&m_specCenter);
			}	
			if(pt=strstr(txt,"PERCENT="))
			{
				double percent;
				pt=strstr(txt,"=");
				sscanf(&pt[1],"%lf",&percent);
				m_percent = (long)(100.0 * percent);
			}

			if(pt=strstr(txt,"TIMERESOLUTION="))
			{
				pt=strstr(txt,"=");
				sscanf(&pt[1],"%ld",&m_timeResolution);
			}	

			// ---------------- The GPS ------------------
			if(pt=strstr(txt,"SKIPGPS="))
			{
				int skipGps;
				pt=strstr(txt,"=");
				sscanf(&pt[1],"%d",&skipGps);
				if(skipGps==1)
					m_useGPS= 0;
				else
					m_useGPS= 1;
			}
			if(pt=strstr(txt,"GPSBAUD="))
			{
				pt=strstr(txt,"=");
				sscanf(&pt[1],"%ld",&m_gpsBaudrate);
			}
			if(pt=strstr(txt,"GPSPORT="))
			{
				pt=strstr(txt,"=");
				sscanf(pt+1,"%9s", gpsPort);
				m_gpsPort.Format(gpsPort);
			}

			// ---------------- Evaluation -------------------------

			if(pt=strstr(txt,"FIXSHIFT="))
			{
				int fShift;
				pt=strstr(txt,"=");
				sscanf(&pt[1],"%d",&fShift);
				if(fShift == 1){
					m_fitWindow[0].ref[0].m_shiftOption = Evaluation::SHIFT_FIX;
					m_fitWindow[0].ref[0].m_shiftValue =	0.0;
				}else{
					m_fitWindow[0].ref[0].m_shiftOption = Evaluation::SHIFT_FREE;
					m_fitWindow[0].ref[0].m_shiftValue =	0.0;
				}
			}	

			if(pt=strstr(txt,"FIXSQUEEZE="))
			{
				int fSqueeze;
				pt=strstr(txt,"=");
				sscanf(&pt[1],"%d",&fSqueeze);
				if(fSqueeze == 1){
					m_fitWindow[0].ref[0].m_squeezeOption	= Evaluation::SHIFT_FIX;
					m_fitWindow[0].ref[0].m_squeezeValue		=	1.0;
				}else{
					m_fitWindow[0].ref[0].m_squeezeOption	= Evaluation::SHIFT_FREE;
					m_fitWindow[0].ref[0].m_squeezeValue		=	1.0;
				}
			}
			if(pt=strstr(txt,"FITFROM="))
			{
				pt=strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_fitWindow[0].fitLow);
			}	
			if(pt=strstr(txt,"FITTO="))
			{
				pt=strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_fitWindow[0].fitHigh);
			}	
			if(pt=strstr(txt,"POLYNOM="))
			{
				pt=strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_fitWindow[0].polyOrder);
			}	

			if(pt=strstr(txt,"REFFILE="))
			{
				pt=strstr(txt,"=");
				sscanf(pt+1,"%498s",refFile);
				m_fitWindow[0].ref[0].m_path.Format(refFile);
				m_nFitWindows = 1;
			}
			//if(pt=strstr(txt,"GASFACTOR="))
			//{
			//	pt=strstr(txt,"=");
			//	sscanf(&pt[1],"%lf",&m_gasFactor);
			//}

			// --------------- MISC -------------------
			if(pt=strstr(txt,"MAXCOLUMN="))
			{
				pt=strstr(txt,"=");
				sscanf(&pt[1],"%lf",&m_maxColumn);
			}
      if(pt = strstr(txt, "OFFSETFROM=")){
        pt = strstr(txt, "=");
				sscanf(&pt[1], "%d", &m_offsetFrom);
      }
      if(pt = strstr(txt, "OFFSETTO=")){
        pt = strstr(txt, "=");
				sscanf(&pt[1], "%d", &m_offsetTo);
      }
		}
	}
	fclose(fil);

	// At last, save the name of the configuration-file we just read
	this->m_cfgFile.Format(fileName);
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

	// Exposure-Time
	m_expTimeMode = EXPOSURETIME_AUTOMATIC;
	m_specCenter = 1144;
	m_percent = 70;
	m_timeResolution	= 5000;

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

		// The Fit-window Settings
		if(Equals(szToken, "FitWindow")){
			ParseFitWindow();
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
		if(strlen(szToken) < 3)
		continue;

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
	}
	return 0;
}

/** Parses a section on how to subtract the offset */
int CMobileConfiguration::ParseOffset(){

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 2)
		continue;

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
		if(strlen(szToken) < 3)
		continue;

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
		if(strlen(szToken) < 3)
		continue;

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
		if(strlen(szToken) < 3)
		continue;

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
