#pragma once
#include "afxwin.h"

#include "MeasGrid.h"
#include "Common.h"

namespace Dialogs
{
#define MAX_SITES 1024

// CSourceSelectionDlg dialog

class CSourceSelectionDlg : public CDialog
{
	DECLARE_DYNAMIC(CSourceSelectionDlg)

public:
	CSourceSelectionDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSourceSelectionDlg();

// Dialog Data
	enum { IDD = IDD_SOURCE_SELECTION_DLG };
private:
  int   ReadSourceLog();

  CString   m_siteName[MAX_SITES];
  double    m_lat[MAX_SITES];
  double    m_lon[MAX_SITES];
  double    m_alt[MAX_SITES];
  long      m_nSites;

public:
  CString   m_selectedSite;
  double    m_selectedLat;
  double    m_selectedLon;
  double    m_selectedAlt;
    
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
  virtual BOOL OnInitDialog();
  CMeasGrid m_grid;
  CStatic m_gridBorder;
  afx_msg void OnBnClickedOk();
};

}