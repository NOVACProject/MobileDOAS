#pragma once
#include "afxwin.h"

class CFluxPathListBox :
	public CListBox
{
public:
	CFluxPathListBox(void);
	~CFluxPathListBox(void);

	CWnd *m_parentWindow;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

public:
	void UpdateWidth();
};
