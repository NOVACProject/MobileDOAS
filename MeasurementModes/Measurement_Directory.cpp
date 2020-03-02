#include "stdafx.h"
#include "Measurement_Directory.h"

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

CMeasurement_Directory::CMeasurement_Directory()
{
}


CMeasurement_Directory::~CMeasurement_Directory()
{
}

void CMeasurement_Directory::Run() {
	ShowMessageBox("START", "NOTICE");

	// Read configuration file and apply settings
	CString cfgFile = g_exePath + TEXT("cfg.xml");
	m_conf.reset(new Configuration::CMobileConfiguration(cfgFile));
	ApplySettings();
	if (CheckSettings()) {
		ShowMessageBox("Error in configuration settings.", "Error");
		return;
	}

	// Get directory to watch
	std::string m_directory = m_conf->m_directory;
	if (m_directory == "") {
		ShowMessageBox("<Directory> parameter not set in configuration file.", "Error");
		return;
	}
	m_subFolder = m_directory.c_str();

	// Update MobileLog.txt 
	UpdateMobileLog();

	// Read reference files
	m_statusMsg.Format("Reading References");
	pView->PostMessage(WM_STATUSMSG);
	if (ReadReferenceFiles()) {
		ShowMessageBox("Error reading reference files.", "Error");
		return;
	}

	// Initialize the evaluator 
	for (int fitRgn = 0; fitRgn < m_fitRegionNum; ++fitRgn) {
		for (int i = 0; i < m_NChannels; ++i) {
			m_fitRegion[fitRgn].window.specLength = this->m_detectorSize;
			m_fitRegion[fitRgn].eval[i]->SetFitWindow(m_fitRegion[fitRgn].window);
		}
	}

	// Create the output directory and start the evaluation log file.
	CreateDirectories();
	for (int j = 0; j < m_fitRegionNum; ++j) {
		WriteBeginEvFile(j);
	}

	// Check directory for STD files
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	// get sky 
	std::string filter = m_directory + "\\sky_0.STD";
	hFind = FindFirstFile(filter.c_str(), &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		ShowMessageBox("No sky file in directory.", "Error");
		return;
	}
	CSpectrum spec;
	CString specfile;
	specfile.Format("%s\\%s", m_directory.c_str(), ffd.cFileName);
	if (CSpectrumIO::readSTDFile(specfile, &spec) == 1) {
		m_statusMsg.Format("Error reading %s.", specfile);
		pView->PostMessage(WM_STATUSMSG);
	}
	else {
		memcpy((void*)m_sky[0], (void*)spec.I, sizeof(double)*MAX_SPECTRUM_LENGTH); 
	}

	// get dark (dark or darkcur)
	filter = m_directory + "\\dark*0.STD";
	hFind = FindFirstFile(filter.c_str(), &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		ShowMessageBox("No dark file in directory.", "Error");
		return;
	}
	specfile.Format("%s\\%s", m_directory.c_str(), ffd.cFileName);
	if (CSpectrumIO::readSTDFile(specfile, &spec) == 1) {
		m_statusMsg.Format("Error reading %s.", specfile);
		pView->PostMessage(WM_STATUSMSG);
	}
	else {
		memcpy((void*)m_dark[0], (void*)spec.I, sizeof(double)*MAX_SPECTRUM_LENGTH);
	}

	// variables used for adaptive mode
	int roundResult[MAX_N_CHANNELS];
	long serialDelay, gpsDelay;
	// get offset if in adaptive mode
	if (m_conf->m_expTimeMode == m_conf->EXPOSURETIME_ADAPTIVE) {
		filter = m_directory + "\\offset_0.STD";
		hFind = FindFirstFile(filter.c_str(), &ffd);
		if (INVALID_HANDLE_VALUE == hFind)
		{
			ShowMessageBox("No offset file in directory. Required for adaptive mode.", "Error");
			return;
		}
		specfile.Format("%s\\%s", m_directory.c_str(), ffd.cFileName);
		if (CSpectrumIO::readSTDFile(specfile, &spec) == 1) {
			m_statusMsg.Format("Error reading %s.", specfile);
			pView->PostMessage(WM_STATUSMSG);
		}
		else {
			memcpy((void*)m_offset[0], (void*)spec.I, sizeof(double)*MAX_SPECTRUM_LENGTH); 
			// subtrace offset from dark
			for (int i = 0; i < MAX_SPECTRUM_LENGTH; ++i) {
				m_dark[0][i] = m_dark[0][i] - m_offset[0][i];
			}
			m_averageSpectrumIntensity[0] = AverageIntens(m_sky[0], 1);

			// remove the dark spectrum from sky
			GetDark();
			for (int iterator = 0; iterator < MAX_SPECTRUM_LENGTH; ++iterator) {
				m_sky[0][iterator] -= m_tmpDark[0][iterator];
			}

			// Tell the evaluator(s) that the dark-spectrum does not need to be subtracted from the sky-spectrum
			for (int fitRgn = 0; fitRgn < m_fitRegionNum; ++fitRgn) {
				m_fitRegion[fitRgn].eval[0]->m_subtractDarkFromSky = false;
			}

			/* Set the delays and initialize the USB-Connection */
			if (m_connectViaUsb) {
				serialDelay = 10;
			}
			else {
				serialDelay = 2300;
			}
			gpsDelay = 10;
		}
	}

	CString lastShown;
	while (m_isRunning) { 
		// get only normal spectrum files
		filter = m_directory + "\\?????_?.STD";
		hFind = FindFirstFile(filter.c_str(), &ffd);
		if (INVALID_HANDLE_VALUE == hFind)
		{
			ShowMessageBox("No measured spectrum files in directory.", "Error");
			return;
		}
		FILETIME latestFiletime=ffd.ftLastWriteTime;
		CString latestSpectraFile=ffd.cFileName;
		// loop through files to find latest
		do
		{
			// double check it's not a directory (though shouldn't be with the filter)
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				FILETIME filetime = ffd.ftLastWriteTime;
				CString filename = ffd.cFileName;
				// compare file time
				int comp = CompareFileTime(&filetime, &latestFiletime);
				switch (comp) {
				case 1: 
					// if file has a later timestamp than last spec file processed
					latestFiletime = filetime;
					latestSpectraFile = filename;
					break;
				case 0:
					// if file has same time than last spec file processed,
					// compare file names for spec number and use the larger
					std::string fn = filename;
					std::string lfn = latestSpectraFile;
					int strcomp = fn.compare(lfn);
					if (strcomp == 1) {
						latestFiletime = filetime;
						latestSpectraFile = filename;
					}
				}
			}
			
		} while (FindNextFile(hFind, &ffd) != 0);

		if (latestSpectraFile.Compare(lastShown) == 0) { 
			// skip processing if same as before
			Sleep(m_conf->m_sleep);
			continue;
		}
		specfile.Format("%s\\%s", m_directory.c_str(), latestSpectraFile);
		m_statusMsg.Format("Reading spectra file %s...", specfile);
		pView->PostMessage(WM_STATUSMSG);
		if (CSpectrumIO::readSTDFile(specfile, &spec) == 1) {
			m_statusMsg.Format("Error reading %s", specfile);
			pView->PostMessage(WM_STATUSMSG);			
		}
		else {
			// get spec number and channel num
			m_scanNum = atoi(latestSpectraFile.Left(5)) + 2;
			int channel = atoi(latestSpectraFile.Mid(7, 1));

			// copy data to current spectrum 
			memcpy((void*)m_curSpectrum[channel], (void*)spec.I, sizeof(double)*MAX_SPECTRUM_LENGTH); // for plot

			// calculate average intensity
			m_averageSpectrumIntensity[channel] = AverageIntens(m_curSpectrum[channel], 1);
			vIntensity.Append(m_averageSpectrumIntensity[0]);

			// get spectrum info
			GetSpectrumInfo(m_curSpectrum);

			// do the fit
			GetDark();
			GetSky();
			DoEvaluation(m_tmpSky, m_tmpDark, m_curSpectrum);
			lastShown=latestSpectraFile; // update last shown spectra
			m_statusMsg.Format("Showing spectra file %s", specfile);
			pView->PostMessage(WM_STATUSMSG);

			// Update the exposure-time if adaptive mode
			if (m_conf->m_expTimeMode == m_conf->EXPOSURETIME_ADAPTIVE) {				
				m_integrationTime = AdjustIntegrationTimeToLastIntensity(m_averageSpectrumIntensity[0]);
				m_sumInComputer = CountRound(m_timeResolution, serialDelay, gpsDelay, roundResult);
				m_sumInSpectrometer = roundResult[0];
				m_totalSpecNum = m_sumInComputer * m_sumInSpectrometer;
			}
		}
		Sleep(m_conf->m_sleep);
	}
}
