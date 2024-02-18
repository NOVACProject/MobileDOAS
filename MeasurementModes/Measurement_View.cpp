#include "stdafx.h"
#include "measurement_view.h"
#include <MobileDoasLib/Measurement/SpectrumUtils.h>

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

CMeasurement_View::CMeasurement_View(
    CView& mainForm,
    std::unique_ptr<mobiledoas::SpectrometerInterface> spectrometerInterface,
    std::unique_ptr<Configuration::CMobileConfiguration> conf)
    : CSpectrometer(mainForm, std::move(spectrometerInterface), std::move(conf))
{
    m_spectrometerMode = MODE_VIEW;
}

CMeasurement_View::~CMeasurement_View(void)
{
}

void CMeasurement_View::Run()
{
    mobiledoas::MeasuredSpectrum scanResult;

    ShowMessageBox("START", "NOTICE");

    ApplySettings();

    if (!TestSpectrometerConnection())
    {
        m_isRunning = false;
        return;
    }

    // Set the integration time
    if (m_fixexptime <= 0)
    {
        ShowMessageBox("Please point the spectrometer to sky", "Notice");
        AdjustIntegrationTime();
    }
    else 
    {
        m_integrationTime = (short)m_fixexptime;
    }

    m_sumInComputer = 1;
    m_sumInSpectrometer = 1;
    if (0 != m_fixexptime)
    {
        ShowMessageBox("Suitable exposure-time set", "");
    }
    OnUpdatedIntegrationTime();

    /** --------------------- THE MEASUREMENT LOOP -------------------------- */
    while (m_isRunning)
    {

        /* ----------------  Get the spectrum --------------------  */

        // Get the next spectrum
        if (Scan(m_sumInComputer, m_sumInSpectrometer, scanResult))
        {
            CloseSpectrometerConnection();
            return;
        }

        GetSpectrumInfo(scanResult);

        // Copy the spectrum to the local variables
        scanResult.CopyTo(m_curSpectrum);

        UpdateSpectrumAverageIntensity(scanResult);

        UpdateUserAboutSpectrumAverageIntensity("", true);

        m_intensityOfMeasuredSpectrum.push_back(m_averageSpectrumIntensity[0]);

        UpdateDisplayedSpectrum();
    }

    if (m_spectrumCounter > 1)
    {
        CountFlux(m_windSpeed, m_windAngle);
    }

    scanResult.SetToZero();
    m_scanNum++;

    // we have to call this before exiting the application otherwise we'll have trouble next time we start...
    CloseSpectrometerConnection();

    return;
}