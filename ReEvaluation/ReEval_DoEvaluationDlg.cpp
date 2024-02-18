#include "stdafx.h"
#include "../DMSpec.h"
#include "ReEval_DoEvaluationDlg.h"

// CReEval_DoEvaluationDlg dialog
using namespace ReEvaluation;

IMPLEMENT_DYNAMIC(CReEval_DoEvaluationDlg, CPropertyPage)
CReEval_DoEvaluationDlg::CReEval_DoEvaluationDlg()
    : CPropertyPage(CReEval_DoEvaluationDlg::IDD)
{
    this->pReEvalThread = nullptr;
}

CReEval_DoEvaluationDlg::~CReEval_DoEvaluationDlg()
{
}

void CReEval_DoEvaluationDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROGRESS1, m_progressCtrl);
    DDX_Control(pDX, IDC_CANCEL, m_cancelButton);
    DDX_Control(pDX, IDC_DOEVALUATION, m_btnDoEvaluation);
    DDX_Control(pDX, IDC_OUTPUT_LIST, m_outputList);
    DDX_Control(pDX, IDC_FIT_WINDOW, m_fitWindow);
    DDX_Control(pDX, IDC_RADIO_FIT, m_showFit);

    DDX_Check(pDX, IDC_REEVAL_PAUSE, m_reeval->m_pause);
}


BEGIN_MESSAGE_MAP(CReEval_DoEvaluationDlg, CDialog)
    ON_BN_CLICKED(IDC_DOEVALUATION, OnBnClickedDoEvaluation)

    ON_MESSAGE(WM_PROGRESS, OnProgress)
    ON_MESSAGE(WM_CANCEL, OnCancel)
    ON_MESSAGE(WM_DONE, OnDone)
    ON_MESSAGE(WM_STATUS, OnStatusUpdate)
    ON_MESSAGE(WM_EVAL, OnEvaluatedSpectrum)
    ON_MESSAGE(WM_GOTO_SLEEP, OnEvaluationSleep)

    ON_BN_CLICKED(IDC_CANCEL, OnBnClickedCancel)
    ON_BN_CLICKED(IDC_REEVAL_PAUSE, SaveData)

    ON_BN_CLICKED(IDC_RADIO_FIT, UpdateScreen)
    ON_BN_CLICKED(IDC_RADIO_RESIDUAL, UpdateScreen)

    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CReEval_DoEvaluationDlg message handlers

void CReEval_DoEvaluationDlg::OnBnClickedDoEvaluation()
{
    if (m_reeval->fRun && m_reeval->m_sleeping)
    {
        pReEvalThread->ResumeThread();
    }
    else
    {
        m_reeval->m_mainView = this;

        // start the reevaluation thread
        pReEvalThread = AfxBeginThread(DoEvaluation, (LPVOID)(m_reeval), THREAD_PRIORITY_BELOW_NORMAL, 0, 0, NULL);

        m_outputList.ResetContent();
    }

    // update the window
    SetDlgItemText(IDC_STATUSBAR, "Evaluating");
    m_cancelButton.EnableWindow(TRUE);
    m_btnDoEvaluation.EnableWindow(FALSE);

    //UpdateData(FALSE);
}

LRESULT CReEval_DoEvaluationDlg::OnProgress(WPARAM wp, LPARAM lp)
{

    m_progressCtrl.SetRange(0, 1000);
    m_progressCtrl.SetPos((int)(1000 * m_reeval->m_progress));

    CString statusMsg;
    switch (m_reeval->m_mode)
    {
    case CReEvaluator::MODE_READING_OFFSETS:
        statusMsg.Format("Reading Offsets - %.0lf%% done", 100 * m_reeval->m_progress); break;
    default:
        statusMsg.Format("Evaluating - %.0lf%% done", 100 * m_reeval->m_progress);
    }
    this->SetDlgItemText(IDC_STATUSBAR, statusMsg);

    this->UpdateData(FALSE);
    return 0;
}

LRESULT CReEval_DoEvaluationDlg::OnCancel(WPARAM wp, LPARAM lp)
{
    // update the window
    m_progressCtrl.SetRange(0, 1000);
    m_progressCtrl.SetPos(0);

    SetDlgItemText(IDC_STATUSBAR, "Evaluation cancelled");
    m_cancelButton.EnableWindow(FALSE);
    m_btnDoEvaluation.SetWindowText("Do Evaluation");
    m_btnDoEvaluation.EnableWindow(TRUE);

    UpdateData(FALSE);
    return 0;
}

LRESULT CReEval_DoEvaluationDlg::OnDone(WPARAM wp, LPARAM lp)
{

    // update the window
    m_progressCtrl.SetRange(0, 1000);
    m_progressCtrl.SetPos(1000);

    // update the graph, show the average residual
    static double residual[MAX_SPECTRUM_LENGTH];

    // the width of the fit region (fitHigh - fitLow)
    double fitWidth = m_reeval->m_settings.m_window.fitHigh - m_reeval->m_settings.m_window.fitLow;

    // copy the spectrum to the local variable
    memcpy(residual + m_reeval->m_settings.m_window.fitLow, &m_reeval->m_avgResidual[0], (size_t)fitWidth * sizeof(double));

    // draw the residual
    m_graph.CleanPlot();
    m_graph.SetPlotColor(RGB(255, 0, 0));
    m_graph.Plot(residual + m_reeval->m_settings.m_window.fitLow, (int)fitWidth);


    SetDlgItemText(IDC_STATUSBAR, "Evaluation done");
    m_cancelButton.EnableWindow(FALSE);
    m_btnDoEvaluation.EnableWindow(TRUE);
    m_btnDoEvaluation.SetWindowText("Do Evaluation");

    UpdateData(FALSE);
    return 0;
}

LRESULT CReEval_DoEvaluationDlg::OnStatusUpdate(WPARAM wp, LPARAM lp)
{

    m_outputList.AddString(m_reeval->m_statusMsg);
    UpdateData(FALSE);

    return 0;
}

// Called when a spectrum has been evaluated, update the graph window
LRESULT CReEval_DoEvaluationDlg::OnEvaluatedSpectrum(WPARAM wp, LPARAM lp)
{
    DrawSpec();

    UpdateScreen();

    return 0;
}

void CReEval_DoEvaluationDlg::DrawSpec()
{
    m_graph.CleanPlot();
    if (m_showFit.GetCheck())
    {
        this->DrawFit();
    }
    else
    {
        this->DrawResidual();
    }

    return;
}
void CReEval_DoEvaluationDlg::DrawFit()
{
    static double spectrum[MAX_SPECTRUM_LENGTH];
    static double fitResult[MAX_SPECTRUM_LENGTH];
    static double residual[MAX_SPECTRUM_LENGTH];
    static double oldMinV = 1e16, oldMaxV = -1e16;
    static double pixels[MAX_SPECTRUM_LENGTH];
    static int firstCall = 1;

    if (firstCall)
    {
        for (int k = 0; k < MAX_SPECTRUM_LENGTH; ++k)
        {
            pixels[k] = k;
        }
        firstCall = 0;
    }


    // the width of the fit region (fitHigh - fitLow)
    double fitWidth = m_reeval->m_settings.m_window.fitHigh - m_reeval->m_settings.m_window.fitLow;

    /* show the fit */
    // copy the spectrum to the local variable
    memcpy(spectrum, &m_reeval->m_spectrum, MAX_SPECTRUM_LENGTH * sizeof(double));

    // copy the fitted result to the local variable
    memcpy(fitResult, &m_reeval->m_fitResult, MAX_SPECTRUM_LENGTH * sizeof(double));

    // draw the spectrum
    m_graph.SetPlotColor(RGB(255, 0, 0));
    m_graph.XYPlot(pixels + m_reeval->m_settings.m_window.fitLow, spectrum + m_reeval->m_settings.m_window.fitLow, (int)fitWidth);

    // draw the fitted result
    m_graph.SetPlotColor(RGB(0, 0, 255), false);
    m_graph.XYPlot(pixels + m_reeval->m_settings.m_window.fitLow, fitResult + m_reeval->m_settings.m_window.fitLow, (int)fitWidth, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);

    return;
}
void CReEval_DoEvaluationDlg::DrawResidual()
{
    static double spectrum[MAX_SPECTRUM_LENGTH];
    static double fitResult[MAX_SPECTRUM_LENGTH];
    static double residual[MAX_SPECTRUM_LENGTH];
    static double oldMinV = 1e16, oldMaxV = -1e16;

    /* show the residual */

    // the width of the fit region (fitHigh - fitLow)
    double fitWidth = m_reeval->m_settings.m_window.fitHigh - m_reeval->m_settings.m_window.fitLow;

    // copy the fitted result to the local variable
    memcpy(&residual[m_reeval->m_settings.m_window.fitLow], &m_reeval->m_residual[0], (size_t)fitWidth * sizeof(double));

    // draw the spectrum
    m_graph.SetPlotColor(RGB(255, 0, 0));
    m_graph.Plot(residual + m_reeval->m_settings.m_window.fitLow, (int)fitWidth);
}

UINT DoEvaluation(LPVOID pParam)
{
    CReEvaluator* m_reeval = (CReEvaluator*)pParam;

    m_reeval->fRun = true;
    m_reeval->DoEvaluation();
    return 0;
}


void CReEval_DoEvaluationDlg::OnBnClickedCancel()
{

    if (m_reeval->fRun && m_reeval->m_sleeping)
    {
        pReEvalThread->ResumeThread();
    }

    DWORD dwExitCode;
    HANDLE hThread = this->pReEvalThread->m_hThread;
    if (hThread != nullptr && GetExitCodeThread(hThread, &dwExitCode) && dwExitCode == STILL_ACTIVE)
    {
        AfxGetApp()->BeginWaitCursor();
        this->m_reeval->Stop();
        this->m_reeval->m_pause = FALSE; // un-pause the thread to let it terminate

        WaitForSingleObject(hThread, INFINITE);
        AfxGetApp()->EndWaitCursor();
    }

    // update the window
    SetDlgItemText(IDC_STATUSBAR, "Evaluation canceled");
    m_cancelButton.EnableWindow(FALSE);
    m_btnDoEvaluation.EnableWindow(TRUE);

    UpdateData(FALSE);
}

// initialize the dialog window
BOOL CReEval_DoEvaluationDlg::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    CRect rect;
    int margin = 2;
    m_fitWindow.GetWindowRect(&rect);
    rect.bottom -= rect.top + margin;
    rect.right -= rect.left + margin;
    rect.top = margin + 10;
    rect.left = margin;
    m_graph.Create(WS_VISIBLE | WS_CHILD, rect, &m_fitWindow);
    m_graph.SetRange(0, MAX_SPECTRUM_LENGTH, 1, -1.0, 1.0, 1);
    m_graph.SetYUnits("Intensity");
    m_graph.SetXUnits("Channel");
    m_graph.SetBackgroundColor(RGB(0, 0, 0));
    m_graph.SetGridColor(RGB(255, 255, 255));
    m_graph.SetPlotColor(RGB(255, 0, 0));
    m_graph.CleanPlot();

    m_showFit.SetCheck(TRUE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CReEval_DoEvaluationDlg::UpdateScreen()
{
    CString str;

    // the spectrum number
    str.Format("Spectrum: %d", m_reeval->m_curSpec);
    SetDlgItemText(IDC_REEVAL_FITRESULT_SPECTRUMNO, str);

    // chi2 of the fit
    str.Format("Chi²: %.2e", m_reeval->m_chiSquare);
    SetDlgItemText(IDC_REEVAL_FITRESULT_CHI2, str);

    // clear the columns
    str.Format("");
    SetDlgItemText(IDC_REEVAL_FITRESULT_REFERENCE1, str);
    SetDlgItemText(IDC_REEVAL_FITRESULT_REFERENCE2, str);
    SetDlgItemText(IDC_REEVAL_FITRESULT_REFERENCE3, str);
    SetDlgItemText(IDC_REEVAL_FITRESULT_REFERENCE4, str);
    SetDlgItemText(IDC_REEVAL_FITRESULT_REFERENCE5, str);

    // update the columns
    if (m_reeval->m_settings.m_window.fitType != Evaluation::FIT_HP_DIV)
    {
        int skyRef = m_reeval->m_settings.m_window.nRef - 1;
        str.Format("Sky shift: %.5lf ± %.5lf", m_reeval->m_evResult[skyRef][2], m_reeval->m_evResult[skyRef][3]);
        SetDlgItemText(IDC_REEVAL_FITRESULT_REFERENCE5, str);
    }

    int nReferencesToShow = (m_reeval->m_settings.m_window.fitType == Evaluation::FIT_HP_DIV) ? m_reeval->m_settings.m_window.nRef : m_reeval->m_settings.m_window.nRef - 1;
    for (int i = 0; i < nReferencesToShow; ++i)
    {
        str.Format("%4s: %2.2e ± %2.2e", m_reeval->m_settings.m_window.ref[i].m_specieName.c_str(),
            m_reeval->m_evResult[i][0], m_reeval->m_evResult[i][1]);
        switch (i)
        {
        case 0: SetDlgItemText(IDC_REEVAL_FITRESULT_REFERENCE1, str); break;
        case 1: SetDlgItemText(IDC_REEVAL_FITRESULT_REFERENCE2, str); break;
        case 2: SetDlgItemText(IDC_REEVAL_FITRESULT_REFERENCE3, str); break;
        case 3: SetDlgItemText(IDC_REEVAL_FITRESULT_REFERENCE4, str); break;
        }
    }
}

void CReEval_DoEvaluationDlg::SaveData()
{
    UpdateData(TRUE);
}

LRESULT CReEval_DoEvaluationDlg::OnEvaluationSleep(WPARAM wp, LPARAM lp)
{

    SetDlgItemText(IDC_STATUSBAR, "Waiting...");
    m_cancelButton.EnableWindow(TRUE);
    m_btnDoEvaluation.EnableWindow(TRUE);
    m_btnDoEvaluation.SetWindowText("&Next...");

    UpdateData(FALSE);
    return 0;
}

void ReEvaluation::CReEval_DoEvaluationDlg::OnOK()
{
    OnCancel(NULL, NULL);

    CPropertyPage::OnOK();
}

void ReEvaluation::CReEval_DoEvaluationDlg::OnClose()
{
    OnCancel(NULL, NULL);

    CPropertyPage::OnClose();
}
