#pragma once
#include "afxwin.h"
#include "../Graphs/GraphCtrl.h"
#include <MobileDoasLib/DualBeam/PlumeHeightCalculator.h>

// CPostPlumeHeightDlg dialog

class CPostPlumeHeightDlg : public CDialog
{
    DECLARE_DYNAMIC(CPostPlumeHeightDlg)

public:
    static const int MAX_N_SERIES = 2;

    CPostPlumeHeightDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CPostPlumeHeightDlg();

    // Dialog Data
    enum { IDD = IDD_POST_PLUMEHEIGHTDLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    // ------------------ DIALOG COMPONENTS -----------------------
    /** The frame around the column graph */
    CStatic m_frameColumn;

    /** The column graph */
    Graph::CGraphCtrl	m_columnGraph;

    // ---------------------- EVENT HANDLERS ----------------------
    /** Called when the user browses for a new evaluation-log for series 1 */
    afx_msg void OnBrowseEvallogSeries1();

    /** Called when the user browses for a new evaluation-log for series 2 */
    afx_msg void OnBrowseEvallogSeries2();

    /** Intitializes the dialog */
    virtual BOOL OnInitDialog();

    afx_msg void OnBnClickedButtonSourceLat();
    afx_msg void OnBnClickedButtonSourceLon();

    /** Called when the user presses the 'Calculate plumeHeight' - button.
            Here lies the actual work of the dialog. */
    afx_msg void OnCalculatePlumeHeight();


private:

    // ------------------ PRIVATE DATA ---------------------
    /** The colors for the time-series */
    COLORREF	m_colorSeries[MAX_N_SERIES];

    /** Used to separate when the user changes an interface item or when
            the program does */
    bool m_automatic;

    /** The currently opened evaluation - logs, one per series*/
    CString	m_evalLog[MAX_N_SERIES];

    /** Original measurement series, as they are in the file */
    mobiledoas::CDualBeamCalculator::CMeasurementSeries* m_OriginalSeries[MAX_N_SERIES];

    /** Treated measurement series, low pass filtered etc. */
    mobiledoas::CDualBeamCalculator::CMeasurementSeries* m_PreparedSeries[MAX_N_SERIES];

    /** The settings for how the plumeheight calculations should be done */
    mobiledoas::CDualBeamMeasSettings m_settings;

    /** The plume height measurement-calculator. */
    mobiledoas::CPlumeHeightCalculator m_calc;

    /** The source that we measure on */
    double m_sourceLat, m_sourceLon;

    // ---------------- PRIVATE METHODS ---------------------

    /** Corrects one time-series m_OriginalSeries for some common problems */
    void CorrectTimeSeries(int seriesIndex);

    // --------------------- PUBLIC METHODS -----------------------
    /** Reads the evaluation log */
    bool ReadEvaluationLog(int channelIndex);

    /** Draws the column graph */
    void DrawColumn();

    /** Performes a low pass filtering procedure on series number 'seriesNo'.
    The number of iterations is taken from 'm_settings.lowPassFilterAverage'
    The treated series is m_OriginalSeries[seriesNo]
    The result is saved as m_PreparedSeries[seriesNo]	*/
    int	LowPassFilter(int seriesNo);

};
