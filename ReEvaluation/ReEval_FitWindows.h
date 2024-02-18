#pragma once

#include "../Controls/ReferenceFileControl.h"
#include "../Evaluation/FitWindow.h"
#include "../Common.h"
#include "ReEvaluator.h"
#include "../DMSpec.h"

// CReEval_FitWindowsDlg dialog

namespace ReEvaluation
{
class CReEval_FitWindowsDlg : public CPropertyPage
{
    DECLARE_DYNAMIC(CReEval_FitWindowsDlg)

public:
    CReEval_FitWindowsDlg();   // standard constructor
    virtual ~CReEval_FitWindowsDlg();

    // Dialog Data
    enum { IDD = IDD_REEVAL_FITWINDOW };

    CReEvaluator* m_reeval;

    /** Initialize the dialog and its controls */
    virtual BOOL OnInitDialog();

    /** Fill the reference grid with values */
    void PopulateReferenceFileControl();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    CStatic m_referenceFrame;

    CListBox m_windowList;
private:
    /** The grid, controlling which references to use */
    DlgControls::CReferenceFileControl m_referenceGrid;

    /** Initializes the reference grid */
    void InitReferenceFileControl();

    /** Fills the window list with names */
    void PopulateWindowList();

    /** Called when the user wants to remove
        a reference from the fit window. */
    void OnRemoveReference();

    /** Called when the user wants to insert
        a new reference to the fit window. */
    void OnInsertReference();

    /** Called when the user wants to see the
        properties of one reference */
    void OnShowProperties();

    /** Called when the user wants to see the
        size of the references */
    void OnShowReferenceGraph();

    /** Stores the data in the dialog */
    void SaveData();
};
}