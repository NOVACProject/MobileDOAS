#pragma once
#include "GraphCtrl.h"

namespace Graph{
	class CColumnGraph :
		public CGraphCtrl
	{
	public:
		CColumnGraph(void);
		~CColumnGraph(void);

		CWnd *m_parentWindow;

		DECLARE_MESSAGE_MAP()
		afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	};
}