#include "stdafx.h"
#include "measurement_wind.h"

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

CMeasurement_Wind::CMeasurement_Wind(void)
{
}

CMeasurement_Wind::~CMeasurement_Wind(void)
{
}

void CMeasurement_Wind::Run(){
	CString cfgFile;
	double scanResult[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];
	double tmpSpec[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];
	int i, fitRgn;

	char* startDate;
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
	if(fUseUSB){
		serialDelay = 10;
		if(!TestUSBConnection()){
			fRun = false;
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
	if(m_spectrometerMode != MODE_VIEW){
		m_statusMsg.Format("Reading References");
		pView->PostMessage(WM_STATUSMSG);
		if(ReadReferenceFiles()){

			// we have to call this before exiting the application otherwise we'll have trouble next time we start...
			CloseUSBConnection();

			return;
		}

		/* Initialize the evaluator */
		for(fitRgn = 0; fitRgn < m_fitRegionNum; ++fitRgn){
			for(i = 0; i < m_NChannels; ++i){
				m_fitRegion[fitRgn].window.specLength = this->m_detectorSize;
				m_fitRegion[fitRgn].eval[i]->SetFitWindow(m_fitRegion[fitRgn].window);
			}
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

	if(m_spectrometerMode != MODE_VIEW){
		// Create the output directory and start the evaluation log file.
		CreateDirectories();
		for(int j = 0; j < m_fitRegionNum; ++j){
			WriteBeginEvFile(j);
		}

		/* Start the GPS collection thread */
		if(m_skipgps == 0){
			m_gps	= new CGPS(GPSPort, GPSBaud);
			m_gps->Run(); /* start the gps-reading thread */
		}
	}else{
		this->scanNum = 2; // start directly on the measured spectra, skip dark and sky
	}

	// The various spectra to collect, this defines the order in which they are collected
	int DARK_SPECTRUM = 1;
	int SKY_SPECTRUM = 2;
	
	// Set the integration time
	if(0 == m_fixexptime){
		MessageBox(pView->m_hWnd, "Please point the spectrometer to sky","Notice",MB_OK);  // tell the user to point the telescope to zenith
		AdjustIntegrationTime();
	}else{
		integrationTime = (short)m_fixexptime;
	}

	if(m_spectrometerMode != MODE_VIEW){
		/* Calculate the number of spectra to integrate in spectrometer and in computer */
		scanNum++;
		m_sumInComputer     = CountRound(m_timeResolution, serialDelay, gpsDelay, roundResult);
		m_sumInSpectrometer = roundResult[0];
		totalSpecNum        = m_sumInComputer*m_sumInSpectrometer;
		pView->PostMessage(WM_SHOWINTTIME);

		/*  -- Collect the dark spectrum -- */
		MessageBox(pView->m_hWnd, "Cover the spectrometer", "Notice", MB_OK);
		m_statusMsg.Format("Measuring the dark spectrum");
		pView->PostMessage(WM_STATUSMSG);
	}else{
		m_sumInComputer      = 1;
		m_sumInSpectrometer  = 1;
		totalSpecNum         = 1;
		if(0 != m_fixexptime){
			MessageBox(pView->m_hWnd,  "Suitable exposure-time set", "", MB_OK);
		}
		pView->PostMessage(WM_SHOWINTTIME);
	}


	/** --------------------- THE MEASUREMENT LOOP -------------------------- */
	while(fRun){

		#ifdef _DEBUG
		cStart = clock();
		#endif

		SetFileName();

		/* ------------ Get the date, time and position --------------- */
		startDate = ReadGpsDate();
		startTime = ReadGpsStartTime();

		/** ---------------- if the user wants to change the exposure time, 
									calculate a new exposure time. --------------------- */
		if(m_adjustIntegrationTime && m_fixexptime >= 0){
			integrationTime         = AdjustIntegrationTime();
			pView->PostMessage(WM_SHOWDIALOG, CHANGED_EXPOSURETIME);
			m_adjustIntegrationTime = FALSE;
			m_sumInComputer         = CountRound(m_timeResolution, serialDelay, gpsDelay, roundResult);
			m_sumInSpectrometer     = roundResult[0];
			totalSpecNum            = m_sumInComputer*m_sumInSpectrometer;
			pView->PostMessage(WM_SHOWINTTIME);
		}

		/* ----------------  Get the spectrum --------------------  */
		#ifdef _DEBUG
		cFinish = clock();
		gpsSecond = ((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);
		#endif

		cStart = clock();

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

		cFinish = clock();
		elapsedSecond = (long)((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);

		#ifdef _DEBUG
			scanSecond = ((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);
			cStart = clock();
		#endif

		// Copy the spectrum to the local variables
		for(i = 0; i < m_NChannels; ++i){
			memcpy((void*)tmpSpec[i], (void*)scanResult[i], sizeof(double)*MAX_SPECTRUM_LENGTH);
			memcpy((void*)curSpectrum[i], (void*)scanResult[i], sizeof(double)*MAX_SPECTRUM_LENGTH);// for plot
		}

		/* ----------------- Save the spectrum(-a) -------------------- */
		if(m_spectrometerMode != MODE_VIEW){
			if(m_skipgps == 0){
				for(i = 0; i  < m_NChannels; ++i)
					CSpectrumIO::WriteStdFile(m_stdfileName[i], tmpSpec[i], m_detectorSize, startDate, specTime[counter], specTime[counter]+elapsedSecond, pos[counter].latitude, pos[counter].longitude, pos[counter].altitude, integrationTime, spectrometerName, strBaseName, totalSpecNum);
			}else{
				for(i = 0; i < m_NChannels; ++i)
					CSpectrumIO::WriteStdFile(m_stdfileName[i], tmpSpec[i], m_detectorSize, startDate, startTime, startTime+elapsedSecond, 0, 0, 0, integrationTime, spectrometerName, strBaseName, totalSpecNum);
			}
		}

		#ifdef _DEBUG
			cFinish = clock();
			writeSecond = ((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);
			cStart = clock();
		#endif

		if(scanNum == DARK_SPECTRUM){
			/* -------------- IF THE MEASURED SPECTRUM WAS THE DARK SPECTRUM ------------- */
			memcpy((void*)dark, (void*)scanResult, sizeof(double)*MAX_N_CHANNELS*MAX_SPECTRUM_LENGTH);

			pView->PostMessage(WM_DRAWSPECTRUM);//draw dark spectrum
			for(i = 0; i < m_NChannels; ++i)
				averageValue[i] = AverageIntens(scanResult[i],1);
			m_statusMsg.Format("Average value around center channel(dark) %d: %d", m_conf->m_specCenter, averageValue[0]);
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

		}else if(scanNum == SKY_SPECTRUM){
			/* -------------- IF THE MEASURED SPECTRUM WAS THE SKY SPECTRUM ------------- */

			memcpy((void*)sky, (void*)scanResult, sizeof(double)*MAX_N_CHANNELS*MAX_SPECTRUM_LENGTH);

			pView->PostMessage(WM_DRAWSPECTRUM);//draw sky spectrum

			for(i = 0; i < m_NChannels; ++i){
				averageValue[i] = AverageIntens(scanResult[i],1);

				// remove the dark spectrum
				for(int iterator = 0; iterator < MAX_SPECTRUM_LENGTH; ++iterator)
					sky[i][iterator] -= dark[i][iterator];

				// Tell the evaluator(s) that the dark-spectrum does not need to be subtracted from the sky-spectrum
				for(fitRgn = 0; fitRgn < m_fitRegionNum; ++fitRgn){
					m_fitRegion[fitRgn].eval[i]->m_subtractDarkFromSky = false;
				}
			}

			m_statusMsg.Format("Average value around center channel(sky) %d: %d",m_conf->m_specCenter, averageValue[0]);
			pView->PostMessage(WM_STATUSMSG);

			/* Get the information about the spectrum */
			GetSpectrumInfo(scanResult);
			#ifndef _DEBUG
				if(specInfo->isDark)
				MessageBox(pView->m_hWnd,  "It seems like the sky spectrum is dark, consider restarting the program", "Error", MB_OK);
			#endif

		}else if(scanNum > SKY_SPECTRUM){
			/* -------------- IF THE MEASURED SPECTRUM WAS A NORMAL SPECTRUM ------------- */

			for(i = 0; i < m_NChannels; ++i)
				averageValue[i] = AverageIntens(tmpSpec[i],1);

			/* Get the information about the spectrum */
			GetSpectrumInfo(scanResult);

			if(specInfo->isDark)
				m_statusMsg.Format("Average value around center channel %d: %d (Dark)",m_conf->m_specCenter, averageValue[0]);
			else
				m_statusMsg.Format("Average value around center channel %d: %d",m_conf->m_specCenter, averageValue[0]);

			pView->PostMessage(WM_STATUSMSG);
			vIntensity.Append(averageValue[0]);

			if(m_spectrometerMode != MODE_VIEW){
				/* Evaluate */
				GetDark();
				GetSky();
				DoEvaluation(tmpSky, tmpDark, scanResult);
			}else{
				pView->PostMessage(WM_DRAWSPECTRUM);
			}
		}
		
		if(counter > 1)
			CountFlux(windSpeed,windAngle);

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
		scanNum++;
	}
	
	// we have to call this before exiting the application otherwise we'll have trouble next time we start...
	CloseUSBConnection();
	
	return;
}