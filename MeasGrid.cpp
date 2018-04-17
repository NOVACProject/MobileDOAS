#include "stdafx.h"
#include "resource.h"
#include "measgrid.h"

CMeasGrid::CMeasGrid(void)
{
	parent = nullptr;
}

CMeasGrid::~CMeasGrid(void)
{
	parent = nullptr;
}

BEGIN_MESSAGE_MAP(CMeasGrid, CGridCtrl)
  ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void CMeasGrid::OnContextMenu(CWnd* pWnd, CPoint point){
   CMenu menu;
/*   VERIFY(menu.LoadMenu(IDR_MENU1));
   CMenu* pPopup = menu.GetSubMenu(0);
   ASSERT(pPopup != nullptr);

  CCellRange cellRange = GetSelectedCellRange();
  int minRow = cellRange.GetMinRow() - 1;
  int nRows = cellRange.GetRowSpan();

  if(nRows <= 0){ // nothing selected
    pPopup->EnableMenuItem(ID__INSERTROW, MF_DISABLED | MF_GRAYED);
    pPopup->EnableMenuItem(ID__DELETEROW, MF_DISABLED | MF_GRAYED);
  }

  if(GetColumnCount() != cellRange.GetColSpan()+1){ // not a full row selected 
    pPopup->EnableMenuItem(ID__INSERTROW, MF_DISABLED | MF_GRAYED);
    pPopup->EnableMenuItem(ID__DELETEROW, MF_DISABLED | MF_GRAYED);
  }

   pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, parent);
*/
}

/* Called when the user has edited one cell */
void CMeasGrid::OnEndEditCell(int nRow, int nCol, CString str){
  CGridCtrl::OnEndEditCell(nRow, nCol, str);  

	// tell the parent that we've done editing one box
	if(parent != nullptr)
		parent->PostMessage(WM_END_EDIT, 0, 0);

  return;
}

