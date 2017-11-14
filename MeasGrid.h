#pragma once
#include "gridctrl\gridctrl.h"

using namespace DlgControls;

#define WM_END_EDIT		WM_USER + 35

class CMeasGrid : public CGridCtrl
{
	public:
	CMeasGrid(void);
	~CMeasGrid(void);

	CWnd *parent;

	void CMeasGrid::OnEndEditCell(int nRow, int nCol, CString str);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnInsertRow();
};
