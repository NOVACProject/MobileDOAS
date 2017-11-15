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
	int margin = 0;
	rect.right -= margin;
	rect.bottom -= margin;
	rect.left = margin;
	rect.top = margin;

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
	m_gpsPlot.SetMarginSpace(0.05f);			// Set some space around the graph
	m_gpsPlot.SetRange(m_range.minLat, m_range.maxLat, 1, m_range.minLon, m_range.maxLon, 1);

	DrawRouteGraph();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRealTimeRoute::DrawRouteGraph(){

	// Read the data from the CSpectrometer class
	ReadData();

	/* draw the traverse */
	m_gpsPlot.SetPlotColor(RGB(255, 255, 255)) ;

	//  m_gpsPlot.DrawCirclesColor(m_lon, m_lat, m_col, (maxLat-minLat + 2*marginSpace),minLat-marginSpace,m_pointNum);
	m_gpsPlot.DrawCircles(m_lon, m_lat, m_pointNum);
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
	if(NULL == m_spectrometer)
		return;

	int i, j, sum;

	sum = m_spectrometer->GetColumnNumber();
	m_spectrometer->GetLatLongAlt(m_lat, m_lon, NULL, sum);
	m_spectrometer->GetColumns(m_col, sum);
	m_spectrometer->GetIntensity(m_int, sum);

	memset(&m_range, 0, sizeof(struct plotRange));

	/* delete bad points (points without gps or dark points) */
	for(i = 0; i < sum; ++i){

		if((m_lat[i] == 0 && m_lon[i] == 0)){
			for(j = i; j < sum; ++j){
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
	for(i = 0; i < sum; i++){
		m_lat[i] = m_lat[i];
		m_lon[i] = m_lon[i];	
		m_range.maxLat = std::max(m_range.maxLat,m_lat[i]);
		m_range.maxLon = std::max(m_range.maxLon,m_lon[i]);
		m_range.minLat = std::min(m_range.minLat,m_lat[i]);
		m_range.minLon = std::min(m_range.minLon,m_lon[i]);
	}
}

void CRealTimeRoute::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	fVisible = false;
	this->DestroyWindow();
	CDialog::OnClose();
}
