// ReEval_FitWindows.cpp : implementation file
//

#include "stdafx.h"
#include "../DMSpec.h"
#include "ReEval_FitWindows.h"
#include "../Dialogs/ReferencePropertiesDlg.h"
#include "../Dialogs/ReferencePlotDlg.h"
#include <vector>

// CReEval_FitWindowsDlg dialog

using namespace ReEvaluation;

IMPLEMENT_DYNAMIC(CReEval_FitWindowsDlg, CPropertyPage)
CReEval_FitWindowsDlg::CReEval_FitWindowsDlg()
    : CPropertyPage(CReEval_FitWindowsDlg::IDD)
{
    m_reeval = nullptr;
}

CReEval_FitWindowsDlg::~CReEval_FitWindowsDlg()
{
}

void CReEval_FitWindowsDlg::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_REEVAL_WINDOWS_GRIDFRAME, m_referenceFrame);
    DDX_Control(pDX, IDC_REEVAL_FITWINDOWLIST, m_windowList);

    DDX_Text(pDX, IDC_REEVAL_WINDOW_FITFROM, m_reeval->m_settings.m_window.fitLow);
    DDX_Text(pDX, IDC_REEVAL_WINDOW_FITTO, m_reeval->m_settings.m_window.fitHigh);
    DDX_Text(pDX, IDC_REEVAL_WINDOW_POLYNOM, m_reeval->m_settings.m_window.polyOrder);

    DDX_Radio(pDX, IDC_REEVAL_FITTYPE_DIVIDE, (int&)m_reeval->m_settings.m_window.fitType);
}



BEGIN_MESSAGE_MAP(CReEval_FitWindowsDlg, CPropertyPage)

    // The user has pressed the 'insert' item on the reference-grid context menu
    ON_COMMAND(ID__INSERT, OnInsertReference)
    ON_BN_CLICKED(IDC_INSERT_REFERENCE, OnInsertReference)

    // The user has pressed the 'remove' item on the reference-grid context menu
    ON_COMMAND(ID__REMOVE, OnRemoveReference)
    ON_BN_CLICKED(IDC_REMOVE_REFERENCE, OnRemoveReference)

    // The user wants to see the properties for the currently marked reference
    ON_BN_CLICKED(IDC_REFERENCE_PROPERTIES, OnShowProperties)

    // The user wants to see the window with the references
    ON_BN_CLICKED(IDC_REFERENCE_VIEW, OnShowReferenceGraph)

    ON_EN_KILLFOCUS(IDC_REEVAL_WINDOW_FITFROM, SaveData)
    ON_EN_KILLFOCUS(IDC_REEVAL_WINDOW_FITTO, SaveData)
    ON_EN_KILLFOCUS(IDC_REEVAL_WINDOW_POLYNOM, SaveData)

    ON_BN_CLICKED(IDC_REEVAL_FITTYPE_DIVIDE, SaveData)
    ON_BN_CLICKED(IDC_REEVAL_FITTYPE_SUBTRACT, SaveData)
    ON_BN_CLICKED(IDC_REEVAL_FITTYPE_POLYNOMIAL, SaveData)
END_MESSAGE_MAP()


// CReEval_FitWindowsDlg message handlers

BOOL CReEval_FitWindowsDlg::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    // Populate the fit window - list 
    PopulateWindowList();

    // Initialize the reference grid control
    InitReferenceFileControl();

    // Populate the reference grid control
    PopulateReferenceFileControl();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CReEval_FitWindowsDlg::InitReferenceFileControl()
{

    // Get the dimensions of the reference frame
    CRect rect;
    m_referenceFrame.GetWindowRect(&rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    rect.top = 20;
    rect.left = 10;
    rect.right = width - 20;
    rect.bottom = height - 10;

    // Create the grid
    m_referenceGrid.Create(rect, &m_referenceFrame, 0);
    m_referenceGrid.parent = this;

    // Set the columns of the grid
    m_referenceGrid.InsertColumn("Name");
    m_referenceGrid.SetColumnWidth(0, (int)(rect.right / 9));
    m_referenceGrid.InsertColumn("Reference File");
    m_referenceGrid.SetColumnWidth(1, (int)(rect.right * 5 / 8));
    m_referenceGrid.InsertColumn("Shift");
    m_referenceGrid.SetColumnWidth(2, (int)(rect.right / 8));
    m_referenceGrid.InsertColumn("Squeeze");
    m_referenceGrid.SetColumnWidth(3, (int)(rect.right / 8));

    // Make sure that there are two empty rows
    m_referenceGrid.SetRowCount(3);

    // Makes sure that the user cannot edit the titles of the grid
    m_referenceGrid.SetFixedRowCount(1);

    // make sure the user can edit items in the grid
    m_referenceGrid.SetEditable(TRUE);
}

void CReEval_FitWindowsDlg::PopulateReferenceFileControl()
{
    int curWindow = m_windowList.GetCurSel();

    if (curWindow == -1)
        return;

    Evaluation::CFitWindow& window = m_reeval->m_settings.m_window;
    m_referenceGrid.m_window = &window;

    // make sure that there's always one empty line at the end
    m_referenceGrid.SetRowCount(window.nRef + 2);

    int i;
    for (i = 0; i < window.nRef; ++i)
    {

        novac::CReferenceFile& ref = window.ref[i];

        m_referenceGrid.SetItemTextFmt(1 + i, 0, ref.m_specieName.c_str());
        m_referenceGrid.SetItemTextFmt(1 + i, 1, ref.m_path.c_str());

        if (ref.m_shiftOption == novac::SHIFT_TYPE::SHIFT_FREE)
            m_referenceGrid.SetItemTextFmt(1 + i, 2, "free");

        if (ref.m_shiftOption == novac::SHIFT_TYPE::SHIFT_FIX)
            m_referenceGrid.SetItemTextFmt(1 + i, 2, "fixed to %.2lf", ref.m_shiftValue);

        if (ref.m_shiftOption == novac::SHIFT_TYPE::SHIFT_LINK)
            m_referenceGrid.SetItemTextFmt(1 + i, 2, "linked to %s", window.ref[(int)ref.m_shiftValue].m_specieName.c_str());

        if (ref.m_shiftOption == novac::SHIFT_TYPE::SHIFT_OPTIMAL)
            m_referenceGrid.SetItemTextFmt(1 + i, 2, "find optimal");

        if (ref.m_squeezeOption == novac::SHIFT_TYPE::SHIFT_FREE)
            m_referenceGrid.SetItemTextFmt(1 + i, 3, "free");

        if (ref.m_squeezeOption == novac::SHIFT_TYPE::SHIFT_FIX)
            m_referenceGrid.SetItemTextFmt(1 + i, 3, "fixed to %.2lf", ref.m_squeezeValue);

        if (ref.m_squeezeOption == novac::SHIFT_TYPE::SHIFT_LINK)
            m_referenceGrid.SetItemTextFmt(1 + i, 3, "linked to %s", window.ref[(int)ref.m_squeezeValue].m_specieName.c_str());

        if (ref.m_squeezeOption == novac::SHIFT_TYPE::SHIFT_OPTIMAL)
            m_referenceGrid.SetItemTextFmt(1 + i, 3, "find optimal");
    }

    // make sure that the last line is clear
    m_referenceGrid.SetItemTextFmt(1 + i, 0, "");
    m_referenceGrid.SetItemTextFmt(1 + i, 1, "");
    m_referenceGrid.SetItemTextFmt(1 + i, 2, "");
    m_referenceGrid.SetItemTextFmt(1 + i, 3, "");
}

/** Called when the user wants to remove a reference file */
void CReEval_FitWindowsDlg::OnRemoveReference()
{

    // save the data in the dialog
    UpdateData(TRUE);

    // Get the currently selected fit window
    int curSel = m_windowList.GetCurSel();
    if (curSel < 0)
        return;
    Evaluation::CFitWindow& window = m_reeval->m_settings.m_window;

    // if there's no reference file, then there's nothing to remove
    if (window.nRef <= 0)
        return;

    // Get the selected reference file
    CCellRange cellRange = m_referenceGrid.GetSelectedCellRange();
    int minRow = cellRange.GetMinRow() - 1;
    int nRows = cellRange.GetRowSpan();

    if (nRows <= 0)
    {
        return;
    }
    // move every reference file in the list down one step
    for (int i = minRow; i < window.nRef; i++)
    {
        window.ref[i] = window.ref[i + nRows];
    }

    // reduce the number of references by number deleted
    window.nRef -= nRows;

    // Update the reference grid
    PopulateReferenceFileControl();
}

/** Called when the user wants to insert a new reference file */
void CReEval_FitWindowsDlg::OnInsertReference()
{

    // save the data in the dialog
    UpdateData(TRUE);

    // Get the currently selected fit window
    int curSel = m_windowList.GetCurSel();
    if (curSel < 0)
        return;
    Evaluation::CFitWindow& window = m_reeval->m_settings.m_window;

    // Check so that there is space for more references in this fit window
    if (window.nRef == MAX_N_REFERENCES)
    {
        CString errorMessage;
        errorMessage.Format("Cannot insert more references. The maximum number of references in a fit window is %d", MAX_N_REFERENCES);
        MessageBox(errorMessage, "Cannot insert");
        return;
    }

    // Let the user browse for the reference files
    Common common;
    std::vector<CString> filenames = common.BrowseForFiles();
    for (int i = 0; i < filenames.size(); i++)
    {
        const CString fileName = filenames[i];

        // The user has selected a new reference file, insert it into the list

        // 1. Set the path
        window.ref[window.nRef].m_path = std::string((LPCSTR)fileName);

        // 2. make a guess of the specie name
        CString specie;
        Common::GuessSpecieName(fileName, specie);
        if (strlen(specie) != 0)
        {
            window.ref[window.nRef].m_specieName = std::string((LPCSTR)specie);

            if (Equals(specie, "NO2"))
            {
                window.ref[window.nRef].m_gasFactor = GASFACTOR_NO2;
            }
            else if (Equals(specie, "O3"))
            {
                window.ref[window.nRef].m_gasFactor = GASFACTOR_O3;
            }
            else if (Equals(specie, "HCHO"))
            {
                window.ref[window.nRef].m_gasFactor = GASFACTOR_HCHO;
            }
        }

        // 3. Set the shift and squeeze options for this reference
        if (window.nRef == 0)
        {
            // If it is the first one, select 'optimal' 
            window.ref[window.nRef].m_shiftOption = novac::SHIFT_TYPE::SHIFT_FIX;
            window.ref[window.nRef].m_squeezeOption = novac::SHIFT_TYPE::SHIFT_FIX;
        }
        else
        {
            window.ref[window.nRef].m_shiftOption = novac::SHIFT_TYPE::SHIFT_LINK;
            window.ref[window.nRef].m_shiftValue = 0;
            window.ref[window.nRef].m_squeezeOption = novac::SHIFT_TYPE::SHIFT_LINK;
            window.ref[window.nRef].m_squeezeValue = 1;
        }

        // 4. update the number of references
        window.nRef += 1;

        // If this is the first reference inserted, also make a guess for the window name; if there isn't already a window name
        if (window.nRef == 1 && strlen(specie) != 0 && window.name == "NEW")
        {
            window.name.Format("%s", specie);
            PopulateWindowList();
        }
    }

    // Update the grid
    PopulateReferenceFileControl();
}

/** Called when the user wants to see the
            properties of one reference */
void CReEval_FitWindowsDlg::OnShowProperties()
{

    // save the data in the dialog
    UpdateData(TRUE);

    // Get the currently selected fit window
    int curSel = m_windowList.GetCurSel();
    if (curSel < 0)
        return;
    Evaluation::CFitWindow& window = m_reeval->m_settings.m_window;

    // if there's no reference file, then there's nothing to remove
    if (window.nRef <= 0)
        return;

    // Get the selected reference file
    CCellRange cellRange = m_referenceGrid.GetSelectedCellRange();
    int minRow = cellRange.GetMinRow() - 1;
    int nRows = cellRange.GetRowSpan();

    if (nRows <= 0 || nRows > 1)
    { /* nothing selected or several lines selected */
        MessageBox("Please select a reference file.", "Properties");
        return;
    }

    // Show the properties dialog
    Dialogs::CReferencePropertiesDlg dlg;
    dlg.m_ref = &window.ref[minRow];
    dlg.DoModal();

    // Update the reference grid
    PopulateReferenceFileControl();
}

/** Called when the user wants to see the
    size of the references */
void CReEval_FitWindowsDlg::OnShowReferenceGraph()
{
    // save the data in the dialog
    UpdateData(TRUE);

    // Get the currently selected fit window
    int curSel = m_windowList.GetCurSel();
    if (curSel < 0)
        return;
    Evaluation::CFitWindow& window = m_reeval->m_settings.m_window;

    // if there's no reference file, then there's nothing to show
    if (window.nRef <= 0)
        return;

    // Open the dialog
    Dialogs::CReferencePlotDlg dlg;
    dlg.m_window = &m_reeval->m_settings.m_window;
    dlg.DoModal();
}

void CReEval_FitWindowsDlg::PopulateWindowList()
{
    this->m_windowList.ResetContent(); // clear the list

    //for(int i = 0; i < m_reeval->m_windowNum; ++i){
    //  m_windowList.AddString(m_reeval->m_settings.m_window[i].name);
    //}
    m_windowList.AddString(m_reeval->m_settings.m_window.name);

    m_windowList.SetCurSel(0);
}

void CReEval_FitWindowsDlg::SaveData()
{
    UpdateData(TRUE);
}