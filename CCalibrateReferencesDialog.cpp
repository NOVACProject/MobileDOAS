// CCalibrateReferenes.cpp : implementation file
//

#include "StdAfx.h"
#include "CCalibrateReferencesDialog.h"
#include "afxdialogex.h"
#include "resource.h"


// CCalibrateReferenes dialog

IMPLEMENT_DYNAMIC(CCalibrateReferencesDialog, CPropertyPage)

CCalibrateReferencesDialog::CCalibrateReferencesDialog(CWnd* pParent /*=nullptr*/)
	: CPropertyPage(IDD_CALIBRATE_REFERENCES)
{

}

CCalibrateReferencesDialog::~CCalibrateReferencesDialog()
{
}

void CCalibrateReferencesDialog::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCalibrateReferencesDialog, CPropertyPage)
END_MESSAGE_MAP()


// CCalibrateReferenes message handlers
