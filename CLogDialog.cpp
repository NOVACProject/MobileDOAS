// CLogDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CLogDialog.h"
#include "afxdialogex.h"
#include "resource.h"


// CLogDialog dialog

IMPLEMENT_DYNAMIC(CLogDialog, CDialog)

CLogDialog::CLogDialog(novac::ILogger& log, CWnd* pParent /*=nullptr*/)
    : m_log(log), CDialog(IDD_VIEW_LOG_DIALOG, pParent)
{
}

CLogDialog::~CLogDialog()
{
}

BOOL CLogDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO: How to handle this ??
    /*    for (const std::string& msg : this->m_logEntries)
        {
            m_listBox.AddString(msg.c_str());
        }
    */

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CLogDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_MESSAGE_LIST, m_listBox);
}


BEGIN_MESSAGE_MAP(CLogDialog, CDialog)
END_MESSAGE_MAP()


// CLogDialog message handlers
