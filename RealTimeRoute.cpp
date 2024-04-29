// RealTimeRoute.cpp : implementation file
#undef min
#undef max

#include "stdafx.h"
#include "DMSpec.h"
#include "RealTimeRoute.h"
#include "Spectrometer.h"
#include <algorithm>

// CRealTimeRoute dialog

using namespace Dialogs;

IMPLEMENT_DYNAMIC(CRealTimeRoute, CDialog)
CRealTimeRoute::CRealTimeRoute(CWnd* pParent /*=NULL*/)
    : CDialog(CRealTimeRoute::IDD, pParent),
    m_pointNum(0), fVisible(false), m_intensityLimit(400), m_legendWidth(0), m_spectrometer(nullptr), m_srcLat(0.0), m_srcLon(0.0)
{
    // There is a maximum size to the dataset available, stick to that
    m_lat.resize(65536);
    m_lon.resize(65536);
    m_col.resize(65536);
    m_int.resize(65536);
}

CRealTimeRoute::~CRealTimeRoute()
{
}

void CRealTimeRoute::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_REALTIME_PLOT, m_plotArea);
}


BEGIN_MESSAGE_MAP(CRealTimeRoute, CDialog)
    ON_WM_SIZE()
    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CRealTimeRoute message handlers

BOOL CRealTimeRoute::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect dlgrect;
    this->GetClientRect(&dlgrect);

    CRect rect;
    m_plotArea.GetClientRect(&rect);
    m_legendWidth = dlgrect.right - rect.right;

    m_range.maxLat = 10.0f;
    m_range.maxLon = 10.0f;
    m_range.minLat = -10.0f;
    m_range.minLon = -10.0f;

    m_gpsPlot.Create(WS_VISIBLE | WS_CHILD, rect, &m_plotArea);
    m_gpsPlot.parentWnd = &m_plotArea;
    m_gpsPlot.SetYUnits("Latitude");
    m_gpsPlot.SetXUnits("Longitude");
    m_gpsPlot.EnableGridLinesX(true);
    m_gpsPlot.EnableGridLinesY(true);
    m_gpsPlot.SetBackgroundColor(RGB(0, 0, 0));
    m_gpsPlot.SetGridColor(RGB(255, 255, 255));
    m_gpsPlot.SetAxisEqual();					// make sure that the latitude scale and the longitude scale are the same
    m_gpsPlot.SetMarginSpace(0.01f);			// Set some space around the graph
    m_gpsPlot.SetRange(m_range.minLat, m_range.maxLat, 2, m_range.minLon, m_range.maxLon, 2);

    DrawRouteGraph();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CRealTimeRoute::DrawRouteGraph()
{
    const int nofDecimals = 4;

    // Read the data from the CSpectrometer class
    ReadData();

    // set range
    m_gpsPlot.SetRange(m_range.minLat, m_range.maxLat, nofDecimals, m_range.minLon, m_range.maxLon, nofDecimals);

    // Draw route plot
    m_gpsPlot.DrawCircles(m_lon.data(), m_lat.data(), m_col.data(), m_pointNum);

    // draw min/max col label
    m_gpsPlot.SetPlotColor(RGB(255, 255, 255));
}

void CRealTimeRoute::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);
    if (nType != SIZE_RESTORED)
        return;

    if (IsWindow(m_plotArea.m_hWnd))
    {
        int legendSpace = 70;
        m_plotArea.MoveWindow(0, 0, cx - m_legendWidth, cy, FALSE);
        m_gpsPlot.MoveWindow(0, 0, cx - m_legendWidth, cy, FALSE);
        m_gpsPlot.CleanPlot();
    }

    DrawRouteGraph();
}

BOOL CRealTimeRoute::Create(UINT nID, CWnd* pParentWnd)
{
    // TODO: Add your specialized code here and/or call the base class

    fVisible = false;
    this->m_spectrometer = 0;

    return CDialog::Create(nID, pParentWnd);
}

void CRealTimeRoute::ReadData()
{
    if (nullptr == m_spectrometer)
        return;

    long sum = std::min(65535L, m_spectrometer->GetColumnNumber()); // limit the number here since the vectors aren't long enough...
    m_spectrometer->GetLatLongAlt(m_lat.data(), m_lon.data(), nullptr, sum);
    m_spectrometer->GetColumns(m_col, sum);
    m_spectrometer->GetIntensity(m_int, sum);

    memset(&m_range, 0, sizeof(struct plotRange));

    /* delete bad points (points without gps or dark points) */
    for (int i = 0; i < sum; ++i)
    {

        if ((m_lat[i] == 0 && m_lon[i] == 0))
        {
            for (int j = i; j < sum; ++j)
            {
                m_lat[j] = m_lat[j + 1];
                m_lon[j] = m_lon[j + 1];
                m_col[j] = m_col[j + 1];
            }
            --i;
            --sum;
        }
    }

    if (sum == 0)
        return;

    m_pointNum = sum;

    m_range.maxLat = m_lat[0];
    m_range.maxLon = m_lon[0];
    m_range.minLat = m_lat[0];
    m_range.minLon = m_lon[0];
    double maximumColumn = m_col[0];
    double minimumColumn = m_col[0];
    for (int i = 0; i < sum; i++)
    {
        m_range.maxLat = std::max(m_range.maxLat, m_lat[i]);
        m_range.maxLon = std::max(m_range.maxLon, m_lon[i]);
        m_range.minLat = std::min(m_range.minLat, m_lat[i]);
        m_range.minLon = std::min(m_range.minLon, m_lon[i]);
        if (m_col[i] > maximumColumn)
        {
            maximumColumn = m_col[i];
        }
        if (m_col[i] < minimumColumn)
        {
            minimumColumn = m_col[i];
        }
    }
    // update legend max/min col
    CString maxcol;
    maxcol.Format("%.1f", maximumColumn);
    this->SetDlgItemText(IDC_STATIC_MAXCOL, maxcol);
    CString mincol;
    mincol.Format("%.1f", minimumColumn);
    this->SetDlgItemText(IDC_STATIC_MINCOL, mincol);
    CString midcol;
    midcol.Format("%.1f", (maximumColumn + minimumColumn) / 2);
    this->SetDlgItemText(IDC_STATIC_MIDCOL, midcol);
}

void CRealTimeRoute::OnClose()
{
    fVisible = false;
    this->DestroyWindow();
    CDialog::OnClose();
}
