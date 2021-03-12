#pragma once


// CCalibrationWavelength dialog

class CCalibrationWavelength : public CPropertyPage
{
    DECLARE_DYNAMIC(CCalibrationWavelength)

public:
    CCalibrationWavelength(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CCalibrationWavelength();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CALIBRATE_WAVELENGTH_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:

    CString m_inputSpectrum;
    CString m_solarSpectrum;
    CString m_initialCalibrationFile;

    afx_msg void OnBnClickedButtonBrowseSpectrum();
    afx_msg void OnClickedButtonBrowseSolarSpectrum();
    afx_msg void OnClickedButtonRun();
    afx_msg void OnClickedButtonBrowseInitialCalibration();
};
