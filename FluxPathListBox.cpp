#include "stdafx.h"
#include "fluxpathlistbox.h"
#include "DMSpec.h"

CFluxPathListBox::CFluxPathListBox(void)
{
  m_parentWindow = NULL;
}

CFluxPathListBox::~CFluxPathListBox(void)
{
}

BEGIN_MESSAGE_MAP(CFluxPathListBox, CListBox)
  ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


/* Show the context menu */
void CFluxPathListBox::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if(m_parentWindow == NULL)
		return;

  CMenu menu;
  VERIFY(menu.LoadMenu(IDR_FLUX_PATH_MENU));
  CMenu* pPopup = menu.GetSubMenu(0);
  ASSERT(pPopup != NULL);

  int selected = this->GetCurSel();
  if(selected == LB_ERR){
  /* no selection made */
  pPopup->EnableMenuItem(ID_RELOAD_LOGFILE, MF_DISABLED | MF_GRAYED);
  }else{
  pPopup->EnableMenuItem(ID_RELOAD_LOGFILE, MF_ENABLED);
  }

  pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, m_parentWindow);
}
void CFluxPathListBox::UpdateWidth(){

	// Find the longest string in the list box.
	CString      str;
	CSize      sz;
	int      dx = 0;
	TEXTMETRIC   tm;
	CDC*      pDC = this->GetDC();
	CFont*      pFont = this->GetFont();

	// Select the listbox font, save the old font
	CFont* pOldFont = pDC->SelectObject(pFont);
	// Get the text metrics for avg char width
	pDC->GetTextMetrics(&tm); 

	for (int i = 0; i < this->GetCount(); i++)
	{
				this->GetText(i, str);
		sz = pDC->GetTextExtent(str);

		// Add the avg width to prevent clipping
		sz.cx += tm.tmAveCharWidth;

		if (sz.cx > dx)
							dx = sz.cx;
	}
	// Select the old font back into the DC
	pDC->SelectObject(pOldFont);
	this->ReleaseDC(pDC);

	// Set the horizontal extent so every character of all strings 
	// can be scrolled to.
	this->SetHorizontalExtent(dx);
}
