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

	// Read configuration file
	CString cfgFile = g_exePath + TEXT("cfg.xml");
	m_conf.reset(new Configuration::CMobileConfiguration(cfgFile));

	// Get directory to watch
	std::string m_directory = m_conf->m_directory;
	if (m_directory == "") {
		ShowMessageBox("<Directory> parameter not set in configuration file.", "Error");
		return;
	}


	while (2 > 1) { //TODO: write code to break out
		m_statusMsg.Format("Reading directory file %s...", m_directory);
		pView->PostMessage(WM_STATUSMSG);
		// Check directory for STD files
		WIN32_FIND_DATA ffd;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		// attempt to filter out non STD files and sky/dark files
		std::string filter = m_directory + "\\?????_?.STD";
		hFind = FindFirstFile(filter.c_str(), &ffd);
		if (INVALID_HANDLE_VALUE == hFind)
		{
			ShowMessageBox("No STD files in directory.", "Error");
			return;
		}
		FILETIME latestFiletime=ffd.ftLastWriteTime;
		CString latestSpectraFile=ffd.cFileName;
		// loop through files to find latest
		do
		{
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				FILETIME filetime = ffd.ftLastWriteTime;
				CString filename = ffd.cFileName;
				int comp = CompareFileTime(&filetime, &latestFiletime);
				switch (comp) {
				case 1:
					latestFiletime = filetime;
					latestSpectraFile = filename;
					break;
				case 0:
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

		CString fullFilename;
		fullFilename.Format("%s\\%s", m_directory.c_str(), latestSpectraFile);
		m_statusMsg.Format("Reading spectra file %s...", fullFilename);
		pView->PostMessage(WM_STATUSMSG);
		CSpectrum spec;
		if (CSpectrumIO::readSTDFile(fullFilename, &spec) == 1) {
			m_statusMsg.Format("Error reading %s.", fullFilename);
			pView->PostMessage(WM_STATUSMSG);
			
		}
		else {
			memcpy((void*)m_curSpectrum[0], (void*)spec.I, sizeof(double)*MAX_SPECTRUM_LENGTH); // for plot
			pView->PostMessage(WM_DRAWSPECTRUM);
		}
		Sleep(500);
	}
}