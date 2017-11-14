#pragma once
#include "afxwin.h"

#include "Common.h"

namespace Dialogs{
	// CCommentDlg dialog

	class CCommentDlg : public CDialog
	{
		DECLARE_DYNAMIC(CCommentDlg)

	public:
		CCommentDlg(CWnd* pParent = NULL);   // standard constructor
		virtual ~CCommentDlg();

	// Dialog Data
		enum { IDD = IDD_COMMENT_DLG };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	public:
		double  lat, lon, alt;
		long    t; // current time;
		CEdit   m_commentEdit;
		CString outputDir;

		virtual BOOL Create(UINT nID, CWnd* pParentWnd = NULL);
		virtual BOOL OnInitDialog();
		afx_msg void OnBnClickedOk();
	};
}