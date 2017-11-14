#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "../Graphs/SpectrumGraph.h"
#include "../Common/Spectrum.h"

namespace Dialogs{
	// CSpectrumInspectionDlg dialog

	class CSpectrumInspectionDlg : public CDialog
	{
		DECLARE_DYNAMIC(CSpectrumInspectionDlg)

	public:
		CSpectrumInspectionDlg(CWnd* pParent = NULL);   // standard constructor
		virtual ~CSpectrumInspectionDlg();

	// Dialog Data
		enum { IDD = IDD_INSPECT_SPECTRA_DLG };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	public:
		/** Initializes the controls and the dialog */
		virtual BOOL  OnInitDialog();
	
		/** Initializes the list of spectrum properties */
		void InitPropertiesList();
	
		/** The spectrum plot */
		Graph::CSpectrumGraph   m_spectrumGraph;

		/** This is the border for the spectrum graph */
		CStatic m_graphFrame;
		
		/** The scroll bar. Lets the user select which spectrum to see */
		CScrollBar		m_spectrumScrollBar;
		
		/** Called when the user browses an evaluation log */
		afx_msg void OnBrowseEvallog();
		
		/** Called when the user wants to see another spectrum (by pressing the slider bar) */
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

		/** Zooming in the graph */
		LRESULT OnZoomGraph(WPARAM wParam, LPARAM lParam);

		/** Called on timer-events */
		afx_msg void OnTimer(UINT_PTR nIDEvent);

		/** Gets the range of the plot, this is to enable zooming of the graph */
		void GetPlotRange(Graph::CSpectrumGraph::plotRange &range, const CSpectrum *spectrum = NULL);

		/** The directory where the spectra are stored (and where the evaluation-log file is stored) */
		CString m_spectrumPath;
		
		/** The evaluation log file */
		CString m_evaluationLog;
		
		/** The currently show spectrum (ranging from -2 to n) 
			-3 is the offset-spectrum (if any)			
			-2 is the dark-spectrum (or dark-current spectrum)
			-1 is the sky-spectrum
			0  is the first measured spectrum (00000_0.STD)		
		*/
		int m_selectedSpectrum;
		
		/** The total amount of spectra in the currently selected measurement */
		int m_totalSpecNum;
		
		/** Draws the currently selected spectrum */
		void DrawSpectrum();
		
		/** Fills in the property list with information about the current spectrum 
			if spectrum is NULL then the list is just cleared */
		void FillInSpectrumPropertiesList(const CSpectrum *spec);
		
		/** This is the x-axis */
		double *m_number;
		
		/** the color of the spectrum */
		COLORREF m_spectrumColor;
		
		/** the frame for the properties list */
		CStatic m_propertiesFrame;
		
		/** The list of the properties of the currently shown spectrum */
		CListCtrl m_propertyList;
		
		/** Reads the given evaluation log file.
			@return 0 on success. */
		int ReadEvaluationLog(const CString &fullFileName);

		/** The timer, to restore the zoomability of the graph 0.5 seconds after
				the user has opened the file */
		UINT_PTR		m_timer;
	};
}