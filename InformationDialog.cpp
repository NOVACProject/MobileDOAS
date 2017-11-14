// InformationDialog.cpp : implementation file
//

#include "stdafx.h"
#include "DMSpec.h"
#include "InformationDialog.h"

using namespace Dialogs;
// CInformationDialog dialog

IMPLEMENT_DYNAMIC(CInformationDialog, CDialog)
CInformationDialog::CInformationDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CInformationDialog::IDD, pParent)
  , informationString(_T(""))
{
}

CInformationDialog::~CInformationDialog()
{
}

void CInformationDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

  DDX_Text(pDX, IDC_INFORMATION, informationString);
}


BEGIN_MESSAGE_MAP(CInformationDialog, CDialog)
END_MESSAGE_MAP()


// CInformationDialog message handlers
