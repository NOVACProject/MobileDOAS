// ReEval_DarkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\DMSpec.h"
#include "ReEval_DarkDlg.h"


// CReEval_DarkDlg dialog

using namespace ReEvaluation;

IMPLEMENT_DYNAMIC(CReEval_DarkDlg, CPropertyPage)
CReEval_DarkDlg::CReEval_DarkDlg()
    : CPropertyPage(CReEval_DarkDlg::IDD)
{
    m_numDarkSpectra = 0;

    this->pReEvalThread = nullptr;
}

CReEval_DarkDlg::~CReEval_DarkDlg()
{
}

void CReEval_DarkDlg::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);

    DDX_Radio(pDX, IDC_SINGLE_DARKSPECTRUM, m_reeval->m_settings.m_fInterpolateDark);
    DDX_Control(pDX, IDC_BOUNDARY, m_graphBoundary);
    DDX_Control(pDX, IDC_PROGRESS1, m_progressCtrl);
    DDX_Control(pDX, IDC_CHECK_OFFSETS, m_OffsetButton);
}


BEGIN_MESSAGE_MAP(CReEval_DarkDlg, CPropertyPage)
    ON_BN_CLICKED(IDC_SEVERAL_DARKSPECTRA, OnBnClickedSeveralDarkspectra)
    ON_BN_CLICKED(IDC_SINGLE_DARKSPECTRUM, OnBnClickedSingleDarkspectrum)
    ON_BN_CLICKED(IDC_CHECK_OFFSETS, OnBnClickedCheckOffsets)

    ON_MESSAGE(WM_PROGRESS, OnProgress)
    ON_MESSAGE(WM_DONE, OnDone)
END_MESSAGE_MAP()


// CReEval_DarkDlg message handlers

void CReEval_DarkDlg::OnBnClickedSeveralDarkspectra()
{

    m_reeval->m_settings.m_fInterpolateDark = true;
}

void CReEval_DarkDlg::OnBnClickedSingleDarkspectrum()
{
    m_reeval->m_settings.m_fInterpolateDark = false;
}

BOOL CReEval_DarkDlg::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    CRect rect;
    int margin = 2;
    m_graphBoundary.GetWindowRect(&rect);
    rect.bottom -= rect.top + margin;
    rect.right -= rect.left + margin;
    rect.top = margin + 7;
    rect.left = margin;
    m_offsetPlot.Create(WS_VISIBLE | WS_CHILD, rect, &m_graphBoundary);
    m_offsetPlot.SetRange(0, 500, 1, -100.0, 100.0, 1);
    m_offsetPlot.SetYUnits("Offset");
    m_offsetPlot.SetXUnits("Number");
    m_offsetPlot.SetBackgroundColor(RGB(0, 0, 0));
    m_offsetPlot.SetGridColor(RGB(255, 255, 255));//(192, 192, 255)) ;
    m_offsetPlot.SetPlotColor(RGB(255, 0, 0));
    m_offsetPlot.CleanPlot();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CReEval_DarkDlg::OnBnClickedCheckOffsets()
{

    if (strlen(m_reeval->m_evalLogFileName) == 0)
    {
        MessageBox("Select an Evaluation Log first", "Error");
        return;
    }

    m_reeval->m_mainView = this;

    pReEvalThread = AfxBeginThread(ReadOffsets, (LPVOID)(m_reeval), THREAD_PRIORITY_NORMAL, 0, 0, NULL);
}

UINT ReadOffsets(LPVOID pParam)
{
    CReEvaluator* m_reeval = (CReEvaluator*)pParam;

    m_reeval->fRun = true;
    m_reeval->ReadAllOffsets();
    return 0;
}


void CReEval_DarkDlg::DrawOffsetPlot()
{
    Common common;
    long left, right;
    double y[128], x[128];

    if (m_reeval->m_recordNum[0] < MAX_TRAVERSE_SHOWN)
    {
        left = 0;
        right = m_reeval->m_recordNum[0] - 1;
    }
    else
    {
        left = 0;
        right = MAX_TRAVERSE_SHOWN - 1;
    }

    double maxOffset = MaxValue(m_reeval->m_offset[0], m_reeval->m_recordNum[0]);
    double minOffset = MinValue(m_reeval->m_offset[0], m_reeval->m_recordNum[0]);

    if (left == right)
        return;

    m_offsetPlot.CleanPlot();


    /* Draw the offset */
    m_offsetPlot.SetPlotColor(RGB(255, 0, 0));	// draw the offset-line in red
    m_offsetPlot.Plot(m_reeval->m_offset[0] + left, (int)(right - left + 1));

    /* Draw the measured dark spectra */
    m_offsetPlot.SetCircleColor(RGB(0, 0, 255));	// draw the dark-spectra in blue
    int length = 0;
    for (int i = 0; i < m_reeval->m_darkSpecListLength[0]; ++i)
    {
        x[i] = (double)m_reeval->m_darkSpecList[0][i];
        y[i] = m_reeval->m_offset[0][m_reeval->m_darkSpecList[0][i]] - minOffset;
    }
    m_offsetPlot.DrawCircles(x, y, m_reeval->m_darkSpecListLength[0], Graph::CGraphCtrl::PLOT_FIXED_AXIS);

}

LRESULT CReEval_DarkDlg::OnProgress(WPARAM wp, LPARAM lp)
{

    m_progressCtrl.SetRange(0, 1000);
    m_progressCtrl.SetPos((int)(1000 * m_reeval->m_progress));

    return 0;
}

LRESULT CReEval_DarkDlg::OnDone(WPARAM wp, LPARAM lp)
{

    DrawOffsetPlot();

    return 0;
}