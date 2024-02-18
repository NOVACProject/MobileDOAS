#pragma once
#include "afxwin.h"
#include "afxcmn.h"

// CSpectrumScaleDlg dialog
namespace Dialogs
{

class CSpectrumScaleDlg : public CDialog
{
    DECLARE_DYNAMIC(CSpectrumScaleDlg)

public:
    CSpectrumScaleDlg(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CSpectrumScaleDlg();

    void SetMainForm(CView* mainForm) { this->m_mainForm = mainForm; }

    // Dialog Data
    enum { IDD = IDD_SPECTRUM_SCALE_DLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

private:
    CView* m_mainForm = nullptr;

public:

    /** The minimum value of the saturation scale */
    int m_minValue;

    /** The maximum value of the saturation scale */
    int m_maxValue;

    /** The edit boxes controls */
    CEdit m_editMin;
    CEdit m_editMax;

    /** The spin-buttons */
    CSpinButtonCtrl m_spinMin;
    CSpinButtonCtrl m_spinMax;

    /** Called when the user has changed the values in the edit boxes */
    afx_msg void OnChangeMin();
    afx_msg void OnChangeMax();

    /** Called when the user has pushed the spin-buttons up or down */
    afx_msg void OnChangeSpinMinvalue(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnChangeSpinMaxvalue(NMHDR* pNMHDR, LRESULT* pResult);

    /** Initializes the dialog and its controls */
    virtual BOOL OnInitDialog();


};
}