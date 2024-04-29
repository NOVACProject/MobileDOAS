#pragma once

#include "ReEvaluator.h"
#include "afxwin.h"

#include "../Graphs/GraphCtrl.h"
#include "afxcmn.h"

// CReEval_DarkDlg dialog

namespace ReEvaluation
{
class CReEval_DarkDlg : public CPropertyPage
{
    DECLARE_DYNAMIC(CReEval_DarkDlg)

public:
    CReEval_DarkDlg();
    virtual ~CReEval_DarkDlg();

    // Dialog Data
    enum { IDD = IDD_REEVAL_DARK };

    CReEvaluator* m_reeval;

    void    DrawOffsetPlot();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

private:
    int   m_numDarkSpectra;

    CStatic m_graphBoundary;
    Graph::CGraphCtrl m_offsetPlot;

public:

    CWinThread* pReEvalThread;  // used when running some lengthy function in ReEvaluator as a separate thread
    afx_msg void OnBnClickedSeveralDarkspectra();
    afx_msg void OnBnClickedSingleDarkspectrum();
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedCheckOffsets();

    LRESULT OnProgress(WPARAM wp, LPARAM lp);
    LRESULT OnDone(WPARAM wp, LPARAM lp);

    CProgressCtrl m_progressCtrl;
    CButton m_OffsetButton;
};
}

UINT ReadOffsets(LPVOID pParam);
