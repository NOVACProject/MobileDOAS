// CommentDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DMSpec.h"
#include "CommentDlg.h"

using namespace Dialogs;

// CCommentDlg dialog

IMPLEMENT_DYNAMIC(CCommentDlg, CDialog)
CCommentDlg::CCommentDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCommentDlg::IDD, pParent)
{
}

CCommentDlg::~CCommentDlg()
{
}

void CCommentDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_COMMENT_EDIT, m_commentEdit);
}


BEGIN_MESSAGE_MAP(CCommentDlg, CDialog)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CCommentDlg message handlers

BOOL CCommentDlg::Create(UINT nID, CWnd* pParentWnd)
{
  // TODO: Add your specialized code here and/or call the base class

  return CDialog::Create(nID, pParentWnd);
}

BOOL CCommentDlg::OnInitDialog()
{
  CDialog::OnInitDialog();


  CString tmpStr;
  int hr = t/10000;
  int mi = (t - hr*10000)/100;
	int se = t % 100;
  tmpStr.Format("Lat: %lf\tLong:%lf\tAlt:%lf\tTime: %d:%d%d", lat, lon, alt, hr, mi, se);
  this->SetDlgItemText(IDC_GPS_LABEL, tmpStr);

  this->m_commentEdit.SetFocus();

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void CCommentDlg::OnBnClickedOk(){
  int nLines;
  char txt[512];
  Common common;
  
  FILE *f = fopen(outputDir + "\\Comments.txt", "a");
  if(0 == f){
    MessageBox("could not create/access comments.txt, nothing saved.");
    return;
  }
  nLines = m_commentEdit.GetLineCount();
  if(nLines == 0){
    fclose(f);
    return;
  }
  int hr = t/10000;
  int mi = (t - hr*10000)/100;
	int se = t % 100;
  int length;
  fprintf(f, "-------- Comment added on: %02d:%02d:%02d  at lat: %lf\tlong: %lf\talt: %lf\n", hr, mi, se, lat, lon, alt);
  for(int i = 0; i < nLines; ++i){
    length = m_commentEdit.GetLine(i, txt, 512);
    txt[length] = 0;
    fprintf(f, "%s\n", txt);
  }
  fprintf(f, "\n");

  fclose(f);
  OnOK();
}
