// RealTimeRoute.cpp : implementation file
#undef min
#undef max

#include "stdafx.h"
#include "DMSpec.h"
#include "RealTimeRoute.h"
#include <algorithm>

// CRealTimeRoute dialog

using namespace Dialogs;

IMPLEMENT_DYNAMIC(CRealTimeRoute, CDialog)
CRealTimeRoute::CRealTimeRoute(CWnd* pParent /*=NULL*/)
	: CDialog(CRealTimeRoute::IDD, pParent)
{
	m_pointNum = 0;
	fVisible = false;
	m_intensityLimit = 400;
}

CRealTimeRoute::~CRealTimeRoute()
{
}

void CRealTimeRoute::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRealTimeRoute, CDialog)
	ON_WM_SIZE()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CRealTimeRoute message handlers

BOOL CRealTimeRoute::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rect;
	this->GetClientRect(&rect);

	m_range.maxLat = 10.0f;
	m_range.maxLon = 10.0f;
	m_range.minLat = -10.0f;
	m_range.minLon = -10.0f;

	m_gpsPlot.Create(WS_VISIBLE | WS_CHILD, rect, this); 
	m_gpsPlot.SetYUnits("Latitude");
	m_gpsPlot.SetXUnits("Longitude");
	m_gpsPlot.EnableGridLinesX();
	m_gpsPlot.EnableGridLinesY();
	m_gpsPlot.SetBackgroundColor(RGB(0, 0, 0));
	m_gpsPlot.SetGridColor(RGB(255,255,255));
	m_gpsPlot.SetAxisEqual();					// make sure that the latitude scale and the longitude scale are the same
	m_gpsPlot.SetMarginSpace(0.01f);			// Set some space around the graph
	m_gpsPlot.SetRange(m_range.minLat, m_range.maxLat, 2, m_range.minLon, m_range.maxLon, 2);

	DrawRouteGraph();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRealTimeRoute::DrawRouteGraph(){

	// Read the data from the CSpectrometer class
	ReadData();

	// set range
	m_gpsPlot.SetRange(m_range.minLat, m_range.maxLat, 2, m_range.minLon, m_range.maxLon, 2);
	
	// Draw route plot
	m_gpsPlot.DrawCircles(m_lon, m_lat, m_col, m_pointNum);

	// draw min/max col label
	m_gpsPlot.SetPlotColor(RGB(255, 255, 255));
	CString maxcol;
	maxcol.Format("Max column: %d", (int)m_colmax);
	m_gpsPlot.DrawTextBox(65, 40, maxcol);
	CString mincol;
	mincol.Format("Min column: %d", (int)m_colmin);
	m_gpsPlot.DrawTextBox(65, 20, mincol);
}

void CRealTimeRoute::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if(nType != SIZE_RESTORED)
		return;

	if(IsWindow(m_gpsPlot.m_hWnd)){
		m_gpsPlot.MoveWindow(0, 0, cx, cy, FALSE);
		m_gpsPlot.CleanPlot();
	}

	DrawRouteGraph();
}

BOOL CRealTimeRoute::Create(UINT nID, CWnd* pParentWnd){
	// TODO: Add your specialized code here and/or call the base class

	fVisible = false;
	this->m_spectrometer = 0;

	return CDialog::Create(nID, pParentWnd);
}

void CRealTimeRoute::ReadData(){
	if(nullptr == m_spectrometer)
		return;

	int sum;

	sum = m_spectrometer->GetColumnNumber();
	m_spectrometer->GetLatLongAlt(m_lat, m_lon, NULL, sum);
	m_spectrometer->GetColumns(m_col, sum);
	m_spectrometer->GetIntensity(m_int, sum);

	memset(&m_range, 0, sizeof(struct plotRange));

	/* delete bad points (points without gps or dark points) */
	for(int i = 0; i < sum; ++i){

		if((m_lat[i] == 0 && m_lon[i] == 0)){
			for(int j = i; j < sum; ++j){
				m_lat[j] = m_lat[j + 1];
				m_lon[j] = m_lon[j + 1];
				m_col[j] = m_col[j + 1];
			}
			--i;
			--sum;
		}
	}

	if(sum == 0)
		return;

	m_pointNum = sum;

	m_range.maxLat = m_lat[0];
	m_range.maxLon = m_lon[0];
	m_range.minLat = m_lat[0];
	m_range.minLon = m_lon[0];
	m_colmax = m_col[0];
	m_colmin = m_col[0];
	for(int i = 0; i < sum; i++){
		//m_lat[i] = m_lat[i];
		//m_lon[i] = m_lon[i];	
		m_range.maxLat = std::max(m_range.maxLat,m_lat[i]);
		m_range.maxLon = std::max(m_range.maxLon,m_lon[i]);
		m_range.minLat = std::min(m_range.minLat,m_lat[i]);
		m_range.minLon = std::min(m_range.minLon,m_lon[i]);
		if (m_col[i] > m_colmax) {
			m_colmax = m_col[i];
		}
		if (m_col[i] < m_colmin) {
			m_colmin = m_col[i];
		}
	}
}

void CRealTimeRoute::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	fVisible = false;
	this->DestroyWindow();
	CDialog::OnClose();
}
