#include "stdafx.h"
#include "../DMSpec.h"
#include "Configure_Spectrometer.h"
#include "ConfigurationFile.h"
#include <SpectralEvaluation/StringUtils.h>
#include <sstream>

using namespace Configuration;

// CConfigure_Spectrometer dialog

IMPLEMENT_DYNAMIC(CConfigure_Spectrometer, CPropertyPage)
CConfigure_Spectrometer::CConfigure_Spectrometer()
    : CPropertyPage(CConfigure_Spectrometer::IDD)
{
    m_conf = nullptr;
    m_availableBaudrates[0] = 4800;
    m_availableBaudrates[1] = 9600;
    m_availableBaudrates[2] = 19200;
    m_availableBaudrates[3] = 38400;
    m_availableBaudrates[4] = 57600;
    m_availableBaudrates[5] = 115200;
}

CConfigure_Spectrometer::~CConfigure_Spectrometer()
{
    m_conf = nullptr;
}

void CConfigure_Spectrometer::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);

    // The type of connection to the spectrometer
    DDX_Radio(pDX, IDC_RADIO_CONNECTION_USB, m_conf->m_spectrometerConnection);

    // The determination of the exposure-time
    DDX_Radio(pDX, IDC_RADIO_EXPTIME_AUTOMATIC, m_conf->m_expTimeMode);

    // set point for the CCD temperature
    DDX_Text(pDX, IDC_EDIT_SETPOINT, m_conf->m_setPointTemperature);

    // The parameters for judging how to calculate the exp-time
    DDX_Text(pDX, IDC_EDIT_SPECCENTER, m_conf->m_specCenter);
    DDX_Text(pDX, IDC_EDIT_PERCENT, m_conf->m_percent);
    DDX_Text(pDX, IDC_EDIT_FIXEXPTIME, m_conf->m_fixExpTime);
    DDX_Text(pDX, IDC_EDIT_TIMERESOLUTION, m_conf->m_timeResolution);
    DDX_Text(pDX, IDC_EDIT_SATURATION_LOW, m_conf->m_saturationLow);
    DDX_Text(pDX, IDC_EDIT_SATURATION_HIGH, m_conf->m_saturationHigh);
    DDX_Control(pDX, IDC_EDIT_SPECCENTER, m_editSpecCenter);
    DDX_Control(pDX, IDC_EDIT_PERCENT, m_editPercent);
    DDX_Control(pDX, IDC_EDIT_FIXEXPTIME, m_editFixExpTime);
    DDX_Control(pDX, IDC_EDIT_SATURATION_LOW, m_editSaturationLow);
    DDX_Control(pDX, IDC_EDIT_SATURATION_HIGH, m_editSaturationHigh);

    // Audio settings
    DDX_Check(pDX, IDC_CHECK_USEAUDIO, m_conf->m_useAudio);
    DDX_Text(pDX, IDC_EDIT_MAXCOLUMN, m_conf->m_maxColumn);

    // The time resolution
    DDX_Control(pDX, IDC_EDIT_TIMERESOLUTION, m_editTimeResolution);

    // The saturation range for adaptive mode
    //DDX_Control(pDX, IDC_EDIT_SATURATION_LOW, m_conf->m_saturationLow);
    //DDX_Control(pDX, IDC_EDIT_SATURATION_HIGH, m_conf->m_saturationHigh);

    // The removal of the offset
    DDX_Control(pDX, IDC_EDIT_OFFSETFROM, m_editOffsetFrom);
    DDX_Control(pDX, IDC_EDIT_OFFSETTO, m_editOffsetTo);
    DDX_Check(pDX, IDC_CHECK_NODARK, m_conf->m_noDark);

    DDX_Text(pDX, IDC_EDIT_OFFSETFROM, m_conf->m_offsetFrom);
    DDX_Text(pDX, IDC_EDIT_OFFSETTO, m_conf->m_offsetTo);

    // The combo-boxes
    DDX_Control(pDX, IDC_COMBO_PORT, m_specPort);
    DDX_Control(pDX, IDC_COMBO_BAUDRATE, m_specBaudrate);
    DDX_Control(pDX, IDC_COMBO_NCHANNELS, m_nChannels);


    DDX_Control(pDX, IDC_EDIT_SETPOINT, m_editSetPoint);
}


BEGIN_MESSAGE_MAP(CConfigure_Spectrometer, CPropertyPage)
    // Changing the contents of the radio-boxes:
    ON_LBN_SELCHANGE(IDC_COMBO_PORT, SaveSettings)
    ON_LBN_SELCHANGE(IDC_COMBO_BAUDRATE, SaveSettings)
    ON_LBN_SELCHANGE(IDC_COMBO_NCHANNELS, SaveSettings)

    // Changing the selection using the radio-buttons:
    ON_BN_CLICKED(IDC_RADIO_CONNECTION_USB, SaveSettings)
    ON_BN_CLICKED(IDC_RADIO_CONNECTION_SERIAL, SaveSettings)
    ON_BN_CLICKED(IDC_RADIO_CONNECTION_DIRECTORY, SaveSettings)
    ON_BN_CLICKED(IDC_RADIO_EXPTIME_AUTOMATIC, SaveSettings)
    ON_BN_CLICKED(IDC_RADIO_EXPTIME_FIXED, SaveSettings)
    ON_BN_CLICKED(IDC_RADIO_EXPTIME_ADAPTIVE, SaveSettings)

    // Changing the contents of the edit-boxes
    ON_EN_CHANGE(IDC_EDIT_SPECCENTER, SaveSettings)
    ON_EN_CHANGE(IDC_EDIT_PERCENT, SaveSettings)
    ON_EN_CHANGE(IDC_EDIT_FIXEXPTIME, SaveSettings)
    ON_EN_CHANGE(IDC_EDIT_TIMERESOLUTION, SaveSettings)
    ON_EN_CHANGE(IDC_EDIT_OFFSETFROM, SaveSettings)
    ON_EN_CHANGE(IDC_EDIT_OFFSETTO, SaveSettings)
    ON_EN_CHANGE(IDC_EDIT_SETPOINT, SaveSettings)
    ON_EN_CHANGE(IDC_EDIT_MAXCOLUMN, SaveSettings)

    // Changing the check box options
    ON_BN_CLICKED(IDC_CHECK_USEAUDIO, SaveSettings)
    ON_BN_CLICKED(IDC_CHECK_NODARK, SaveSettings)
END_MESSAGE_MAP()


// CConfigure_Spectrometer message handlers
BOOL CConfigure_Spectrometer::OnInitDialog() {

    CPropertyPage::OnInitDialog();

    int k;
    CString str;

    // Initialize the serial-port combo-box
    int toSelect = -1;
    for (k = 1; k < 22; ++k) {
        str.Format("COM%d", k);
        m_specPort.AddString(str);
        if (EqualsIgnoringCase(std::string((LPCSTR)str), m_conf->m_serialPort))
            toSelect = k - 1;
    }
    m_specPort.SetCurSel(toSelect);

    // Initialize the baudrate-combo-box
    toSelect = -1;
    for (k = 0; k < 6; ++k) {
        str.Format("%d", m_availableBaudrates[k]);
        m_specBaudrate.AddString(str);
        if (m_availableBaudrates[k] == m_conf->m_baudrate)
            toSelect = k;
    }
    m_specBaudrate.SetCurSel(toSelect);

    // Initialize the channels - combo-box
    str.Format("1");		m_nChannels.AddString(str);
    str.Format("2");		m_nChannels.AddString(str);
    m_nChannels.SetCurSel(m_conf->m_nChannels - 1);

    EnableControls();

    // setup the tool tips
    InitToolTips();

    return TRUE;
}

void CConfigure_Spectrometer::SaveSettings() {
    UpdateData(TRUE); // <-- save the data in the dialog
    int curPort = m_specPort.GetCurSel();
    int curBaud = m_specBaudrate.GetCurSel();
    int curChan = m_nChannels.GetCurSel();

    // The port
    std::stringstream portNumber;
    portNumber << "COM" << (curPort + 1);
    m_conf->m_serialPort = portNumber.str();

    // The baudrate
    if (curBaud == -1) {
        m_conf->m_baudrate = 9600;
        m_specBaudrate.SetCurSel(1);
    }
    else {
        m_conf->m_baudrate = m_availableBaudrates[curBaud];
    }

    // The channel
    m_conf->m_nChannels = max(curChan, 0) + 1;

    EnableControls();
}

/** Enables the controls that should be enabled, and disables
        the ones which should be disabled */
void CConfigure_Spectrometer::EnableControls() {
    UpdateData(TRUE); // <-- save the data in the dialog

    // Enable or disable the combo-boxes based on the preferred method of connection
    if (m_conf->m_spectrometerConnection == CMobileConfiguration::CONNECTION_USB) {
        m_specPort.EnableWindow(FALSE);
        m_specBaudrate.EnableWindow(FALSE);
    }
    else if (m_conf->m_spectrometerConnection == CMobileConfiguration::CONNECTION_RS232) {
        m_specPort.EnableWindow(TRUE);
        m_specBaudrate.EnableWindow(TRUE);
    }
    else if (m_conf->m_spectrometerConnection == CMobileConfiguration::CONNECTION_DIRECTORY) {
        m_specPort.EnableWindow(FALSE);
        m_specBaudrate.EnableWindow(FALSE);
    }

    // Enable or disable the edit-boxes based on the preferred method 
    //		of calculating the exposure time
    if (m_conf->m_expTimeMode == CMobileConfiguration::EXPOSURETIME_AUTOMATIC) {
        m_editSpecCenter.EnableWindow(TRUE);
        m_editPercent.EnableWindow(TRUE);
        m_editFixExpTime.EnableWindow(FALSE);
        m_editSaturationLow.EnableWindow(FALSE);
        m_editSaturationHigh.EnableWindow(FALSE);
    }
    else if (m_conf->m_expTimeMode == CMobileConfiguration::EXPOSURETIME_FIXED) {
        m_editSpecCenter.EnableWindow(FALSE);
        m_editPercent.EnableWindow(FALSE);
        m_editFixExpTime.EnableWindow(TRUE);
        m_editSaturationLow.EnableWindow(FALSE);
        m_editSaturationHigh.EnableWindow(FALSE);
    }
    else if (m_conf->m_expTimeMode == CMobileConfiguration::EXPOSURETIME_ADAPTIVE) {
        m_editSpecCenter.EnableWindow(TRUE);
        m_editPercent.EnableWindow(TRUE);
        m_editFixExpTime.EnableWindow(FALSE);
        m_editSaturationLow.EnableWindow(TRUE);
        m_editSaturationHigh.EnableWindow(TRUE);
    }
}

/** Called when the 'Save' - button is pressed */
void CConfigure_Spectrometer::OnOK()
{
    try
    {
        ConfigurationFile::Write(*m_conf);
    }
    catch (std::exception& e)
    {
        MessageBox(e.what(), "Error", MB_OK);
    }
}

void CConfigure_Spectrometer::InitToolTips() {
    // Don't initialize the tool tips twice
    if (m_toolTip.m_hWnd != nullptr)
        return;

    // Enable the tool tips
    if (!m_toolTip.Create(this)) {
        TRACE0("Failed to create tooltip control\n");
    }
    m_toolTip.AddTool(&m_specPort, IDC_COMBO_PORT);
    m_toolTip.AddTool(&m_specBaudrate, IDC_COMBO_BAUDRATE);
    m_toolTip.AddTool(&m_nChannels, IDC_COMBO_NCHANNELS);
    m_toolTip.AddTool(&m_editSetPoint, IDC_EDIT_SETPOINT);
    m_toolTip.AddTool(&m_editSpecCenter, IDC_EDIT_SPECCENTER);
    m_toolTip.AddTool(&m_editPercent, IDC_EDIT_PERCENT);
    m_toolTip.AddTool(&m_editFixExpTime, IDC_EDIT_FIXEXPTIME);
    m_toolTip.AddTool(&m_editTimeResolution, IDC_EDIT_TIMERESOLUTION);
    m_toolTip.AddTool(&m_editSaturationLow, IDC_EDIT_SATURATION_LOW);
    m_toolTip.AddTool(&m_editSaturationHigh, IDC_EDIT_SATURATION_HIGH);
    m_toolTip.AddTool(&m_editOffsetTo, IDC_EDIT_OFFSETTO);
    m_toolTip.AddTool(&m_editOffsetFrom, IDC_EDIT_OFFSETFROM);

    m_toolTip.SetMaxTipWidth(SHRT_MAX);
    m_toolTip.Activate(TRUE);
}

BOOL CConfigure_Spectrometer::PreTranslateMessage(MSG* pMsg) {
    m_toolTip.RelayEvent(pMsg);

    return CPropertyPage::PreTranslateMessage(pMsg);
}

