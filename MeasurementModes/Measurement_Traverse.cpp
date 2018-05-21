#include "stdafx.h"
#include "measurement_traverse.h"

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

CMeasurement_Traverse::CMeasurement_Traverse(void)
{
}

CMeasurement_Traverse::~CMeasurement_Traverse(void)
{
}

void CMeasurement_Traverse::Run(){
	CString cfgFile;
	double scanResult[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];
	double tmpSpec[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];

	std::string startDate;
	long startTime,elapsedSecond;
	clock_t cStart, cFinish;

	#ifdef _DEBUG
		/* timing the progress */
		double evSecond, writeSecond, gpsSecond, scanSecond;
		FILE *timerFile = 0;
	#endif
	CString tmpString;

	int roundResult[MAX_N_CHANNELS];
	long serialDelay,gpsDelay;

	MessageBox(pView->m_hWnd, TEXT("START "),TEXT("NOTICE"),MB_OK);

	// Read configuration file
	cfgFile = g_exePath + TEXT("cfg.xml");
	if(!IsExistingFile(cfgFile)){
		cfgFile = g_exePath + TEXT("cfg.txt");
	}
	m_conf = new Configuration::CMobileConfiguration(cfgFile);

	// Convert the settings from the CMobileConfiuration-format to the internal CSpectrometer-format
	ApplySettings();

	/* Check the settings in the configuration file */
	if(CheckSettings()){
		return;
	}

	/* Update the MobileLog.txt - file */
	UpdateMobileLog();

	/* Set the delays and initialize the USB-Connection */
	if(m_connectViaUsb){
		serialDelay = 10;
		if(!TestUSBConnection()){
			m_isRunning = false;
			return;
		}
	}else{
		serialDelay = 2300;
	}
	gpsDelay = 10;

	/* Error Check */
	if(serialDelay >= this->m_timeResolution){
		CString tmpStr;
		tmpStr.Format("Error In cfg.xml: The time resolution is smaller than the serial delay. Please Change and restart. Set Time Resolution = %d [s]. Set Serial Delay = %d [s]", this->m_timeResolution, serialDelay);
		MessageBox(pView->m_hWnd, tmpStr, "Error", MB_OK);
		
		// we have to call this before exiting the application otherwise we'll have trouble next time we start...
		CloseUSBConnection();
		return;
	}

	/* -- The Reference Files -- */
	// NB!! It is important that the USB-connection is tested before
	//	the references are read. Since the testing of the USB connection
	//	counts the number of pixels on the detector, and that number
	//	is here compared to the number of data-points found in the reference-files
	m_statusMsg.Format("Reading References");
	pView->PostMessage(WM_STATUSMSG);
	if(ReadReferenceFiles()){
		// we have to call this before exiting the application otherwise we'll have trouble next time we start...
		CloseUSBConnection();

		return;
	}

	/* Initialize the evaluator */
	for(int fitRgn = 0; fitRgn < m_fitRegionNum; ++fitRgn){
		for(int i = 0; i < m_NChannels; ++i){
			m_fitRegion[fitRgn].window.specLength = this->m_detectorSize;
			m_fitRegion[fitRgn].eval[i]->SetFitWindow(m_fitRegion[fitRgn].window);
		}
	}

	/* -- Init Serial Communication -- */
	m_statusMsg.Format("Initializing communication with spectrometer");
	pView->PostMessage(WM_STATUSMSG);
	if(!m_connectViaUsb && serial.InitCommunication()){
		MessageBox(pView->m_hWnd,TEXT("Can not initialize the communication"),TEXT("Error"),MB_OK);	

		// we have to call this before exiting the application otherwise we'll have trouble next time we start...
		CloseUSBConnection();

		return;
	}

	// Create the output directory and start the evaluation log file.
	CreateDirectories();
	for(int j = 0; j < m_fitRegionNum; ++j){
		WriteBeginEvFile(j);
	}

	/* Start the GPS collection thread */
	if(m_useGps){
		m_gps	= new CGPS(m_GPSPort, m_GPSBaudRate);
		m_gps->Run(); /* start the gps-reading thread */
	}

	// Check if we are to be running with adaptive or with fixed exposure-time
	if(m_fixexptime < 0){
		return Run_Adaptive();
	}

	// The various spectra to collect, this defines the order in which they are collected
	int DARK_SPECTRUM = 1;
	int SKY_SPECTRUM = 2;
	
	// Set the integration time
	if(0 == m_fixexptime){
		MessageBox(pView->m_hWnd, "Please point the spectrometer to sky","Notice",MB_OK);  // tell the user to point the telescope to zenith
		AdjustIntegrationTime();
	}else{
		m_integrationTime = (short)m_fixexptime;
	}

	/* Calculate the number of spectra to integrate in spectrometer and in computer */
	m_scanNum++;
	m_sumInComputer     = CountRound(m_timeResolution, serialDelay, gpsDelay, roundResult);
	m_sumInSpectrometer = roundResult[0];
	m_totalSpecNum      = m_sumInComputer*m_sumInSpectrometer;
	pView->PostMessage(WM_SHOWINTTIME);

	/*  -- Collect the dark spectrum -- */
	MessageBox(pView->m_hWnd, "Cover the spectrometer", "Notice", MB_OK);
	m_statusMsg.Format("Measuring the dark spectrum");
	pView->PostMessage(WM_STATUSMSG);


	/** --------------------- THE MEASUREMENT LOOP -------------------------- */
	while(m_isRunning){

		#ifdef _DEBUG
		cStart = clock();
		#endif

		SetFileName();

		/* ------------ Get the date, time and position --------------- */
		startDate = GetCurrentDate();
		startTime = GetCurrentTime();

		/** ---------------- if the user wants to change the exposure time, 
									calculate a new exposure time. --------------------- */
		if(m_adjustIntegrationTime && m_fixexptime >= 0){
			m_integrationTime       = AdjustIntegrationTime();
			pView->PostMessage(WM_SHOWDIALOG, CHANGED_EXPOSURETIME);
			m_adjustIntegrationTime = FALSE;
			m_sumInComputer         = CountRound(m_timeResolution, serialDelay, gpsDelay, roundResult);
			m_sumInSpectrometer     = roundResult[0];
			m_totalSpecNum          = m_sumInComputer*m_sumInSpectrometer;
			pView->PostMessage(WM_SHOWINTTIME);
		}

		/* ----------------  Get the spectrum --------------------  */
		#ifdef _DEBUG
		cFinish = clock();
		gpsSecond = ((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);
		#endif

		cStart = clock();

		// Initialize the spectrometer, if using the serial-port
		if(!m_connectViaUsb){
			if(InitSpectrometer(0, m_integrationTime, m_sumInSpectrometer)){
				serial.CloseAll();
			}
		}

		// Get the next spectrum
		if(Scan(m_sumInComputer,m_sumInSpectrometer,scanResult)){
			if(!m_connectViaUsb) {
				serial.CloseAll();
			}

			// we have to call this before exiting the application otherwise we'll have trouble next time we start...
			CloseUSBConnection();
			
			return;
		}

		cFinish = clock();
		elapsedSecond = (long)((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);

		#ifdef _DEBUG
			scanSecond = ((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);
			cStart = clock();
		#endif

		// Copy the spectrum to the local variables
		for(int i = 0; i < m_NChannels; ++i){
			memcpy((void*)tmpSpec[i], (void*)scanResult[i], sizeof(double)*MAX_SPECTRUM_LENGTH);
			memcpy((void*)m_curSpectrum[i], (void*)scanResult[i], sizeof(double)*MAX_SPECTRUM_LENGTH);// for plot
		}

		/* ----------------- Save the spectrum(-a) -------------------- */
		if(m_useGps){
			for(int i = 0; i  < m_NChannels; ++i)
				CSpectrumIO::WriteStdFile(m_stdfileName[i], tmpSpec[i], m_detectorSize, startDate, m_spectrumGpsData[m_spectrumCounter].time, m_spectrumGpsData[m_spectrumCounter].time + elapsedSecond, m_spectrumGpsData[m_spectrumCounter].latitude, m_spectrumGpsData[m_spectrumCounter].longitude, m_spectrumGpsData[m_spectrumCounter].altitude, m_integrationTime, m_spectrometerName, m_measurementBaseName, m_totalSpecNum);
		}else{
			for(int i = 0; i < m_NChannels; ++i)
				CSpectrumIO::WriteStdFile(m_stdfileName[i], tmpSpec[i], m_detectorSize, startDate, startTime, startTime+elapsedSecond, 0, 0, 0, m_integrationTime, m_spectrometerName, m_measurementBaseName, m_totalSpecNum);
		}

		#ifdef _DEBUG
			cFinish = clock();
			writeSecond = ((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);
			cStart = clock();
		#endif

		if(m_scanNum == DARK_SPECTRUM){
			/* -------------- IF THE MEASURED SPECTRUM WAS THE DARK SPECTRUM ------------- */
			memcpy((void*)m_dark, (void*)scanResult, sizeof(double)*MAX_N_CHANNELS*MAX_SPECTRUM_LENGTH);

			pView->PostMessage(WM_DRAWSPECTRUM);//draw dark spectrum
			for(int i = 0; i < m_NChannels; ++i) {
				m_averageSpectrumIntensity[i] = AverageIntens(scanResult[i],1);
			}
			m_statusMsg.Format("Average value around center channel(dark) %d: %d", m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
			pView->PostMessage(WM_STATUSMSG);

			/* Get the information about the spectrum */
			GetSpectrumInfo(scanResult);
			#ifndef _DEBUG
				if(!specInfo->isDark)
					MessageBox(pView->m_hWnd,  "It seems like the dark spectrum is not completely dark, consider restarting the program", "Error", MB_OK);
			#endif

			MessageBox(pView->m_hWnd, "Point the spectrometer to sky","Notice",MB_OK);

			m_statusMsg.Format("Measuring the sky spectrum");
			pView->PostMessage(WM_STATUSMSG);

		}else if(m_scanNum == SKY_SPECTRUM){
			/* -------------- IF THE MEASURED SPECTRUM WAS THE SKY SPECTRUM ------------- */

			memcpy((void*)m_sky, (void*)scanResult, sizeof(double)*MAX_N_CHANNELS*MAX_SPECTRUM_LENGTH);

			pView->PostMessage(WM_DRAWSPECTRUM);//draw sky spectrum

			for(int i = 0; i < m_NChannels; ++i){
				m_averageSpectrumIntensity[i] = AverageIntens(scanResult[i],1);

				// remove the dark spectrum
				for(int iterator = 0; iterator < MAX_SPECTRUM_LENGTH; ++iterator)
					m_sky[i][iterator] -= m_dark[i][iterator];

				// Tell the evaluator(s) that the dark-spectrum does not need to be subtracted from the sky-spectrum
				for(int fitRgn = 0; fitRgn < m_fitRegionNum; ++fitRgn){
					m_fitRegion[fitRgn].eval[i]->m_subtractDarkFromSky = false;
				}
			}

			m_statusMsg.Format("Average value around center channel(sky) %d: %d",m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
			pView->PostMessage(WM_STATUSMSG);

			/* Get the information about the spectrum */
			GetSpectrumInfo(scanResult);
			#ifndef _DEBUG
				if(specInfo->isDark)
				MessageBox(pView->m_hWnd,  "It seems like the sky spectrum is dark, consider restarting the program", "Error", MB_OK);
			#endif

		}else if(m_scanNum > SKY_SPECTRUM){
			/* -------------- IF THE MEASURED SPECTRUM WAS A NORMAL SPECTRUM ------------- */

			for(int i = 0; i < m_NChannels; ++i) {
				m_averageSpectrumIntensity[i] = AverageIntens(tmpSpec[i],1);
			}

			/* Get the information about the spectrum */
			GetSpectrumInfo(scanResult);

			if(m_specInfo->isDark)
				m_statusMsg.Format("Average value around center channel %d: %d (Dark)",m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
			else
				m_statusMsg.Format("Average value around center channel %d: %d",m_conf->m_specCenter, m_averageSpectrumIntensity[0]);

			pView->PostMessage(WM_STATUSMSG);
			vIntensity.Append(m_averageSpectrumIntensity[0]);

			/* Evaluate */
			GetDark();
			GetSky();
			DoEvaluation(m_tmpSky, m_tmpDark, scanResult);
		}
		
		if(m_spectrumCounter > 1)
			CountFlux(m_windSpeed, m_windAngle);

		#ifdef _DEBUG
			cFinish = clock();
			evSecond = ((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);

			/* aggressive timing */
			tmpString.Format("GPS-Reading took: %lf [s] \t Scanning took %lf [s] \t Evaluation took %lf [s] \t Writing to file took: %lf [s]\n", gpsSecond, scanSecond, evSecond, writeSecond);
			timerFile = fopen("times.txt", "a");
			fprintf(timerFile, tmpString);
			fclose(timerFile);
			timerFile = 0;
		#endif

		memset((void*)scanResult,0,sizeof(double)*4096);
		m_scanNum++;
	}
	
	// we have to call this before exiting the application otherwise we'll have trouble next time we start...
	CloseUSBConnection();

	return;
}

void CMeasurement_Traverse::Run_Adaptive(){
	CString cfgFile;
	double scanResult[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];
	double tmpSpec[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];

	int roundResult[MAX_N_CHANNELS];
	long serialDelay,gpsDelay;

	std::string startDate;
	long startTime,elapsedSecond;
	clock_t cStart, cFinish;

	/* Set the delays and initialize the USB-Connection */
	if(m_connectViaUsb){
		serialDelay = 10;
	}else{
		serialDelay = 2300;
	}
	gpsDelay = 10;

	// The various spectra to collect, this defines the order in which they are collected
	int OFFSET_SPECTRUM       = 1;
	int DARKCURRENT_SPECTRUM  = 2;
	int SKY_SPECTRUM          = 3;

	m_scanNum = OFFSET_SPECTRUM;

	// 1. Start collecting the offset spectrum.
	m_integrationTime     = 3;
	m_sumInComputer       = 400;
	m_sumInSpectrometer   = 15;
	m_totalSpecNum        = m_sumInSpectrometer * m_sumInComputer;
	pView->PostMessage(WM_SHOWINTTIME);

	/*  -- Collect the dark spectrum -- */
	MessageBox(pView->m_hWnd, "Cover the spectrometer", "Notice", MB_OK);
	m_statusMsg.Format("Measuring the offset spectrum");
	pView->PostMessage(WM_STATUSMSG);

	/** --------------------- THE MEASUREMENT LOOP -------------------------- */
	while(m_isRunning){

		cStart = clock();

		SetFileName();

		/* ------------ Get the date, time and position --------------- */
		startDate = GetCurrentDate();
		startTime = GetCurrentTime();
	
		// Initialize the spectrometer, if using the serial-port
		if(!m_connectViaUsb){
			if(InitSpectrometer(0, m_integrationTime, m_sumInSpectrometer)){
				serial.CloseAll();
			}
		}

		// Get the next spectrum
		if(Scan(m_sumInComputer,m_sumInSpectrometer,scanResult)){
			if(!m_connectViaUsb) {
				serial.CloseAll();
			}

			// we have to call this before exiting the application otherwise we'll have trouble next time we start...
			CloseUSBConnection();

			return;
		}

		cFinish = clock();
		elapsedSecond = (long)((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);

		// Copy the spectrum to the local variables
		for(int i = 0; i < m_NChannels; ++i){
			memcpy((void*)tmpSpec[i], (void*)scanResult[i], sizeof(double)*MAX_SPECTRUM_LENGTH);
			memcpy((void*)m_curSpectrum[i], (void*)scanResult[i], sizeof(double)*MAX_SPECTRUM_LENGTH);// for plot
		}

		/* ----------------- Save the spectrum(-a) -------------------- */
		if(m_useGps){
			const gpsData& spectrumGpsData = m_spectrumGpsData[m_spectrumCounter];
			for(int i = 0; i  < m_NChannels; ++i) {
				CSpectrumIO::WriteStdFile(m_stdfileName[i], tmpSpec[i], m_detectorSize, startDate, spectrumGpsData.time, spectrumGpsData.time + elapsedSecond, 
					spectrumGpsData.latitude, spectrumGpsData.longitude, spectrumGpsData.altitude, m_integrationTime, m_spectrometerName, m_measurementBaseName, m_totalSpecNum);
			}
		}else{
			for(int i = 0; i < m_NChannels; ++i) {
				CSpectrumIO::WriteStdFile(m_stdfileName[i], tmpSpec[i], m_detectorSize, startDate, startTime, startTime+elapsedSecond, 0, 0, 0, m_integrationTime, m_spectrometerName, m_measurementBaseName, m_totalSpecNum);
			}
		}

		if(m_scanNum == OFFSET_SPECTRUM){
			/* -------------- IF THE MEASURED SPECTRUM WAS THE OFFSET SPECTRUM ------------- */
			memcpy((void*)m_offset, (void*)scanResult, sizeof(double)*4096);

			pView->PostMessage(WM_DRAWSPECTRUM);//draw offset spectrum
			for(int i = 0; i < m_NChannels; ++i) {
				m_averageSpectrumIntensity[i] = AverageIntens(scanResult[i],1);
			}
			m_statusMsg.Format("Average value around center channel(offset) %d: %d", m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
			pView->PostMessage(WM_STATUSMSG);

			/* Get the information about the spectrum */
			GetSpectrumInfo(scanResult);
			#ifndef _DEBUG
				if(!specInfo->isDark)
				MessageBox(pView->m_hWnd,  "It seems like the offset spectrum is not completely dark, consider restarting the program", "Error", MB_OK);
			#endif

			m_statusMsg.Format("Measuring the dark_current spectrum");
			pView->PostMessage(WM_STATUSMSG);

			// Set the exposure-time and the number of spectra to co-add...
			m_integrationTime      = DARK_CURRENT_EXPTIME;
			m_sumInComputer        = 1;
			m_sumInSpectrometer    = 1;
			m_totalSpecNum         = m_sumInSpectrometer * m_sumInComputer;
			pView->PostMessage(WM_SHOWINTTIME);
		}else if(m_scanNum == DARKCURRENT_SPECTRUM){

			/* -------------- IF THE MEASURED SPECTRUM WAS THE DARK-CURRENT SPECTRUM ------------- */
			for(int j = 0; j < m_NChannels; ++j){
				for(int i = 0; i < MAX_SPECTRUM_LENGTH; ++i){
					scanResult[j][i] = scanResult[j][i] - m_offset[j][i];
				}
			}
			memcpy((void*)m_darkCur, (void*)scanResult, sizeof(double)*4096);

			pView->PostMessage(WM_DRAWSPECTRUM);//draw dark spectrum
			for(int i = 0; i < m_NChannels; ++i) {
				m_averageSpectrumIntensity[i] = AverageIntens(scanResult[i],1);
			}
			m_statusMsg.Format("Average value around center channel(dark current) %d: %d", m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
			pView->PostMessage(WM_STATUSMSG);

			/* Get the information about the spectrum */
			GetSpectrumInfo(scanResult);

				MessageBox(pView->m_hWnd, "Point the spectrometer to sky","Notice",MB_OK);

			m_statusMsg.Format("Measuring the sky spectrum");
			pView->PostMessage(WM_STATUSMSG);

			// Set the exposure-time
			m_integrationTime     = AdjustIntegrationTime();
			m_sumInComputer       = CountRound(m_timeResolution, serialDelay, gpsDelay, roundResult);
			m_sumInSpectrometer   = roundResult[0];
			m_totalSpecNum        = m_sumInComputer*m_sumInSpectrometer;
			pView->PostMessage(WM_SHOWINTTIME);
		}else if(m_scanNum == SKY_SPECTRUM){
			/* -------------- IF THE MEASURED SPECTRUM WAS THE SKY SPECTRUM ------------- */

			memcpy((void*)m_sky, (void*)scanResult, sizeof(double)*4096);

			pView->PostMessage(WM_DRAWSPECTRUM);//draw sky spectrum

			for(int i = 0; i < m_NChannels; ++i){
				m_averageSpectrumIntensity[i] = AverageIntens(scanResult[i],1);

				// remove the dark spectrum
				GetDark();
				for(int iterator = 0; iterator < MAX_SPECTRUM_LENGTH; ++iterator)
					m_sky[i][iterator] -= m_tmpDark[i][iterator];

				// Tell the evaluator(s) that the dark-spectrum does not need to be subtracted from the sky-spectrum
				for(int fitRgn = 0; fitRgn < m_fitRegionNum; ++fitRgn){
					m_fitRegion[fitRgn].eval[i]->m_subtractDarkFromSky = false;
				}
			}

			m_statusMsg.Format("Average value around center channel(sky) %d: %d",m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
			pView->PostMessage(WM_STATUSMSG);

			/* Get the information about the spectrum */
			GetSpectrumInfo(scanResult);
			#ifndef _DEBUG
				if(specInfo->isDark)
				MessageBox(pView->m_hWnd,  "It seems like the sky spectrum is dark, consider restarting the program", "Error", MB_OK);
			#endif

			// Set the exposure-time
			m_integrationTime			= AdjustIntegrationTime();
			m_sumInComputer				= CountRound(m_timeResolution, serialDelay, gpsDelay, roundResult);
			m_sumInSpectrometer			= roundResult[0];
			m_totalSpecNum				= m_sumInComputer*m_sumInSpectrometer;
			pView->PostMessage(WM_SHOWINTTIME);

		}else if(m_scanNum > SKY_SPECTRUM){
			/* -------------- IF THE MEASURED SPECTRUM WAS A NORMAL SPECTRUM ------------- */
			for(int i = 0; i < m_NChannels; ++i) {
				m_averageSpectrumIntensity[i] = AverageIntens(tmpSpec[i],1);
			}
	
			/* Get the information about the spectrum */
			GetSpectrumInfo(scanResult);

			if(m_specInfo->isDark)
				m_statusMsg.Format("Average value around center channel %d: %d (Dark)",m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
			else
				m_statusMsg.Format("Average value around center channel %d: %d",m_conf->m_specCenter, m_averageSpectrumIntensity[0]);

			pView->PostMessage(WM_STATUSMSG);
			vIntensity.Append(m_averageSpectrumIntensity[0]);

			/* Evaluate */
			GetDark();
			GetSky();
			DoEvaluation(m_tmpSky, m_tmpDark, scanResult);

			// Update the exposure-time
			//if(averageValue[0] >= 4000){
			m_integrationTime = AdjustIntegrationTime();
			//}else{
			//	integrationTime					= GetInttime(averageValue[0], GetOffset(scanResult[0]), integrationTime);
			//}
			m_sumInComputer				= CountRound(m_timeResolution, serialDelay, gpsDelay, roundResult);
			m_sumInSpectrometer			= roundResult[0];
			m_totalSpecNum				= m_sumInComputer*m_sumInSpectrometer;
			pView->PostMessage(WM_SHOWINTTIME);
		}
		
		if(m_spectrumCounter > 1)
			CountFlux(m_windSpeed, m_windAngle);

		memset((void*)scanResult,0,sizeof(double)*4096);
		m_scanNum++;
	}
	
	// we have to call this before exiting the application otherwise we'll have trouble next time we start...
	CloseUSBConnection();

	return;
}