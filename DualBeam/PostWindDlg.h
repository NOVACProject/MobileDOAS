#pragma once
#include "afxwin.h"
#include "../Graphs/GraphCtrl.h"
#include <MobileDoasLib/DualBeam/WindSpeedCalculator.h>
#include <MobileDoasLib/DualBeam/DualBeamMeasSettings.h>

// CPostWindDlg dialog

class CPostWindDlg : public CDialog
{
    DECLARE_DYNAMIC(CPostWindDlg)

public:
    /** The number of time series which we can handle, normally two. */
    static const int MAX_N_SERIES = 2;

    /* The maximum length of 'corr', 'delay' and 'ws'. */
    static const int MAX_CORRELATION_NUM = 4096;

    CPostWindDlg(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CPostWindDlg();

    // Dialog Data
    enum { IDD = IDD_POST_WINDDLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:

    // ------------------ DIALOG COMPONENTS -----------------------
    /** The frame around the column graph */
    CStatic m_frameColumn;

    /** The column graph */
    Graph::CGraphCtrl m_columnGraph;

    /** The frame around the result graph */
    CStatic m_frameResult;

    /** The result graph */
    Graph::CGraphCtrl m_resultGraph;

    // ---------------------- EVENT HANDLERS ----------------------
    /** Called when the user browses for a new evaluation-log for Master */
    afx_msg void OnBrowseEvallogSeries1();

    /** Called when the user browses for a new evaluation-log for Slave*/
    afx_msg void OnBrowseEvallogSeries2();

    /** Intitializes the dialog */
    virtual BOOL OnInitDialog();

    /** Called when the user changes number of Low pass iterations to perform */
    afx_msg void OnChangeLPIterations();

    /** Called when the user changes the plume-height settings in the dialog */
    afx_msg void OnChangePlumeHeight();

    /** Called when the user presses the 'Calculate wind speed' - button.
            Here lies the actual work of the dialog. */
    afx_msg void OnCalculateWindspeed();

private:

    // ---------------------- PRIVATE DATA -------------------------

    /** The currently opened evaluation-log(s), one for each measurement series. */
    CString m_evalLog[MAX_N_SERIES];

    /** Original measurement series, as they are in the file */
    mobiledoas::CDualBeamCalculator::CMeasurementSeries* m_OriginalSeries[MAX_N_SERIES];

    /** Treated measurement series, low pass filtered etc. */
    mobiledoas::CDualBeamCalculator::CMeasurementSeries* m_PreparedSeries[MAX_N_SERIES];

    /** The settings for how the windspeed calculations should be done */
    mobiledoas::CDualBeamMeasSettings m_settings;

    /** The wind speed measurement-calculator. */
    mobiledoas::CWindSpeedCalculator m_calc;

    /** Choosing what to show in the dialog */
    int m_showOption;

    /** The arrays for the data shown in the result - graph */
    std::vector<double> correlations;
    std::vector<double> timeDelay;
    std::vector<double> windspeeds;

    /** The colors for the time-series */
    COLORREF m_colorSeries[MAX_N_SERIES];

    // --------------------- PRIVATE METHODS -----------------------

    /** Reads one of the evaluation logs.
        @param channelIndex set to zero for the master or 1 for the slave channel.
        @return SUCCESS if all went well. */
    bool ReadEvaluationLog(int channelIndex);

    /** Draws the column graph */
    void DrawColumn();

    /** Draws the result on the screen */
    void DrawResult();

    /** Saves the result to a log-file */
    void SaveResult();

    /** Performes a low pass filtering procedure on series number 'seriesNo'.
        The number of iterations is taken from 'm_settings.lowPassFilterAverage'
        The treated series is m_OriginalSeries[seriesNo]
        The result is saved as m_PreparedSeries[seriesNo]
        @return 1 on success. */
    int LowPassFilter(int seriesNo);
};
