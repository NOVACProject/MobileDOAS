#pragma once


// CCalibrateInstrumentLineShape dialog

class CCalibrateInstrumentLineShape : public CPropertyPage
{
    DECLARE_DYNAMIC(CCalibrateInstrumentLineShape)

public:
    CCalibrateInstrumentLineShape(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CCalibrateInstrumentLineShape();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CALIBRATE_LINESHAPE_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonBrowseSpectrum();
    CString m_inputSpectrum;
};
