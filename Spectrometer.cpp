// Communication.cpp: implementation of the CSpectrometer class.
//
//////////////////////////////////////////////////////////////////////

#undef min
#undef max

#include "stdafx.h"	
#include <Mmsystem.h>	// used for PlaySound
#include "DMSpec.h"
#include "Spectrometer.h"
#include <algorithm>
#include "Dialogs/SelectionDialog.h"

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define OFFSET 10		//the distance from the m_conf->m_specCenter
#define SERIALDELAY 2000

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormView* pView;

CSpectrometer::CSpectrometer()
	: m_useGps(true), fUseUSB(true) {

	sprintf(m_GPSPort, "COM5");

	fRun = TRUE;
	m_spectrometerMode = MODE_TRAVERSE; // default mode
	counter = 0;
	scanNum = 0;
	posFlag = 1;
	zeroPosNum = 0;
	m_timeResolution = 0;
	windSpeed = 1.0;
	windAngle = 0;
	m_Base = "base1";
	m_detectorSize = 2048;
	m_spectrometerDynRange = 4095;
	m_spectrometerModel.Format("Unknown");
	m_flux = 0;	//initiate flux value in this variable
	m_maxColumn = 100;
	integrationTime = 100;
	totalSpecNum = 1;
	m_adjustIntegrationTime = FALSE;
	m_gasFactor = GASFACTOR_SO2;

	m_fitRegionNum = 1;
	m_fitRegion[0].eval[0] = new Evaluation::CEvaluation();
	m_fitRegion[0].eval[1] = new Evaluation::CEvaluation();
	for (int k = 1; k < MAX_FIT_WINDOWS; ++k) {
		m_fitRegion[k].eval[0] = nullptr;
		m_fitRegion[k].eval[1] = nullptr;
	}

	spectrometerName = TEXT("..........");

	m_NChannels = 1;
	m_spectrometerChannel = 0;

	lastDarkOffset[0] = 0;
	lastDarkOffset[1] = 0;

	if (!fUseUSB)
		serial.fRun = &fRun;

	timeDiff = 0;

	m_fitRegion[0].window.nRef = 0;
	m_fitRegion[0].window.specLength = MAX_SPECTRUM_LENGTH;

	m_gps = nullptr;

	// Make an initial guess of the spectrometer wavelengths
	for (int k = 0; k < MAX_SPECTRUM_LENGTH; ++k) {
		wavelength[0][k] = k;
		wavelength[1][k] = k;
	}

}

CSpectrometer::~CSpectrometer()
{
	delete this->m_fitRegion[0].eval[0];
	delete this->m_fitRegion[0].eval[1];

	if (m_gps != nullptr) {
		delete m_gps;
	}
}



long CSpectrometer::GetTimeValue_UMT()
{
	struct tm *tim;
	long timeValue;
	time_t t;
	time(&t);
	tim = localtime(&t);

	/* tim is the local time according to the computer clock
		We now need to adjust it to UMT using the time difference between the
		GPS-time and computer time that we (should have) retrieved
		at the first GPS-reading */
	long tid = timeDiff + 3600 * tim->tm_hour + 60 * tim->tm_min + tim->tm_sec;
	int hr = (int)tid / 3600;
	int mi = (int)(tid / 60 - 60 * hr);
	int se = (int)(tid - 3600 * hr - 60 * mi);

	timeValue = hr * 10000 + mi * 100 + se;
	return timeValue;
}

//-----------------------------------------------------------------
int CSpectrometer::InitSpectrometer(short channel, short inttime, short sumSpec) {

	if (fUseUSB) {
		return 0;
	}
	else {
		/* If we are using the serial connection */
		unsigned short sbuf;
		char txt[256];
		unsigned char *p;
		sbuf = 257;
		p = (unsigned char *)&channel;
		txt[0] = 'H';
		txt[1] = p[1];
		txt[2] = p[0];
		serial.Write(txt, 3);

		while (serial.Check(30))
			serial.Read(&txt, 1);

		p = (unsigned char *)&inttime;
		txt[0] = 'I';
		txt[1] = p[1];
		txt[2] = p[0];
		serial.Write(txt, 3);

		while (serial.Check(30))
			serial.Read(&txt, 1);

		p = (unsigned char *)&sumSpec;
		txt[0] = 'A';
		txt[1] = p[1];
		txt[2] = p[0];
		serial.Write(txt, 3);

		while (serial.Check(30))
			serial.Read(&txt, 1);

		return 0;
	}
}
//-----------------------------------------------------------------
/**This function is to collect data from single spectrometer
** @sumInComputer - the times to collect data
** @sumInSpectrometer - the number of spectra to gather in spectrometer
** @chn - 0 , use one spectrometer
*/
int CSpectrometer::Scan(long sumInComputer, long sumInSpectrometer, double pResult[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH]) {

	if (fUseUSB)
		return ScanUSB(sumInComputer, sumInSpectrometer, pResult);

	unsigned short sbuf[8192];
	char txt[256];
	char *bptr;

	long maxlen, isum;
	int i, j, n;
	double *smem1;

	smem1 = 0;

	maxlen = 65536;
	smem1 = (double *)malloc(sizeof(double)*(2 + maxlen));	// initialize one buffer
	if (smem1 == 0) {
		MessageBox(pView->m_hWnd, TEXT("Not enough memory"), NULL, MB_OK);
		return 1;
	}
	memset((void*)smem1, 0, sizeof(double)*(2 + maxlen));

	//get gps information

	//SetFileName();
	//scan the serial port
	for (isum = 0; isum < sumInComputer; ++isum) {

		//Empty the serial buffer
		serial.FlushSerialPort(100);
		//Send command to spectrometer for sending data
		txt[0] = 'S';
		bptr = (char *)&sbuf[0];
		serial.Write(txt, 1);

		txt[0] = 0;

		if (serial.Check(1000)) {
			serial.Read(txt, 1);
		}

		//wait first byte to come
		long waitTime = sumInSpectrometer*integrationTime + SERIALDELAY;

		serial.Check(waitTime);


		//checkSerial 100 ms is for reading all data from buffer
		i = 0;
		while (serial.Check(300) && i < 16384)
		{
			j = serial.Read(&bptr[i], 16384 - i);
			i += j;
		}
		if (i == 0)
		{
			free(smem1);
			MessageBox(pView->m_hWnd, TEXT("Timeout"), TEXT("SERROR"), MB_OK);
			return 1;
		}
		if ((sbuf[0] != 0xffff) && fRun)
		{

			free(smem1);
			MessageBox(pView->m_hWnd, TEXT("First byte of transmission is incorrect"), NULL, MB_OK);
			return 1;
		}

		//delete header byte number
		i = i / 2 - 8;


		for (j = 0; j < i; j++)
			sbuf[j] = Common::Swp(sbuf[j + 8]);


		smem1[0] = i;
		for (j = 0; j < smem1[0]; j++) {
			//smem1[2+j*2]+=j*dw1;
			smem1[3 + j * 2] += sbuf[j];
		}
	}


	for (n = 0; n < smem1[0]; n++)
		pResult[0][n] = smem1[3 + n * 2] / (sumInComputer * sumInSpectrometer);

	if (smem1)
		free(smem1);

	return 0;
}

/**This function is to collect data from a spectrometer using the USB-Port
** @sumInComputer - the times to collect data
** @sumInSpectrometer - the number of spectra to gather in spectrometer

*/
int CSpectrometer::ScanUSB(long sumInComputer, long sumInSpectrometer, double pResult[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH]) {

	// clear the old spectrum
	memset(pResult, 0, MAX_N_CHANNELS * MAX_SPECTRUM_LENGTH * sizeof(double));

	// Set the parameters for acquiring the spectrum
	if (m_NChannels == 1) {
		// Set the exposure time to use (this function takes exp-time in micro-seconds)
		m_wrapper.setIntegrationTime(m_spectrometerIndex, m_spectrometerChannel, integrationTime * 1000);

		// Set the number of co-adds
		m_wrapper.setScansToAverage(m_spectrometerIndex, m_spectrometerChannel, sumInSpectrometer);
	}
	else {
		for (int chn = 0; chn < m_NChannels; ++chn) {
			// Set the exposure time to use (this function takes exp-time in micro-seconds)
			m_wrapper.setIntegrationTime(m_spectrometerIndex, chn, integrationTime * 1000);

			// Set the number of co-adds
			m_wrapper.setScansToAverage(m_spectrometerIndex, chn, sumInSpectrometer);
		}
	}


	// if we only use one channel
	if (m_NChannels == 1) {
		// Get the spectrum
		for (int k = 0; k < sumInComputer; ++k) {
			if (!fRun)
				return 0;

			// Retreives the spectrum from the spectrometer
			DoubleArray spectrumArray = m_wrapper.getSpectrum(m_spectrometerIndex, m_spectrometerChannel);

			// copies the spectrum-values to the output array
			double *spectrum = spectrumArray.getDoubleValues();	// Sets a pointer to the values of the Spectrum array 
			for (int i = 0; i < m_detectorSize; i++) {				// Loop to print the spectral data to the screen
				pResult[0][i] += spectrum[i];
			}
		}

		// make the spectrum an average 
		if (sumInComputer > 0) {
			for (int i = 0; i < m_detectorSize; i++) {
				pResult[0][i] /= sumInComputer;
			}
		}

		return 0;
	}
	else if (m_NChannels == 2) {

		//// create a new ADC1000-USB spectrometer
		//ADC1000USB doubleSpec; = ADC1000USB(m_spectrometerIndex);
		//
		//// Make sure that this spectrometer has enough available channels
		//if(m_NChannels > doubleSpec.getNumberOfEnabledChannels()){
		//	MessageBox(pView->m_hWnd, "Error: present spectrometer does not have enough channels. Exiting spectrum collection", "Error", MB_OK);
		//	return 0;
		//}
		//
		//doubleSpec.setIntegrationTime(5000 * 1000);

		// enable the collection of two spectra at once
		// doubleSpec.setRotatorEnabled(1);

//		a = doubleSpec.getNumberOfChannels();
//		a = doubleSpec.getNumberOfEnabledChannels();
//		a = doubleSpec.isRotatorEnabled();
//	
//		ADC1000Channel master = ADC1000Channel(doubleSpec, doubleSpec.getNewCoefficients(0), 0);
//		ADC1000Channel slave  = ADC1000Channel(doubleSpec, doubleSpec.getNewCoefficients(1), 1);
//
//		// Get the spectrum
//		for(int k = 0; k < sumInComputer; ++k){
//			if(!fRun)
//				return 0;
//
//			unsigned char ism, ise;
//			ism = master.isMaster();
//			ism = slave.isMaster();
//			ise = master.isEnabled();
//			ise = slave.isEnabled();
//
//			// Retreives the spectrum from the spectrometer
////			DoubleArray spectrumArray0 = m_wrapper.getSpectrum(m_spectrometerIndex, 0);
//			Spectrum s0 = master.getSpectrum();
//			Spectrum s1 = slave.getSpectrum();
//
//			DoubleArray spectrumArray0 = s0.getSpectrum();
//			DoubleArray spectrumArray1 = s1.getSpectrum();
//
//			// copies the spectrum-values to the output array
//			double *spectrum0	= spectrumArray0.getDoubleValues();	// Sets a pointer to the values of the Spectrum array 
//			double *spectrum1	= spectrumArray1.getDoubleValues();	// Sets a pointer to the values of the Spectrum array 
//			for(int i = 0; i < m_detectorSize; i++){				// Loop to print the spectral data to the screen
//				pResult[0][i] += spectrum0[i];
//				pResult[1][i] += spectrum1[i];
//			}
//		}
	}

	return 0;
}

//-----------------------------------------------------------------
void CSpectrometer::SetUserParameters(double windspeed, double winddirection, char *baseName) {
	m_Base = new char[100];
	windSpeed = windspeed;
	windAngle = winddirection;
	sprintf(m_Base, "%s", baseName);
}

void CSpectrometer::ApplySettings() {
	CString msg;

	// The settings for the serial-port
	serial.baudrate = m_conf->m_baudrate;
	sprintf(serial.serialPort, m_conf->m_serialPort);
	fUseUSB = (m_conf->m_spectrometerConnection == Configuration::CMobileConfiguration::CONNECTION_USB);
	m_NChannels = m_conf->m_nChannels;
	bool error = false;
	if (m_NChannels > MAX_N_CHANNELS) {
		msg.Format("Cannot handle more than %d channels. Changed number of channels to %d", MAX_N_CHANNELS, MAX_N_CHANNELS);
		m_NChannels = MAX_N_CHANNELS;   error = true;
	}
	if (m_NChannels < 0) {
		msg.Format("A negative amount of channels defined in the configuration file. This does not make sense, will change number of channels to 1");
		m_NChannels = 1;  error = true;
	}
	if (m_NChannels == 0) {
		msg.Format("Zero channels defined in the configuration file. This does not make sense, will change number of channels to 1");
		m_NChannels = 1; error = true;
	}
	if (m_spectrometerMode == MODE_WIND && m_NChannels < 2) {
		msg.Format("%d channels defined in the configuration file, this does not agree with wind-measurements. Will change number of channels to 2", m_NChannels);
		m_NChannels = 2; error = true;
	}
	if (error) {
		MessageBox(pView->m_hWnd, msg, "Error in settings", NULL);
	}

	// The GPS-settings
	m_GPSBaudRate = m_conf->m_gpsBaudrate;
	sprintf(m_GPSPort, "%s", (LPCTSTR)m_conf->m_gpsPort);
	m_useGps = (m_conf->m_useGPS != 0);

	// The exposure-time settings
	if (m_conf->m_expTimeMode == Configuration::CMobileConfiguration::EXPOSURETIME_FIXED) {
		m_fixexptime = m_conf->m_fixExpTime;
	}
	else if (m_conf->m_expTimeMode == Configuration::CMobileConfiguration::EXPOSURETIME_ADAPTIVE) {
		m_fixexptime = -1;
	}
	else {
		m_fixexptime = 0;
	}
	this->m_percent = m_conf->m_percent / 100.0;
	this->m_timeResolution = m_conf->m_timeResolution;

	// The settings for the evaluation
	for (int k = 0; k < MAX_FIT_WINDOWS; ++k) {
		m_fitRegion[k].window = m_conf->m_fitWindow[k];
		if (k > 0) {
			m_fitRegion[k].eval[0] = new Evaluation::CEvaluation();
			m_fitRegion[k].eval[1] = new Evaluation::CEvaluation();
		}
		// the offset
		m_fitRegion[k].window.offsetFrom = m_conf->m_offsetFrom;
		m_fitRegion[k].window.offsetTo = m_conf->m_offsetTo;
	}
	this->m_fitRegionNum = m_conf->m_nFitWindows;

	// Misc. Settings
	this->m_maxColumn = m_conf->m_maxColumn;
}

/* makes a test of the settings, returns 0 if all is ok, else 1 */
int CSpectrometer::CheckSettings() {
	CString msgStr, fileName;
	double tmpDouble1, tmpDouble2, tmpDouble3;
	char buf[512];
	int  nColumns;

	// 1. Check so that there are at least one reference-file defined
	if (m_fitRegion[0].window.nRef <= 0) {
		MessageBox(pView->m_hWnd, "There are no reference-files defined in the configuration file. Please check settings and restart", "Error", MB_OK);
		return 1;
	}

	//2. Check so that the reference-files exists, can be read and
	//		have the correct format.
	for (int k = 0; k < m_fitRegion[0].window.nRef; ++k) {
		int nRows = 0;
		FILE *f = fopen(m_fitRegion[0].window.ref[k].m_path, "r");
		if (nullptr == f) {
			fileName.Format("%s%s", g_exePath, m_fitRegion[0].window.ref[k].m_path);
			f = fopen(fileName, "r");
			if (nullptr == f) {
				msgStr.Format("Cannot open reference file : %s for reading.", m_fitRegion[0].window.ref[k].m_path);
				MessageBox(pView->m_hWnd, msgStr, "Error", MB_OK);
				return 1;
			}
		}

		while (fgets(buf, 511, f)) {
			nColumns = sscanf(buf, "%lf\t%lf\t%lf", &tmpDouble1, &tmpDouble2, &tmpDouble3);
			if (nColumns == 0 || nColumns == EOF)
				break;

			++nRows;
			if (nColumns == 3) {
				msgStr.Format("There are %d columns in the reference file. This programs wants reference files with one or two columns", nColumns);
				MessageBox(pView->m_hWnd, msgStr, "Error", MB_OK);
				fclose(f);
				return 1;
			}
		}

		fclose(f);

		if (nRows > MAX_SPECTRUM_LENGTH) {
			msgStr.Format("Length of the reference file is: %d values. Cannot handle references with more than %d data-points.", nRows, MAX_SPECTRUM_LENGTH);
			MessageBox(pView->m_hWnd, msgStr, "Error", MB_OK);
			return 1;
		}
	}

	// 3. If there is a reference file with the name SO2, the it should
	//		be the first reference file (since the main window will only
	//		show the result of evaluating the first reference-file)
	if (!Equals(m_fitRegion[0].window.ref[0].m_specieName, "SO2")) {
		int k;
		for (k = 1; k < m_fitRegion[0].window.nRef; ++k) {
			if (Equals(m_fitRegion[0].window.ref[k].m_specieName, "SO2"))
				break;
		}
		if (k != m_fitRegion[0].window.nRef) {
			Evaluation::CReferenceFile tmpRef = m_fitRegion[0].window.ref[0];
			m_fitRegion[0].window.ref[0] = m_fitRegion[0].window.ref[k];
			m_fitRegion[0].window.ref[k] = tmpRef;
		}
	}

	return 0;
}



/**This function is to write the log file to record evaluation result and time
**
*/

void CSpectrometer::WriteEvFile(CString filename, FitRegion *fitRegion) {
	double *result = NULL;

	FILE *f;
	CString wholePath = m_subFolder + "\\" + strBaseName + "_" + strDateTime + filename;
	f = fopen(wholePath, "a+");
	if (f < (FILE*)1) {
		MessageBox(pView->m_hWnd, "Could not open evaluation log file. No data was written!", "Error", MB_OK);
		return;
	}

	int hr = specTime[counter] / 10000;
	int min = (specTime[counter] - hr * 10000) / 100;
	int sec = specTime[counter] % 100;

	// 1. Write the time of the spectrum
	fprintf(f, "%02d:%02d:%02d\t", hr, min, sec);

	// 2. Write the GPS-information about the spectrum
	fprintf(f, "%f\t%f\t%.1f\t", pos[counter].latitude, pos[counter].longitude, pos[counter].altitude);

	// 3. The number of spectra averaged and the exposure-time
	fprintf(f, "%ld\t%d\t", totalSpecNum, integrationTime);

	// 4. The intensity
	fprintf(f, "%ld\t", averageValue[0]);

	// 5. The evaluated column values
	for (int k = 0; k < fitRegion->window.nRef; ++k) {
		result = fitRegion->eval[0]->GetResult(k);
		fprintf(f, "%lf\t%lf\t", result[0], result[1]);
	}

	// 6. The std-file
	fprintf(f, "%s\n", (LPCTSTR)m_stdfileName[0]);

	fclose(f);
}

/** This function is to adjust the integration time
**@pSky, @pDark - the average intensity in the sky and dark spectra
@intT - the integration time used when measuring the sky and dark spectra
*/
long CSpectrometer::GetInttime(long pSky, long pDark, int intT)
{
	double xRate;
	long intTime = intT;

	if (m_fixexptime <= 0)
	{
		xRate = (m_spectrometerDynRange*m_percent - (double)pDark) / (double)pSky;

		if (xRate == 0.0)
			intTime = MAX_EXPOSURETIME;
		else {
			intTime = (long)(xRate*intT);
			if (intTime > MAX_EXPOSURETIME) {
				intTime = MAX_EXPOSURETIME;
			}
			if (intTime < MIN_EXPOSURETIME)
				intTime = MIN_EXPOSURETIME;
		}

	}
	else
	{
		intTime = m_fixexptime;
		if (intTime < MIN_EXPOSURETIME)
			intTime = MIN_EXPOSURETIME;
		else if (intTime > 65535)
			intTime = 65535;
	}


	return intTime;
}



/** This function is to count the flux between two points
** if the position is (0,0), calculate average distance when the coordinates go above 0.
** @windAngle - wind angle with the North
** @windSpeed - wind speed
*/

double CSpectrometer::CountFlux(double windSpeed, double windAngle)
{
	double flux;
	double column, windFactor, distance, lat1, lat2, lon1, lon2;
	long   columnSize;
	double  accColumn = 0;	//total column when gps position is 0
	if (posFlag == 0)		// when the gps coordinate is (0,0)
	{
		zeroPosNum++;
		columnSize = m_fitRegion[0].vColumn[0].GetSize();
		accColumn += 1E-6*(m_fitRegion[0].vColumn[0].GetAt(columnSize - 1))*m_gasFactor;
		flux = 0;
		m_flux += flux;
		WriteFluxLog();
		return flux;
	}
	else
	{
		columnSize = m_fitRegion[0].vColumn[0].GetSize();
		lat1 = pos[counter - 1].latitude;
		lat2 = pos[counter - 2].latitude;
		lon1 = pos[counter - 1].longitude;
		lon2 = pos[counter - 2].longitude;

		if ((lat2 == 0) && (lon2 == 0)) // when the gps coordinate just become not equal to (0,0)
		{
			lat2 = pos[counter - 2 - zeroPosNum].latitude;
			lon2 = pos[counter - 2 - zeroPosNum].longitude;
			distance = GPSDistance(lat1, lon1, lat2, lon2) / (zeroPosNum + 1);
			column = 1E-6*(m_fitRegion[0].vColumn[0].GetAt(columnSize - 1))*m_gasFactor + accColumn;
		}
		else					// the gps coordinate is not (0,0)
		{
			distance = GPSDistance(lat1, lon1, lat2, lon2);
			column = 1E-6*(m_fitRegion[0].vColumn[0].GetAt(columnSize - 1))*m_gasFactor;
		}
		accColumn = 0;
		zeroPosNum = 0;
		windFactor = GetWindFactor(lat1, lon1, lat2, lon2, windAngle);
		flux = column * distance * windSpeed * windFactor;

		m_flux += flux;
		WriteFluxLog();
		return flux;
	}
}
/** This function is to return the current column value and angle(in degree)
**
*/
double* CSpectrometer::GetLastColumn() {
	m_result[0] = evaluateResult[0][0][0];	//column
	m_result[1] = evaluateResult[0][0][1];	//columnError
	m_result[2] = evaluateResult[0][0][2];	//shift
	m_result[3] = evaluateResult[0][0][3];	//shiftError
	m_result[4] = evaluateResult[0][0][4];	//squeeze
	m_result[5] = evaluateResult[0][0][5];	//squeezeError

	return m_result;
}

void CSpectrometer::GetDark() {
	m_NChannels = std::max(std::min(m_NChannels, MAX_N_CHANNELS), 1);

	if (m_fixexptime >= 0) {
		// fixed exposure-time throughout the traverse
		for (int j = 0; j < m_NChannels; ++j) {
			for (int i = 0; i < MAX_SPECTRUM_LENGTH; ++i) {
				tmpDark[j][i] = dark[j][i];
			}
		}
	}
	else {
		// Adaptive exposure-time
		for (int j = 0; j < m_NChannels; ++j) {
			for (int i = 0; i < MAX_SPECTRUM_LENGTH; ++i) {
				tmpDark[j][i] = offset[j][i] + darkCur[j][i] * (integrationTime / DARK_CURRENT_EXPTIME);
			}
		}
	}
}

void CSpectrometer::GetSky() {

	m_NChannels = std::max(std::min(m_NChannels, MAX_N_CHANNELS), 1);

	for (int j = 0; j < m_NChannels; ++j) {
		for (int i = 0; i < MAX_SPECTRUM_LENGTH; ++i) {
			tmpSky[j][i] = sky[j][i];
		}
	}
}

int CSpectrometer::Stop()
{
	// Stop this thread
	fRun = FALSE;

	// Also stop the GPS-reading thread
	if (m_gps != nullptr) {
		m_gps->Stop();
	}

	return 1;
}

int CSpectrometer::Start()
{
	fRun = TRUE;
	return 1;
}


/* Return value = 0 if all is ok, else 1 */
int CSpectrometer::ReadReferenceFiles() {
	CString refFileList[10];
	CString tmpFileName, errorMessage;

	for (int j = 0; j < m_fitRegionNum; ++j) {
		for (int k = 0; k < m_fitRegion[j].window.nRef; ++k) {
			if (IsExistingFile(m_fitRegion[j].window.ref[k].m_path)) {
				refFileList[k].Format("%s", m_fitRegion[j].window.ref[k].m_path);
			}
			else {
				tmpFileName.Format("%s\\%s", g_exePath, m_fitRegion[j].window.ref[k].m_path);
				if (IsExistingFile(tmpFileName)) {
					refFileList[k].Format("%s", tmpFileName);
				}
				else {
					errorMessage.Format("Can not read reference file\n %s\n Please check the file location and restart collection", m_fitRegion[j].window.ref[k].m_path);
					MessageBox(pView->m_hWnd, errorMessage, TEXT("Error"), MB_OK);
					return 1;
				}
			}
		}

		/* Init the Master Channel Evaluator */
		if (!(m_fitRegion[j].eval[0]->ReadRefList(refFileList, m_fitRegion[j].window.nRef, m_detectorSize))) {
			MessageBox(pView->m_hWnd, TEXT("Can not read reference file\n Please check the file location and restart collection"), TEXT("Error"), MB_OK);
			return 1;
		}

		/* Init the 1:st Slave Channel Evaluator */
		if (!(m_fitRegion[j].eval[1]->ReadRefList(refFileList, m_fitRegion[j].window.nRef, m_detectorSize))) {
			MessageBox(pView->m_hWnd, TEXT("Can not read reference file\n Please check the file location and restart collection"), TEXT("Error"), MB_OK);
			return 1;
		}
	}

	return 0;
}



void CSpectrometer::DoEvaluation(double pSky[][MAX_SPECTRUM_LENGTH], double pDark[][MAX_SPECTRUM_LENGTH], double pSpectrum[][MAX_SPECTRUM_LENGTH]) {
	CString fileName;
	double curColumn[8];
	double curColumnError[8];

	memset(m_fitResult, 0, MAX_FIT_WINDOWS*MAX_SPECTRUM_LENGTH * sizeof(double));

	for (int j = 0; j < m_fitRegionNum; ++j) {
		int chn = m_fitRegion[j].window.channel;

		// Evaluate
		m_fitRegion[j].eval[chn]->Evaluate(pDark[chn], pSky[chn], pSpectrum[chn]);

		// Store the results
		double *tmpResult = m_fitRegion[j].eval[chn]->GetResult();
		evaluateResult[j][chn][0] = tmpResult[0];
		evaluateResult[j][chn][1] = tmpResult[1];
		evaluateResult[j][chn][2] = tmpResult[2];
		evaluateResult[j][chn][3] = tmpResult[3];
		evaluateResult[j][chn][4] = tmpResult[4];
		evaluateResult[j][chn][5] = tmpResult[5];

		curColumn[chn] = evaluateResult[j][chn][0];
		curColumnError[chn] = evaluateResult[j][chn][1];

		// Save the results in the lists
		for (int i = 0; i < 6; ++i) {
			m_fitRegion[j].vColumn[i].Append(evaluateResult[j][chn][i]);
		}

		// copy the high pass filtered spectrum
		for (int i = 0; i < m_fitRegionNum; ++i) {
			memcpy(m_spectrum[i], m_fitRegion[i].eval[chn]->m_filteredSpectrum, MAX_SPECTRUM_LENGTH * sizeof(double));
		}

		// copy the fitted reference
		for (int r = 0; r < m_fitRegion[j].window.nRef + 1; ++r) {
			for (int i = m_fitRegion[j].window.fitLow; i < m_fitRegion[j].window.fitHigh; ++i) {
				m_fitResult[j][i] += m_fitRegion[j].eval[chn]->m_fitResult[r].GetAt(i);
			}
		}

		fileName.Format("evaluationLog_%s.txt", m_fitRegion[j].window.name);
		WriteEvFile(fileName, &m_fitRegion[j]);
	}

	Sing(curColumn[0] / m_maxColumn);
	pView->PostMessage(WM_DRAWCOLUMN);

	++counter;
	if (counter == 65535) {
		memset((void*)pos, 0, sizeof(struct position) * 65536);
		memset((void*)specTime, 0, sizeof(long) * 65536);
		counter = 0;
		zeroPosNum = 0;
	}
}


/**Create directories for the data files
*
*/

void CSpectrometer::CreateDirectories()
{
	char dateText[11];
	char cDateTime[14];
	int strLength;
	struct tm *tim;
	time_t t;

	strBaseName.Format(m_Base);
	strBaseName.TrimLeft(' ');
	strBaseName.TrimRight(' ');
	strLength = strBaseName.GetLength();
	if (strLength == 0)
		strBaseName = TEXT("base1");

	time(&t);
	tim = localtime(&t);
	sprintf(cDateTime, "%04d%02d%02d_%02d%02d", tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday, tim->tm_hour, tim->tm_min);

	Common::GetDateText(dateText);

	CString folderName(g_exePath + dateText);

	if (0 == CreateDirectory(folderName, NULL)) {
		DWORD errorCode = GetLastError();
		if (errorCode != ERROR_ALREADY_EXISTS) { /* We shouldn't quit just because the directory that we want to create already exists. */
			CString tmpStr, errorStr;
			if (FormatErrorCode(errorCode, errorStr)) {
				tmpStr.Format("Could not create output directory. Reason: %s", errorStr);
			}
			else {
				tmpStr.Format("Could not create output directory, not enough free disk space?. Error code returned %ld", errorCode);
			}
			MessageBox(pView->m_hWnd, tmpStr, "ERROR", MB_OK);
			exit(EXIT_FAILURE);
		}
	}

	strDateTime = TEXT(cDateTime);
	m_subFolder = folderName + TEXT("\\") + strBaseName + TEXT("_") + TEXT(cDateTime);

	if (0 == CreateDirectory(m_subFolder, NULL)) {
		DWORD errorCode = GetLastError();
		CString tmpStr, errorStr;
		if (FormatErrorCode(errorCode, errorStr)) {
			tmpStr.Format("Could not create output directory. Reason: %s", errorStr);
		}
		else {
			tmpStr.Format("Could not create output directory, not enough free disk space?. Error code returned %ld", errorCode);
		}
		MessageBox(pView->m_hWnd, tmpStr, "ERROR", MB_OK);
		exit(EXIT_FAILURE);
	}
}




void CSpectrometer::SetFileName()
{
	static long lastidx = 0; /* The number of the last std-file written */

	int i, j;

	if (m_fixexptime >= 0) {
		// Normal mode, fixed exposure time
		for (j = 0; j < m_NChannels; ++j) {
			i = lastidx;
			do {
				if (scanNum == 1)
					m_stdfileName[j].Format("\\dark_%d.STD", j);
				else if (scanNum == 2)
					m_stdfileName[j].Format("\\sky_%d.STD", j);
				else
					m_stdfileName[j].Format("\\%05d_%d.STD", i, j);

				m_stdfileName[j] = m_subFolder + m_stdfileName[j];
				i++;
			} while (IsExistingFile(m_stdfileName[j]));
		}
	}
	else {
		// Automatic adjustment of the exposure-time
		for (j = 0; j < m_NChannels; ++j) {
			i = lastidx;
			do {
				if (scanNum == 1)
					m_stdfileName[j].Format("\\offset_%d.STD", j);
				else if (scanNum == 2)
					m_stdfileName[j].Format("\\darkcur_%d.STD", j);
				else if (scanNum == 3)
					m_stdfileName[j].Format("\\sky_%d.STD", j);
				else
					m_stdfileName[j].Format("\\%05d_%d.STD", i, j);

				m_stdfileName[j] = m_subFolder + m_stdfileName[j];
				i++;
			} while (IsExistingFile(m_stdfileName[j]));
		}
	}

	lastidx = i - 1;
}

/**Get columns
**@columnList - the array to contain columns
**@sum		  - the number of columns
*/
long CSpectrometer::GetColumns(double *columnList, long sum, int fitRegion)
{
	int i = 0;

	long columnSize = m_fitRegion[fitRegion].vColumn[0].GetSize();
	if (columnSize > 0)
	{
		if (sum > columnSize)
			sum = columnSize;

		for (i = 0; i < sum; ++i) {
			columnList[i] = m_fitRegion[fitRegion].vColumn[0].GetAt(columnSize - sum + i);
		}
	}

	return i;
}

/**Get columnerrors
**@columnList - the array to contain column errors
**@sum		  - the number of column errors
*/
long CSpectrometer::GetColumnErrors(double *columnErrList, long sum, int fitRegion)
{
	int i = 0;

	long columnSize = m_fitRegion[fitRegion].vColumn[0].GetSize();
	if (columnSize > 0)
	{
		if (sum > columnSize)
			sum = columnSize;

		for (i = 0; i < sum; ++i) {
			columnErrList[i] = m_fitRegion[fitRegion].vColumn[1].GetAt(columnSize - sum + i);
		}
	}

	return i;
}

/* Get the gps-positions */
long CSpectrometer::GetLatLongAlt(double *la, double *lo, double *al, long sum) {
	int i = 0;

	/* Check that we dont return more values than what we've got */
	long columnSize = m_fitRegion[0].vColumn[0].GetSize();
	if (columnSize > 0) {
		if (sum > columnSize)
			sum = columnSize;

		for (i = 0; i < sum; i++) {
			if (la != 0)
				la[i] = pos[i].latitude;
			if (lo != 0)
				lo[i] = pos[i].longitude;
			if (al != 0)
				al[i] = pos[i].altitude;
		}
	}

	return i;
}

/* returns the latest position */
void CSpectrometer::GetCurrentPos(double *la, double *lo, double *al) {
	long nColumns = m_fitRegion[0].vColumn[0].GetSize();

	if (la != 0)
		la[0] = pos[nColumns - 1].latitude;
	if (lo != 0)
		lo[0] = pos[nColumns - 1].longitude;
	if (al != 0)
		al[0] = pos[nColumns - 1].altitude;
}

long CSpectrometer::GetCurrentGPSTime() {
	long nColumns = m_fitRegion[0].vColumn[0].GetSize();

	return this->specTime[nColumns];
}

long CSpectrometer::GetIntensity(double *list, long sum)
{
	int i = 0;

	long size = vIntensity.GetSize();
	if (size > 0)
	{
		if (sum > size)
			sum = size;

		for (i = 0; i < sum; i++)
		{

			list[i] = vIntensity.GetAt(size - sum + i);

		}

	}

	return i;
}

long CSpectrometer::GetColumnNumber()
{
	long columnSize = m_fitRegion[0].vColumn[0].GetSize();
	return columnSize;
}

void CSpectrometer::GetNSpecAverage(int N[]) {
	N[0] = this->m_sumInSpectrometer;
	N[1] = this->m_sumInComputer;
	return;
}

void CSpectrometer::WriteFluxLog()
{
	FILE *f;
	CString fileName = m_subFolder + "\\" + strBaseName + "_" + strDateTime + TEXT("FluxLog.txt");
	f = fopen(fileName, "a+");

	if (f < (FILE*)1)
		return;

	fprintf(f, "%f\n", m_flux);

	fclose(f);
}



int CSpectrometer::GetGpsPos(gpsData& data) const
{
	const int c = this->counter; // local buffer, to avoid race conditions

	data.latitude    = pos[c].latitude;
	data.longitude   = pos[c].longitude;
	data.time        = specTime[c];
	data.altitude    = pos[c].altitude;
	data.nSatellites = pos[c].nSat;

	return c;
}

void CSpectrometer::Sing(double factor)
{
	CString fileToPlay;
	TCHAR windowsDir[MAX_PATH + 1];
	GetWindowsDirectory(windowsDir, MAX_PATH + 1);
	DWORD volume;

	if (factor > 0)
	{
		volume = (DWORD)(0xFFFF * factor);
		MMRESULT res = waveOutSetVolume(0, volume);
		fileToPlay.Format("%s\\Media\\ringout.wav", windowsDir);
	}
	else
	{
		volume = (DWORD)(0xFFFF * (fabs(factor) + 0.0000001));
		MMRESULT res = waveOutSetVolume(0, volume);
		fileToPlay.Format("%s\\Media\\start.wav", windowsDir);
	}

	PlaySound(fileToPlay, 0, SND_SYNC);
}

long CSpectrometer::AverageIntens(double *pSpectrum, long ptotalNum) {
	double sum = 0.0;
	long num;
	int j;
	if (m_conf->m_specCenter <= OFFSET)
		m_conf->m_specCenter = OFFSET;
	if (m_conf->m_specCenter >= MAX_SPECTRUM_LENGTH - OFFSET)
		m_conf->m_specCenter = MAX_SPECTRUM_LENGTH - 2 * OFFSET;

	for (j = m_conf->m_specCenter - OFFSET; j < m_conf->m_specCenter + OFFSET; j++) {
		sum += pSpectrum[j];
	}

	if (ptotalNum != 0) {
		num = 2 * OFFSET*ptotalNum;
		sum = fabs(sum / (double)num);
	}
	else {
		num = 2 * OFFSET;
		sum = fabs(sum / (double)num);
		MessageBox(pView->m_hWnd, TEXT("TOTAL SUM = 0"), TEXT("ERROR"), MB_OK);
		//Show information on screen
	}
	return (long)sum;
}

bool CSpectrometer::UpdateGpsData() {
	if (nullptr == m_gps) {
		return false;
	}

	bool gpsDataIsValid = true;

	m_gps->GetDate(specDate[counter]);
	specTime[counter] = m_gps->GetTime();
	pos[counter].latitude = m_gps->GetLatitude();
	pos[counter].longitude = m_gps->GetLongitude();
	pos[counter].altitude = m_gps->GetAltitude();
	pos[counter].nSat = m_gps->GetNumberOfSatellites();

	if ((pos[counter].latitude == 0.0) && (pos[counter].longitude == 0.0)) {
		// Invalid data, no latitude / longitude could be retrieved
		gpsDataIsValid = false;
	}
	else if (pos[counter].nSat == 0) {
		// Invalid data, the receiver couldn't connect to any satellite
		gpsDataIsValid = false;
	}

	pView->PostMessage(WM_READGPS);

	return gpsDataIsValid;
}


void CSpectrometer::WriteLogFile(CString filename, CString txt) {
	FILE *f;
	f = fopen(filename, "a+");

	if (f < (FILE*)1) {
		CString tmpStr;
		tmpStr.Format("Could not write log file: %s. Not enough free space?", filename);
		MessageBox(pView->m_hWnd, tmpStr, "Big Error", MB_OK);
		return;
	}

	fprintf(f, txt + "\n");

	fclose(f);
}

void CSpectrometer::WriteBeginEvFile(int fitRegion) {

	// Write a copy of the old cfg-file into the evaluation-log
	CString evPath = m_subFolder + "\\" + strBaseName + "_" + strDateTime + TEXT("evaluationLog_" + m_fitRegion[fitRegion].window.name + ".txt");
	CString str1, str2, str3, str4, str5, str6, str7, channelName;
	str1.Format("***Desktop Mobile Program***\nVERSION=%1d.%1d\nFILETYPE=evaluationlog\n", CVersion::majorNumber, CVersion::minorNumber);
	str2.Format("BASENAME=%s\nWINDSPEED=%f\nWINDDIRECTION=%f\n", m_Base, windSpeed, windAngle);
	str3 = TEXT("***copy of related configuration file ***\n");

	if (!fUseUSB)
		str4.Format("SPEC_BAUD=%d\nSERIALPORT=%s\nGPSBAUD=%d\nGPSPORT=%s\nTIMERESOLUTION=%d\n",
			serial.baudrate, serial.serialPort, m_GPSBaudRate, m_GPSPort, m_timeResolution);
	else
		str4.Format("SERIALPORT=USB\nGPSBAUD=%d\nGPSPORT=%s\nTIMERESOLUTION=%d\n",
			m_GPSBaudRate, m_GPSPort, m_timeResolution);

	str5.Format("FIXEXPTIME=%d\nFITFROM=%d\nFITTO=%d\nPOLYNOM=%d\n",
		m_fixexptime, m_fitRegion[fitRegion].window.fitLow, m_fitRegion[fitRegion].window.fitHigh, m_fitRegion[fitRegion].window.polyOrder);
	str5.AppendFormat("FIXSHIFT=%d\nFIXSQUEEZE=%d\n",
		m_fitRegion[fitRegion].window.ref[0].m_shiftOption == Evaluation::SHIFT_FIX, m_fitRegion[fitRegion].window.ref[0].m_squeezeOption == Evaluation::SHIFT_FIX);
	str6.Format("SPECCENTER=%d\nPERCENT=%f\nMAXCOLUMN=%f\nGASFACTOR=%f\n",
		m_conf->m_specCenter, m_percent, m_maxColumn, m_gasFactor);
	for (int k = 0; k < m_fitRegion[fitRegion].window.nRef; ++k) {
		str6.AppendFormat("REFFILE=%s\n", m_fitRegion[fitRegion].window.ref[k].m_path);
	}
	WriteLogFile(evPath, str1 + str2 + str3 + str4 + str5 + str6);

	// Write some additional information about the spectrometer
	str1.Format("***Spectrometer Information***\n");
	str1.AppendFormat("SERIAL=%s\n", spectrometerName);
	str1.AppendFormat("DETECTORSIZE=%d\n", m_detectorSize);
	str1.AppendFormat("DYNAMICRANGE=%d\n", m_spectrometerDynRange);
	str1.AppendFormat("MODEL=%s\n", m_spectrometerModel);
	WriteLogFile(evPath, str1);

	// The header-line
	if (m_fitRegion[fitRegion].window.channel == 0) {
		channelName.Format("Master");
	}
	else {
		channelName.Format("Slave");
	}

	str7.Format("\n#Time\tLat\tLong\tAlt\tNSpec\tExpTime\tIntens(%s)\t", channelName);
	for (int k = 0; k < m_fitRegion[fitRegion].window.nRef; ++k) {
		str7.AppendFormat("%s_Column_%s\t%s_ColumnError_%s\t",
			channelName, m_fitRegion[fitRegion].window.ref[k].m_specieName, channelName, m_fitRegion[fitRegion].window.ref[k].m_specieName);
	}
	str7.AppendFormat("STD-File(%s)\n", channelName);


	WriteLogFile(evPath, str7);
}

int CSpectrometer::CountRound(long timeResolution, long serialDelay, long gpsDelay, int* pResults)
{
	int i, index, sumOne, nRound;
	long results[15], rounds[15];
	double nSpec[15];
	long totalTime;
	double maxSpecPerTime;
	index = 0;
	sumOne = 0;
	nRound = 0;

	if (Equals(m_spectrometerModel, "USB2000+")) {
		// the USB2000+ can sum as many spectra as we want in the
		//	spectrometer, we therefore don't need to sum anything
		//	in the computer
		nRound = 1;

		sumOne = (int)(timeResolution / (1.1 * integrationTime));
	}
	else {

		for (sumOne = 1; sumOne <= 15; sumOne++) {
			nRound = (timeResolution - gpsDelay) / (sumOne*integrationTime + serialDelay);
			rounds[sumOne - 1] = nRound;
			totalTime = (sumOne*integrationTime + serialDelay)*nRound + gpsDelay;
			results[sumOne - 1] = totalTime;
			nSpec[sumOne - 1] = (double)(sumOne*nRound);
		}
		maxSpecPerTime = nSpec[0] / (double)results[0];

		for (i = 1; i < 15; ++i) {
			if (rounds[i - 1] > 0) {
				if (results[i] <= 1.1*timeResolution && nSpec[i] / (double)results[i] >= maxSpecPerTime) {
					maxSpecPerTime = nSpec[i] / (double)results[i];
					index = i;
				}
			}
		}
		sumOne = index + 1;
		nRound = (timeResolution - gpsDelay) / (sumOne*integrationTime + serialDelay);
		if (nRound <= 0) {
			sumOne = 1;
			nRound = 1;
		}
	}
	pResults[0] = sumOne;
	pResults[1] = nRound;
	return nRound;
}

double* CSpectrometer::GetSpectrum(int channel) {
	return curSpectrum[channel];
}

double* CSpectrometer::GetWavelengths(int channel) {
	return wavelength[channel];
}

/** This retrieves a list of all spectrometers that are connected to this computer */
void CSpectrometer::GetConnectedSpecs(CList <CString, CString&> &connectedSpectrometers) {
	// Clear the list
	connectedSpectrometers.RemoveAll();

	// Get the number of spectrometers attached to the computer
	int numberOfSpectrometersAttached = m_wrapper.openAllSpectrometers();

	for (int k = 0; k < numberOfSpectrometersAttached; ++k) {
		connectedSpectrometers.AddTail(CString(m_wrapper.getSerialNumber(k).getASCII()));
	}

	return;
}

int CSpectrometer::TestUSBConnection() {
	m_spectrometerIndex = 0; // assume that we will use spectrometer #1

	m_statusMsg.Format("Searching for attached spectrometers"); pView->PostMessage(WM_STATUSMSG);

	// Get the number of spectrometers attached to the computer
	m_numberOfSpectrometersAttached = m_wrapper.openAllSpectrometers();

	// Check the number of spectrometers attached
	if (m_numberOfSpectrometersAttached == -1) {
		// something went wrong!
		m_wrapper.getLastException();
	}
	else if (m_numberOfSpectrometersAttached == 0) {
		MessageBox(pView->m_hWnd, "No spectrometer found. Make sure that the spectrometer is attached properly to the USB-port and the driver is installed.", "Error", MB_OK);
		return 0;
	}
	else if (m_numberOfSpectrometersAttached > 1) {
		Dialogs::CSelectionDialog dlg;
		CString selectedSerial;

		m_statusMsg.Format("Several spectrometers found."); pView->PostMessage(WM_STATUSMSG);

		dlg.m_windowText.Format("Select which spectrometer to use");
		for (int k = 0; k < m_numberOfSpectrometersAttached; ++k) {
			dlg.m_option[k].Format(m_wrapper.getSerialNumber(k).getASCII());
		}
		dlg.m_currentSelection = &selectedSerial;
		dlg.DoModal();

		for (int k = 0; k < m_numberOfSpectrometersAttached; ++k) {
			if (Equals(selectedSerial, m_wrapper.getSerialNumber(k).getASCII())) {
				m_spectrometerIndex = k;
			}
		}
		spectrometerName.Format(selectedSerial);
	}
	else {
		spectrometerName.Format(m_wrapper.getSerialNumber(0).getASCII());
	}

	m_statusMsg.Format("Will use spectrometer %s.", spectrometerName); pView->PostMessage(WM_STATUSMSG);


	// Change the selected spectrometer. This will also fill in the parameters about the spectrometer
	this->m_spectrometerIndex = ChangeSpectrometer(m_spectrometerIndex);

	return 1;
}

/** This will change the spectrometer to use, to the one with the
	given spectrometerIndex. If no spectrometer exist with the given
	index then no changes will be made */
int CSpectrometer::ChangeSpectrometer(int selectedspec, int channel) {
	if (selectedspec < 0)
		return m_spectrometerIndex;

	// Check the number of spectrometers attached
	if (m_numberOfSpectrometersAttached == 0) {
		selectedspec = 0; // here it doesn't matter what the user wanted to have. there's only one spectrometer let's use it.
		return 0;
	}
	else {
		if (m_numberOfSpectrometersAttached > selectedspec) {
			m_spectrometerIndex = selectedspec;

			if (channel == 0) {
				this->m_spectrometerChannel = 0;
			}
			else if (channel > 0) {
				int nAvailableChannels = m_wrapper.getWrapperExtensions().getNumberOfChannels(m_spectrometerIndex);
				if (channel < nAvailableChannels) {
					m_spectrometerChannel = channel;
				}
				else {
					m_spectrometerChannel = nAvailableChannels - 1;
				}
			}
			else {
				this->m_spectrometerChannel = 0;
			}
		}
		else {
			return m_spectrometerIndex;
		}
	}

	// Tell the user that we have changed the spectrometer
	pView->PostMessage(WM_CHANGEDSPEC);

	// Get the spectrometer model
	m_spectrometerModel.Format(m_wrapper.getName(m_spectrometerIndex).getASCII());

	WrapperExtensions ext = m_wrapper.getWrapperExtensions();
	m_spectrometerDynRange = ext.getSaturationIntensity(m_spectrometerIndex);
	const int nofChannelsAvailable = ext.getNumberOfEnabledChannels(m_spectrometerIndex);


	if (m_NChannels > nofChannelsAvailable) {
		CString msg;
		msg.Format("Cfg.txt specifies %d channels to be used but spectrometer can only handle %d. Changing configuration to handle only %d channel(s)", m_NChannels, nofChannelsAvailable, nofChannelsAvailable);
		MessageBox(pView->m_hWnd, msg, "Error", MB_OK);
		m_NChannels = nofChannelsAvailable;
	}

	if (m_spectrometerDynRange < 0) {
		if (Equals(m_spectrometerModel, "USB4000") || Equals(m_spectrometerModel, "HR4000") ||
			Equals(m_spectrometerModel, "USB2000+") || Equals(m_spectrometerModel, "QE65000")) {
			m_spectrometerDynRange = 65536;
		}
		else {
			m_spectrometerDynRange = 4096;
		}
	}

	m_statusMsg.Format("Will use spectrometer #%d (%s).", m_spectrometerIndex, m_spectrometerModel); pView->PostMessage(WM_STATUSMSG);


	// Get a spectrum
	m_statusMsg.Format("Attempting to retrieve a spectrum from %s", spectrometerName); pView->PostMessage(WM_STATUSMSG);

	m_wrapper.setIntegrationTime(m_spectrometerIndex, m_spectrometerChannel, 3000); // use 3 ms exp-time
	m_wrapper.setScansToAverage(m_spectrometerIndex, m_spectrometerChannel, 1);		// only retrieve one single spectrum
	DoubleArray spectrumArray = m_wrapper.getSpectrum(m_spectrometerIndex);		// Retreives the spectrum from the spectrometer
	DoubleArray wavelengthArray = m_wrapper.getWavelengths(m_spectrometerIndex);	// Retreives the wavelengths of the spectrometer 
	m_detectorSize = spectrumArray.getLength();					// Sets numberOfPixels to the length of the spectrumArray 

	// Get the wavelength calibration from the spectrometer
	double *wavelengths = wavelengthArray.getDoubleValues();	// Sets a pointer to the values of the wavelength array 
	for (int k = 0; k < m_detectorSize; ++k) {
		wavelength[m_spectrometerChannel][k] = wavelengths[k];
	}

	m_statusMsg.Format("Detector size is %d", m_detectorSize); pView->PostMessage(WM_STATUSMSG);

	return m_spectrometerIndex;
}


int	CSpectrometer::CloseUSBConnection() {

	this->m_wrapper.closeAllSpectrometers();

	return 1;
}

void CSpectrometer::GetSpectrumInfo(double spectrum[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH]) {
	/* The nag flag makes sure that we dont remind the user to take a new dark
		spectrum too many times in a row. */
	static bool nagFlag = false;

	if (m_fixexptime < 0)
		nagFlag = true;

	// The board-temperature, if one could be retrieved
	double temperature = NOT_A_NUMBER;

	/* Cut from the S2000 manual:
		pixel   Description
			0-1     Not usable
			2-24    Optical dark pixels
			24-25   Transition pixels
			26-2074 Optical active pixels

			Cut from the USB4000 manual:
			 Pixel			Description
				 1–5				Not usable
				 6–18				Optical black pixels
				 19–21			Transition pixels
				 22–3669		Optical active pixels
				 3670–3681	Not usable
  */

  /* The offset is judged as the average intensity in pixels 6 - 18 */
	for (int n = 0; n < m_NChannels; ++n) {
		specInfo[n].offset = GetOffset(spectrum[n]);

		// Check if this spectrum is dark
		bool isDark = false;
		if (fabs(averageValue[n] - specInfo[n].offset) < 4.0)
			isDark = true;


		if (isDark) {
			specInfo[n].isDark = true;
			lastDarkOffset[n] = specInfo[n].offset;
			nagFlag = false;

			// use the new spectrum as dark spectrum
			memcpy((void*)dark, (void*)spectrum, sizeof(double) * 4096);

		}
		else {
			specInfo[n].isDark = false;

			/* If the offset level has changed alot since the last dark, encourage the user to take a new dark spectrum */
			if (lastDarkOffset[n] > 10) {/* Make sure this test is not run on the first spectrum collected */
				if (!nagFlag) {
					if (fabs(lastDarkOffset[n] - specInfo[n].offset) > 20.0) {
						pView->PostMessage(WM_SHOWDIALOG, DARK_DIALOG);
						nagFlag = true;
					}
				}
			}
		}
	}

	/** If possible, get the temperature of the spectrometer */
	if (m_wrapper.isFeatureSupportedBoardTemperature(m_spectrometerIndex) == 1) {
		// Board temperature feature is supported by this spectrometer
		BoardTemperature boardTemperature = m_wrapper.getFeatureControllerBoardTemperature(m_spectrometerIndex);
		temperature = boardTemperature.getBoardTemperatureCelsius();
	}

	/* Print the information to a file */
	CString fileName = m_subFolder + "\\" + strBaseName + "_" + strDateTime + "AdditionalLog.txt";
	FILE *f = nullptr;

	if (!IsExistingFile(fileName)) {
		f = fopen(fileName, "w");
		fprintf(f, "#--Additional log file to the Mobile DOAS program---\n");
		fprintf(f, "#This file has only use as test file for further development of the Mobile DOAS program\n\n");
		fprintf(f, "#SpectrumNumber\t");
		if (temperature != NOT_A_NUMBER) {
			fprintf(f, "BoardTemperature\t");
		}
		if (m_NChannels == 1) {
			fprintf(f, "Offset\tSpecCenterIntensity\tisDark\n");
		}
		else {
			fprintf(f, "#SpectrumNumber\tOffset(Master)\tSpecCenterIntensity(Master)\tisDark(Master)\tOffset(Slave)\tSpecCenterIntensity\tisDark(Slave)\t\n");
		}
		fclose(f);
	}

	f = fopen(fileName, "a+");
	if (f == nullptr) {
		this->m_statusMsg.Format("ERROR! Could not open the Additional Log file - Information has been lost!");
		pView->PostMessage(WM_STATUSMSG);
	}
	else {
		// Spectrum number
		fprintf(f, "%ld\t", counter);
		// The board temperature (?)
		if (temperature != NOT_A_NUMBER)
			fprintf(f, "%.3lf\t", temperature);

		// The master-channel
		fprintf(f, "%lf\t%ld\t%d", specInfo[0].offset, averageValue[0], specInfo[0].isDark);
		if (m_NChannels > 1)
			fprintf(f, "%lf\t%ld\t%d", specInfo[1].offset, averageValue[1], specInfo[1].isDark);

		fprintf(f, "\n");

		fclose(f);
	}
}

double CSpectrometer::GetOffset(double spectrum[MAX_SPECTRUM_LENGTH]) {
	double offset = 0.0;
	for (int i = 6; i < 18; ++i) {
		offset += spectrum[i];
	}
	offset /= 12;

	return offset;
}

void CSpectrometer::UpdateMobileLog() {
	char txt[256];

	/* Open the mobile log file */
	FILE *f = fopen(g_exePath + "MobileLog.txt", "r");

	if (0 == f) {
		/* File might not exist */
		f = fopen(g_exePath + "MobileLog.txt", "w");
		if (0 == f) {
			/* File cannot be opened */
			return;
		}
		else {
			fprintf(f, "BASENAME=%s\n", m_Base);
			fprintf(f, "WINDSPEED=%.2lf\n", windSpeed);
			fprintf(f, "WINDDIRECTION=%.2lf\n", windAngle);
			fclose(f);
			return;
		}
	}
	else {
		CString tmpStr;
		/* read in all the funny strings we dont understand */
		while (fgets(txt, sizeof(txt) - 1, f)) {
			if ((0 == strstr(txt, "BASENAME=")) && (0 == strstr(txt, "WINDSPEED=")) && (0 == strstr(txt, "WINDDIRECTION="))) {
				tmpStr.AppendFormat("%s", txt);
			}
		}
		fclose(f);
		f = fopen(g_exePath + "MobileLog.txt", "w");
		fprintf(f, "%s", (LPCTSTR)tmpStr);
		fprintf(f, "BASENAME=%s\n", m_Base);
		fprintf(f, "WINDSPEED=%.2lf\n", windSpeed);
		fprintf(f, "WINDDIRECTION=%.2lf\n", windAngle);

		fclose(f);
	}
}

short CSpectrometer::AdjustIntegrationTime() {
	double skySpec[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];
	m_sumInSpectrometer = 1;
	m_sumInComputer = 1;
	int darkInt = 0;
	int skyInt = 0;
	int oldIntTime = integrationTime;

	int highLimit = MAX_EXPOSURETIME; // maximum exposure time
	int lowLimit = MIN_EXPOSURETIME; // minimum exposure time

	// if the exposure time is set by the user, don't even bother to calculate it
	if (m_fixexptime > 0) {
		integrationTime = (short)m_fixexptime;
		return integrationTime;
	}

	// First make a try at setting the integration time by collecting one 
	//	spectrum at 10 ms exposure-time and one at 50 ms exposure-time
	if (-1 != AdjustIntegrationTime_Calculate(10, 50)) {
		return integrationTime;
	}

	// The clever setting failed... revert to simple trial and error...
	integrationTime = 100;
	while (1) {
		// if necessary, initialize the spectrometer
		if (!fUseUSB) {
			if (InitSpectrometer(0, integrationTime, m_sumInSpectrometer)) {
				serial.CloseAll();
				MessageBox(pView->m_hWnd, "Failed to initialize spectrometer", "Error", MB_OK);
				return -1;
			}
		}

		m_statusMsg.Format("Measuring the intensity");
		pView->PostMessage(WM_STATUSMSG);

		// measure the intensity
		if (Scan(1, m_sumInSpectrometer, skySpec)) {
			if (!fUseUSB)
				serial.CloseAll();
			return -1;
		}

		// Get the intensity of the sky and the dark spectra
		skyInt = AverageIntens(skySpec[0], 1);
		darkInt = (long)GetOffset(skySpec[0]);

		// Draw the measured sky spectrum on the screen.
		for (int i = 0; i < m_NChannels; ++i) {
			memcpy((void*)curSpectrum[i], skySpec[i], sizeof(double)*MAX_SPECTRUM_LENGTH);
		}
		if (m_fixexptime >= 0)
			pView->PostMessage(WM_DRAWSPECTRUM);
		pView->PostMessage(WM_SHOWINTTIME);

		// Check if we have reached our goal
		if (fabs(skyInt - m_spectrometerDynRange*m_percent) < 200)
			return integrationTime;

		// Adjust the exposure time so that we come closer to the desired value
		if (skyInt - m_spectrometerDynRange*m_percent > 0)
			highLimit = integrationTime;
		else
			lowLimit = integrationTime;

		// calculate the neccessary exposure time
		oldIntTime = integrationTime;
		integrationTime = (short)GetInttime(skyInt, darkInt, integrationTime);

		// if we don't change the exposure time, just quit.
		if (fabs(oldIntTime - integrationTime) < 0.1*integrationTime)
			return integrationTime;

		// check if we can reach the desired level at all
		if ((integrationTime == MAX_EXPOSURETIME) || (integrationTime == MIN_EXPOSURETIME)) {
			return integrationTime;
		}
	}

	return integrationTime;
}

short CSpectrometer::AdjustIntegrationTime_Calculate(long minExpTime, long maxExpTime) {
	double skySpec[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];
	m_sumInSpectrometer = 1;
	m_sumInComputer = 1;

	// Sanity check
	if (maxExpTime < minExpTime || (maxExpTime - minExpTime) < 2)
		return -1;

	integrationTime = (short)minExpTime;
	if (!fUseUSB) {
		if (InitSpectrometer(0, integrationTime, m_sumInSpectrometer)) {
			serial.CloseAll();
			MessageBox(pView->m_hWnd, "Failed to initialize spectrometer", "Error", MB_OK);
			return -1;
		}
	}
	// measure the intensity
	if (Scan(1, m_sumInSpectrometer, skySpec)) {
		if (!fUseUSB)
			serial.CloseAll();
		return -1;
	}
	int int_short = AverageIntens(skySpec[0], 1);

	integrationTime = (short)maxExpTime;
	if (!fUseUSB) {
		if (InitSpectrometer(0, integrationTime, m_sumInSpectrometer)) {
			serial.CloseAll();
			MessageBox(pView->m_hWnd, "Failed to initialize spectrometer", "Error", MB_OK);
			return -1;
		}
	}
	// measure the intensity
	if (Scan(1, m_sumInSpectrometer, skySpec)) {
		if (!fUseUSB)
			serial.CloseAll();
		return -1;
	}
	int int_long = AverageIntens(skySpec[0], 1);

	// This will only work if the spectrum is not saturated at the maximum exposure-time
	if (int_long > 0.9 * m_spectrometerDynRange) {
		return AdjustIntegrationTime_Calculate(minExpTime, (maxExpTime - minExpTime) / 2);
	}

	// Calculate the exposure-time
	integrationTime = (short)((m_spectrometerDynRange - int_short) * (maxExpTime - minExpTime) * m_percent / (int_long - int_short));

	if (integrationTime > MAX_EXPOSURETIME)
		integrationTime = MAX_EXPOSURETIME;
	if (integrationTime < MIN_EXPOSURETIME)
		integrationTime = MIN_EXPOSURETIME;

	// Try out the calculated intensity to see if it works...
	if (!fUseUSB) {
		if (InitSpectrometer(0, integrationTime, m_sumInSpectrometer)) {
			serial.CloseAll();
			MessageBox(pView->m_hWnd, "Failed to initialize spectrometer", "Error", MB_OK);
			return -1;
		}
	}
	if (Scan(1, m_sumInSpectrometer, skySpec)) {
		if (!fUseUSB)
			serial.CloseAll();
		return -1;
	}
	int finalInt = AverageIntens(skySpec[0], 1);

	int desiredInt = (int)(m_spectrometerDynRange * m_percent);
	if ((finalInt - desiredInt) / desiredInt > 0.2) {
		return -1;
	}
	else {
		// Draw the measured sky spectrum on the screen.
		for (int i = 0; i < m_NChannels; ++i) {
			memcpy((void*)curSpectrum[i], skySpec[i], sizeof(double)*MAX_SPECTRUM_LENGTH);
		}
		if (m_fixexptime >= 0)
			pView->PostMessage(WM_DRAWSPECTRUM);
		pView->PostMessage(WM_SHOWINTTIME);

		return integrationTime;
	}
}

std::string CSpectrometer::GetCurrentDate() {

	const bool couldReadValidGPSData = (m_useGps) ? UpdateGpsData() : false;

	if (!couldReadValidGPSData) {
		char startDate[7];

		struct tm *tim;
		time_t t;
		time(&t);
		tim = localtime(&t);
		int mon = tim->tm_mon+1;
		int day = tim->tm_mday;
		int year = tim->tm_year - 100; // only good for 21st century
		sprintf(startDate, "%02d%02d%02d", mon, day, year);

		return std::string(startDate, 6);
	}
	else {
		const int c = this->counter; // local buffer, to avoid race conditions...
		m_gps->GetDate(specDate[c]);
		return specDate[c];
	}
}

long CSpectrometer::GetCurrentTime() {
	long startTime = 0;

	const bool couldReadValidGPSData = (m_useGps) ? UpdateGpsData() : false;

	if (!couldReadValidGPSData) {
		startTime = GetTimeValue_UMT();
		specTime[counter] = startTime;
		pos[counter].altitude = pos[counter].latitude = pos[counter].longitude = pos[counter].nSat = 0;
	}
	else if (timeDiff == 0) {
		time_t t;
		time(&t);
		struct tm *localTime = localtime(&t);

		long gpstime = m_gps->GetTime();
		long hr = (long)gpstime / 10000;
		long mi = ((long)gpstime - hr * 10000) / 100;
		long se = ((long)gpstime) % 100;

		/* get the difference between the local time and the GPS-time */
		timeDiff = 3600 * (hr - localTime->tm_hour) + 60 * (mi - localTime->tm_min) + (se - localTime->tm_sec);
	}

	return startTime;
}