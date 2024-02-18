#include "stdafx.h"
#include "measurement_traverse.h"
#include "../Common/SpectrumIO.h"
#include <MobileDoasLib/Measurement/SpectrumUtils.h>

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

CMeasurement_Traverse::CMeasurement_Traverse(
    CView& mainForm,
    std::unique_ptr<mobiledoas::SpectrometerInterface> spectrometerInterface,
    std::unique_ptr<Configuration::CMobileConfiguration> conf)
    : CSpectrometer(mainForm, std::move(spectrometerInterface), std::move(conf))
{
    m_spectrometerMode = MODE_TRAVERSE;
}

CMeasurement_Traverse::~CMeasurement_Traverse(void)
{
}

void CMeasurement_Traverse::Run()
{

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

    // Create the output directory
    CreateDirectories();

    /* Start the GPS collection thread */
    if (m_useGps)
    {
        m_gps = new mobiledoas::GpsAsyncReader(m_GPSPort, m_GPSBaudRate, std::string((LPCSTR)g_exePath));
    }

    // Check if we are to be running with adaptive or with fixed exposure-time
    if (m_fixexptime < 0)
    {
        return Run_Adaptive();
    }

    mobiledoas::MeasuredSpectrum scanResult;
    CSpectrum measuredSpectrum[MAX_N_CHANNELS];

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

    /* Calculate the number of spectra to integrate in spectrometer and in computer */
    m_scanNum++;
    mobiledoas::SpectrumSummation spectrumSummation;
    m_sumInComputer = CountRound(m_timeResolution, spectrumSummation);
    m_sumInSpectrometer = spectrumSummation.SumInSpectrometer;
    OnUpdatedIntegrationTime();

    /*  -- Collect the dark spectrum -- */
    if (!m_conf->m_noDark)
    {
        ShowMessageBox("Cover the spectrometer", "Notice");
        UpdateStatusBarMessage("Measuring the dark spectrum");
    }

    /** --------------------- THE MEASUREMENT LOOP -------------------------- */
    while (m_isRunning)
    {

#ifdef _DEBUG
        cStart = clock();
#endif

        SetFileName();

        /* ------------ Get the date, time and position --------------- */
        GetCurrentDateAndTime(startDate, startTime);


        /** ---------------- if the user wants to change the exposure time,
                                    calculate a new exposure time. --------------------- */
        if (m_adjustIntegrationTime && m_fixexptime >= 0)
        {
            m_integrationTime = AdjustIntegrationTime();
            DisplayDialog(CHANGED_EXPOSURETIME);
            m_adjustIntegrationTime = FALSE;
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
        if (m_scanNum == DARK_SPECTRUM && m_conf->m_noDark)
        {
            // don't bother with dark if skipping dark; just set all to 0
            scanResult.SetToZero();
        }
        else
        {
            if (Scan(m_sumInComputer, m_sumInSpectrometer, scanResult))
            {
                CloseSpectrometerConnection();
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
        scanResult.CopyTo(m_curSpectrum); // for the plot

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

            UpdateUserAboutSpectrumAverageIntensity("dark", false);

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

            UpdateSpectrumAverageIntensity(scanResult);

            for (int i = 0; i < m_NChannels; ++i)
            {
                // remove the dark spectrum
                _ASSERT(m_sky.NumberOfChannels() == m_dark.NumberOfChannels());
                _ASSERT(m_sky.SpectrumLength() == m_dark.SpectrumLength());
                for (int iterator = 0; iterator < m_sky.SpectrumLength(); ++iterator)
                {
                    m_sky[i][iterator] -= m_dark[i][iterator];
                }
            }

            UpdateUserAboutSpectrumAverageIntensity("sky", false);

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

            /* Get the information about the spectrum */
            GetSpectrumInfo(scanResult);

            UpdateUserAboutSpectrumAverageIntensity("", true);

            m_intensityOfMeasuredSpectrum.push_back(m_averageSpectrumIntensity[0]);

            /* Evaluate */
            GetDark();
            GetSky();
            DoEvaluation(m_tmpSky, m_tmpDark, scanResult);
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

void CMeasurement_Traverse::Run_Adaptive()
{

    mobiledoas::MeasuredSpectrum scanResult;
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
    OnUpdatedIntegrationTime();

    /*  -- Collect the dark spectrum -- */

    if (!m_conf->m_noDark)
    {
        ShowMessageBox("Cover the spectrometer", "Notice");
        UpdateStatusBarMessage("Measuring the offset spectrum");
    }

    /** --------------------- THE MEASUREMENT LOOP -------------------------- */
    while (m_isRunning)
    {

        cStart = clock();

        SetFileName();

        /* ------------ Get the date, time and position --------------- */
        GetCurrentDateAndTime(startDate, startTime);

        // Get the next spectrum
        if (m_scanNum == DARKCURRENT_SPECTRUM && m_conf->m_noDark)
        {
            // don't bother with dark if skipping dark; just set all to 0
            scanResult.SetToZero();
        }
        else
        {
            if (Scan(m_sumInComputer, m_sumInSpectrometer, scanResult))
            {
                CloseSpectrometerConnection();
                return;
            }
        }

        cFinish = clock();
        elapsedSecond = (long)((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC);

        // Copy the spectrum to the local variables
        scanResult.CopyTo(m_curSpectrum);

        /* ----------------- Save the spectrum(-a) -------------------- */
        for (int i = 0; i < m_NChannels; ++i)
        {
            CreateSpectrum(measuredSpectrum[i], scanResult[i], startDate, startTime, elapsedSecond);
            CSpectrumIO::WriteStdFile(m_stdfileName[i], measuredSpectrum[i]);
        }

        if (m_scanNum == OFFSET_SPECTRUM)
        {
            /* -------------- IF THE MEASURED SPECTRUM WAS THE OFFSET SPECTRUM ------------- */
            scanResult.CopyTo(m_offset);

            UpdateDisplayedSpectrum();

            UpdateSpectrumAverageIntensity(scanResult);

            UpdateUserAboutSpectrumAverageIntensity("offset");

            GetSpectrumInfo(scanResult);
#ifndef _DEBUG
            if (!m_specInfo->isDark)
            {
                ShowMessageBox("It seems like the offset spectrum is not completely dark, consider restarting the program", "Error");
            }
#endif
            UpdateStatusBarMessage("Measuring the dark_current spectrum");

            // Set the exposure-time and the number of spectra to co-add...
            m_integrationTime = DARK_CURRENT_EXPTIME;
            m_sumInComputer = 1;
            m_sumInSpectrometer = 1;
            OnUpdatedIntegrationTime();

        }
        else if (m_scanNum == DARKCURRENT_SPECTRUM)
        {

            /* -------------- IF THE MEASURED SPECTRUM WAS THE DARK-CURRENT SPECTRUM ------------- */
            _ASSERT(m_NChannels == scanResult.NumberOfChannels());
            _ASSERT(m_offset.SpectrumLength() == scanResult.SpectrumLength());
            for (int j = 0; j < m_NChannels; ++j)
            {
                for (int i = 0; i < scanResult.SpectrumLength(); ++i)
                {
                    scanResult[j][i] = scanResult[j][i] - m_offset[j][i];
                }
            }
            scanResult.CopyTo(m_darkCur);

            UpdateDisplayedSpectrum();

            UpdateSpectrumAverageIntensity(scanResult);

            UpdateUserAboutSpectrumAverageIntensity("dark current");

            GetSpectrumInfo(scanResult);

            ShowMessageBox("Point the spectrometer to sky", "Notice");

            UpdateStatusBarMessage("Measuring the sky spectrum");

            // Set the exposure-time
            m_integrationTime = AdjustIntegrationTime();
            mobiledoas::SpectrumSummation spectrumSummation;
            m_sumInComputer = CountRound(m_timeResolution, spectrumSummation);
            m_sumInSpectrometer = spectrumSummation.SumInSpectrometer;
            OnUpdatedIntegrationTime();
        }
        else if (m_scanNum == SKY_SPECTRUM)
        {
            /* -------------- IF THE MEASURED SPECTRUM WAS THE SKY SPECTRUM ------------- */
            scanResult.CopyTo(m_sky);

            UpdateDisplayedSpectrum();

            UpdateSpectrumAverageIntensity(scanResult);

            for (int i = 0; i < m_NChannels; ++i)
            {
                // remove the dark spectrum
                GetDark();
                _ASSERT(m_sky.SpectrumLength() == m_tmpDark.SpectrumLength());
                for (int iterator = 0; iterator < m_sky.SpectrumLength(); ++iterator)
                {
                    m_sky[i][iterator] -= m_tmpDark[i][iterator];
                }
            }

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

            // Set the exposure-time
            m_integrationTime = AdjustIntegrationTime();
            mobiledoas::SpectrumSummation spectrumSummation;
            m_sumInComputer = CountRound(m_timeResolution, spectrumSummation);
            m_sumInSpectrometer = spectrumSummation.SumInSpectrometer;
            OnUpdatedIntegrationTime();

            }
        else if (m_scanNum > SKY_SPECTRUM)
        {
            /* -------------- IF THE MEASURED SPECTRUM WAS A NORMAL SPECTRUM ------------- */
            UpdateSpectrumAverageIntensity(scanResult);

            GetSpectrumInfo(scanResult);

            UpdateUserAboutSpectrumAverageIntensity("", true);

            m_intensityOfMeasuredSpectrum.push_back(m_averageSpectrumIntensity[0]);

            /* Evaluate */
            GetDark();
            GetSky();
            DoEvaluation(m_tmpSky, m_tmpDark, scanResult);

            // Update the exposure time based on the current integration time, saturation ratio and the limits from the settings.
            m_integrationTime = mobiledoas::AdjustIntegrationTimeToLastIntensity(
                mobiledoas::SpectrumIntensityMeasurement(m_averageSpectrumIntensity[0], m_spectrometerDynRange, m_integrationTime),
                m_conf->m_saturationLow / 100.0,
                m_conf->m_saturationHigh / 100.0,
                MIN_EXPOSURETIME,
                this->m_timeResolution);

            mobiledoas::SpectrumSummation spectrumSummation;
            m_sumInComputer = CountRound(m_timeResolution, spectrumSummation);
            m_sumInSpectrometer = spectrumSummation.SumInSpectrometer;
            OnUpdatedIntegrationTime();
        }

        if (m_spectrumCounter > 1)
        {
            CountFlux(m_windSpeed, m_windAngle);
        }

        scanResult.SetToZero();
        m_scanNum++;
        }

    // we have to call this before exiting the application otherwise we'll have trouble next time we start...
    CloseSpectrometerConnection();

    return;
    }