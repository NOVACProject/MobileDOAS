#pragma once

#include <memory>
#include "MobileConfiguration.h"

// CConfigure_Spectrometer dialog

namespace Configuration {
class CConfigure_Spectrometer : public CPropertyPage
{
    DECLARE_DYNAMIC(CConfigure_Spectrometer)

public:
    CConfigure_Spectrometer();
    virtual ~CConfigure_Spectrometer();

    // Dialog Data
    enum { IDD = IDD_CONFIGURE_SPECTROMETER };

    /** The local currentSpectrometerHandle to the configuration that we're changing */
    std::shared_ptr<CMobileConfiguration> m_conf;

    // ----------------------- EVENT HANDLERS -------------------------
    /** Saves the settings in the dialog */
    afx_msg void SaveSettings();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    /** Called when the dialog is to be shown */
    virtual BOOL OnInitDialog();

    /** Enables the controls that should be enabled, and disables
            the ones which should be disabled */
    afx_msg void	EnableControls();

    /** Writes the configuration to file again and closes the dialog */
    afx_msg void OnOK();

    /** Setup the tool tips */
    void InitToolTips();

    /** Handling the tool tips */
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    // -------------- DIALOG COMPONENTS -----------------

    /** The selection of port to use for the RS232 communication with the spectrometer */
    CComboBox	m_specPort;

    /** The baudrate to use when talking to the spectrometer through RS232 */
    CComboBox	m_specBaudrate;

    /** The number of channels to use on the spectrometer */
    CComboBox	m_nChannels;

    /** The set point for CCD temperature */
    CEdit m_editSetPoint;

    /** The edit-box for the spectrum-center*/
    CEdit m_editSpecCenter;

    /** The edit-box for the percent */
    CEdit m_editPercent;

    /** The edit-box for the fixed exposure-time */
    CEdit m_editFixExpTime;

    /** The edit boxes for tolerable saturation ratios */
    CEdit m_editSaturationLow;
    CEdit m_editSaturationHigh;

    /** The tooltip control */
    CToolTipCtrl m_toolTip;

    /** The edit-box for the time-resolution */
    CEdit m_editTimeResolution;

    /** The edit-boxes for the offset-removal */
    CEdit	m_editOffsetTo, m_editOffsetFrom;

    // -------------------- PROTECTED DATA --------------------------

    /** The baudrates that the user can choose from */
    long	m_availableBaudrates[6];
public:

};
}