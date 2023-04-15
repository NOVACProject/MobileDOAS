#include "stdafx.h"
#include "measurement_view.h"
#include <MobileDoasLib/Measurement/SpectrumUtils.h>

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp
extern CFormView* pView; // <-- The main window

CMeasurement_View::CMeasurement_View(std::unique_ptr<mobiledoas::SpectrometerInterface> spectrometerInterface, std::unique_ptr<Configuration::CMobileConfiguration> conf)
    : CSpectrometer(std::move(spectrometerInterface), std::move(conf))
{
    m_spectrometerMode = MODE_VIEW;
}

CMeasurement_View::~CMeasurement_View(void)
{
}

void CMeasurement_View::Run() {
    double scanResult[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];
    double tmpSpec[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];

    ShowMessageBox("START", "NOTICE");

    ApplySettings();

    if (!TestSpectrometerConnection()) {
        m_isRunning = false;
        return;
    }

    // Set the integration time
    if (0 == m_fixexptime) {
        ShowMessageBox("Please point the spectrometer to sky", "Notice");
        AdjustIntegrationTime();
    }
    else {
        m_integrationTime = (short)m_fixexptime;
    }

    m_sumInComputer = 1;
    m_sumInSpectrometer = 1;
    if (0 != m_fixexptime) {
        ShowMessageBox("Suitable exposure-time set", "");
    }
    pView->PostMessage(WM_SHOWINTTIME);

    /** --------------------- THE MEASUREMENT LOOP -------------------------- */
    while (m_isRunning) {

        /* ----------------  Get the spectrum --------------------  */

        // Get the next spectrum
        if (Scan(m_sumInComputer, m_sumInSpectrometer, scanResult)) {
            CloseSpectrometerConnection();
            return;
        }

        GetSpectrumInfo(scanResult);

        // Copy the spectrum to the local variables
        for (int i = 0; i < m_NChannels; ++i) {
            memcpy((void*)tmpSpec[i], (void*)scanResult[i], sizeof(double) * MAX_SPECTRUM_LENGTH);
            memcpy((void*)m_curSpectrum[i], (void*)scanResult[i], sizeof(double) * MAX_SPECTRUM_LENGTH);// for plot
        }

        /* -------------- IF THE MEASURED SPECTRUM WAS A NORMAL SPECTRUM ------------- */

        for (int i = 0; i < m_NChannels; ++i) {
            m_averageSpectrumIntensity[i] = mobiledoas::AverageIntensity(tmpSpec[i], m_conf->m_specCenter, m_conf->m_specCenterHalfWidth);
        }

        if (m_specInfo->isDark)
            m_statusMsg.Format("Average value around center channel %d: %d (Dark)", m_conf->m_specCenter, m_averageSpectrumIntensity[0]);
        else
            m_statusMsg.Format("Average value around center channel %d: %d", m_conf->m_specCenter, m_averageSpectrumIntensity[0]);

        pView->PostMessage(WM_STATUSMSG);
        m_intensityOfMeasuredSpectrum.push_back(m_averageSpectrumIntensity[0]);

        pView->PostMessage(WM_DRAWSPECTRUM);
    }

    if (m_spectrumCounter > 1) {
        CountFlux(m_windSpeed, m_windAngle);
    }

    memset((void*)scanResult, 0, sizeof(double) * 4096);
    m_scanNum++;

    // we have to call this before exiting the application otherwise we'll have trouble next time we start...
    CloseSpectrometerConnection();

    return;
}