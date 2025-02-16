// ShowFitDlg.cpp : implementation file
//

#undef min
#undef max

#include "stdafx.h"
#include "DMSpec.h"
#include "ShowFitDlg.h"
#include "Spectrometer.h"
#include <algorithm>


// CShowFitDlg dialog
using namespace Dialogs;

IMPLEMENT_DYNAMIC(CShowFitDlg, CDialog)
CShowFitDlg::CShowFitDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CShowFitDlg::IDD, pParent)
{
    m_isVisible = false;
}

CShowFitDlg::~CShowFitDlg()
{
}

void CShowFitDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CShowFitDlg, CDialog)
    ON_WM_SIZE()
    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CShowFitDlg message handlers

// CRealTimeRoute message handlers

BOOL CShowFitDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    for (int i = 0; i < MAX_SPECTRUM_LENGTH; ++i)
    {
        pixel[i] = i;
    }

    CRect rect;
    GetClientRect(&rect);
    int margin = 0;
    rect.right -= margin;
    rect.bottom -= margin;
    rect.left = margin;
    rect.top = margin;

    m_fitPlot.Create(WS_VISIBLE | WS_CHILD, rect, this);
    m_fitPlot.SetYUnits("Intensity");
    m_fitPlot.SetXUnits("Pixel");
    m_fitPlot.EnableGridLinesX();
    m_fitPlot.EnableGridLinesY();
    m_fitPlot.SetBackgroundColor(RGB(0, 0, 0));
    m_fitPlot.SetGridColor(RGB(255, 255, 255));
    m_fitPlot.SetRange(0, 200, 0, -1.0, 1.0, 3);

    m_fitPlot2.Create(WS_VISIBLE | WS_CHILD, rect, this);
    m_fitPlot2.SetYUnits("Intensity");
    m_fitPlot2.SetXUnits("Pixel");
    m_fitPlot2.EnableGridLinesX();
    m_fitPlot2.EnableGridLinesY();
    m_fitPlot2.SetBackgroundColor(RGB(0, 0, 0));
    m_fitPlot2.SetGridColor(RGB(255, 255, 255));
    m_fitPlot2.SetRange(0, 200, 0, -1.0, 1.0, 3);

    DrawFit();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CShowFitDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);
    if (nType != SIZE_RESTORED)
    {
        return;
    }

    if (IsWindow(m_fitPlot.m_hWnd))
    {
        if (m_spectrometer != nullptr && m_spectrometer->GetFitRegionNum() > 1)
        {
            m_fitPlot.MoveWindow(0, 0, cx, cy / 2, FALSE);
            m_fitPlot.CleanPlot();
            m_fitPlot2.MoveWindow(0, cy / 2, cx, cy / 2, FALSE);
            m_fitPlot2.CleanPlot();
        }
        else
        {
            m_fitPlot.MoveWindow(0, 0, cx, cy, FALSE);
            m_fitPlot.CleanPlot();
        }
    }

    DrawFit();
}

BOOL CShowFitDlg::Create(UINT nID, CWnd* pParentWnd)
{
    m_isVisible = false;
    m_spectrometer = nullptr;

    return CDialog::Create(nID, pParentWnd);
}

void CShowFitDlg::OnClose()
{
    m_isVisible = false;
    DestroyWindow();
    CDialog::OnClose();
}

void CShowFitDlg::DrawFit()
{
    DrawFit1();
    DrawFit2();
}

void CShowFitDlg::DrawFit1()
{
    double marginSpace = 1e-5;
    static double spectrum[MAX_SPECTRUM_LENGTH];
    static double fitResult[MAX_SPECTRUM_LENGTH];
    static double residual[MAX_SPECTRUM_LENGTH];

    if (m_spectrometer == nullptr)
    {
        return;
    }

    const int fitLow = m_spectrometer->GetFitLow();
    const int fitHigh = m_spectrometer->GetFitHigh();
    int fitWidth = fitHigh - fitLow;

    double minV = 1000;
    double maxV = -1000;
    static double oldMinV = 1e16, oldMaxV = -1e16;

    // copy the high pass filtered spectrum to the local variable
    m_spectrometer->GetProcessedSpectrum(spectrum, MAX_SPECTRUM_LENGTH, 0);

    // copy the fitted result to the local variable
    memcpy(fitResult, m_spectrometer->m_fitResult[0].data(), MAX_SPECTRUM_LENGTH * sizeof(double));

    // find the minimum and maximum value
    for (int i = fitLow; i < fitHigh; ++i)
    {
        minV = std::min(minV, spectrum[i]);
        minV = std::min(minV, fitResult[i]);
        maxV = std::max(maxV, spectrum[i]);
        maxV = std::max(maxV, fitResult[i]);
    }
    if ((maxV - minV) < 0.25 * (oldMaxV - oldMinV))
    {
        oldMaxV = maxV;
        oldMinV = minV;
    }
    else
    {
        if (minV < oldMinV)
            oldMinV = minV;
        else
            minV = oldMinV;

        if (maxV > oldMaxV)
            oldMaxV = maxV;
        else
            maxV = oldMaxV;
    }

    // set the range for the fit
    m_fitPlot.SetRange(fitLow, fitHigh, 0, minV - marginSpace, maxV + marginSpace, 3);

    // draw the spectrum
    m_fitPlot.SetPlotColor(red);
    m_fitPlot.XYPlot(pixel + fitLow, spectrum + fitLow, fitWidth, Graph::CGraphCtrl::PLOT_CONNECTED);

    // draw the fitted result
    m_fitPlot.SetPlotColor(cyan);
    m_fitPlot.XYPlot(pixel + fitLow, fitResult + fitLow, fitWidth, Graph::CGraphCtrl::PLOT_CONNECTED | Graph::CGraphCtrl::PLOT_FIXED_AXIS);
}

void CShowFitDlg::DrawFit2()
{
    double marginSpace = 1e-5;
    static double spectrum[MAX_SPECTRUM_LENGTH];
    static double fitResult[MAX_SPECTRUM_LENGTH];
    static double residual[MAX_SPECTRUM_LENGTH];

    if (m_spectrometer == nullptr || m_spectrometer->GetFitRegionNum() == 1)
    {
        return;
    }

    int fitLow = m_spectrometer->GetFitLow(1);
    int fitHigh = m_spectrometer->GetFitHigh(1);
    int fitWidth = fitHigh - fitLow;

    double minV = 1000;
    double maxV = -1000;
    static double oldMinV = 1e16, oldMaxV = -1e16;

    // copy the high pass filtered spectrum to the local variable
    m_spectrometer->GetProcessedSpectrum(spectrum, MAX_SPECTRUM_LENGTH, 1);

    // copy the fitted result to the local variable
    memcpy(fitResult, m_spectrometer->m_fitResult[1].data(), MAX_SPECTRUM_LENGTH * sizeof(double));

    // find the minimum and maximum value
    for (int i = fitLow; i < fitHigh; ++i)
    {
        minV = std::min(minV, spectrum[i]);
        minV = std::min(minV, fitResult[i]);
        maxV = std::max(maxV, spectrum[i]);
        maxV = std::max(maxV, fitResult[i]);
    }
    if ((maxV - minV) < 0.25 * (oldMaxV - oldMinV))
    {
        oldMaxV = maxV;
        oldMinV = minV;
    }
    else
    {
        if (minV < oldMinV)
            oldMinV = minV;
        else
            minV = oldMinV;

        if (maxV > oldMaxV)
            oldMaxV = maxV;
        else
            maxV = oldMaxV;
    }

    // set the range for the fit
    m_fitPlot2.SetRange(fitLow, fitHigh, 0, minV - marginSpace, maxV + marginSpace, 3);

    // draw the spectrum
    m_fitPlot2.SetPlotColor(red);
    m_fitPlot2.XYPlot(pixel + fitLow, spectrum + fitLow, (int)fitWidth, Graph::CGraphCtrl::PLOT_CONNECTED);

    // draw the fitted result
    m_fitPlot2.SetPlotColor(cyan);
    m_fitPlot2.XYPlot(pixel + fitLow, fitResult + fitLow, (int)fitWidth, Graph::CGraphCtrl::PLOT_CONNECTED | Graph::CGraphCtrl::PLOT_FIXED_AXIS);
}