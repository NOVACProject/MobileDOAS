#pragma once

#include "Graphs/GraphCtrl.h"

class CSpectrometer;

namespace Dialogs
{

// CShowFitDlg shows the realtime DOAS fit in a small dialog window on top of the main window.
// There are two plots available, with the ability to show the result of two fit windows simultaneously.
class CShowFitDlg : public CDialog
{
    DECLARE_DYNAMIC(CShowFitDlg)

public:
    CShowFitDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CShowFitDlg();

    // Dialog Data
    enum { IDD = IDD_VIEW_FIT_DLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:

    /** Called when the dialog is opened */
    virtual BOOL OnInitDialog();

    /** True if the plot is shown */
    bool m_isVisible;

    /** A pointer to the spectrometer in question. */
    CSpectrometer* m_spectrometer;

    afx_msg void OnSize(UINT nType, int cx, int cy);

    virtual BOOL Create(UINT nID, CWnd* pParentWnd = NULL);
    afx_msg void OnClose();

    /** The actual drawing of the fit */
    void DrawFit();

private:

    static const COLORREF red = RGB(255, 0, 0);
    static const COLORREF cyan = RGB(0, 255, 255);

    void DrawFit1();
    void DrawFit2();

    Graph::CGraphCtrl m_fitPlot;
    Graph::CGraphCtrl m_fitPlot2;

    double pixel[MAX_SPECTRUM_LENGTH];
};

}