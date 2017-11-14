#pragma once

#include "Graphs/GraphCtrl.h"
#include "Common.h"
#include "Spectrometer.h"

// CShowFitDlg dialog

namespace Dialogs
{

class CShowFitDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowFitDlg)

public:
	CShowFitDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CShowFitDlg();

// Dialog Data
	enum { IDD = IDD_VIEW_FIT_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	double pixel[MAX_SPECTRUM_LENGTH];

  /* the plot */
	Graph::CGraphCtrl   m_fitPlot;
	Graph::CGraphCtrl   m_fitPlot2;

public:

  /** Called when the dialog is opened */
  virtual BOOL OnInitDialog();

  /** True if the plot is shown */
  bool          fVisible;

  /** A pointer to the spectrometer in question. */
  CSpectrometer *m_spectrometer;


  afx_msg void OnSize(UINT nType, int cx, int cy);

  virtual BOOL Create(UINT nID, CWnd* pParentWnd = NULL);
  afx_msg void OnClose();

  /** The actual drawing of the fit */
  void    DrawFit();

protected:

	void    DrawFit1();
	void    DrawFit2();

};

}