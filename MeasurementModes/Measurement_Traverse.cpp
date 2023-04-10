#include "stdafx.h"
#include "measurement_traverse.h"

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

CMeasurement_Traverse::CMeasurement_Traverse(void)
{
}

CMeasurement_Traverse::~CMeasurement_Traverse(void)
{
}

void CMeasurement_Traverse::Run() {

    std::string startDate;
    long startTime, elapsedSecond;
    clock_t cStart, cFinish;

#ifdef _DEBUG
    /* timing the progress */
    double evSecond, writeSecond, gpsSecond, scanSecond;
    FILE* timerFile = 0;
#endif
    CString tmpString;

    ShowMessageBox("START", "NOTICE");

    // Read configuration file
    CString cfgFile = g_exePath + TEXT("cfg.xml");
    m_conf.reset(new Configuration::CMobileConfiguration(cfgFile));

    // Convert the settings from the CMobileConfiguration-format to the internal CSpectrometer-format
    ApplySettings();

    /* Check the settings in the configuration file */
    if (CheckSettings()) {
        return;
    }

    /* Update the MobileLog.txt - file */
    UpdateMobileLog();

    /* Set the delays and initialize the USB-Connection */
    if (!TestSpectrometerConnection()) {
        m_isRunning = false;
        return;
    }

    // Create the output directory
    CreateDirectories();

    /* Start the GPS collection thread */
    if (m_useGps) {
        m_gps = new GpsAsyncReader(m_GPSPort, m_GPSBaudRate);
    }

    // Check if we are to be running with adaptive or with fixed exposure-time
    if (m_fixexptime < 0) {
        return Run_Adaptive();
    }

    double scanResult[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];
    CSpectrum measuredSpectrum[MAX_N_CHANNELS];

    // The various spectra to collect, this defines the order in which they are collected
    int DARK_SPECTRUM = 1;
    int SKY_SPECTRUM = 2;

    // Set the integration time
    if (0 == m_fixexptime) {
        ShowMessageBox("Please point the spectrometer to sky", "Notice");
        AdjustIntegrationTime();
    } else {
        m_integrationTime = (short)m_fixexptime;
    }

    /* Calculate the number of spectra to integrate in spectrometer and in computer */
    m_scanNum++;
    SpectrumSummation spectrumSummation;
    m_sumInComputer = CountRound(m_timeResolution, spectrumSummation);
    m_sumInSpectrometer = spectrumSummation.SumInSpectrometer;
    m_totalSpecNum = m_sumInComputer * m_sumInSpectrometer;
    pView->PostMessage(WM_SHOWINTTIME);

    /*  -- Collect the dark spectrum -- */
    if (!m_conf->m_noDark) {
        ShowMessageBox("Cover the spectrometer", "Notice");
        m_statusMsg.Format("Measuring the dark spectrum");
        pView->PostMessage(WM_STATUSMSG);
    }

    /** --------------------- THE MEASUREMENT LOOP -------------------------- */
    while (m_isRunning) {

#ifdef _DEBUG
        cStart = clock();
#endif

        SetFileName();

        /* ------------ Get the date, time and position --------------- */
        GetCurrentDateAndTime(startDate, startTime);


        /** ---------------- if the user wants to change the exposure time,
                                    calculate a new exposure time. --------------------- */
        if (m_adjustIntegrationTime && m_fixexptime >= 0) {
            m_integrationTime = AdjustIntegrationTime();
            pView->PostMessage(WM_SHOWDIALOG, CHANGED_EXPOSURETIME);
            m_adjustIntegrationTime = FALSE;
            m_sumInComputer = CountRound(m_timeResolution, spectrumSummation);
            m_sumInSpectrometer = spectrumSummation.SumInSpectrometer;
            m_totalSpecNum = m_sumInComputer * m_sumInSpectrometer;
            pView->PostMessage(WM_SHOWINTTIME);
        }

        /* ----------------  Get the spectrum --------------------  */
#ifdef _DEBUG
        cFinish = clock();
        gpsSecond = ((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);
#endif

        cStart = clock();

        // Get the next spectrum
        if (m_scanNum == DARK_SPECTRUM && m_conf->m_noDark) {
            // don't bother with dark if skipping dark; just set all to 0
            for (int i = 0; i < m_NChannels; ++i) {
                for (int j = 0; j < MAX_SPECTRUM_LENGTH; j++) {
                    scanResult[i][j] = 0;
                }
            }
        } else {
            if (Scan(m_sumInComputer, m_sumInSpectrometer, scanResult)) {
                CloseUSBConnection();
                return;
            }
        }

        cFinish = clock();
        elapsedSecond = (long)((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);

#ifdef _DEBUG
        scanSecond = ((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);
        cStart = clock();
#endif

        // Copy the spectrum to the local variables
        for (int i = 0; i < m_NChannels; ++i) {
            memcpy((void*)m_curSpectrum[i], (void*)scanResult[i], sizeof(double) * MAX_SPECTRUM_LENGTH);// for plot
        }

        /* ----------------- Save the spectrum(-a) -------------------- */
        for (int i = 0; i < m_NChannels; ++i) {
            CreateSpectrum(measuredSpectrum[i], scanResult[i], startDate, startTime, elapsedSecond);
            CSpectrumIO::WriteStdFile(m_stdfileName[i], measuredSpectrum[i]);
        }

#ifdef _DEBUG
        cFinish = clock();
        writeSecond = ((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);
        cStart = clock();
#endif

        if (m_scanNum == DARK_SPECTRUM) {
            /* -------------- IF THE MEASURED SPECTRUM WAS THE DARK SPECTRUM ------------- */
            memcpy((void*)m_dark, (void*)scanResult, sizeof(double) * MAX_N_CHANNELS * MAX_SPECTRUM_LENGTH);

            pView->PostMessage(WM_DRAWSPECTRUM);//draw dark spectrum
            for (int i = 0; i < m_NChannels; ++i) {
                m_averageSpectrumIntensity[i] = AverageIntens(scanResult[i], 1);
            }
            m_statusMsg.Format("Average value around center channel(dark) %d: %d", m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
            pView->PostMessage(WM_STATUSMSG);

            /* Get the information about the spectrum */
            GetSpectrumInfo(scanResult);

#ifndef _DEBUG
            if (!m_specInfo->isDark) {
                ShowMessageBox("It seems like the dark spectrum is not completely dark, consider restarting the program", "Error");
            }
#endif

            ShowMessageBox("Point the spectrometer to sky", "Notice");

            m_statusMsg.Format("Measuring the sky spectrum");
            pView->PostMessage(WM_STATUSMSG);

        } else if (m_scanNum == SKY_SPECTRUM) {
            /* -------------- IF THE MEASURED SPECTRUM WAS THE SKY SPECTRUM ------------- */

            memcpy((void*)m_sky, (void*)scanResult, sizeof(double) * MAX_N_CHANNELS * MAX_SPECTRUM_LENGTH);

            pView->PostMessage(WM_DRAWSPECTRUM);//draw sky spectrum

            for (int i = 0; i < m_NChannels; ++i) {
                m_averageSpectrumIntensity[i] = AverageIntens(scanResult[i], 1);

                // remove the dark spectrum
                for (int iterator = 0; iterator < MAX_SPECTRUM_LENGTH; ++iterator) {
                    m_sky[i][iterator] -= m_dark[i][iterator];
                }
            }

            m_statusMsg.Format("Average value around center channel(sky) %d: %d", m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
            pView->PostMessage(WM_STATUSMSG);

            /* Get the information about the spectrum */
            GetSpectrumInfo(scanResult);

            // If we should do an automatic calibration, then do so now
            if (m_conf->m_calibration.m_enable) {
                RunInstrumentCalibration(m_sky[0], nullptr, m_detectorSize);
            }

            m_statusMsg.Format("Reading References");
            pView->PostMessage(WM_STATUSMSG);
            if (ReadReferenceFiles()) {
                // we have to call this before exiting the application otherwise we'll have trouble next time we start...
                CloseUSBConnection();
                return;
            }

            InitializeEvaluators(true);

            WriteEvaluationLogFileHeaders();

#ifndef _DEBUG
            if (m_specInfo->isDark) {
                ShowMessageBox("It seems like the sky spectrum is dark, consider restarting the program", "Error");
            }
#endif

        } else if (m_scanNum > SKY_SPECTRUM) {
            /* -------------- IF THE MEASURED SPECTRUM WAS A NORMAL SPECTRUM ------------- */

            for (int i = 0; i < m_NChannels; ++i) {
                m_averageSpectrumIntensity[i] = AverageIntens(scanResult[i], 1);
            }

            /* Get the information about the spectrum */
            GetSpectrumInfo(scanResult);

            if (m_specInfo->isDark) {
                m_statusMsg.Format("Average value around center channel %d: %d (Dark)", m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
            } else {
                m_statusMsg.Format("Average value around center channel %d: %d", m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
            }

            pView->PostMessage(WM_STATUSMSG);
            vIntensity.Append(m_averageSpectrumIntensity[0]);

            /* Evaluate */
            GetDark();
            GetSky();
            DoEvaluation(m_tmpSky, m_tmpDark, scanResult);
        }

        if (m_spectrumCounter > 1) {
            CountFlux(m_windSpeed, m_windAngle);
        }

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

        memset((void*)scanResult, 0, sizeof(double) * 4096);
        m_scanNum++;
    }

    // we have to call this before exiting the application otherwise we'll have trouble next time we start...
    CloseUSBConnection();

    return;
}

void CMeasurement_Traverse::Run_Adaptive() {

    double scanResult[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];
    CSpectrum measuredSpectrum[MAX_N_CHANNELS];

    std::string startDate;
    long startTime, elapsedSecond;
    clock_t cStart, cFinish;

    // The various spectra to collect, this defines the order in which they are collected
    int OFFSET_SPECTRUM = 1;
    int DARKCURRENT_SPECTRUM = 2;
    int SKY_SPECTRUM = 3;

    m_scanNum = OFFSET_SPECTRUM;

    // 1. Start collecting the offset spectrum.
    m_integrationTime = 3;
    m_sumInComputer = 100;
    m_sumInSpectrometer = 15;
    m_totalSpecNum = m_sumInSpectrometer * m_sumInComputer;
    pView->PostMessage(WM_SHOWINTTIME);

    /*  -- Collect the dark spectrum -- */

    if (!m_conf->m_noDark) {
        ShowMessageBox("Cover the spectrometer", "Notice");
        m_statusMsg.Format("Measuring the offset spectrum");
        pView->PostMessage(WM_STATUSMSG);
    }

    /** --------------------- THE MEASUREMENT LOOP -------------------------- */
    while (m_isRunning) {

        cStart = clock();

        SetFileName();

        /* ------------ Get the date, time and position --------------- */
        GetCurrentDateAndTime(startDate, startTime);

        // Get the next spectrum
        if (m_scanNum == DARKCURRENT_SPECTRUM && m_conf->m_noDark) {
            // don't bother with dark if skipping dark; just set all to 0
            for (int i = 0; i < m_NChannels; ++i) {
                for (int j = 0; j < MAX_SPECTRUM_LENGTH; j++) {
                    scanResult[i][j] = 0;
                }
            }
        } else {
            if (Scan(m_sumInComputer, m_sumInSpectrometer, scanResult)) {
                CloseUSBConnection();
                return;
            }
        }

        cFinish = clock();
        elapsedSecond = (long)((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);

        // Copy the spectrum to the local variables
        for (int i = 0; i < m_NChannels; ++i) {
            memcpy((void*)m_curSpectrum[i], (void*)scanResult[i], sizeof(double) * MAX_SPECTRUM_LENGTH);// for plot
        }

        /* ----------------- Save the spectrum(-a) -------------------- */
        for (int i = 0; i < m_NChannels; ++i) {
            CreateSpectrum(measuredSpectrum[i], scanResult[i], startDate, startTime, elapsedSecond);
            CSpectrumIO::WriteStdFile(m_stdfileName[i], measuredSpectrum[i]);
        }

        if (m_scanNum == OFFSET_SPECTRUM) {
            /* -------------- IF THE MEASURED SPECTRUM WAS THE OFFSET SPECTRUM ------------- */
            memcpy((void*)m_offset, (void*)scanResult, sizeof(double) * MAX_N_CHANNELS * MAX_SPECTRUM_LENGTH);

            pView->PostMessage(WM_DRAWSPECTRUM);//draw offset spectrum
            for (int i = 0; i < m_NChannels; ++i) {
                m_averageSpectrumIntensity[i] = AverageIntens(scanResult[i], 1);
            }
            m_statusMsg.Format("Average value around center channel(offset) %d: %d", m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
            pView->PostMessage(WM_STATUSMSG);

            /* Get the information about the spectrum */
            GetSpectrumInfo(scanResult);
#ifndef _DEBUG
            if (!m_specInfo->isDark)
            {
                ShowMessageBox("It seems like the offset spectrum is not completely dark, consider restarting the program", "Error");
            }
#endif

            m_statusMsg.Format("Measuring the dark_current spectrum");
            pView->PostMessage(WM_STATUSMSG);

            // Set the exposure-time and the number of spectra to co-add...
            m_integrationTime = DARK_CURRENT_EXPTIME;
            m_sumInComputer = 1;
            m_sumInSpectrometer = 1;
            m_totalSpecNum = m_sumInSpectrometer * m_sumInComputer;
            pView->PostMessage(WM_SHOWINTTIME);

        } else if (m_scanNum == DARKCURRENT_SPECTRUM) {

            /* -------------- IF THE MEASURED SPECTRUM WAS THE DARK-CURRENT SPECTRUM ------------- */
            for (int j = 0; j < m_NChannels; ++j) {
                for (int i = 0; i < MAX_SPECTRUM_LENGTH; ++i) {
                    scanResult[j][i] = scanResult[j][i] - m_offset[j][i];
                }
            }
            memcpy((void*)m_darkCur, (void*)scanResult, sizeof(double) * MAX_N_CHANNELS * MAX_SPECTRUM_LENGTH);

            pView->PostMessage(WM_DRAWSPECTRUM);//draw dark spectrum
            for (int i = 0; i < m_NChannels; ++i) {
                m_averageSpectrumIntensity[i] = AverageIntens(scanResult[i], 1);
            }
            m_statusMsg.Format("Average value around center channel(dark current) %d: %d", m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
            pView->PostMessage(WM_STATUSMSG);

            /* Get the information about the spectrum */
            GetSpectrumInfo(scanResult);

            ShowMessageBox("Point the spectrometer to sky", "Notice");

            m_statusMsg.Format("Measuring the sky spectrum");
            pView->PostMessage(WM_STATUSMSG);

            // Set the exposure-time
            m_integrationTime = AdjustIntegrationTime();
            SpectrumSummation spectrumSummation;
            m_sumInComputer = CountRound(m_timeResolution, spectrumSummation);
            m_sumInSpectrometer = spectrumSummation.SumInSpectrometer;
            m_totalSpecNum = m_sumInComputer * m_sumInSpectrometer;
            pView->PostMessage(WM_SHOWINTTIME);
        }
        else if (m_scanNum == SKY_SPECTRUM) {
            /* -------------- IF THE MEASURED SPECTRUM WAS THE SKY SPECTRUM ------------- */

            memcpy((void*)m_sky, (void*)scanResult, sizeof(double) * MAX_N_CHANNELS * MAX_SPECTRUM_LENGTH);

            pView->PostMessage(WM_DRAWSPECTRUM);//draw sky spectrum

            for (int i = 0; i < m_NChannels; ++i) {
                m_averageSpectrumIntensity[i] = AverageIntens(scanResult[i], 1);

                // remove the dark spectrum
                GetDark();
                for (int iterator = 0; iterator < MAX_SPECTRUM_LENGTH; ++iterator) {
                    m_sky[i][iterator] -= m_tmpDark[i][iterator];
                }
            }

            m_statusMsg.Format("Average value around center channel(sky) %d: %d", m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
            pView->PostMessage(WM_STATUSMSG);

            /* Get the information about the spectrum */
            GetSpectrumInfo(scanResult);

            // If we should do an automatic calibration, then do so now
            if (m_conf->m_calibration.m_enable)
            {
                RunInstrumentCalibration(m_sky[0], nullptr, m_detectorSize);
            }

            m_statusMsg.Format("Reading References");
            pView->PostMessage(WM_STATUSMSG);
            if (ReadReferenceFiles())
            {
                // we have to call this before exiting the application otherwise we'll have trouble next time we start...
                CloseUSBConnection();
                return;
            }

            InitializeEvaluators(true);

            WriteEvaluationLogFileHeaders();

#ifndef _DEBUG
            if (m_specInfo->isDark)
            {
                ShowMessageBox("It seems like the sky spectrum is dark, consider restarting the program", "Error");
            }
#endif

            // Set the exposure-time
            m_integrationTime = AdjustIntegrationTime();
            SpectrumSummation spectrumSummation;
            m_sumInComputer = CountRound(m_timeResolution, spectrumSummation);
            m_sumInSpectrometer = spectrumSummation.SumInSpectrometer;
            m_totalSpecNum = m_sumInComputer * m_sumInSpectrometer;
            pView->PostMessage(WM_SHOWINTTIME);

        } else if (m_scanNum > SKY_SPECTRUM) {
            /* -------------- IF THE MEASURED SPECTRUM WAS A NORMAL SPECTRUM ------------- */
            for (int i = 0; i < m_NChannels; ++i) {
                m_averageSpectrumIntensity[i] = AverageIntens(scanResult[i], 1);
            }

            /* Get the information about the spectrum */
            GetSpectrumInfo(scanResult);

            if (m_specInfo->isDark) {
                m_statusMsg.Format("Average value around center channel %d: %d (Dark)", m_conf->m_specCenter, m_averageSpectrumIntensity[0]);

            } else {
                m_statusMsg.Format("Average value around center channel %d: %d", m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
            }

            pView->PostMessage(WM_STATUSMSG);
            vIntensity.Append(m_averageSpectrumIntensity[0]);

            /* Evaluate */
            GetDark();
            GetSky();
            DoEvaluation(m_tmpSky, m_tmpDark, scanResult);

            // Update the exposure-time
            m_integrationTime = AdjustIntegrationTimeToLastIntensity(m_averageSpectrumIntensity[0]);
            SpectrumSummation spectrumSummation;
            m_sumInComputer = CountRound(m_timeResolution, spectrumSummation);
            m_sumInSpectrometer = spectrumSummation.SumInSpectrometer;
            m_totalSpecNum = m_sumInComputer * m_sumInSpectrometer;
            pView->PostMessage(WM_SHOWINTTIME);
        }

        if (m_spectrumCounter > 1) {
            CountFlux(m_windSpeed, m_windAngle);
        }

        memset((void*)scanResult, 0, sizeof(double) * 4096);
        m_scanNum++;
    }

    // we have to call this before exiting the application otherwise we'll have trouble next time we start...
    CloseUSBConnection();

    return;
}