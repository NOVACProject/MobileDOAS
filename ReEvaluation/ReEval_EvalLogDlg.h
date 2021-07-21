#ifndef _EVALLOGDLG_
#define _EVALLOGDLG_

#include "ReEvaluator.h"

#include "../Graphs/GraphCtrl.h"
#include "afxwin.h"

#pragma once

// CReEval_EvalLogDlg dialog
namespace ReEvaluation
{

class CReEval_EvalLogDlg : public CPropertyPage
{
    DECLARE_DYNAMIC(CReEval_EvalLogDlg)

public:
    CReEval_EvalLogDlg();
    virtual ~CReEval_EvalLogDlg();

    // Dialog Data
    enum { IDD = IDD_REEVAL_EVALUATIONLOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
private:

    /** The column plot - shows the data from the traverse */
    Graph::CGraphCtrl   m_ColumnPlot;

    /** The (static) frame for the graph showing the data in the traverse */
    CStatic m_graphBoundary;

    double* number;
    bool InitializeNumber();

    /** The wavelength region radio - button */
    int m_wavelengthRegion;

    /** The check button 'ignore dark spectra' */
    CButton m_ignoreDarkCheck;

    CButton m_viewButton;
public:

    /** Called to draw the column plot */
    void DrawColumn();

    /** Initializes the controls and the dialog */
    virtual BOOL OnInitDialog();

    afx_msg void OnEnChangeNmergeEdit();
    afx_msg void OnEnChangeEditIgnoreSpectra();
    afx_msg void OnBnClickedButtonBrowseEvlog();
    afx_msg void SaveData();

    afx_msg void ReadEvaluationLog();

    /** Called when the user presses the button 'view spectra' */
    afx_msg void OnViewSpectra();

    /** A pointer to the CReEvaluator object that we're modifying */
    CReEvaluator* m_reeval;

};
}
#endif