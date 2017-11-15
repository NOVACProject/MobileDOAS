#include "stdafx.h"
#include "../DMSpec.h"
#include "configurationdialog.h"
#include "../Common.h"

using namespace Configuration;

CConfigurationDialog::CConfigurationDialog(void)
{
}

CConfigurationDialog::~CConfigurationDialog(void)
{
}

BEGIN_MESSAGE_MAP(CConfigurationDialog, CPropertySheet)
END_MESSAGE_MAP()

BOOL Configuration::CConfigurationDialog::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	CRect rectAppl, rectCancel;

	// Get the buttons...
	CWnd *pApply = this->GetDlgItem(ID_APPLY_NOW);
	CWnd *pCancel = this->GetDlgItem(IDCANCEL);
	CWnd *pOk = this->GetDlgItem(IDOK);

	// Get the position of the 'Apply'-button, and then remove it
	if(pApply){
		pApply->GetWindowRect(rectAppl);
		ScreenToClient(rectAppl);
		pApply->DestroyWindow();
	}

	// Get the position of the 'Cancel'-button and then
	//	move it to where the 'Apply'-button was
	if(pCancel){
		pCancel->GetWindowRect(rectCancel);
		ScreenToClient(rectCancel);
		pCancel->MoveWindow(rectAppl);
	}

	// Change the 'OK'-button to a 'Save'-button and move it to where
	//	the 'cancel'-button was
	if(pOk){
		pOk->SetWindowText("Save");
		pOk->MoveWindow(rectCancel);
	}

	return bResult;
}

