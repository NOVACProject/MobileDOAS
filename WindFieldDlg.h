#pragma once

#include <MobileDoasLib/Flux/Flux1.h>
#include <MobileDoasLib/Flux/WindField.h>
#include "afxwin.h"
#include "Graphs/GraphCtrl.h"

// CWindFieldDlg dialog

//namespace Flux
//{
class CWindFieldDlg : public CDialog
{
    DECLARE_DYNAMIC(CWindFieldDlg)

public:
    CWindFieldDlg(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CWindFieldDlg();

    // Dialog Data
    enum { IDD = IDD_FLUX_WINDFIELD_DLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

        /** Called when the dialog is first opened */
        virtual BOOL OnInitDialog();

    /** Called when the user presses the 'Use' button. This closes the dialog and
        creates a copy of the windfield for the currently selected altitude in 'm_flux'
        and initializes an interpolated wind field for the currently selected
        traverse in the 'm_flux' object. */
    virtual void OnOK();

public:
    /** a pointer to the flux object */
    mobiledoas::CFlux* m_flux;

    /** the path to the windfield file */
    CString         m_windfile;

    /** Called when the user pressed the 'browse' button and wants to browse
        for a file containing a wind field */
    afx_msg void OnBrowseWindFieldFile();

    /** Called when the user changes selection of the altitude layer */
    afx_msg void OnChangeAltitude();

    /** Called when the user changes selection of the hour */
    afx_msg void OnChangeHour();

    /** Called when the user changes the selection of the wind field file */
    afx_msg void OnChangeWindFieldFile();

    /** The actual wind field */
    mobiledoas::CWindField* m_windField;


private:
    // ------------- PRIVATE DATA ----------------
    /** The altitude selection */
    CComboBox   m_altitudeCombo;

    /** The hour selection combo box */
    CComboBox   m_hourCombo;

    /** The combo box to select a wind field file */
    CComboBox m_windFileCombo;

    /** The defined hours. If string number 'i' is selected in the
        'm_hourCombo' then 'm_hours[i]' tells us which hour this really is. */
    char m_hours[mobiledoas::CWindField::MAX_HOURS];

    /** The wind field graph */
    Graph::CGraphCtrl m_graph;

    /** The frame around the graph */
    CStatic m_graphFrame;

    /** Draws the field */
    void DrawField();

    // ---------- PRIVATE METHODS --------------
    void FillCombos();
    void FillCombo_Altitude();
    void FillCombo_Hour();

    void ReadMobileLog();
    void UpdateMobileLog();
public:
    int m_useTimeShift;
    int m_timeShift;
};
//}