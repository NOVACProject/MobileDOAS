// ReEval_EvalLogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\DMSpec.h"
#include "../Dialogs/SpectrumInspectionDlg.h"
#include "ReEval_EvalLogDlg.h"


// CReEval_EvalLogDlg dialog
using namespace ReEvaluation;

IMPLEMENT_DYNAMIC(CReEval_EvalLogDlg, CPropertyPage)
CReEval_EvalLogDlg::CReEval_EvalLogDlg()
    : CPropertyPage(CReEval_EvalLogDlg::IDD)
{
    m_reeval = 0;
    number = nullptr;
    m_wavelengthRegion = 0;
}

CReEval_EvalLogDlg::~CReEval_EvalLogDlg()
{
    if (number != nullptr)
        free(number);
}

void CReEval_EvalLogDlg::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);

    DDX_Text(pDX, IDC_EDIT_EVLOG, m_reeval->m_evalLogFileName);
    DDX_Control(pDX, IDC_GRAPH_BOUNDARY, m_graphBoundary);

    DDX_Text(pDX, IDC_NMERGE_EDIT, m_reeval->m_settings.m_nAverageSpectra);

    DDX_Radio(pDX, IDC_RADIO_IGNORE_DARK, m_reeval->m_settings.m_ignoreDark.selection);

    // the options for ignoring spectra
    DDX_Text(pDX, IDC_EDIT_IGNORE_INTENSITY, m_reeval->m_settings.m_ignoreDark.intensity);
    DDX_Text(pDX, IDC_EDIT_IGNORE_CHANNEL, m_reeval->m_settings.m_ignoreDark.channel);

    DDX_Check(pDX, IDC_CHECK_IGNORE_SELECTION_UPPER, m_reeval->m_settings.m_ignoreSaturated.selection);
    DDX_Text(pDX, IDC_EDIT_IGNORE_INTENSITY_UPPER, m_reeval->m_settings.m_ignoreSaturated.intensity);
    DDX_Text(pDX, IDC_EDIT_IGNORE_CHANNEL_UPPER, m_reeval->m_settings.m_ignoreSaturated.channel);

    CString avgSpecString;
    if (m_reeval->m_nspec[0] > 0)
        avgSpecString.Format("Averaged Spectra: %d", m_reeval->m_nspec[0]);
    else
        avgSpecString.Format("Averaged Spectra:");
    DDX_Text(pDX, IDC_INFORMATION_AVERAGED_SPECTRA, avgSpecString);

    CString expTimeString;
    if (m_reeval->m_exptime[0] > 0)
        expTimeString.Format("Exposure Time: %d ms", m_reeval->m_exptime[0]);
    else
        expTimeString.Format("Exposure Time:");
    DDX_Text(pDX, IDC_INFORMATION_EXPOSURE_TIME, expTimeString);

    DDX_Radio(pDX, IDC_RADIO_UV, m_wavelengthRegion);

    // The 'view spectra' button
    DDX_Control(pDX, IDC_BUTTON_VIEWSPECTRA, m_viewButton);
}


BEGIN_MESSAGE_MAP(CReEval_EvalLogDlg, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_EVLOG, OnBnClickedButtonBrowseEvlog)
    ON_EN_CHANGE(IDC_NMERGE_EDIT, OnEnChangeNmergeEdit)
    ON_EN_CHANGE(IDC_EDIT_IGNORE_SPECTRA, OnEnChangeEditIgnoreSpectra)
    ON_EN_CHANGE(IDC_EDIT_IGNORE_INTENSITY_UPPER, SaveData)
    ON_EN_CHANGE(IDC_EDIT_IGNORE_CHANNEL_UPPER, SaveData)
    ON_EN_CHANGE(IDC_EDIT_IGNORE_INTENSITY, SaveData)
    ON_EN_CHANGE(IDC_EDIT_IGNORE_CHANNEL, SaveData)

    ON_BN_CLICKED(IDC_RADIO_VISIBLE, SaveData)
    ON_BN_CLICKED(IDC_RADIO_UV, SaveData)

    ON_BN_CLICKED(IDC_BUTTON_VIEWSPECTRA, OnViewSpectra)

    ON_EN_CHANGE(IDC_EDIT_EVLOG, ReadEvaluationLog)

END_MESSAGE_MAP()


// CReEval_EvalLogDlg message handlers

void CReEval_EvalLogDlg::OnBnClickedButtonBrowseEvlog()
{
    CString fileName;
    Common common;
    TCHAR filter[512];
    int n = _stprintf(filter, "Evaluation Logs\0");
    n += _stprintf(filter + n + 1, "*.txt\0");
    filter[n + 2] = 0;

    if (!common.BrowseForFile(filter, fileName))
        return;

    /** Save the filename */
    m_reeval->m_evalLogFileName.Format(fileName);

    // enable the 'View spectra' button
    m_viewButton.EnableWindow(TRUE);

    /** Update the screen (will cause a call to ReadEvaluationLog) */
    UpdateData(FALSE);

    ReadEvaluationLog();
}

/* Initialize the 'number' vector */
bool CReEval_EvalLogDlg::InitializeNumber()
{
    if (number != nullptr)
        return true;

    number = (double*)calloc(MAX_TRAVERSE_LENGTH, sizeof(double));

    if (number[1] == 0)
    {
        for (int i = 1; i < MAX_TRAVERSE_LENGTH; ++i)
            number[i] = i;
    }
    return true;
}

/* Draw the traverse */
void CReEval_EvalLogDlg::DrawColumn()
{
    if (number == nullptr)
    {
        InitializeNumber();
    }

    long xMin = 0;
    long xMax = 0;

    if (m_reeval->m_recordNum[0] < MAX_TRAVERSE_SHOWN)
    {
        xMax = m_reeval->m_recordNum[0] - 1;
    }
    else
    {
        xMax = MAX_TRAVERSE_SHOWN - 1;
    }
    if (xMin >= xMax)
    {
        return;
    }

    m_ColumnPlot.CleanPlot();

    // Get the intensity and convert it to saturation-ratio
    const int numberOfValuesToShow = static_cast<int>(xMax - xMin + 1);
    std::vector<double> intens;
    intens.resize(numberOfValuesToShow);
    memcpy(intens.data(), m_reeval->m_int[0], numberOfValuesToShow * sizeof(double));
    double dynRange_inv = 100.0 / m_reeval->m_spectrometerDynRange;
    for (int k = 0; k < numberOfValuesToShow; ++k)
    {
        intens[k] = intens[k] * dynRange_inv;
    }

    /* Draw the column */
    m_ColumnPlot.Plot(m_reeval->m_oldCol[0], (numberOfValuesToShow));

    /* draw the intensity */
    m_ColumnPlot.DrawCircles(number, intens.data(), numberOfValuesToShow, Graph::CGraphCtrl::PLOT_SECOND_AXIS);

    return;
}


BOOL CReEval_EvalLogDlg::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    CRect rect;
    int margin = 2;
    m_graphBoundary.GetWindowRect(&rect);
    rect.bottom -= rect.top + margin;
    rect.right -= rect.left + margin;
    rect.top = margin + 7;
    rect.left = margin;
    m_ColumnPlot.Create(WS_VISIBLE | WS_CHILD, rect, &m_graphBoundary);
    m_ColumnPlot.SetRange(0, 500, 1, -100.0, 100.0, 1);
    m_ColumnPlot.SetYUnits("Column");
    m_ColumnPlot.SetXUnits("Number");
    m_ColumnPlot.SetBackgroundColor(RGB(0, 0, 0));
    m_ColumnPlot.SetGridColor(RGB(255, 255, 255));
    m_ColumnPlot.SetPlotColor(RGB(255, 0, 0));
    m_ColumnPlot.SetSecondYUnit("Intensity");
    m_ColumnPlot.SetSecondRangeY(0, 100, 0);
    m_ColumnPlot.CleanPlot();

    //  m_ignoreDarkCheck.SetCheck(m_reeval->m_fIgnoreDark);

    if (strlen(m_reeval->m_evalLogFileName) != 0)
        DrawColumn();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

// if the user changes the number of spectra that are to be averaged together
void CReEval_EvalLogDlg::OnEnChangeNmergeEdit()
{
    UpdateData(TRUE);

    if (m_reeval->m_settings.m_nAverageSpectra < 0)
    {
        m_reeval->m_settings.m_nAverageSpectra = 1;
        UpdateData(FALSE);
        return;
    }

    if (m_reeval->m_settings.m_nAverageSpectra > 1)
    {
        m_ignoreDarkCheck.EnableWindow(FALSE);
    }
    else
    {
        m_ignoreDarkCheck.EnableWindow(TRUE);
    }
}

void CReEval_EvalLogDlg::OnEnChangeEditIgnoreSpectra()
{
    char  buffer[4096];
    int   noAssigned = 0;
    bool  sequence = false;

    m_reeval->m_settings.m_ignoreDark.selection = IGNORE_LIST;

    memset(m_reeval->m_settings.m_ignoreList_Lower, -1, MAX_IGNORE_LIST_LENGTH * sizeof(long));

    ::GetDlgItemText(this->m_hWnd, IDC_EDIT_IGNORE_SPECTRA, buffer, 4096);
    char* szToken = buffer;

    // parse the list
    while (szToken = strtok(szToken, " .,;:"))
    {
        // if the user has typed in a sequence
        if (*szToken == '-')
        {
            sequence = true;
            szToken = NULL;
            continue;
        }

        // if we are reading a sequence
        if (sequence)
        {
            sequence = false;
            long to;
            long from = (noAssigned > 0) ? m_reeval->m_settings.m_ignoreList_Lower[noAssigned - 1] : 0;
            if (!sscanf(szToken, "%ld", &to))
            {
                break;
            }
            for (int j = from; j < to && noAssigned < MAX_IGNORE_LIST_LENGTH; ++j)
            {
                m_reeval->m_settings.m_ignoreList_Lower[noAssigned++] = (short)j;
            }
        }

        // if we should read a common number
        if (!sscanf(szToken, "%ld", &m_reeval->m_settings.m_ignoreList_Lower[noAssigned]))
        {
            break;
        }
        else
        {
            ++noAssigned;
        }
        szToken = NULL;
    }

    UpdateData(FALSE);
}

void CReEval_EvalLogDlg::SaveData()
{
    UpdateData(TRUE);

    if (m_wavelengthRegion == 0)
    {
        m_reeval->m_settings.m_window.offsetFrom = 50;
        m_reeval->m_settings.m_window.offsetTo = 200;
    }
    else
    {
        m_reeval->m_settings.m_window.offsetFrom = 5;
        m_reeval->m_settings.m_window.offsetTo = 20;
    }
}

void CReEval_EvalLogDlg::ReadEvaluationLog()
{
    CString fileName;

    UpdateData(TRUE); // <-- get the data in the dialog

    // get the name of the chosen file
    fileName.Format(m_reeval->m_evalLogFileName);

    // Extract the directory
    int pos = fileName.ReverseFind('\\');
    if (pos != -1)
        m_reeval->m_specFileDir.Format(fileName.Left(pos));
    else
        m_reeval->m_specFileDir.Format(fileName);

    // read the chosen evaluation log
    m_reeval->ReadEvaluationLog();

    // update the graph
    DrawColumn();
}

void CReEval_EvalLogDlg::OnViewSpectra()
{
    CString directory;

    // Get the directory of the spectra
    directory.Format(m_reeval->m_evalLogFileName);
    Common::GetDirectory(directory);

    Dialogs::CSpectrumInspectionDlg* dlg = new Dialogs::CSpectrumInspectionDlg();
    dlg->ReadEvaluationLog(m_reeval->m_evalLogFileName);
    dlg->m_spectrumPath.Format(directory);
    dlg->m_evaluationLog.Format(m_reeval->m_evalLogFileName);
    dlg->DoModal();

    delete dlg;
}
