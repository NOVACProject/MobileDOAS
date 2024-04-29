#pragma once

#include "../DMSpec.h"
#include "../Graphs/GraphCtrl.h"
#include "ReEvaluator.h"
#include "afxcmn.h"
#include "afxwin.h"

// CReEval_DoEvaluationDlg dialog

namespace ReEvaluation
{
class CReEval_DoEvaluationDlg : public CPropertyPage
{
    DECLARE_DYNAMIC(CReEval_DoEvaluationDlg)

public:
    CReEval_DoEvaluationDlg();   // standard constructor
    virtual ~CReEval_DoEvaluationDlg();

    CReEvaluator* m_reeval;

    // Dialog Data
    enum { IDD = IDD_REEVAL_EVALUATE };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:

    LRESULT OnProgress(WPARAM wp, LPARAM lp);
    LRESULT OnCancel(WPARAM wp, LPARAM lp);
    LRESULT OnDone(WPARAM wp, LPARAM lp);
    LRESULT OnStatusUpdate(WPARAM wp, LPARAM lp);
    LRESULT OnEvaluatedSpectrum(WPARAM wp, LPARAM lp);

    /** Called when the reevaluator has gone to sleep and is waiting for the user
        to continue. */
    LRESULT OnEvaluationSleep(WPARAM wp, LPARAM lp);

    /** A pointer to a thread. The reevaluator is run as a separate thread
        when doing some lengthy operations */
    CWinThread* pReEvalThread;

    afx_msg void OnBnClickedDoEvaluation();
    afx_msg void OnBnClickedCancel();
    afx_msg void SaveData();

    /** The Progress Bar */
    CProgressCtrl m_progressCtrl;

    /** The Cancel Button */
    CButton m_cancelButton;

    /** The DoEvaluation/Next Button */
    CButton m_btnDoEvaluation;

    /** ?? */
    CButton m_showFit;

    /** The list box for the output messages */
    CListBox m_outputList;

    virtual BOOL OnInitDialog();

    afx_msg void DrawSpec();
    void DrawFit();
    void DrawResidual();
    void UpdateScreen();

    CStatic m_fitWindow;

private:
    Graph::CGraphCtrl m_graph;
public:
    virtual void OnOK();
    afx_msg void OnClose();
};
}
UINT DoEvaluation(LPVOID pParam);
