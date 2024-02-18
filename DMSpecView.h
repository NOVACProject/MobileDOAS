// DMSpecView.h : interface of the CDMSpecView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DMSPECVIEW_H__8A3F075C_24B5_4BB0_897A_E1285CBD8628__INCLUDED_)
#define AFX_DMSPECVIEW_H__8A3F075C_24B5_4BB0_897A_E1285CBD8628__INCLUDED_

#include <MobileDoasLib/GPS.h>
#include "Spectrometer.h"
#include "Graphs/GraphCtrl.h"
#include "PostFluxDlg.h"

#include "RealTimeRoute.h"
#include "ShowFitDlg.h"
#include "Dialogs/SpectrumSettingsDlg.h"
#include "Dialogs/SpectrumScaleDlg.h"

#include "Common.h"
#include "Controls/Label.h"

#include "afxcmn.h"
#include "afxwin.h"

#pragma once

UINT CollectSpectra(LPVOID pParam);

class CDMSpecView : public CFormView
{
protected: // create from serialization only
    CDMSpecView();
    virtual ~CDMSpecView();

    DECLARE_DYNCREATE(CDMSpecView)

public:

    // ------------------- DIALOG COMPONENTS -------------------------

    /** The edit-box for the basename */
    CEdit m_BaseEdit;

    /** The background color for the main-plot */
    COLORREF m_bkColor;

    /** The colors for drawing the results from the master and slave channels */
    COLORREF m_PlotColor[2];

    /** The color of the master-channel spectrum */
    COLORREF m_Spectrum0Color;

    /** The color of the fit-region of the master-channel spectrum */
    COLORREF m_Spectrum0FitColor;

    /** The color of the slave-channel spectrum */
    COLORREF m_Spectrum1Color;

    /** The color of the fit-region of the slave-channel spectrum */
    COLORREF m_Spectrum1FitColor;

    /* Measurement points with intensity below 'intensityLimit' does not affect the column scale */
    CSliderCtrl m_intensitySliderLow;

    /** The actual graph, shows the columns, the spectra, the intensities, etc...*/
    Graph::CGraphCtrl m_ColumnPlot;

    DlgControls::CLabel m_expLabel;
    DlgControls::CLabel m_scanNoLabel;
    DlgControls::CLabel m_colLabel;
    DlgControls::CLabel m_noSpecLabel;
    DlgControls::CLabel m_shiftLabel;
    DlgControls::CLabel m_squeezeLabel;
    DlgControls::CLabel m_tempLabel;

    DlgControls::CLabel m_gpsLatLabel;
    DlgControls::CLabel m_gpsLonLabel;
    DlgControls::CLabel m_gpsTimeLabel;
    DlgControls::CLabel m_gpsNSatLabel;

    DlgControls::CLabel m_colorLabelSpectrum1;
    DlgControls::CLabel m_colorLabelSpectrum2;
    DlgControls::CLabel m_colorLabelSeries1;
    DlgControls::CLabel m_colorLabelSeries2;
    CStatic m_legendSeries1, m_legendSeries2, m_legendSpectrum1, m_legendSpectrum2;



    // ---------------------- PUBLIC DATA ----------------------------

    enum { IDD = IDD_DMSPEC_FORM };

    /** The wind-direction and wind-speed, used in the main window so that the user
            can set the parameters for calculating flux. */
    double m_WindDirection;
    double m_WindSpeed;

    /** The spectrometer. Takes care of spectral collection, evaluation
            and storing of spectra and results. */
    CSpectrometer* m_Spectrometer;

    /** The option whether we shall show the column-error bars or not */
    BOOL m_showErrorBar;

    // --------------------- EVENT HANDLERS ---------------------------

    /** Starts the spectrum collection */
    afx_msg void OnControlStart();

    /** Starts the spectrum collection for wind measurements*/
    afx_msg void OnControlStartWindMeasurement();

    /** Stoppes the spectrum collections */
    afx_msg void OnControlStop();

    /** Starts the viewing of spectra from the spectrometer
        and makes it possible for the user to control the
        exposure-time and number of co-adds */
    afx_msg void OnControlViewSpectra();

    /** Starts the viewing of latest spectra from STD files sent to a directory. */
    afx_msg void OnControlProcessSpectraFromDirectory();

    /** Show the post-flux dialog */
    afx_msg void OnMenuShowPostFluxDialog();

    /** Shows the spectrum inspector */
    afx_msg void OnMenuShowSpectrumInspectionDialog();

    /** Calculate flux in real-time */
    afx_msg void OnControlCountflux();

    /** Called when the user wants to change the background color of the plot */
    afx_msg void OnConfigurationPlotChangebackground();

    /** Called when the user wants to change the plot color of the plot */
    afx_msg void OnConfigurationPlotChangeplotcolor();
    afx_msg void OnConfigurationPlotChangeplotcolor_Slave();

    /** Shows the configuration dialog. Lets the user change
            the operation-configuration */
    afx_msg void OnConfigurationOperation();

    /** ??? */
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

    /** Called to draw a new column value, and to show the number in the
            label on the lower part of the screen */
    afx_msg LRESULT OnDrawColumn(WPARAM wParam, LPARAM lParam);

    /** Updates the status-bar*/
    afx_msg LRESULT OnShowStatus(WPARAM wParam, LPARAM lParam);

    /** Called to update the GPS - reading in the bottom part of the screen */
    afx_msg LRESULT OnReadGPS(WPARAM wParam, LPARAM lParam);

    /** Called to update the integration time shown in the bottom part of the screen */
    afx_msg LRESULT OnShowIntTime(WPARAM wParam, LPARAM lParam);

    /** Called when the selected spectrometer has changed (very seldom!!) */
    afx_msg LRESULT OnChangeSpectrometer(WPARAM wParam, LPARAM lParam);

    /** Draws the last-collected spectrum */
    afx_msg LRESULT OnDrawSpectrum(WPARAM wParam, LPARAM lParam);

    /** Changes the saturation-ratio scale of the spectrum-view */
    afx_msg LRESULT OnChangedSpectrumScale(WPARAM wParam, LPARAM lParam);

    /** Shows a small-popup window above the main window, without anything
            actually stopping. This is used instead of the MessageBox since
            the program will then stop. The parameter wParam tells the kind
            of information to show. */
    afx_msg LRESULT OnShowInformationDialog(WPARAM wParam, LPARAM lParam);

    /** Show the real-time route dialog */
    afx_msg void OnViewRealtimeroute();

    /** Show the real-time spectrum fit window */
    afx_msg void OnViewSpectrumFit();

    afx_msg void OnUpdateViewRealtimeroute(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewSpectrumFit(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewColumnError(CCmdUI* pCmdUI);

    /** When the user wants to add a comment at this place in the traverse */
    afx_msg void OnControlAddComment();

    /** Called when the user wants to show the ReEvaluation dialog */
    afx_msg void OnControlReevaluate();
    afx_msg void OnUpdateControlReevaluate(CCmdUI* pCmdUI);

    /** Called when the user wants to change the exposure time of the
            spectrometer on the fly, while the program is running. */
    afx_msg void OnConfigurationChangeexposuretime();

    /** Called to show the wind-speed measurement analysis window */
    afx_msg void OnMenuAnalysisWindSpeedMeasurement();

    /**  Called to show the plume-height measurement analysis window */
    afx_msg void OnMenuAnalysisPlumeheightmeasurement();

    afx_msg void OnMenuControlTestTheGPS();
    afx_msg void OnMenuControlRunTheGPS();
    //afx_msg void OnUpdate_TestTheGPS(CCmdUI *pCmdUI);

    // Updating the interface
    afx_msg void OnUpdate_EnableOnRun(CCmdUI* pCmdUI);
    afx_msg void OnUpdate_DisableOnRun(CCmdUI* pCmdUI);

    afx_msg void OnUpdateWindMeasurement(CCmdUI* pCmdUI);
    afx_msg void OnUpdate_StartTheGps(CCmdUI* pCmdUI);

    /** Toggles the showing of the column error bars */
    afx_msg void OnViewColumnError();

    afx_msg void OnClose();
    afx_msg void OnDestroy();
    afx_msg void OnAnalysisCalibratespectrometer();

    // --------------------- PUBLIC METHODS ----------------------------
    void DrawSpectrum();

    CDMSpecDoc* GetDocument();


    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    /* Drawing of the spectrum */
    int m_SpectrumLineWidth;

    /** Updates the labels explaining which color belongs to which series */
    void UpdateLegend();

    void ShowStatusMsg(CString& str);

    /** Sound error alarm **/
    void SoundAlarm();

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

    // ------------------- PROTECTED DATA -------------------------

    /** The real-time route window */
    Dialogs::CRealTimeRoute m_realTimeRouteGraph;

    /** The real-time spectrum fit window */
    Dialogs::CShowFitDlg m_showFitDlg;

    Dialogs::CSpectrumSettingsDlg m_specSettingsDlg;

    Dialogs::CSpectrumScaleDlg m_specScaleDlg;

    /** The spectrometer thread */
    CWinThread* pSpecThread = nullptr;

    DECLARE_MESSAGE_MAP()

    /** The X-Axis of the column bar-chart */
    std::vector<double> m_columnChartXAxisValues;

    /** The X-Axis of the spectrum chart */
    std::vector<double> m_spectrumChartXAxisValues;

    /** Used to set the range for the column plot*/
    double m_columnLimit;

    /* m_minSaturationRatio is where the spectrum display y-axis starts at, in percent. Default is 0.0 */
    double m_minSaturationRatio = 0.0;

    /* m_maxSaturationRatio is where the spectrum display y-axis ends at, in percent. Default is 100.0 */
    double m_maxSaturationRatio = 100.0;

    /** Tells us the mode the spectrometer is running in, if it's running */
    SPECTROMETER_MODE m_spectrometerMode;

    // ------------------- PROTECTED METHODS -------------------------

    /** Called to read the settings log-file that is generated by this program. */
    void ReadMobileLog();

    /** Exchanges data between the interface and the variables in this class */
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    /** Called when the window is created */
    virtual void OnInitialUpdate();
};

#ifndef _DEBUG  // debug version in DMSpecView.cpp
inline CDMSpecDoc* CDMSpecView::GetDocument()
{
    return (CDMSpecDoc*)m_pDocument;
}
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DMSPECVIEW_H__8A3F075C_24B5_4BB0_897A_E1285CBD8628__INCLUDED_)
