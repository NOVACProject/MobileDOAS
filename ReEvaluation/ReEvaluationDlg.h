#pragma once
#include "afxdlgs.h"

#include "ReEval_FitWindows.h"
#include "ReEval_SkyDlg.h"
#include "ReEval_DarkDlg.h"
#include "ReEval_EvalLogDlg.h"
#include "ReEval_DoEvaluationDlg.h"
#include "ReEvalSettingsFileHandler.h"
#include "ReEval_ScriptDlg.h"

namespace ReEvaluation
{

class CReEvaluationDlg :
    public CPropertySheet
{
public:
    CReEvaluationDlg(void);
    ~CReEvaluationDlg(void);

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnClose();
    virtual INT_PTR DoModal();

    afx_msg void OnMenuBrowseForEvalLog();
    afx_msg void OnMenuClose();
    afx_msg void OnMenuLoadSettingsFromFile();
    afx_msg void OnMenuSaveSettingsToFile();
    afx_msg void OnMenuCreateScript();
    afx_msg void OnMenuRunScript();

    CReEvaluator* m_reeval;
    CReEval_EvalLogDlg m_page1;
    CReEval_DarkDlg m_page2;
    CReEval_SkyDlg m_page3;
    CReEval_FitWindowsDlg m_page4;
    CReEval_DoEvaluationDlg m_page5;

};
}