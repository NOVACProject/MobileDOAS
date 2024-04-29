// ExportEvLogDlg.cpp : implementation file
#include "stdafx.h"
#include "DMSpec.h"
#include "ExportEvLogDlg.h"
#include "Common.h"

#include <math.h>

using namespace Dialogs;

const enum COLUMNS {
    COL_NOTHING,
    COL_TIME,
    COL_LAT,
    COL_LONG,
    COL_ALT,
    COL_INT_PEAK,
    COL_COLUMN,
    COL_COLUMN_OFFSET,
    COL_COLUMN_ERR,
    N_STRINGS
};

// CExportEvLogDlg dialog

IMPLEMENT_DYNAMIC(CExportEvLogDlg, CDialog)
CExportEvLogDlg::CExportEvLogDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CExportEvLogDlg::IDD, pParent)
{
    m_nStrings = N_STRINGS;
    m_strings = new CString[N_STRINGS]();

    m_strings[COL_NOTHING].Format("Nothing");
    m_strings[COL_TIME].Format("Time");
    m_strings[COL_LAT].Format("Latitude");
    m_strings[COL_LONG].Format("Longitude");
    m_strings[COL_ALT].Format("Altitude");
    m_strings[COL_INT_PEAK].Format("Intensity");
    m_strings[COL_COLUMN].Format("Column");
    m_strings[COL_COLUMN_OFFSET].Format("Offset Corrected Column");
    m_strings[COL_COLUMN_ERR].Format("Column Error");

    m_traverse = nullptr;
}

CExportEvLogDlg::~CExportEvLogDlg()
{
    m_traverse = nullptr;

    delete[] m_strings;
}

void CExportEvLogDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COLUMNS_FRAME, m_columnFrame);
    DDX_Control(pDX, IDC_STATIC_1, m_labelModel);
    DDX_Control(pDX, IDC_COMBO1, m_comboModel);

    // The export file
    DDX_Text(pDX, IDC_EDIT_EXPORTFILE, m_exportFileName);
}


BEGIN_MESSAGE_MAP(CExportEvLogDlg, CDialog)
    ON_BN_CLICKED(IDC_BROWSE_BUTTON, OnBrowseOutputFile)
END_MESSAGE_MAP()


// CExportEvLogDlg message handlers

BOOL CExportEvLogDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    CString string;
    CFont* font = new CFont();

    // There are Nx combo boxes in one line and Ny lines of boxes
    int	margin = 20;
    int	space = 5;
    int Nx = (int)max(5, ceil(sqrt(MAX_N_EXPORT_COLUMNS)));

    // Get the size of the main rect
    CRect totalRect, smallRect, labelRect, comboRect;
    m_columnFrame.GetWindowRect(&totalRect);
    m_labelModel.GetWindowRect(&labelRect);
    m_comboModel.GetWindowRect(&comboRect);
    int	totalWidth = max(labelRect.Width(), comboRect.Width()) + space;
    int totalHeight = labelRect.Height() + comboRect.Height() + space;

    // The font to use
    font->CreateFont(14, 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, 0, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS, "Serif");

    // Intitialize the labels and the combo-boxes
    for (int it = 0; it < MAX_N_EXPORT_COLUMNS; ++it) {

        // 1. ---- The labels -------
        smallRect.left = (it % Nx) * (totalWidth + space) + margin;
        smallRect.right = smallRect.left + totalWidth;
        smallRect.top = (it / Nx) * (totalHeight + space) + margin;
        smallRect.bottom = smallRect.top + totalHeight;

        string.Format("Column %d", it + 1);

        m_labels[it].Create(string, WS_VISIBLE | WS_CHILD, smallRect, &m_columnFrame);
        m_labels[it].SetFont(font);

        // 2. ---- The combo-boxes -----
        smallRect.left = (it % Nx) * (totalWidth + space) + margin;
        smallRect.right = smallRect.left + totalWidth;
        smallRect.top = (it / Nx) * (totalHeight + space) + margin + labelRect.Height();
        smallRect.bottom = smallRect.top + totalHeight;

        m_combo[it].Create(WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, smallRect, &m_columnFrame, 1);
        m_combo[it].SetFont(font);

        // Add the strings
        for (int it2 = 0; it2 < m_nStrings; ++it2) {
            m_combo[it].AddString(m_strings[it2]);
        }
        if (it + 1 >= m_nStrings)
            m_combo[it].SetCurSel(0);
        else
            m_combo[it].SetCurSel(it + 1);

        // Finally adjust the size of the list
        int nLines = min(m_nStrings, 10);
        CRect lprect;
        m_combo[it].GetWindowRect(&lprect);
        lprect.bottom = lprect.top + nLines * m_combo[it].GetItemHeight(-1) + lprect.Height();
        m_combo[it].SetWindowPos(NULL, 0, 0, lprect.Width(), lprect.Height(), SWP_NOMOVE | SWP_NOZORDER);

    }

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CExportEvLogDlg::OnOK()
{
    UpdateData(TRUE);			// Save the data in the dialog

    if (strlen(m_exportFileName) < 3) {
        MessageBox("Please choose where to save data by pressing the 'Browse' button");
        return; // don't close the dialog
    }

    FILE* f = fopen(this->m_exportFileName, "w");
    if (f == nullptr) {
        MessageBox("Cannot open given output-file for writing. Please check settings and try again", "Error");
        return; // don't close the dialog
    }

    for (int row = 0; row < m_traverse->m_recordNum; ++row) {
        for (int col = 0; col < MAX_N_EXPORT_COLUMNS; ++col) {
            int index = m_combo[col].GetCurSel();
            if (index < 0 || index == COL_NOTHING)
                continue;

            if (index == COL_TIME) {
                // Time
                fprintf(f, "%02d:%02d:%02d\t", m_traverse->time[row].hour, m_traverse->time[row].minute, m_traverse->time[row].second);
            }
            else if (index == COL_LAT) {
                // Latitude
                fprintf(f, "%.6lf\t", m_traverse->latitude[row]);
            }
            else if (index == COL_LONG) {
                // Longitude
                fprintf(f, "%.6lf\t", m_traverse->longitude[row]);
            }
            else if (index == COL_ALT) {
                // Altitude
                fprintf(f, "%.0lf\t", m_traverse->altitude[row]);
            }
            else if (index == COL_INT_PEAK) {
                // Peak intensity
                fprintf(f, "%.0lf\t", m_traverse->intensArray[row]);
            }
            else if (index == COL_COLUMN) {
                // Column
                fprintf(f, "%.6lf\t", m_traverse->columnArray[row]);
            }
            else if (index == COL_COLUMN_OFFSET) {
                // Offset Corrected Column
                fprintf(f, "%.6lf\t", m_traverse->columnArray[row] - m_traverse->m_Offset);
            }
            else if (index == COL_COLUMN_ERR) {
                // Column Error
                fprintf(f, "%.6lf\t", 0.0);
            }
        }
        fprintf(f, "\n");
    }

    fclose(f);

    MessageBox("Export Successful");

    CDialog::OnOK();
}

void CExportEvLogDlg::OnCancel()
{
    this->m_traverse = nullptr;

    CDialog::OnCancel();
}

void CExportEvLogDlg::OnBrowseOutputFile()
{
    Common::BrowseForFile_SaveAs("*.txt", this->m_exportFileName);

    // Fill in the data in the dialog
    UpdateData(FALSE);
}

