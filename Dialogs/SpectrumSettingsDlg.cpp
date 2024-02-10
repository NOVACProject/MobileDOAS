// SpectrumSettingsDlg.cpp : implementation file
//

#undef min
#undef max

#include "stdafx.h"
#include "../DMSpec.h"
#include "SpectrumSettingsDlg.h"
#include "../Common/SpectrumIO.h"
#include "../Spectrometer.h"
#include <algorithm>
#include <SpectralEvaluation/StringUtils.h>

int GetLargestDivisorBelow16(int n) {
    for (int k = 15; k > 0; --k) {
        if (n % k == 0)
            return k;
    }
    return 1;
}

// CSpectrumSettingsDlg dialog

using namespace Dialogs;

IMPLEMENT_DYNAMIC(CSpectrumSettingsDlg, CDialog)
CSpectrumSettingsDlg::CSpectrumSettingsDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CSpectrumSettingsDlg::IDD, pParent)
{
}

CSpectrumSettingsDlg::~CSpectrumSettingsDlg()
{
    m_Spectrometer = nullptr;
}

void CSpectrumSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_SPIN_EXPTIME, m_exptimeSpin);
    DDX_Control(pDX, IDC_SPIN_NAVERAGE, m_averageSpin);

    DDX_Control(pDX, IDC_EDIT_EXPOSURETIME, m_exptimeEdit);
    DDX_Control(pDX, IDC_EDIT_NAVERAGE, m_averageEdit);

    DDX_Text(pDX, IDC_EDIT_EXPOSURETIME, m_integrationTimeMs);
    DDX_Text(pDX, IDC_EDIT_NAVERAGE, m_average);

    DDX_Control(pDX, IDC_COMBO_SPECTROMETERS, m_comboSpecs);

    DDX_Radio(pDX, IDC_RADIO_CHANNEL0, m_channel);
}


BEGIN_MESSAGE_MAP(CSpectrumSettingsDlg, CDialog)
    ON_MESSAGE(WM_SHOWINTTIME, UpdateDialogWithDataFromSpectrometer)
    ON_MESSAGE(WM_CHANGEDSPEC, OnChangeSpectrometer)
    ON_BN_CLICKED(IDC_BUTTON_SAVESPEC, SaveSpectrum)

    ON_EN_CHANGE(IDC_EDIT_EXPOSURETIME, SaveDialogDataToSpectrometer)
    ON_EN_CHANGE(IDC_EDIT_NAVERAGE, SaveDialogDataToSpectrometer)

    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_EXPTIME, OnChangeSpinExptime)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_NAVERAGE, OnChangeSpinAverage)

    ON_CBN_SELCHANGE(IDC_COMBO_SPECTROMETERS, OnUserChangeSpectrometer)

    ON_BN_CLICKED(IDC_RADIO_CHANNEL0, OnUserChangeSpectrometer)
    ON_BN_CLICKED(IDC_RADIO_CHANNEL1, OnUserChangeSpectrometer)

END_MESSAGE_MAP()

// CSpectrumSettingsDlg message handlers

BOOL CSpectrumSettingsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_exptimeSpin.SetBuddy(&m_exptimeEdit);
    m_averageSpin.SetBuddy(&m_averageEdit);

    UpdateDialogWithDataFromSpectrometer(0, 0);

    return 0;
}

LRESULT CSpectrumSettingsDlg::UpdateDialogWithDataFromSpectrometer(WPARAM wParam, LPARAM lParam) {

    if (this->m_Spectrometer == nullptr) {
        return 0;
    }

    // Get the parameters from the spectrometer
    this->m_average = m_Spectrometer->NumberOfSpectraToAverage();
    this->m_integrationTimeMs = m_Spectrometer->m_integrationTime;

    // Update the window
    if (this->m_hWnd != nullptr)
        UpdateData(FALSE);

    return 0;
}

void CSpectrumSettingsDlg::SaveDialogDataToSpectrometer() {

    if (this->m_Spectrometer == nullptr) {
        return;
    }

    // Get the values from the window...
    UpdateData(TRUE);

    // sanity check
    if (m_integrationTimeMs < 3 || m_integrationTimeMs > 65536 || m_average < 1 || m_average > 50000) {
        return;
    }

    // set the parameters
    m_Spectrometer->m_integrationTime = m_integrationTimeMs;
    if (EqualsIgnoringCase(m_Spectrometer->m_spectrometerModel, "USB2000+")) {
        m_Spectrometer->m_sumInSpectrometer = m_average;
        m_Spectrometer->m_sumInComputer = 1;
    }
    else {
        if (m_average > 15) {
            // Get the largest possible number to add together in the spectrometer
            int largestDivisor = GetLargestDivisorBelow16(m_average);

            m_Spectrometer->m_sumInSpectrometer = std::max(largestDivisor, 1);
            m_Spectrometer->m_sumInComputer = m_average / m_Spectrometer->m_sumInSpectrometer;
        }
        else {
            m_Spectrometer->m_sumInSpectrometer = m_average;
            m_Spectrometer->m_sumInComputer = 1;
        }
    }
}

/** Saves the last spectrum to file */
void CSpectrumSettingsDlg::SaveSpectrum()
{

    // let the user browse for a place where to store the spectrum
    CString stdFileName;
    if (!Common::BrowseForFile_SaveAs("Spectrum file\0*.std", stdFileName))
    {
        return;
    }

    // add the file-ending .std if the user hasn't done so
    if (!Equals(stdFileName.Right(4), ".std"))
    {
        stdFileName.AppendFormat(".std");
    }

    // create CSpectrum data object
    std::string startDate;
    long startTime;
    m_Spectrometer->GetCurrentDateAndTime(startDate, startTime);
    CSpectrum spectrum;
    int channel = m_Spectrometer->m_spectrometerChannel;
    std::vector<double> spectrumData = m_Spectrometer->GetSpectrum(channel);
    m_Spectrometer->CreateSpectrum(spectrum, spectrumData, startDate, startTime, 0);

    // write the file
    CSpectrumIO::WriteStdFile(stdFileName, spectrum);
}

// Called when the user has pressed the spin button that controlls the exposure-time
void CSpectrumSettingsDlg::OnChangeSpinExptime(NMHDR* pNMHDR, LRESULT* pResult)
{
    CString str;
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    // Check which direction the user has pressed the spinbutton (which is upside down)
    //  and change the circle radius accordingly
    if (pNMUpDown->iDelta > 0)
        --m_integrationTimeMs;
    if (pNMUpDown->iDelta < 0)
        ++m_integrationTimeMs;

    // Enforce the limits
    m_integrationTimeMs = std::max(3, m_integrationTimeMs);
    m_integrationTimeMs = std::min(65536, m_integrationTimeMs);

    *pResult = 0;

    // Update the screen
    str.Format("%d", m_integrationTimeMs);
    m_exptimeEdit.SetWindowText(str);

}

// Called when the user has pressed the spin button that controlls the number of spectra to average
void CSpectrumSettingsDlg::OnChangeSpinAverage(NMHDR* pNMHDR, LRESULT* pResult)
{
    CString str;
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    // Check which direction the user has pressed the spinbutton (which is upside down)
    //  and change the circle radius accordingly
    if (pNMUpDown->iDelta > 0)
        --m_average;
    if (pNMUpDown->iDelta < 0)
        ++m_average;

    // Enforce the limits
    m_average = std::max(1, m_average);
    m_average = std::min(20000, m_average);

    *pResult = 0;

    // Update the screen
    str.Format("%d", m_average);
    m_averageEdit.SetWindowText(str);
}

/** Retrieves the list of spectrometers from the CSpectrometer and
    updates the combo-box */
void CSpectrumSettingsDlg::UpdateListOfSpectrometers()
{
    const auto spectrometers = m_Spectrometer->GetConnectedSpectrometers();

    if (spectrometers.size() == 0 || (m_Spectrometer->m_spectrometerIndex < 0 || m_Spectrometer->m_spectrometerIndex >= spectrometers.size()))
    {
        return;
    }
    else
    {
        // build the list 
        m_comboSpecs.ResetContent();
        for (const std::string& serial : spectrometers)
        {
            m_comboSpecs.AddString(serial.c_str());
        }

        // also select the currently used one
        if (m_Spectrometer->m_spectrometerIndex >= 0)
        {
            m_comboSpecs.SetCurSel(m_Spectrometer->m_spectrometerIndex);
        }

        // Not least, set the channel to use
        this->m_channel = m_Spectrometer->m_spectrometerChannel;

        UpdateData(FALSE);
    }

    return;
}

/** Updates the dialog with the list of spectrometers from CSpectrometer */
LRESULT CSpectrumSettingsDlg::OnChangeSpectrometer(WPARAM wParam, LPARAM lParam)
{

    const auto spectrometers = m_Spectrometer->GetConnectedSpectrometers();

    // build the list 
    m_comboSpecs.ResetContent();
    for (const std::string& serial : spectrometers)
    {
        m_comboSpecs.AddString(serial.c_str());
    }

    // also select the currently used one
    if (m_Spectrometer->m_spectrometerIndex >= 0)
    {
        m_comboSpecs.SetCurSel(m_Spectrometer->m_spectrometerIndex);
    }

    // Not least, set the channel to use
    m_channel = m_Spectrometer->m_spectrometerChannel;
    GetDlgItem(IDC_RADIO_CHANNEL0)->EnableWindow(m_Spectrometer->m_NChannels >= 2);
    GetDlgItem(IDC_RADIO_CHANNEL1)->EnableWindow(m_Spectrometer->m_NChannels >= 2);

    UpdateData(FALSE);

    return 0;
}

void CSpectrumSettingsDlg::OnUserChangeSpectrometer() {
    UpdateData(TRUE);

    int curSel = m_comboSpecs.GetCurSel();
    if (curSel == -1 || (curSel == m_Spectrometer->m_spectrometerIndex && m_channel == m_Spectrometer->m_spectrometerChannel))
        return;

    // change the spectrometer to use...
    if (m_channel == 1 && m_Spectrometer->m_NChannels < 2) {
        MessageBox("Please set N Channels to 2 in configuration to view slave channel output.");
    }

    std::vector<int> channelsToUse{ m_channel };
    m_Spectrometer->ChangeSpectrometer(curSel, channelsToUse);
}
