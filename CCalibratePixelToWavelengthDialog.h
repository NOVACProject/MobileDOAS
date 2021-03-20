#pragma once


// CCalibratePixelToWavelengthDialog dialog

class CCalibratePixelToWavelengthDialog : public CPropertyPage
{
    DECLARE_DYNAMIC(CCalibratePixelToWavelengthDialog)

public:
    CCalibratePixelToWavelengthDialog(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CCalibratePixelToWavelengthDialog();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CALIBRATE_WAVELENGTH_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:

    CString m_inputSpectrumFile;
    CString m_solarSpectrumFile;
    CString m_initialCalibrationFile;
    CString m_instrumentLineshapeFile;
    CString m_darkSpectrumFile;

    afx_msg void OnBnClickedButtonBrowseSpectrum();
    afx_msg void OnClickedButtonBrowseSolarSpectrum();
    afx_msg void OnClickedButtonRun();
    afx_msg void OnClickedButtonBrowseInitialCalibration();
    afx_msg void OnBnClickedButtonBrowseSpectrumDark();
    afx_msg void OnBnClickedButtonBrowseLineShape();
};
