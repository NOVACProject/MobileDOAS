#pragma once
#include "afxwin.h"
#include "../Graphs/GraphCtrl.h"
#include "WindSpeedCalculator.h"
#include "DualBeamMeasSettings.h"
#include "../Flux1.h"

// CPostWindDlg dialog

class CPostWindDlg : public CDialog
{
	DECLARE_DYNAMIC(CPostWindDlg)

public:
	static const int MAX_N_SERIES = 2;

	CPostWindDlg(CWnd* pParent = NULL);   // standard constructor
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
	Graph::CGraphCtrl	m_columnGraph;

	/** The frame around the result graph */
	CStatic m_frameResult;

	/** The result graph */
	Graph::CGraphCtrl	m_resultGraph;

	// ---------------------- EVENT HANDLERS ----------------------
	/** Called when the user browses for a new evaluation-log */
	afx_msg void OnBrowseEvallog();

	/** The user has changed the file in the Eval-log edit-box*/
	afx_msg void OnChangeEvalLog();

	/** Intitializes the dialog */
	virtual BOOL OnInitDialog();

	/** Called when the user changes number of Low pass iterations to perform */
	afx_msg void OnChangeLPIterations();

	/** Called when the user changes the plume-height settings in the dialog */
	afx_msg void OnChangePlumeHeight();

	/** Called when the user presses the 'Calculate wind speed' - button. 
			Here lies the actual work of the dialog. */
	afx_msg void OnCalculateWindspeed();

	// --------------------- PUBLIC METHODS -----------------------
	/** Reads the evaluation log */
	bool ReadEvaluationLog();

	/** Draws the column graph */
	void DrawColumn();

	/** Draws the result on the screen */
	void DrawResult();

	/** Saves the result to a log-file */
	void SaveResult();

	/** Performes a low pass filtering procedure on series number 'seriesNo'.
			The number of iterations is taken from 'm_settings.lowPassFilterAverage'
			The treated series is m_OriginalSeries[seriesNo]
			The result is saved as m_PreparedSeries[seriesNo]	*/
	int	LowPassFilter(int seriesNo);

	// ---------------------- PUBLIC DATA -------------------------

	/** The currently opened evaluation - log */
	CString	m_evalLog;

	/** A flux-object, used to read the evaluation data */
	Flux::CFlux	*m_flux;

	/** Original measurement series, as they are in the file */
	DualBeamMeasurement::CDualBeamCalculator::CMeasurementSeries		*m_OriginalSeries[MAX_N_SERIES];

	/** Treated measurement series, low pass filtered etc. */
	DualBeamMeasurement::CDualBeamCalculator::CMeasurementSeries		*m_PreparedSeries[MAX_N_SERIES];

	/** The settings for how the windspeed calculations should be done */
	DualBeamMeasurement::CDualBeamMeasSettings	m_settings;

	/** The wind speed measurement-calculator. */
	DualBeamMeasurement::CWindSpeedCalculator		m_calc;

	/** Choosing what to show in the dialog */
	int m_showOption;

protected:

	// ------------------ PROTECTED DATA ---------------------
	/** The arrays for the data shown in the result - graph */
	double *corr, *delay, *ws;

	/** The colors for the time-series */
	COLORREF	m_colorSeries[MAX_N_SERIES];

};
