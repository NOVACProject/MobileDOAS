// SourceSelectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DMSpec.h"
#include "SourceSelectionDlg.h"

using namespace Dialogs;

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

// CSourceSelectionDlg dialog

IMPLEMENT_DYNAMIC(CSourceSelectionDlg, CDialog)
CSourceSelectionDlg::CSourceSelectionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSourceSelectionDlg::IDD, pParent)
{
  m_nSites = 0;
}

CSourceSelectionDlg::~CSourceSelectionDlg()
{
}

void CSourceSelectionDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_GRID_BORDER, m_gridBorder);
}


BEGIN_MESSAGE_MAP(CSourceSelectionDlg, CDialog)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CSourceSelectionDlg message handlers

BOOL CSourceSelectionDlg::OnInitDialog(){
  CDialog::OnInitDialog();

  CRect rect;
  this->m_gridBorder.GetWindowRect(&rect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  rect.top = 20;
  rect.left = 10;
  rect.right = width - 20;
  rect.bottom = height - 10;

  m_grid.Create(rect, &m_gridBorder, 0);
  m_grid.InsertColumn("Site");
  m_grid.InsertColumn("Lat (dd.dddd)");
  m_grid.InsertColumn("Long (dd.dddd)");
  m_grid.InsertColumn("Alt (mm)");
  m_grid.SetFixedRowCount(1);
  m_grid.SetFixedColumnCount(0);
  for(int i = 0; i < 4; ++i)
    m_grid.SetColumnWidth(i, rect.right/4);

  m_grid.SetEditable(TRUE); /* make sure the user can edit the positions */

  ReadSourceLog();

  m_grid.SetRowCount(m_nSites+3);
  for(int i = 0; i < m_nSites; ++i){
    m_grid.SetItemTextFmt(i+1, 0, "%s", m_siteName[i]);
    m_grid.SetItemTextFmt(i+1, 1, "%.5lf", m_lat[i]);
    m_grid.SetItemTextFmt(i+1, 2, "%.5lf", m_lon[i]);
    m_grid.SetItemTextFmt(i+1, 3, "%.0lf", m_alt[i]);
  }

  /* make the selection */
  this->m_selectedAlt = m_alt[0];
  this->m_selectedLat = m_lat[0];
  this->m_selectedLon = m_lon[0];
  this->m_selectedSite.Format(m_siteName[0]);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

int CSourceSelectionDlg::ReadSourceLog(){
  char txt[512];
  char *pt = 0;
  int nAssigned;
  double tmpDouble1, tmpDouble2, tmpDouble3;
  char tmpStr[1204];
	char nl[2]={ 0x0a, 0 };
	char lf[2]={ 0x0d, 0 };

  FILE *f = fopen(g_exePath + "Sources.txt", "r");
  if(f == 0){
    CString tmpStr;
    tmpStr.Format("Could not find file: %s", g_exePath + "Sources.txt");
    MessageBox(tmpStr);
    return 1;
  }
  
	while(fgets(txt,sizeof(txt)-1,f)){
		if(strlen(txt) > 4 && txt[0] != '%'){		  
			pt = txt;
			if(pt = strstr(txt,nl)) 
				pt[0] = 0;
			pt = txt;
			if(pt = strstr(txt,lf)) 
				pt[0] = 0;
			pt = txt;

      // replace spaces
      while(pt = strstr(txt, " ")){
        pt[0] = '|';
      }
      pt = txt;

      nAssigned = sscanf(pt, "%s\t%lf\t%lf\t%lf\n", tmpStr, &tmpDouble1, &tmpDouble2, &tmpDouble3);
      if(3 <= nAssigned){
        m_siteName[m_nSites].Format(tmpStr);
        while(pt = strstr(m_siteName[m_nSites].GetBuffer(), "|")){
          pt[0] = ' ';
        }
        m_lat[m_nSites] = tmpDouble1;
        m_lon[m_nSites] = tmpDouble2;
        if(nAssigned == 4){
          m_alt[m_nSites] = tmpDouble3;
        }else{
          m_alt[m_nSites] = 0;
        }
        ++m_nSites;
      }
    }
  }

  fclose(f);
  return 0;
}

void CSourceSelectionDlg::OnBnClickedOk(){
  int nRows, i;
  CString tmpStr;
  CGridCellBase *cell;

  /* firstly find out what has been selected */
  CCellRange cellRange = m_grid.GetSelectedCellRange();
  int minRowSelect = cellRange.GetMinRow();
  int nRowsSelected = cellRange.GetRowSpan();

  if(nRowsSelected <= 0){ /* nothing selected*/
    this->m_selectedAlt = -1;
    this->m_selectedLat = -1;
    this->m_selectedLon = -1;
    this->m_selectedSite.Format("");
  }else{
    cell = m_grid.GetCell(minRowSelect, 0);
    this->m_selectedSite.Format(cell->GetText());

    cell = m_grid.GetCell(minRowSelect, 1);
    this->m_selectedLat = atof(cell->GetText());

    cell = m_grid.GetCell(minRowSelect, 2);
    this->m_selectedLon = atof(cell->GetText());

    cell = m_grid.GetCell(minRowSelect, 3);
    this->m_selectedAlt = atof(cell->GetText());
  }


  /* save the changes and updates */
  FILE *f = fopen(g_exePath + "Sources.txt", "w");
  if(f == 0){
    MessageBox("Could not open 'Sources.txt' for writing", "Error", MB_OK);
    return;
  }

  nRows = m_grid.GetRowCount();
  for(i = 0; i < nRows-1; ++i){
    cell = m_grid.GetCell(i+1, 0);
    tmpStr.Format("%s\t", cell->GetText());
    if(CString::StringLength(tmpStr) <= 1)
      continue;

    cell = m_grid.GetCell(i+1, 1);
    tmpStr.AppendFormat("%lf\t", atof(cell->GetText()));

    cell = m_grid.GetCell(i+1, 2);
    tmpStr.AppendFormat("%lf", atof(cell->GetText()));

    cell = m_grid.GetCell(i+1, 3);
    if(0 != atoi(cell->GetText()))
      tmpStr.AppendFormat("\t%lf", atof(cell->GetText()));

    tmpStr.AppendFormat("\n");
    fprintf(f, tmpStr);
  }


  fclose(f);
  OnOK();
}
