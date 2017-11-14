#pragma once
#include "afxwin.h"
#include "../Graphs/GraphCtrl.h"
#include "PlumeHeightCalculator.h"
#include "../Flux1.h"

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
	/** Called when the user browses for a new evaluation-log */
	afx_msg void OnBrowseEvallog();

	/** The user has changed the file in the Eval-log edit-box*/
	afx_msg void OnChangeEvalLog();

	/** Intitializes the dialog */
	virtual BOOL OnInitDialog();

	// --------------------- PUBLIC METHODS -----------------------
	/** Reads the evaluation log */
	bool ReadEvaluationLog();

	/** Draws the column graph */
	void DrawColumn();

	/** Performes a low pass filtering procedure on series number 'seriesNo'.
			The number of iterations is taken from 'm_settings.lowPassFilterAverage'
			The treated series is m_OriginalSeries[seriesNo]
			The result is saved as m_PreparedSeries[seriesNo]	*/
	int	LowPassFilter(int seriesNo);

	/** Called when the user presses the 'Calculate plumeHeight' - button. 
			Here lies the actual work of the dialog. */
	afx_msg void OnCalculatePlumeHeight();

	// ---------------------- PUBLIC DATA -------------------------

	/** The currently opened evaluation - log */
	CString	m_evalLog;

	/** A flux-object, used to read the evaluation data */
	Flux::CFlux	*m_flux;

	/** Original measurement series, as they are in the file */
	DualBeamMeasurement::CDualBeamCalculator::CMeasurementSeries		*m_OriginalSeries[MAX_N_SERIES];

	/** Treated measurement series, low pass filtered etc. */
	DualBeamMeasurement::CDualBeamCalculator::CMeasurementSeries		*m_PreparedSeries[MAX_N_SERIES];

	/** The settings for how the plumeheight calculations should be done */
	DualBeamMeasurement::CDualBeamMeasSettings	m_settings;

	/** The plume height measurement-calculator. */
	DualBeamMeasurement::CPlumeHeightCalculator		m_calc;

	/** The source that we measure on */
	double	m_sourceLat, m_sourceLon;

	/** The number of channels in the last read evaluation log */
	int 		m_nChannels;

protected:

	// ------------------ PROTECTED DATA ---------------------
	/** The colors for the time-series */
	COLORREF	m_colorSeries[MAX_N_SERIES];

	/** Used to separate when the user changes an interface item or when 
			the program does */
	bool		m_automatic;

	// ---------------- PROTECTED METHODS ---------------------

	/** Corrects the time-series m_OriginalSeries for some common problems */
	void		CorrectTimeSeries();
public:
	afx_msg void OnBnClickedButtonSourceLat();
	afx_msg void OnBnClickedButtonSourceLon();
};
