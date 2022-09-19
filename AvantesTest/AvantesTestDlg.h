
// AvantesTestDlg.h : header file
//

#pragma once
#include <MobileDoasLib/Measurement/SpectrometerInterface.h>

// CAvantesTestDlg dialog
class CAvantesTestDlg : public CDialogEx
{
    // Construction
public:
    CAvantesTestDlg(CWnd* pParent = nullptr);	// standard constructor

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_AVANTESTEST_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


    // Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
public:
    CComboBox m_spectrometerCombo;
    CListBox m_spectrumList;
    CEdit m_numSpectraEdit;
    CEdit m_integrationTimeEdit;

    afx_msg void OnBnClickedSearchForDevices();
    afx_msg void OnBnClickedAcquireSpectra();
    afx_msg void OnCbnSelchangeComboSpectrometers();
    afx_msg void OnChangeSpectraToAverage();
    afx_msg void OnChangeIntegrationTime();

private:
    mobiledoas::SpectrometerInterface* m_spectrometer = nullptr;

};
