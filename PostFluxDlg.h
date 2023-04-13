#if !defined(AFX_FLUXSETDLG_H__9D82D4EE_BF6E_44FC_9877_775FACDBB865__INCLUDED_)
#define AFX_FLUXSETDLG_H__9D82D4EE_BF6E_44FC_9877_775FACDBB865__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CPostFluxDlg.h : header file
//
#include "FluxPathListBox.h"
#include <MobileDoasLib/Flux/Flux1.h>
#include "RouteDlg.h"
#include "Graphs\ColumnGraph.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "Common.h"

#include "ReEvaluation\ReEvaluationDlg.h"
#include "ReEvaluation\ReEval_FitWindows.h"
#include "ReEvaluation\ReEval_SkyDlg.h"
#include "ReEvaluation\ReEval_DarkDlg.h"
#include "ReEvaluation\ReEval_EvalLogDlg.h"
#include "ReEvaluation\ReEval_DoEvaluationDlg.h"

//#include "Common.h"
/////////////////////////////////////////////////////////////////////////////
// CPostFluxDlg dialog

#define UNIT_KGS 0
#define UNIT_TONDAY 1

class CPostFluxDlg : public CDialog
{
    // Construction
public:
    CPostFluxDlg(CWnd* pParent = nullptr);   // standard constructor
    ~CPostFluxDlg();

    enum { IDD = IDD_POST_FLUX_DIALOG };


protected:
    // -------------------------------------------------------------
    // -------------------- PROTECTED METHODS ----------------------
    // ----------- THESE ARE CALLS FROM THE INTERFACE --------------
    // -------------------------------------------------------------

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    /** Opens a new evaluation - log file */
    afx_msg void OnBtnOpenLogFile();

    /** The user wants to calculate the flux */
    afx_msg void OnCalculateFlux();

    /** The user has pressed the '>>' button, show the
        next part of the traverse */
    afx_msg void OnBtnfor();

    /** The user has pressed the '<<' button, show the
        previous part of the traverse */
    afx_msg void OnBtnback();

    /** The user has changed the selected range of data points */
    afx_msg void OnReleasedcaptureSliderfrom(NMHDR* pNMHDR, LRESULT* pResult);

    /** The user has changed the selected range of data points */
    afx_msg void OnReleasedcaptureSliderto(NMHDR* pNMHDR, LRESULT* pResult);

    /** The user has changed the offset */
    afx_msg void OnReleasedcaptureSlideroffset(NMHDR* pNMHDR, LRESULT* pResult);

    /** The user has pressed the button 'Delete Selected' */
    afx_msg void OnBtnDeleteSelected();

    /** The user wants to see the route dialog*/
    afx_msg void OnBtnShowRoute();

    /** The user has pressed the button 'Delete Low/High Intensity Points' */
    afx_msg void OnBnClickedBtnDelIntensity();

    /** */
    afx_msg void OnNMReleasedcaptureSliderintensityLow(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNMReleasedcaptureSliderintensityHigh(NMHDR* pNMHDR, LRESULT* pResult);

    /** Called when the user wants to use the wind direction calculated
            as the line from the source to the maximum column value
            as the wind direction in the flux calculation. */
    afx_msg void OnStnClickedMaxwd();

    /** Called when the user wants to use the wind direction calculated
            as the line from the source to the centre of mass of the plume
            as the wind direction in the flux calculation. */
    afx_msg void OnStnClickedAvwd();

    /** Opens the dialog to select a source from the list */
    afx_msg void OnBnClickedBtnSourceLat();

    /** Opens the dialog to select a source from the list */
    afx_msg void OnBnClickedBtnSourceLong();

    /** */
    afx_msg void OnLbnSelchangePathList();

    /** Enabling or disabling the creation of the
        additional log file */
    afx_msg void OnChangeCreateAdditionalLogFile();

    /** Turning on/off the showing of the intensities */
    afx_msg void OnMenuViewShowIntensity();

    /** Turning on/off the showing of the error bars */
    afx_msg void OnMenuViewShowColumnError();

    /** Updating the interface item 'show intensity' */
    afx_msg void OnUpdateViewShowintensity(CCmdUI* pCmdUI);

    /** Updating the interface item 'show vs time' */
    afx_msg void OnUpdateViewShowVsTime(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewShowVsDistance(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewShowVsNumber(CCmdUI* pCmdUI);

    /** Updating the interface item 'create additional log file' */
    afx_msg void OnUpdateMenuItemCreateAdditionalLogFile(CCmdUI* pCmdUI);

    /** Updating the interface item 'show column error' */
    afx_msg void OnUpdateViewShowColumnError(CCmdUI* pCmdUI);

    /** Updating the interface item 'reload log file' */
    afx_msg void OnUpdateItemReloadLogFile(CCmdUI* pCmdUI);

    /** Updating the interface item 'calculate flux' */
    afx_msg void OnUpdateItemCalculateFlux(CCmdUI* pCmdUI);

    /** Updating the interface item 'show route dialog' */
    afx_msg void OnUpdateItemShowRouteDlg(CCmdUI* pCmdUI);

    /** Updating the interface item 're-evaluate this traverse' */
    afx_msg void OnUpdateItemReEvaluateTraverse(CCmdUI* pCmdUI);

    /** Updating the interface item 'close all log files' */
    afx_msg void OnUpdateItemCloseAllLogFiles(CCmdUI* pCmdUI);

    /** Updating the interface item 'save column graph' */
    afx_msg void OnUpdateItemSaveColumnGraph(CCmdUI* pCmdUI);

    /** Updating the interface item 'export log file' */
    afx_msg void OnUpdateItemExportLogFile(CCmdUI* pCmdUI);

    /** Updating the interface item 'delete selected' */
    afx_msg void OnUpdateItemDeleteSelected(CCmdUI* pCmdUI);

    /** Updating the interface item 'delete low itensity points' */
    afx_msg void OnUpdateItemDeleteLowIntensity(CCmdUI* pCmdUI);

    /** Updating the interface item 'import wind field' */
    afx_msg void OnUpdateItemImportWindField(CCmdUI* pCmdUI);

    /** Increase the line width of the column plot */
    afx_msg void OnColumnGraphIncreaseLineWidth();

    /** Decrease the line width of the column plot */
    afx_msg void OnColumnGraphDecreaseLineWidth();

    /** */
    afx_msg void OnReloadLogfile();

    /** Opens the re-evaluation dialog with the currently
            selected traverse */
    afx_msg void OnReEvaluateThisTraverse();

    /** Closes all log files */
    afx_msg void OnFileCloseAllLogFiles();

    /** */
    afx_msg void OnImportWindField();

    /** Closes the dialog */
    afx_msg void OnClose();

    /** Saves the column graph to file */
    afx_msg void OnFileSaveColumnGraph();

    /** Called when the user changes the latitude/longitude for the source */
    afx_msg void OnChangeSource();

    /** */
    afx_msg void OnFileExportLogfile();

    /** Exports this evaluation log file in .kml format
        (which can then be read by e.g. Google Earth) */
    afx_msg void OnFileExportToKML();

    /** Change if we should draw the plot against time,
        against distance travelled or against number */
    afx_msg void OnViewShowVsTime();
    afx_msg void OnViewShowVsDistance();
    afx_msg void OnViewShowVsNumber();

    DECLARE_MESSAGE_MAP()

public:
    // -------------------------------------------------------------
    // ----------------------- PUBLIC DATA -------------------------
    // -------------------------------------------------------------

    /** The route dialog */
    CRouteDlg* m_routeDlg;

    void ShowPlumeCenter();
    void ShowColumn();

    /* The high and low edges of the calculations */
    int   m_right;
    int   m_left;

    /** ?? */
    BOOL fMovePlot;

    /** The number of datapoints in the traverse we're currently showing */
    long m_recordNumber;

    /** The position of the maximum value in the traverse */
    double maxBuf[5];

    /** The position of the centre of mass value in the traverse */
    double avBuf[5];

    /** The slider that controls the offset */
    CSliderCtrl	m_sliderOffset;

    /** The sliders that controls the range */
    CSliderCtrl	m_sliderTo;
    CSliderCtrl	m_sliderFrom;

    /** The source */
    double	m_srcLat;
    double	m_srcLon;

    /** The wind field */
    double	m_WindDirect;
    double	m_WindSpeed;

    /** The low and high index, decides what spectra are used to calculate the flux */
    long	m_lowIndex;
    long	m_highIndex;

    /** The CFlux object, holds all important data and makes the calculations */
    mobiledoas::CFlux* m_flux;

    // Lets the user delete data points with intensity lower than a certain limit
    CSliderCtrl m_intensitySliderLow;
    CSliderCtrl m_intensitySliderHigh;

    // Lets the user select what unit to use
    CComboBox m_unitSelection;


    // lets the user create an additional log file
    bool m_additionalLogCheck;

    CToolTipCtrl* m_GraphToolTip;

    CFluxPathListBox m_pathList;

    // -------------------------------------------------------------
    // --------------------- PUBLIC METHODS ------------------------
    // -------------------------------------------------------------


    /* Tool tips */
    BOOL OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    /** Initializes the menu somehow, necessary otherwise it is not possible to
            update the status of the menu due to a bug in Microsoft MFC, see:
            http://support.microsoft.com/kb/242577 */
    void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

private:

    // -------------------------------------------------------------
    // ---------------------- PRIVATE DATA -------------------------
    // -------------------------------------------------------------

    /** The data buffers */
    double lonBuffer[MAX_TRAVERSELENGTH];
    double latBuffer[MAX_TRAVERSELENGTH];
    double columnBuffer[MAX_TRAVERSELENGTH];
    double colErrBuffer[MAX_TRAVERSELENGTH];
    double intensityBuffer[MAX_TRAVERSELENGTH];
    double altitudeBuffer[MAX_TRAVERSELENGTH];
    double timeBuffer[MAX_TRAVERSELENGTH];

    /** The plot */
    Graph::CColumnGraph m_ColumnPlot;

    /** The background color of the plot */
    COLORREF m_bkColor;

    /** The color of the line showing the evaluated columns */
    COLORREF m_PlotColor;

    /* m_showIntensity is true if the user wants
            to see the white intensity points */
    bool m_showIntensity;

    /** m_showColumnError is true if the user wants
            to see the column errors */
    bool m_showColumnError;

    /** What should be shown on the X-axis... */
    int m_XAxisUnit;

    /* the wind direction calculated from the average and the max column */
    double m_windDirectionAverage;
    double m_windDirectionMax;

    // -------------------------------------------------------------
    // -------------------- PRIVATE METHODS ------------------------
    // -------------------------------------------------------------

    /* The mobile log is the 'memory' of the program */
    void ReadMobileLog();
    void UpdateMobileLog();

    /** Called to open a new log file */
    int   OpenLogFile(CString& filename, CString& filePath);

    // the user has selected another log file to use 
    int   OnChangeSelectedFile();

    void  UpdatePropertiesWindow();
public:
    afx_msg void ChangeFluxUnit();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLUXSETDLG_H__9D82D4EE_BF6E_44FC_9877_775FACDBB865__INCLUDED_)
