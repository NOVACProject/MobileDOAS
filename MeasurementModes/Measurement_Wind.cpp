#include "stdafx.h"
#include "measurement_wind.h"
#include "../Common/SpectrumIO.h"
#include <MobileDoasLib/Measurement/SpectrumUtils.h>

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

CMeasurement_Wind::CMeasurement_Wind(
    CView& mainForm,
    std::unique_ptr<mobiledoas::SpectrometerInterface> spectrometerInterface,
    std::unique_ptr<Configuration::CMobileConfiguration> conf)
    : CSpectrometer(mainForm, std::move(spectrometerInterface), std::move(conf))
{
    m_spectrometerMode = MODE_WIND;
}

CMeasurement_Wind::~CMeasurement_Wind(void)
{
}

void CMeasurement_Wind::Run()
{
    CString cfgFile;
    mobiledoas::MeasuredSpectrum scanResult;
    CSpectrum measuredSpectrum[MAX_N_CHANNELS];

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

    ApplySettings();

    /* Check the settings in the configuration file */
    if (CheckSettings())
    {
        ShowMessageBox("Error in configuration settings.", "Error");
        return;
    }

    /* Update the MobileLog.txt - file */
    UpdateMobileLog();

    /* Set the delays and initialize the USB-Connection */
    if (!TestSpectrometerConnection())
    {
        m_isRunning = false;
        return;
    }

    if (m_spectrometerMode != MODE_VIEW)
    {
        CreateDirectories();

        /* Start the GPS collection thread */
        if (m_useGps)
        {
            m_gps = new mobiledoas::GpsAsyncReader(m_GPSPort, m_GPSBaudRate, std::string((LPCSTR)g_exePath));
        }
    }

    if (m_spectrometerMode == MODE_VIEW)
    {

        this->m_scanNum = 2; // start directly on the measured spectra, skip dark and sky
    }

    // The various spectra to collect, this defines the order in which they are collected
    int DARK_SPECTRUM = 1;
    int SKY_SPECTRUM = 2;

    // Set the integration time
    if (0 == m_fixexptime)
    {
        ShowMessageBox("Please point the spectrometer to sky", "Notice");
        AdjustIntegrationTime();
    }
    else
    {
        m_integrationTime = (short)m_fixexptime;
    }

    if (m_spectrometerMode != MODE_VIEW)
    {
        /* Calculate the number of spectra to integrate in spectrometer and in computer */
        m_scanNum++;
        mobiledoas::SpectrumSummation spectrumSummation;
        m_sumInComputer = CountRound(m_timeResolution, spectrumSummation);
        m_sumInSpectrometer = spectrumSummation.SumInSpectrometer;
        OnUpdatedIntegrationTime();

        /*  -- Collect the dark spectrum -- */
        ShowMessageBox("Cover the spectrometer", "Notice");
        UpdateStatusBarMessage("Measuring the dark spectrum");
    }
    else
    {
        m_sumInComputer = 1;
        m_sumInSpectrometer = 1;
        if (0 != m_fixexptime)
        {
            ShowMessageBox("Suitable exposure-time set", "");
        }
        OnUpdatedIntegrationTime();
    }

    /** --------------------- THE MEASUREMENT LOOP -------------------------- */
    while (m_isRunning)
    {

#ifdef _DEBUG
        cStart = clock();
#endif

        SetFileName();

        GetCurrentDateAndTime(startDate, startTime);


        /** ---------------- if the user wants to change the exposure time,
                                    calculate a new exposure time. --------------------- */
        if (m_adjustIntegrationTime && m_fixexptime >= 0)
        {
            m_integrationTime = AdjustIntegrationTime();
            DisplayDialog(CHANGED_EXPOSURETIME);
            m_adjustIntegrationTime = FALSE;
            mobiledoas::SpectrumSummation spectrumSummation;
            m_sumInComputer = CountRound(m_timeResolution, spectrumSummation);
            m_sumInSpectrometer = spectrumSummation.SumInSpectrometer;
            OnUpdatedIntegrationTime();
        }

        /* ----------------  Get the spectrum --------------------  */
#ifdef _DEBUG
        cFinish = clock();
        gpsSecond = ((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);
#endif

        cStart = clock();

        // Get the next spectrum
        if (Scan(m_sumInComputer, m_sumInSpectrometer, scanResult))
        {
            CloseSpectrometerConnection();
            return;
        }

        cFinish = clock();
        elapsedSecond = (long)((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);

#ifdef _DEBUG
        scanSecond = ((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);
        cStart = clock();
#endif

        // Copy the spectrum to the local variables
        scanResult.CopyTo(m_curSpectrum); // for plot

        /* ----------------- Save the spectrum(-a) -------------------- */
        for (int i = 0; i < m_NChannels; ++i)
        {
            CreateSpectrum(measuredSpectrum[i], scanResult[i], startDate, startTime, elapsedSecond);
            CSpectrumIO::WriteStdFile(m_stdfileName[i], measuredSpectrum[i]);
        }

#ifdef _DEBUG
        cFinish = clock();
        writeSecond = ((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);
        cStart = clock();
#endif

        if (m_scanNum == DARK_SPECTRUM)
        {
            /* -------------- IF THE MEASURED SPECTRUM WAS THE DARK SPECTRUM ------------- */
            scanResult.CopyTo(m_dark);

            UpdateDisplayedSpectrum();

            UpdateSpectrumAverageIntensity(scanResult);

            UpdateUserAboutSpectrumAverageIntensity("dark");

            GetSpectrumInfo(scanResult);

#ifndef _DEBUG
            if (!m_specInfo->isDark)
            {
                ShowMessageBox("It seems like the dark spectrum is not completely dark, consider restarting the program", "Error");
            }
#endif

            ShowMessageBox("Point the spectrometer to sky", "Notice");

            UpdateStatusBarMessage("Measuring the sky spectrum");

        }
        else if (m_scanNum == SKY_SPECTRUM)
        {
            /* -------------- IF THE MEASURED SPECTRUM WAS THE SKY SPECTRUM ------------- */

            scanResult.CopyTo(m_sky);

            UpdateDisplayedSpectrum();

            for (int i = 0; i < m_NChannels; ++i)
            {
                // remove the dark spectrum
                _ASSERT(m_sky.SpectrumLength() == m_dark.SpectrumLength());
                for (int iterator = 0; iterator < m_sky.SpectrumLength(); ++iterator)
                {
                    m_sky[i][iterator] -= m_dark[i][iterator];
                }

                // Tell the evaluator(s) that the dark-spectrum does not need to be subtracted from the sky-spectrum
                for (int fitRgn = 0; fitRgn < m_fitRegionNum; ++fitRgn)
                {
                    m_fitRegion[fitRgn].eval[i]->m_subtractDarkFromSky = false;
                }
            }

            UpdateSpectrumAverageIntensity(scanResult);

            UpdateUserAboutSpectrumAverageIntensity("sky");

            GetSpectrumInfo(scanResult);

            // If we should do an automatic calibration, then do so now
            if (m_conf->m_calibration.m_enable)
            {
                RunInstrumentCalibration(m_sky[0].data(), nullptr, m_detectorSize);
            }

            if (ReadReferenceFiles())
            {
                // we have to call this before exiting the application otherwise we'll have trouble next time we start...
                CloseSpectrometerConnection();
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

        }
        else if (m_scanNum > SKY_SPECTRUM)
        {
            /* -------------- IF THE MEASURED SPECTRUM WAS A NORMAL SPECTRUM ------------- */

            UpdateSpectrumAverageIntensity(scanResult);

            GetSpectrumInfo(scanResult);

            UpdateUserAboutSpectrumAverageIntensity("", true);

            m_intensityOfMeasuredSpectrum.push_back(m_averageSpectrumIntensity[0]);

            if (m_spectrometerMode != MODE_VIEW)
            {
                /* Evaluate */
                GetDark();
                GetSky();
                DoEvaluation(m_tmpSky, m_tmpDark, scanResult);

            }
            else
            {
                UpdateDisplayedSpectrum();
            }
        }

        if (m_spectrumCounter > 1)
        {
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

        scanResult.SetToZero();
        m_scanNum++;
    }

    // we have to call this before exiting the application otherwise we'll have trouble next time we start...
    CloseSpectrometerConnection();

    return;
}