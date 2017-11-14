/* The following two definitions are necesarry since
	VC6 (and VC7) does not handle the long long data-type 
	used by the OceanOptics OmniDriver
*/
#define HIGHRESTIMESTAMP_H
//#define SPECTROMETERCHANNEL_H

#include "stdafx.h"
#include "columngraph.h"
#include "../DMSpec.h"

using namespace Graph;

CColumnGraph::CColumnGraph(void)
{
  m_parentWindow = NULL;
}

CColumnGraph::~CColumnGraph(void)
{
}

BEGIN_MESSAGE_MAP(CColumnGraph, CGraphCtrl)
  ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

/* Show the context menu */
void CColumnGraph::OnContextMenu(CWnd* pWnd, CPoint point)
{
   CMenu menu;
   VERIFY(menu.LoadMenu(IDR_FLUX_VIEW_MENU));
   CMenu* pPopup = menu.GetSubMenu(0);
   ASSERT(pPopup != NULL);

   pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, m_parentWindow);
}
