#if !defined(AFX_ROUTEDLG_H__DB80A786_6CCE_4B5C_AF7F_2AE96E26BA18__INCLUDED_)
#define AFX_ROUTEDLG_H__DB80A786_6CCE_4B5C_AF7F_2AE96E26BA18__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_FLUX_LOGFILES 32

#include "RouteGraph.h"
#include "Common.h"

#include "MeasGrid.h"

#include "afxwin.h"
#include "afxcmn.h"
#include <MobileDoasLib/Flux/Flux1.h>

#define SHOW_COLUMN        0
#define SHOW_ALTITUDE      1
#define SHOW_INTENSITY     2


// RouteDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CRouteDlg dialog
class CRouteDlg : public CDialog
{
    // Construction
public:
    CRouteDlg(CWnd* pParent = NULL);   // standard constructor

    mobiledoas::CFlux* m_flux;
    double  m_lon[MAX_FLUX_LOGFILES][MAX_TRAVERSELENGTH];
    double  m_lat[MAX_FLUX_LOGFILES][MAX_TRAVERSELENGTH];
    double  m_col[MAX_FLUX_LOGFILES][MAX_TRAVERSELENGTH];
    double  m_alt[MAX_FLUX_LOGFILES][MAX_TRAVERSELENGTH];
    double  m_int[MAX_FLUX_LOGFILES][MAX_TRAVERSELENGTH];
    long    m_recordLen[MAX_FLUX_LOGFILES];

    // The number of traverses to show, this is the smallest of 
    //	m->flux.m_traversNum and MAX_FLUX_LOGFILES
    long	m_traversesToShow;

    // the wind speed and winddirection for every point, used only if there's a defined windfield
    double  m_ws[MAX_FLUX_LOGFILES][MAX_TRAVERSELENGTH];
    double  m_wd[MAX_FLUX_LOGFILES][MAX_TRAVERSELENGTH];
    bool    m_useWindField[MAX_FLUX_LOGFILES];

    void    InitBuffers();

    double m_srcLat, m_srcLon;

    /* the plot */
    Graph::CRouteGraph m_gpsPlot;

    /* Plotting the data */
    void   DrawRouteGraph();

    /* Calculating the range of the plot */
    void   GetPlotRange(struct plotRange& range);

    /* Calculating the color range of the plot */
    void   GetColorRange(double& minValue, double& maxValue);

    // Dialog Data
    //{{AFX_DATA(CRouteDlg)
    enum { IDD = IDD_ROUTEDLG };
    // NOTE: the ClassWizard will add data members here
//}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CRouteDlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CRouteDlg)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    LRESULT OnShowLatLong(WPARAM wParam, LPARAM lParam);
    LRESULT OnEndEditLandMarkPosition(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
public:
    double m_curLatitude;

    /** The route which is selected when the dialog is opened */
    int m_initalSelection;

    afx_msg void OnBnClickedCheckShowSource();
    CButton m_showSourceCheck;
    afx_msg void OnEnChangeSrcLatEdit();
    afx_msg void OnEnChangeSrcLonEdit();

private:
    int m_showSource; /* saves the state of the "m_showSourceCheck" button between each time the window is showed */

    int m_showScale;

    int m_showColumnOption;

    /** The color used when drawing the wind-field */
    COLORREF m_windFieldColor;
    COLORREF m_circlesColor;

    /** The color of the grid */
public:
    /* Calculating distance and bearing */
    CButton m_calcDistance;
    double  lat1, lon1, lat2, lon2;
    int     nClicks;
    LRESULT OnLBUInGPSGraph(WPARAM wParam, LPARAM lParam);

    /** Zooming in the graph */
    LRESULT OnZoomGPSGraph(WPARAM wParam, LPARAM lParam);


    afx_msg void OnBnClickedCheckCalcDistance();
    int m_selectedDisplay;
    afx_msg void OnBnClickedShowColumn();
    afx_msg void OnBnClickedShowAltitude();
    afx_msg void OnBnClickedShowIntensity();
    // The list of open evaluation logs
    CListBox m_evalLogList;
    afx_msg void OnLbnSelchangeEvallogList();
    CButton m_showInterpolatedWindField;
    afx_msg void OnBnClickedCheckShowInterpolatedWindfield();

    /** The size of the points that are drawn */
    CSpinButtonCtrl m_pointSizeSpin;
    CEdit m_pointSizeEdit;
    int m_pointSize;
    afx_msg void OnBnClickedBtnImportWindfield();

    /** Called when the user changes the size of the points showing the route */
    afx_msg void OnDeltaposRoutedlgPointsizespin(NMHDR* pNMHDR, LRESULT* pResult);

    /** Called when the user changes the size of the points showing the route */
    afx_msg void OnEnChangeRoutdlgPointsizeedit();
    afx_msg void OnChangeBackgroundColor();
    afx_msg void OnChangeGridColor();
    afx_msg void OnChangeWindFieldColor();
    afx_msg void OnChangeCirclesColor();

    /** Called when the user wants to change the color-scale */
    afx_msg void OnChangeColorScale_BW();
    afx_msg void OnChangeColorScale_Default();

    /** Turns on or off the showing of the scale */
    afx_msg void OnChangeShowScale();

    /** Increases the number of lines in the grid */
    afx_msg void OnGridIncrease();

    /** Decreases the number of lines in the grid */
    afx_msg void OnGridDecrease();

    /** Lets the user save the column-graph as an image-file */
    afx_msg void OnFileSaveGraphAsImage();

    /** Adjusts the width of the list of evaluation-logs */
    void SetWidthOfLogList();

    CStatic m_frameLandmarks;

    /** A grid of landmark coordinates */
    CMeasGrid m_gridLandMarks;

    afx_msg void OnBnClickedSourceLat();
    afx_msg void OnBnClickedSourceLon();
    afx_msg void OnMenuViewShowColumnByColor();
    afx_msg void OnMenuViewShowColumnBySize();
    afx_msg void OnUpdateMenuShowColumnByColor(CCmdUI* pCmdUI);
    afx_msg void OnUpdateMenuShowColumnBySize(CCmdUI* pCmdUI);

    /** Initializes the menu somehow, necessary otherwise it is not possible to
            update the status of the menu due to a bug in Microsoft MFC, see:
            http://support.microsoft.com/kb/242577 */
    void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROUTEDLG_H__DB80A786_6CCE_4B5C_AF7F_2AE96E26BA18__INCLUDED_)

