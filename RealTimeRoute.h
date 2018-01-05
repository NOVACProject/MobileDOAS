#pragma once
#ifndef _REALTIMEROUTE_
#define _REALTIMEROUTE_

#include "Graphs/GraphCtrl.h"
#include "Common.h"
#include "afxwin.h"
#include "Spectrometer.h"

// CRealTimeRoute dialog

namespace Dialogs
{
	class CRealTimeRoute : public CDialog {
		DECLARE_DYNAMIC(CRealTimeRoute)

	public:
		CRealTimeRoute(CWnd* pParent = NULL);   // standard constructor
		virtual ~CRealTimeRoute();

		// Dialog Data
		enum { IDD = IDD_REALTIME_ROUTE_DLG };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	  /* the plot */
		Graph::CGraphCtrl   m_gpsPlot;

		DECLARE_MESSAGE_MAP()
	public:
		virtual BOOL OnInitDialog();

		bool    fVisible;

		CSpectrometer *m_spectrometer;

		long    m_pointNum;
		double  m_lon[65536];
		double  m_lat[65536];
		double  m_col[65536];
		double  m_colmax;
		double  m_colmin;
		double  m_int[65536];

		struct plotRange m_range;
		double m_srcLat, m_srcLon;

		double m_intensityLimit; // points with intensity below 'm_intensityLimit' will not be shown

		/* Plotting the data */
		void   DrawRouteGraph();

		void   ReadData();

		afx_msg void OnSize(UINT nType, int cx, int cy);

		virtual BOOL Create(UINT nID, CWnd* pParentWnd = NULL);
		afx_msg void OnClose();
	};

}
#endif
