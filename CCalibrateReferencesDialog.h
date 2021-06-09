#pragma once

#include <string>
#include "Graphs/GraphCtrl.h"

// CCalibrateReferenes dialog

class ReferenceCreationController;

class CCalibrateReferencesDialog : public CPropertyPage
{
    DECLARE_DYNAMIC(CCalibrateReferencesDialog)

public:
    CCalibrateReferencesDialog(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CCalibrateReferencesDialog();

    /** Initializes the controls and the dialog */
    virtual BOOL OnInitDialog();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CALIBRATE_REFERENCES };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    CComboBox m_crossSectionsCombo;
    CString m_calibrationFile;  //< the full calibration file, in the new xml-based format
    BOOL m_highPassFilterReference;
    BOOL m_inputInVacuum;

    CButton m_saveButton;

    Graph::CGraphCtrl m_graph; // The plot for the spectrum
    CStatic m_graphHolder;

    afx_msg void OnConvolutionOptionChanged();
    afx_msg void OnBnClickedBrowseCalibration();
    afx_msg void OnBnClickedBrowseCrossSection();
    afx_msg void OnBnClickedButtonRunCreateReference();
    afx_msg void OnClickedButtonSave();

private:
    std::string SetupFilePath();
    void SaveSetup();
    void LoadSetup();

    void UpdateReference();

    void UpdateGraph();

    ReferenceCreationController* m_controller;
};
