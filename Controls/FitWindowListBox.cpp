
#include "stdafx.h"
#include "../DMSpec.h"
#include "../Dialogs/QueryStringDialog.h"
#include "FitWindowListBox.h"

using namespace DlgControls;

// CFitWindowListBox

IMPLEMENT_DYNAMIC(CFitWindowListBox, CListBox)
CFitWindowListBox::CFitWindowListBox()
{
	m_conf = nullptr;
}

CFitWindowListBox::~CFitWindowListBox()
{
	m_conf = nullptr;
}


BEGIN_MESSAGE_MAP(CFitWindowListBox, CListBox)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID__INSERTFITWINDOW, OnInsertFitWindow)
	ON_COMMAND(ID__REMOVEWINDOW, OnRemoveFitWindow)
	ON_COMMAND(ID__RENAMEWINDOW, OnRenameWindow)
END_MESSAGE_MAP()



// CFitWindowListBox message handlers

/** Called to populate the list */
void CFitWindowListBox::PopulateList(){
	if(m_conf == nullptr)
		return;

	this->ResetContent(); // clear the list

	for(int i = 0; i < m_conf->m_nFitWindows; ++i){
		this->AddString(m_conf->m_fitWindow[i].name);
	}
}

/** Called when the user presses down the left mouse button */
void CFitWindowListBox::OnLButtonDown(UINT nFlags, CPoint point){
	CListBox::OnLButtonDown(nFlags, point);
}

/** Called to show the context menu */
void CFitWindowListBox::OnContextMenu(CWnd *pWnd, CPoint pos){
	OnLButtonDown(MK_LBUTTON, pos); // make the current menu item marked

	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_FITWINDOWLIST_MENU));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != nullptr);

	// There has to be at least one fit window defined at all times
	//	if there are too few, don't allow the user to remove any
	if(m_conf->m_nFitWindows < 1){
		pPopup->EnableMenuItem(ID__REMOVEWINDOW, MF_DISABLED | MF_GRAYED);
	}

	// If the list of fit windows is full, don't let the user
	//	add any more fit windows
	if(m_conf->m_nFitWindows == MAX_FIT_WINDOWS){
		pPopup->EnableMenuItem(ID__INSERTFITWINDOW, MF_DISABLED | MF_GRAYED);
	}

	// show the popup menu
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this);

}

/** Called to insert a fit window into the list */
void CFitWindowListBox::OnInsertFitWindow() {
	CString name;

	// Make sure the list box is initialized ok.
	if (m_conf == nullptr)
		return;

	// Make sure that there's enough space to store one more window 
	int numFitWindows = m_conf->m_nFitWindows;
	if (numFitWindows == MAX_FIT_WINDOWS)
		return;

	// Ask the user for the name of the window
	Dialogs::CQueryStringDialog nameDialog;
	nameDialog.m_windowText.Format("The name of the fit window?");
	nameDialog.m_inputString = &name;
	INT_PTR ret = nameDialog.DoModal();

	if (IDCANCEL == ret)
		return;

	// insert an empty fit window.
	m_conf->m_fitWindow[numFitWindows].Clear();
	if (numFitWindows == 1) {
		int existChannel = m_conf->m_fitWindow[numFitWindows - 1].channel;
		if (existChannel == 0) {
			m_conf->m_fitWindow[numFitWindows].channel = 1;
		}
		else {
			m_conf->m_fitWindow[numFitWindows].channel = 0;
		}
	}
	if (m_conf->m_fitWindow[numFitWindows].channel == 0) {
		m_conf->m_fitWindow[numFitWindows].name.Format("%s (Master)", (LPCTSTR)name);
	}
	else {
		m_conf->m_fitWindow[numFitWindows].name.Format("%s (Slave)", (LPCTSTR)name);
	}
	m_conf->m_nFitWindows += 1;

	PopulateList();
	
	// Select the fit window
	SetCurSel(m_conf->m_nFitWindows - 1);
	
	CWnd *pWnd = GetParent();
	if (pWnd) {
		pWnd->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), LBN_SELCHANGE), (LPARAM)m_hWnd);
	}
}

/** Called to rename a fit window */
void CFitWindowListBox::OnRenameWindow(){
	CString name;

	if(m_conf == nullptr)
		return;

	int curSel = GetCurSel();
	if(curSel < 0 || curSel > MAX_FIT_WINDOWS)
		return;

	// Let the initial guess for the name be the old name of the window
	name.Format(m_conf->m_fitWindow[curSel].name);

	// Ask the user for the name of the window
	Dialogs::CQueryStringDialog nameDialog;
	nameDialog.m_windowText.Format("The new name of the fit window?");
	nameDialog.m_inputString = &name;
	INT_PTR ret = nameDialog.DoModal();

	if(IDCANCEL == ret)
		return;

	// Change the name (must be different from the other fit window name though)
	int otherCur = (curSel == 0);
	if (strcmp(name, m_conf->m_fitWindow[otherCur].name) == 0) {
		MessageBox("Fit window name is already in use.  Please choose another name.", "Rename fit window");
		OnRenameWindow();
		return;
	}
	m_conf->m_fitWindow[curSel].name.Format("%s", (LPCTSTR)name);

	// Update hte list
	PopulateList();
}

/** Called to remove a fit window from the list */
void CFitWindowListBox::OnRemoveFitWindow(){
	if(nullptr == m_conf)
		return;

	// make sure that there's always at least one window defined
	if (m_conf->m_nFitWindows <= 1) {
		MessageBox("At least one fit window must be defined.", "Remove fit window");
		return;
	}

	int curSel = this->GetCurSel();
	if (curSel < 0 || curSel > MAX_FIT_WINDOWS) {
		return;
	}

	// Are you sure?
	int ret = MessageBox("Are you sure you want to remove this fit window?", "Remove fit window", MB_YESNO);
	if (IDNO == ret) {
		return;
	}

	// Remove the window

	// Shift down all the other windows.
	if (curSel == 0 && m_conf->m_nFitWindows > 1) {
		m_conf->m_fitWindow[0] = m_conf->m_fitWindow[1];
	}
	m_conf->m_nFitWindows -= 1;

	// Update the window and the list
	PopulateList();

	CWnd *pWnd = GetParent();
	if (pWnd) {
		pWnd->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), LBN_SELCHANGE), (LPARAM)m_hWnd);
	}
}
