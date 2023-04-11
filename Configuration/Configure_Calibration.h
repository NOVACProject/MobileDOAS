#pragma once

#include <memory>
#include "MobileConfiguration.h"

namespace novac
{
class StandardCrossSectionSetup;
}

// CConfigure_Calibration dialog
namespace Configuration {

class CConfigure_Calibration : public CPropertyPage
{
    DECLARE_DYNAMIC(CConfigure_Calibration)

public:
    CConfigure_Calibration(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CConfigure_Calibration();

    // Dialog Data
    enum { IDD = IDD_CONFIGURE_CALIBRATION };

    /** The local currentSpectrometerHandle to the configuration that we're changing */
    std::shared_ptr<CMobileConfiguration> m_conf;

    afx_msg void OnClickedButtonBrowseSolarSpectrum();
    afx_msg void OnButtonSelectInitialCalibration();
    afx_msg void OnBnClickedRadioInstrumentLineShapeFitOption();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    /** Called when the dialog is to be shown */
    virtual BOOL OnInitDialog();

    afx_msg void SaveData();

    BOOL OnSetActive();

private:

    void LoadDefaultSetup();

    novac::StandardCrossSectionSetup* m_standardCrossSections = nullptr;

    CEdit m_fitRegionEditLow;
    CEdit m_fitRegionEditHigh;

};

}