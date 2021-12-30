#pragma once

#include <memory>
#include "../Controls/ReferenceFileControl.h"
#include "MobileConfiguration.h"
#include "../Controls/FitWindowListBox.h"

// CConfigure_Evaluation dialog

namespace Configuration {

class CConfigure_Evaluation : public CPropertyPage
{
    DECLARE_DYNAMIC(CConfigure_Evaluation)

public:
    CConfigure_Evaluation();
    virtual ~CConfigure_Evaluation();

    // Dialog Data
    enum { IDD = IDD_CONFIGURE_EVALUATION };

    /** The local handle to the CMobileConfiguration object that we're changing */
    std::shared_ptr<CMobileConfiguration> m_conf;

    /** The local handle to the option of whether to use this window or not */
    int* m_Evaluate;

    /** The main-specie that this window evaluates for */
    CString m_specieName;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    /** Called when the dialog is to be shown */
    virtual BOOL OnInitDialog();

    /** Enables the controls that should be enabled, and disables
            the ones which should be disabled */
            //afx_msg void EnableControls();

    /** Saves the data in the dialog */
    afx_msg void SaveData();

    /** The user has changed the currently selected fit-window */
    afx_msg void OnChangeFitWindow();

    /** Called when the user wants to see the
    properties of one reference */
    void OnShowProperties();

    /** Called when the user wants to see the
        size of the references */
    void OnShowReferenceGraph();

    /** Inititalizes the reference-file control */
    void InitReferenceFileControl();

    /** Fills in the cells of the reference-file control */
    void PopulateReferenceFileControl();

    /** Fills the window list with names */
    void PopulateWindowList();

    /** Called when the user wants to remove a reference from the fit window. */
    void OnRemoveReference();

    /** Called when the user wants to insert a new reference to the fit window. */
    void OnInsertReference();

    /** Setup the tool tips */
    void InitToolTips();

    /** Handling the tool tips */
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    // -------------- DIALOG COMPONENTS -----------------

    /** The frame for defining the size of the reference file control */
    CStatic m_referenceFrame;

    /** The list of fit-windows */
    DlgControls::CFitWindowListBox m_windowList;

    /** The reference grid, enables the user to select reference files */
    DlgControls::CReferenceFileControl m_referenceGrid;

    /** The tooltip control */
    CToolTipCtrl m_toolTip;

    // -------------------- PROTECTED DATA --------------------------

    /** Fit parameters */
    int m_fitHigh, m_fitLow, m_polyOrder;
    int m_fitChannel;

    int m_runCalibrationInMeasurement;

    // -------------------- PROTECTED METHODS --------------------------
};
}