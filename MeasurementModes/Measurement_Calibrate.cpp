#undef min
#undef max

#include "stdafx.h"
#include "measurement_calibrate.h"
#include "../Common.h"
#include <algorithm>

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

CMeasurement_Calibrate::CMeasurement_Calibrate(void)
{
	m_calibration = new CWavelengthCalibration();
}

CMeasurement_Calibrate::~CMeasurement_Calibrate(void)
{
	if(m_calibration != nullptr){
		delete m_calibration;
		m_calibration = nullptr;
	}
}

void CMeasurement_Calibrate::Run(){
	CString cfgFile;
	double scanResult[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];

	ShowMessageBox("Start. Please connect the spectrometer and mercury lamp.", "NOTICE");

	// Read configuration file. This is basically to figure out whether we should
	//	be using the serial or USB port
	cfgFile = g_exePath + TEXT("cfg.xml");
	if(!IsExistingFile(cfgFile)){
		cfgFile = g_exePath + TEXT("cfg.txt");
	}
	m_conf.reset(new Configuration::CMobileConfiguration(cfgFile));

	// Convert the settings from the CMobileConfiuration-format to the internal CSpectrometer-format
	ApplySettings();

	/* Set the delays and initialize the USB-Connection */
	if(m_connectViaUsb){
		if(!TestUSBConnection()){
			m_isRunning = false;
			return;
		}
	}

	/* -- Init Serial Communication -- */
	m_statusMsg.Format("Initializing communication with spectrometer");
	pView->PostMessage(WM_STATUSMSG);
	if(!m_connectViaUsb && serial.InitCommunication()){
		ShowMessageBox("Can not initialize the communication", "Error");
		return;
	}

	// Set the integration time to 3 ms initially
	m_integrationTime    = 3;
	m_sumInComputer      = 10;
	m_sumInSpectrometer  = 15;
	m_totalSpecNum       = 150;
	pView->PostMessage(WM_SHOWINTTIME);


	/** --------------------- THE MEASUREMENT LOOP -------------------------- */
	while(m_isRunning){

		/* ----------------  Get the spectrum --------------------  */
		// Initialize the spectrometer, if using the serial-port
		if(!m_connectViaUsb){
			if(InitSpectrometer(0, m_integrationTime, m_sumInSpectrometer)){
				serial.CloseAll();
			}
		}

		// Get the next spectrum
		if(Scan(m_sumInComputer,m_sumInSpectrometer,scanResult)){
			if(!m_connectViaUsb)
				serial.CloseAll();
			else
				CloseUSBConnection();
			return;
		}

		// Copy the spectrum to the local variables
		memcpy((void*)m_lastSpectrum,	(void*)scanResult[0], sizeof(double)*m_detectorSize);
		memcpy((void*)m_curSpectrum[0],	(void*)scanResult[0], sizeof(double)*m_detectorSize);// for plot

		// Assign a number of mercury lines to the measured spectrum
		AssignLines();

		pView->PostMessage(WM_DRAWSPECTRUM);
	}
	
	memset((void*)scanResult,0,sizeof(double)*4096);
	m_scanNum++;
	
	// we have to call this before exiting the application otherwise we'll have trouble next time we start...
	CloseUSBConnection();
	
	return;
}

	/** Looks at the current spectrum and assigns a 
		mercury line to each of the peaks in the spectrum */
void CMeasurement_Calibrate::AssignLines(){
	double lowestValues[100];
	int indices[100];
	int k; // pixel iterator
	int j; // peak iterator
	Peak peaks[100]; // the peaks found
	int nFoundPeaks = 0;
	int nSaturatedPeaks = 0;
	bool isDoubleLine;
	
	// Reset the prior calibration
	m_calibration->m_lineNum = 0;
	
	// First of all remove the offset from the spectrum
	if(!FindNLowest(m_lastSpectrum, m_detectorSize, lowestValues, 100, indices))
		return;
	
	// As a first approximation take the offset as the average of the 100 lowest pixel-values
	//	(as the second order approximation use a linear or quadratic fit...)
	double offset = Average(lowestValues, 100);

	// this is the lower limit for when we can say that we have a peak
	double lowLimit = (m_spectrometerDynRange - offset) * 0.30;
	double highLimit = m_spectrometerDynRange * 0.95;
	
	// Scan the spectrum and find the peaks (avoid the edges)
	bool inPeak = false;
	for(k = 30; k < m_detectorSize - 30; ++k){
		if(inPeak){
			if(m_lastSpectrum[k] < lowLimit || m_lastSpectrum[k+1] < lowLimit){
				peaks[nFoundPeaks].highPixel = k - 1;
				++nFoundPeaks;
				inPeak = false;
				continue;
			}
			if(m_lastSpectrum[k] > highLimit){
				peaks[nFoundPeaks].saturated = true;
				++nSaturatedPeaks;
			}		
			peaks[nFoundPeaks].maxIntensity = std::max(m_lastSpectrum[k], peaks[nFoundPeaks].maxIntensity);

		}else{
			if(m_lastSpectrum[k] < lowLimit || m_lastSpectrum[k+1] < lowLimit)
				continue;
			// here [k] and [k+1] > lowLimit, we have a new peak!
			peaks[nFoundPeaks].lowPixel = k;
			peaks[nFoundPeaks].maxIntensity = m_lastSpectrum[k];
			peaks[nFoundPeaks].saturated = false; // assume that this peak is not saturated
			inPeak = true;
		}
	}
	
	if(nFoundPeaks - nSaturatedPeaks == 0){
		return; // no useful peaks found...
	}

	// For each line find the centre of mass and assign a pixel value to this
	for(j = 0; j < nFoundPeaks; ++j){

		double sumWeights = 0.0;
		double sumPixels  = 0.0;
		for(k = peaks[j].lowPixel; k <= peaks[j].highPixel; ++k){
			sumWeights += m_lastSpectrum[k] * (double)k;
			sumPixels  += m_lastSpectrum[k];
		}
		
		double centreOfMass = sumWeights / sumPixels;
		
		// what wavelength does this correspond to?
		double approximateWavelength = m_wavelength[0][(int)(round(centreOfMass))];
		
		// Get the HgLine that is closest to this wavelength
		double hgLine = CWavelengthCalibration::GetNearestHgLine(approximateWavelength, isDoubleLine);

		if(hgLine > 0.0){
			m_calibration->m_lines[m_calibration->m_lineNum].pixelNumber	= centreOfMass;
			m_calibration->m_lines[m_calibration->m_lineNum].wavelength		= hgLine;
			m_calibration->m_lines[m_calibration->m_lineNum].maxIntensity	= peaks[j].maxIntensity / m_spectrometerDynRange;
			m_calibration->m_lines[m_calibration->m_lineNum].use			= !(isDoubleLine || peaks[j].saturated);
			m_calibration->m_lineNum++;
		}
	}
}
