#include "stdafx.h"
#include "measurement_view.h"

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

CMeasurement_View::CMeasurement_View(void)
{
}

CMeasurement_View::~CMeasurement_View(void)
{
}

void CMeasurement_View::Run(){
	CString cfgFile;
	double scanResult[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];
	double tmpSpec[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];
	int i;

	MessageBox(pView->m_hWnd, TEXT("START "),TEXT("NOTICE"),MB_OK);

	// Read configuration file
	cfgFile = g_exePath + TEXT("cfg.xml");
	if(!IsExistingFile(cfgFile)){
		cfgFile = g_exePath + TEXT("cfg.txt");
	}
	m_conf = new Configuration::CMobileConfiguration(cfgFile);

	// Convert the settings from the CMobileConfiuration-format to the internal CSpectrometer-format
	ApplySettings();

	/* Set the delays and initialize the USB-Connection */
	if(fUseUSB){
		if(!TestUSBConnection()){
			fRun = false;
			return;
		}
	}

	/* -- Init Serial Communication -- */
	m_statusMsg.Format("Initializing communication with spectrometer");
	pView->PostMessage(WM_STATUSMSG);
	if(!fUseUSB && serial.InitCommunication()){
		MessageBox(pView->m_hWnd,TEXT("Can not initialize the communication"),TEXT("Error"),MB_OK);	

		// we have to call this before exiting the application otherwise we'll have trouble next time we start...
		CloseUSBConnection();

		return;
	}

	// Set the integration time
	if(0 == m_fixexptime){
		MessageBox(pView->m_hWnd, "Please point the spectrometer to sky","Notice",MB_OK);  // tell the user to point the telescope to zenith
		AdjustIntegrationTime();
	}else{
		integrationTime = (short)m_fixexptime;
	}

	m_sumInComputer      = 1;
	m_sumInSpectrometer  = 1;
	totalSpecNum         = 1;
	if(0 != m_fixexptime){
		MessageBox(pView->m_hWnd,  "Suitable exposure-time set", "", MB_OK);
	}
	pView->PostMessage(WM_SHOWINTTIME);


	/** --------------------- THE MEASUREMENT LOOP -------------------------- */
	while(fRun){

		/* ----------------  Get the spectrum --------------------  */

		// Initialize the spectrometer, if using the serial-port
		if(!fUseUSB){
			if(InitSpectrometer(0, integrationTime, m_sumInSpectrometer)){
				serial.CloseAll();
			}
		}

		// Get the next spectrum
		if(Scan(m_sumInComputer,m_sumInSpectrometer,scanResult)){
			if(!fUseUSB)
				serial.CloseAll();

			// we have to call this before exiting the application otherwise we'll have trouble next time we start...
			CloseUSBConnection();

			return;
		}

		// Copy the spectrum to the local variables
		for(i = 0; i < m_NChannels; ++i){
			memcpy((void*)tmpSpec[i], (void*)scanResult[i], sizeof(double)*MAX_SPECTRUM_LENGTH);
			memcpy((void*)curSpectrum[i], (void*)scanResult[i], sizeof(double)*MAX_SPECTRUM_LENGTH);// for plot
		}

		/* -------------- IF THE MEASURED SPECTRUM WAS A NORMAL SPECTRUM ------------- */

		for(i = 0; i < m_NChannels; ++i)
			averageValue[i] = AverageIntens(tmpSpec[i],1);

		if(specInfo->isDark)
			m_statusMsg.Format("Average value around center channel %d: %d (Dark)",m_conf->m_specCenter, averageValue[0]);
		else
			m_statusMsg.Format("Average value around center channel %d: %d",m_conf->m_specCenter, averageValue[0]);

		pView->PostMessage(WM_STATUSMSG);
		vIntensity.Append(averageValue[0]);

		pView->PostMessage(WM_DRAWSPECTRUM);
	}
	
	if(counter > 1)
		CountFlux(windSpeed,windAngle);

	memset((void*)scanResult,0,sizeof(double)*4096);
	scanNum++;
	
	// we have to call this before exiting the application otherwise we'll have trouble next time we start...
	CloseUSBConnection();
	
	return;
}