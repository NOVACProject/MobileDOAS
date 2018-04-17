#pragma once
#include "afxwin.h"

#include "../MeasurementModes/Measurement_Calibrate.h"
#include "../Graphs/GraphCtrl.h"

// CSpectrumCalibrationDlg dialog
namespace Dialogs{

	class CSpectrumCalibrationDlg : public CDialog
	{
		DECLARE_DYNAMIC(CSpectrumCalibrationDlg)

	public:
		CSpectrumCalibrationDlg(CWnd* pParent = nullptr);   // standard constructor
		virtual ~CSpectrumCalibrationDlg();

	// Dialog Data
		enum { IDD = IDD_SPECTRUM_CALIBRATION };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	public:
		CMeasurement_Calibrate *m_Spectrometer;

		virtual BOOL OnInitDialog();

		afx_msg void OnUseCurrentSpectrum();
		afx_msg void OnBnClickedFitPolynomial();
		void SortLines();
		void PopulateLineList();
		CStatic m_hglistFrame;
		CStatic m_graphFrame;

		double m_pixels[256];
		double m_wavelengths[256];
		double m_intensities[256];
		
		unsigned int m_pointNum;

		/** The list of the Hg-Lines */
		CListCtrl m_lineList;
		
		Graph::CGraphCtrl m_graph;
	};
}