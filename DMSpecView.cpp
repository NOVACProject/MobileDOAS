// DMSpecView.cpp : implementation of the CDMSpecView class

#undef min
#undef max

#include "stdafx.h"
#include <afxdlgs.h>
#include "DMSpec.h"

#include "DMSpecDoc.h"
#include "DMSpecView.h"
#include "MainFrm.h"

#include "PostFluxDlg.h"
#include "Flux1.h"
#include "DualBeam/PostWindDlg.h"
#include "DualBeam/PostPlumeHeightDlg.h"

#include "CommentDlg.h"
#include "InformationDialog.h"
#include "Dialogs/SpectrumInspectionDlg.h"

#include "ReEvaluation\ReEvaluationDlg.h"

#include "Configuration/ConfigurationDialog.h"
#include "Configuration/Configure_Evaluation.h"
#include "Configuration/Configure_GPS.h"
#include "Configuration/Configure_Spectrometer.h"

#include "MeasurementModes/Measurement_Traverse.h"
#include "MeasurementModes/Measurement_Wind.h"
#include "MeasurementModes/Measurement_View.h"
#include "MeasurementModes/Measurement_Calibrate.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ReEvaluation;

#define LEFT 50
#define TOP 14
#define RIGHT 970
#define BOTTOM 520//560


CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

/////////////////////////////////////////////////////////////////////////////
// CDMSpecView

IMPLEMENT_DYNCREATE(CDMSpecView, CFormView)

BEGIN_MESSAGE_MAP(CDMSpecView, CFormView)
	// Menu commands
	// Starting and stopping the program
	ON_BN_CLICKED(IDC_BTNSTART,						OnControlStart) // <-- the toolbar button
	ON_BN_CLICKED(ID_CONTROL_STARTWINDMEASUREMENT,	OnControlStartWindMeasurement)
	ON_COMMAND(ID_CONTROL_STOP,						OnControlStop)

	// Just view the spectra from the spectrometer without evaluations
	ON_COMMAND(ID_CONTROL_VIEWSPECTRAFROMSPECTROMETER,	OnControlViewSpectra) // <-- view the output from the spectrometer

	// Calibrate a spectrometer
	ON_COMMAND(ID_CONTROL_CALIBRATESPECTROMETER,		OnControlCalibrateSpectrometer)

	ON_COMMAND(ID_ANALYSIS_POSTFLUX,					OnMenuShowPostFluxDialog)

	ON_COMMAND(ID_ANALYSIS_VIEWMEASUREDSPECTRA,			OnMenuShowSpectrumInspectionDialog)

	ON_COMMAND(ID_CONTROL_COUNTFLUX,					OnControlCountflux)

	// Changing the plot
	ON_COMMAND(ID_CONFIGURATION_PLOT_CHANGEBACKGROUND,			OnConfigurationPlotChangebackground)
	ON_COMMAND(ID_CONFIGURATION_PLOT_CHANGEPLOTCOLOR,			OnConfigurationPlotChangeplotcolor)
	ON_COMMAND(ID_CONFIGURATION_PLOT_CHANGEPLOTCOLOR_SLAVE,		OnConfigurationPlotChangeplotcolor_Slave)

	ON_COMMAND(ID_ANALYSIS_WINDSPEEDMEASUREMENT, OnMenuAnalysisWindSpeedMeasurement)


	ON_COMMAND(ID_CONFIGURATION_OPERATION,			OnConfigurationOperation)
	ON_MESSAGE(WM_DRAWCOLUMN,						OnDrawColumn)
	ON_MESSAGE(WM_STATUSMSG,						OnShowStatus)
	ON_MESSAGE(WM_READGPS,							OnReadGPS)
	ON_MESSAGE(WM_SHOWINTTIME,						OnShowIntTime)
	ON_MESSAGE(WM_CHANGEDSPEC,						OnChangeSpectrometer)
	ON_MESSAGE(WM_DRAWSPECTRUM,						OnDrawSpectrum)
	ON_MESSAGE(WM_CHANGEDSPECSCALE,					OnChangedSpectrumScale)
	ON_MESSAGE(WM_SHOWDIALOG,						OnShowInformationDialog)
	ON_COMMAND(ID_VIEW_REALTIMEROUTE,				OnViewRealtimeroute)
	ON_COMMAND(ID_VIEW_SPECTRUMFIT,					OnViewSpectrumFit)
	ON_UPDATE_COMMAND_UI(ID_VIEW_REALTIMEROUTE,		OnUpdateViewRealtimeroute)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SPECTRUMFIT,		OnUpdateViewSpectrumFit)
	ON_COMMAND(ID_CONTROL_ADDCOMMENT,				OnControlAddComment)
	ON_COMMAND(ID_CONTROL_REEVALUATE,				OnControlReevaluate)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_REEVALUATE,		OnUpdateControlReevaluate)
	ON_COMMAND(ID_CONFIGURATION_CHANGEEXPOSURETIME,	OnConfigurationChangeexposuretime)
	ON_WM_HELPINFO()
	ON_COMMAND(ID_ANALYSIS_PLUMEHEIGHTMEASUREMENT,	OnMenuAnalysisPlumeheightmeasurement)
	ON_COMMAND(ID_CONTROL_TESTTHEGPS,				OnMenuControlTestTheGPS)

	ON_COMMAND(ID_VIEW_COLUMNERROR,					OnViewColumnError)
	ON_UPDATE_COMMAND_UI(ID_VIEW_COLUMNERROR,		OnUpdateViewColumnError)
	
	ON_UPDATE_COMMAND_UI(ID_CONTROL_TESTTHEGPS,						OnUpdate_DisableOnRun)
	ON_UPDATE_COMMAND_UI(IDC_BTNSTART,								OnUpdate_DisableOnRun)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_STOP,							OnUpdate_EnableOnRun)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_VIEWSPECTRAFROMSPECTROMETER,	OnUpdate_DisableOnRun)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_STARTWINDMEASUREMENT,			OnUpdateWindMeasurement)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_ADDCOMMENT,						OnUpdate_EnableOnRun)
	ON_UPDATE_COMMAND_UI(ID_CONFIGURATION_CHANGEEXPOSURETIME,		OnUpdate_EnableOnRun)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_CALIBRATESPECTROMETER,			OnUpdate_CalibrateSpectrometer)
	
	ON_WM_CLOSE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDMSpecView construction/destruction
BOOL fRunSpec;
CDMSpecView::CDMSpecView()
	: CFormView(CDMSpecView::IDD)
{
	m_WindDirection = 0.0;
	m_WindSpeed = 8.0;
	pView       = this;
	fRunSpec    = FALSE;
	int i;
	for(i = 0; i < 200; i++)
	{
		number[i]		=  i;
	}
	for(i = 0; i < MAX_SPECTRUM_LENGTH; i++)
	{
		number2[i] =  (200.0 * i) / MAX_SPECTRUM_LENGTH;
	}
	number2Length = MAX_SPECTRUM_LENGTH;

	m_Spectrometer = NULL;
	m_showErrorBar = FALSE;
	
	m_minSaturationRatio = 0.0;
	m_maxSaturationRatio = 100.0;
}

CDMSpecView::~CDMSpecView()
{
	if(m_Spectrometer != NULL){
		delete(m_Spectrometer);
		m_Spectrometer = NULL;
	}
}

void CDMSpecView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BASEEDIT, m_BaseEdit);
	DDX_Text(pDX, IDC_WINDDIRECTION, m_WindDirection);
	DDV_MinMaxDouble(pDX, m_WindDirection, 0., 360.);
	DDX_Text(pDX, IDC_WINDSPEED, m_WindSpeed);
	DDV_MinMaxDouble(pDX, m_WindSpeed, 0., 1000000.);

	DDX_Control(pDX, IDC_SIGNALLIMIT_SLIDER, m_intensitySliderLow);

	// The GPS-Labels
	DDX_Control(pDX,	IDC_LAT,		m_gpsLatLabel);
	DDX_Control(pDX,	IDC_LON,		m_gpsLonLabel);
	DDX_Control(pDX,	IDC_GPSTIME,	m_gpsTimeLabel);
	DDX_Control(pDX,	IDC_NGPSSAT,	m_gpsNSatLabel);

	// The legend
	DDX_Control(pDX,	IDC_LABEL_COLOR_SPECTRUM,		m_colorLabelSpectrum1);
	DDX_Control(pDX,	IDC_LABEL_COLOR_SPECTRUM2,		m_colorLabelSpectrum2);
	DDX_Control(pDX,	IDC_LABEL_COLOR_SERIES1,		m_colorLabelSeries1);
	DDX_Control(pDX,	IDC_LABEL_COLOR_SERIES2,		m_colorLabelSeries2);
	DDX_Control(pDX,	IDC_LABEL_SPECTRUM,				m_legendSpectrum1);
	DDX_Control(pDX,	IDC_LABEL_SPECTRUM2,			m_legendSpectrum2);
	DDX_Control(pDX,	IDC_LABEL_SERIES1,				m_legendSeries1);
	DDX_Control(pDX,	IDC_LABEL_SERIES2,				m_legendSeries2);
}

BOOL CDMSpecView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CDMSpecView::OnInitialUpdate()
{
	CRect rect;

	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	/* get the path to the program */
	Common common;
	common.GetExePath();
	g_exePath.Format(common.m_exePath);

	// The size of the graph
	int left	= 50;
	int top		= 14;
	int right	= 970;
	int bottom = 520;

	// Get the resolution of the screen
	int cx	= GetSystemMetrics(SM_CXSCREEN);
	
	// rescale the graph to fit the width of the window
	right	= cx * right / 1024;
	
	// Also move the slider to the right of the window
	m_intensitySliderLow.GetWindowRect(rect);
	rect.left  = rect.left * cx / 1024;
	rect.right = rect.right * cx / 1024;
	this->ScreenToClient(rect);
	m_intensitySliderLow.MoveWindow(rect);

	rect = CRect(LEFT,TOP,right,BOTTOM);
	m_ColumnPlot.Create(WS_VISIBLE | WS_CHILD, rect, this) ; 

	// customize the control
	m_columnLimit = 100.0;
	m_PlotColor[0] = RGB(255, 0, 0);
	m_PlotColor[1] = RGB(0, 0, 255);
	m_ColumnPlot.SetSecondYUnit(TEXT("Intensity [%]"));
	m_ColumnPlot.SetYUnits("Column [ppmm]");
	m_ColumnPlot.SetXUnits("Number");
	m_ColumnPlot.EnableGridLinesX(false);
	m_ColumnPlot.SetBackgroundColor(RGB(0, 0, 0)) ;
	m_ColumnPlot.SetGridColor(RGB(255,255,255));
	m_ColumnPlot.SetPlotColor(m_PlotColor[0]);
	m_ColumnPlot.SetRange(0,200,1,0.0,100.0,1);
	m_ColumnPlot.SetMinimumRangeX(200.0f);
	m_ColumnPlot.SetSecondRange(0.0, 200, 0, 0.0, 100.0, 0);

	m_BaseEdit.LimitText(99);
	ReadMobileLog();

	/* the intensity slider */
	m_intensitySliderLow.SetRange(0, 100); /** The scale of the intensity slider is in percent */
	m_intensitySliderLow.SetPos(100 - 25); /* The slider is upside down - i.e. the real value is "100 - m_intensitySlider.GetPos()"*/
	m_intensitySliderLow.SetTicFreq(25);

	/* The colors for the spectrum plots */
	m_Spectrum0Color    = RGB(0, 255, 0);
	m_Spectrum0FitColor = RGB(0, 150, 0);
	m_Spectrum1Color    = RGB(255, 0, 255);
	m_Spectrum1FitColor = RGB(150, 0, 150);
	m_SpectrumLineWidth = 1;

	// Fix the legend
	UpdateLegend();
}

/////////////////////////////////////////////////////////////////////////////
// CDMSpecView diagnostics

#ifdef _DEBUG
void CDMSpecView::AssertValid() const
{
	CFormView::AssertValid();
}

void CDMSpecView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CDMSpecDoc* CDMSpecView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDMSpecDoc)));
	return (CDMSpecDoc*)m_pDocument;
}


#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDMSpecView message handlers
LRESULT CDMSpecView::OnDrawColumn(WPARAM wParam, LPARAM lParam){
	double column[2][5000];		// the evaluated columns in each of the fit-regions
	double columnErr[2][5000];	// the error bars of the evaluated columns in each of the fit-regions
	double intensity[2][5000];
	long i, size;
	double maxColumn,minColumn,lowLimit;
	CString cCon;		// the concentration str
	CString cShift;		//shift str
	CString cSqueeze;	//squeeze str
	CString cScanNo;	//scanned spectra number after sky and dark spectra
	long scanNo;
	double* result;
	long	dynRange;
	
	// if the program is no longer running, then don't try to draw anything more...
	if(!fRunSpec){
		return 0;
	}
	
	dynRange = m_Spectrometer->m_spectrometerDynRange;
	int intensityLimit = (100 - m_intensitySliderLow.GetPos());

	// Reset, is this really necessary??
	memset((void*)column,		0, sizeof(double)*10000);
	memset((void*)columnErr,	0, sizeof(double)*10000);
	memset((void*)intensity,	0, sizeof(double)*10000);
	
	// Get the last value and the total number of values
	result = m_Spectrometer->GetLastColumn();
	scanNo = m_Spectrometer->scanNum - 2;

	// Get the number of channels used and the number of fit-regions used
	int nChannels		= m_Spectrometer->m_NChannels;
	int fitRegionNum	= m_Spectrometer->GetFitRegionNum();

	// --- Update the concentration, shift and squeeze ---
	cCon.Format("%.2f ± %.2f",result[0], result[1]);
	cShift.Format("%.1f",result[2]);
	cSqueeze.Format("%.1f", result[4]);
	cScanNo.Format("%d",scanNo);
	this->SetDlgItemText(IDC_CONCENTRATION,cCon);
	this->SetDlgItemText(IDC_SH,cShift);
	this->SetDlgItemText(IDC_SQ,cSqueeze);
	this->SetDlgItemText(IDC_SCANNO,cScanNo);


	// --- Get the data ---
	size = std::min(long(199), m_Spectrometer->GetColumnNumber());
	m_Spectrometer->GetIntensity(intensity[0], size);
	m_Spectrometer->GetColumns(column[0], size, 0);
	m_Spectrometer->GetColumnErrors(columnErr[0], size, 0);
	if(fitRegionNum > 1){
		m_Spectrometer->GetColumns(column[1], size, 1);
		m_Spectrometer->GetColumnErrors(columnErr[1], size, 1);
	}

	// -- Convert the intensity to saturation ratio
	for(int k = 0; k < size; ++k)
		intensity[0][k] = intensity[0][k] * 100.0 / dynRange;

	maxColumn = 0.0;
	minColumn = 0.0;

	// -- Get the limits for the data ---
	for(i = 0; i < size; i++){
		if(intensity[0][i] > intensityLimit){
			maxColumn = std::max(maxColumn, fabs(column[0][i]));
			minColumn = std::min(minColumn, column[0][i]);
			if(fitRegionNum > 1){
				maxColumn = std::max(maxColumn, fabs(column[1][i]));
				minColumn = std::min(minColumn, column[1][i]);
			}
		}
	}
	lowLimit     = (-1.25)*fabs(minColumn);
	m_columnLimit = 1.25*maxColumn;

	if(m_columnLimit == 0)
		m_columnLimit = 0.1;

	// Set the range for the plot
	m_ColumnPlot.SetRange(0.0, 199.0, 0,  lowLimit, m_columnLimit, 1);
	m_ColumnPlot.SetSecondRange(0.0, 200, 0, m_minSaturationRatio, m_maxSaturationRatio, 0);

	// Draw the columns (don't change the scale again here...)
	if(m_spectrometerMode == MODE_TRAVERSE){
		if(fitRegionNum == 1){
			m_ColumnPlot.SetPlotColor(m_PlotColor[0]);
			if(m_showErrorBar)
				m_ColumnPlot.BarChart(number, column[0], columnErr[0], size, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
			else
				m_ColumnPlot.BarChart(number, column[0], size, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
		}else{
			m_ColumnPlot.SetPlotColor(m_PlotColor[0]);
			if(m_showErrorBar)
				m_ColumnPlot.BarChart2(number, column[0], column[1], columnErr[0], columnErr[1], m_PlotColor[1], size, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
			else
				m_ColumnPlot.BarChart2(number, column[0], column[1], m_PlotColor[1], size, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
		}
	}else if(m_spectrometerMode == MODE_WIND){
		if(m_showErrorBar){
			m_ColumnPlot.SetPlotColor(m_PlotColor[0]);
			m_ColumnPlot.XYPlot(number, column[0], NULL, NULL, columnErr[0], size, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);

			m_ColumnPlot.SetPlotColor(m_PlotColor[1]);
			m_ColumnPlot.XYPlot(number, column[1], NULL, NULL, columnErr[1], size, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);
		}else{
			m_ColumnPlot.SetPlotColor(m_PlotColor[0]);
			m_ColumnPlot.XYPlot(number, column[0], size, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);

			m_ColumnPlot.SetPlotColor(m_PlotColor[1]);
			m_ColumnPlot.XYPlot(number, column[1], size, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);
		}
	}

	// Draw the intensities
	m_ColumnPlot.DrawCircles(number, intensity[0], size, Graph::CGraphCtrl::PLOT_SECOND_AXIS);

	// Draw the spectrum
	DrawSpectrum();


	if(m_realTimeRouteGraph.fVisible){
		m_realTimeRouteGraph.m_intensityLimit = dynRange * (100 - m_intensitySliderLow.GetPos());
		m_realTimeRouteGraph.DrawRouteGraph();
	}
	if(m_showFitDlg.fVisible){
		m_showFitDlg.DrawFit();
	}

	return 0;
}
LRESULT CDMSpecView::OnDrawSpectrum(WPARAM wParam,LPARAM lParam)
{
	// to not overload the computer, make sure that we don't draw too often...
	static double refreshRate = (m_spectrometerMode == MODE_CALIBRATE) ? 0.2 : 0.05;
	static clock_t cLastCall = 0;
	clock_t now = clock();
	double elapsedTime = (double)(now - cLastCall) / (double)CLOCKS_PER_SEC;
	if(elapsedTime < refreshRate){
		return 0;		
	}
	cLastCall = now;

	m_ColumnPlot.CleanPlot();
	
	// set the ranges for the plot
	if(m_spectrometerMode == MODE_VIEW || m_spectrometerMode == MODE_CALIBRATE){
		m_ColumnPlot.SetRange(0.0, 2048, 0, m_minSaturationRatio, m_maxSaturationRatio, 0);
	}else{
		m_ColumnPlot.SetSecondRange(0.0, 200, 0, m_minSaturationRatio, m_maxSaturationRatio, 0);
	}
	
	DrawSpectrum();
	
	// also update the integration time
	OnShowIntTime(wParam, lParam);
	
	return 0;
}

/** Changes the saturation-ratio scale of the spectrum-view */
LRESULT CDMSpecView::OnChangedSpectrumScale(WPARAM wParam,LPARAM lParam){
	this->m_minSaturationRatio = (int)wParam;
	this->m_maxSaturationRatio = (int)lParam;
	
	return OnDrawSpectrum(wParam, lParam);
}

LRESULT CDMSpecView::OnShowIntTime(WPARAM wParam, LPARAM lParam){
	CString expTime, nAverage;
	int avg[2];

	// if the program is no longer running, then don't try to draw anything more...
	if(!fRunSpec){
		return 0;
	}

	expTime.Format("%d  ms",m_Spectrometer->RequestIntTime());
	this->SetDlgItemText(IDC_INTTIME,expTime);

	m_Spectrometer->GetNSpecAverage(avg);
	nAverage.Format("%dx%d", avg[0], avg[1]);
	this->SetDlgItemText(IDC_SPECNO, nAverage);

	// Update the legend
	UpdateLegend();

	// forward the message to the spectrum-settings dialog(if any);
	if(this->m_specSettingsDlg.m_hWnd != NULL){
		m_specSettingsDlg.PostMessage(WM_SHOWINTTIME);
	}

	return 0;
}

LRESULT CDMSpecView::OnChangeSpectrometer(WPARAM wParam, LPARAM lParam){
	// forward the message to the spectrum-settings dialog(if any);
	if(this->m_specSettingsDlg.m_hWnd != NULL){
		m_specSettingsDlg.PostMessage(WM_CHANGEDSPEC);
	}
	return 0;
}

void CDMSpecView::OnMenuShowPostFluxDialog() 
{
	m_fluxDlg = new CPostFluxDlg;
	m_fluxDlg->DoModal();

	delete(m_fluxDlg);
}

void CDMSpecView::OnMenuShowSpectrumInspectionDialog(){
	Dialogs::CSpectrumInspectionDlg *dlg  = new Dialogs::CSpectrumInspectionDlg();
	dlg->DoModal();

	delete dlg;
}

/** This function is the thread function to start running spectrometer
**
*/
UINT CollectSpectra(LPVOID pParam){
	CSpectrometer* spec = (CSpectrometer*)pParam ;
	spec->Run();
	fRunSpec = FALSE;
	return 0;
}

/** This function is the thread function to start running spectrometer in
			wind-measurement mode
*/
UINT CollectSpectra_Wind(LPVOID pParam){
	CSpectrometer* spec = (CSpectrometer*)pParam ;
	spec->Run();
	fRunSpec	= FALSE;
	return 0;
}

LRESULT CDMSpecView::OnShowStatus(WPARAM wParam, LPARAM lParam)
{
	if(fRunSpec){
		CString str;
		str = m_Spectrometer->m_statusMsg;
		ShowStatusMsg(str);
	}
	return 0;
}

LRESULT CDMSpecView::OnReadGPS(WPARAM wParam, LPARAM lParam)
{
	double data[5];
	static int latNSat = 10;
	
	// if the program is no longer running, then don't try to draw anything more...
	if(!fRunSpec){
		return 0;
	}
	
	m_Spectrometer->GetGPSPos(data);

	CString lat,lon,tim,strHr,strMin,strSec, nSat;
	int hr,min,sec;
	hr = (long)data[2]/10000;
	min = ((long)data[2] - hr*10000)/100;
	sec = (long)data[2]%100;

	if(data[0]>=0.0)
		lat.Format("%f  degree N",data[0]);
	else 
		lat.Format("%f  degree S",-1.0*data[0]);

	if(data[1] >= 0.0)
		lon.Format("%f  degree E",data[1]);
	else
		lon.Format("%f  degree W",-1.0*data[1]);

	if(hr<10)
		strHr.Format("0%d:",hr);
	else
		strHr.Format("%d:",hr);

	if(min<10)
		strMin.Format("0%d:",min);
	else
		strMin.Format("%d:",min);

	if(sec<10)
		strSec.Format("0%d",sec);
	else
		strSec.Format("%d",sec);

	nSat.Format("%d", (long)data[4]);

	tim = strHr + strMin + strSec;
	this->SetDlgItemText(IDC_GPSTIME, tim);
	this->SetDlgItemText(IDC_LAT, lat);
	this->SetDlgItemText(IDC_LON, lon);
	this->SetDlgItemText(IDC_NGPSSAT, nSat);

	if(latNSat == 0 && data[4] != 0){
		COLORREF normal = RGB(236, 233, 216);

		// Set the background color to normal
		m_gpsLatLabel.SetBackgroundColor(normal);
		m_gpsLonLabel.SetBackgroundColor(normal);
		m_gpsTimeLabel.SetBackgroundColor(normal);
		m_gpsNSatLabel.SetBackgroundColor(normal);

	}else if(latNSat != 0 && data[4] == 0){
		COLORREF warning = RGB(255, 75, 75);

		// Set the background color to red
		m_gpsLatLabel.SetBackgroundColor(warning);
		m_gpsLonLabel.SetBackgroundColor(warning);
		m_gpsTimeLabel.SetBackgroundColor(warning);
		m_gpsNSatLabel.SetBackgroundColor(warning);
	}

	// Remember the number of satelites
	latNSat = (int)data[4];

	return 0;
}



void CDMSpecView::ShowStatusMsg(CString &str)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	CStatusBar* pStatus = &pFrame->m_wndStatusBar;
	if(pStatus)
	{
		pStatus->SetPaneText(0,str);
	
	}
}


void CDMSpecView::OnControlCountflux() {
	double flux;
	CString str;
	if(fRunSpec){
		flux = m_Spectrometer->GetFlux();
		m_Spectrometer->WriteFluxLog();
		str.Format("By now the flux is %f",flux);
		MessageBox(str,"Flux",MB_OK);
	}
	else
		MessageBox(TEXT("The spectrometer hasn't been started.\nStart it first,\nthen you can use this function")
		,"Notice",MB_OK);
}

void CDMSpecView::OnConfigurationPlotChangebackground(){
	CColorDialog dlg;

	if(dlg.DoModal() == IDOK){
		m_bkColor = dlg.m_cc.rgbResult;
		m_ColumnPlot.SetBackgroundColor(m_bkColor) ;
	}
}

void CDMSpecView::OnConfigurationPlotChangeplotcolor(){
	CColorDialog dlg;
	if(dlg.DoModal()==IDOK)
	{
		m_PlotColor[0] = dlg.m_cc.rgbResult;
		m_ColumnPlot.SetPlotColor(m_PlotColor[0]);
	}

	// Update the legend
	UpdateLegend();
}

void CDMSpecView::OnConfigurationPlotChangeplotcolor_Slave(){
	CColorDialog dlg;
	if(dlg.DoModal()==IDOK)
	{
		m_PlotColor[1] = dlg.m_cc.rgbResult;
	}

	// Update the legend
	UpdateLegend();
}

void CDMSpecView::OnControlStart() 
{
	char text[100];
	CString tmpStr;

	if(!fRunSpec){
		/* Check that the base name does not contain any illegal characters */
		this->GetDlgItemText(IDC_BASEEDIT, tmpStr);
		if(-1 != tmpStr.FindOneOf("\\/:*?\"<>|")){
			tmpStr.Format("The base name is not allowed to contain any of the following characters: \\ / : * ? \" < > | Please choose another basename and try again.");
			MessageBox(tmpStr, "Error", MB_OK);
			return;
		}

		// Initialize a new CSpectromber object, this is the one
		//  which actually does everything...
		m_Spectrometer = new CMeasurement_Traverse();

		// Copy the settings that the user typed in the dialog
		memset(text,0,(size_t)100);
		if(UpdateData(TRUE)){
			m_BaseEdit.GetWindowText(text,255);
			m_Spectrometer->SetUserParameters(m_WindSpeed, m_WindDirection, text);
		}

		// Start the measurement thread
		pSpecThread			= AfxBeginThread(CollectSpectra,(LPVOID)(m_Spectrometer),THREAD_PRIORITY_NORMAL,0,0,NULL);
		fRunSpec			= TRUE;
		m_spectrometerMode	= MODE_TRAVERSE;

		// If the user wants to see the real-time route then initialize it also
		if(m_realTimeRouteGraph.fVisible){
			m_realTimeRouteGraph.m_spectrometer = m_Spectrometer;
			m_realTimeRouteGraph.DrawRouteGraph();
		}
		if(m_showFitDlg.fVisible){
			m_showFitDlg.m_spectrometer = m_Spectrometer;
			m_showFitDlg.DrawFit();
		}
	}else{
		MessageBox(TEXT("Spectra are collecting"),"Notice",MB_OK);
	}
}

/** Starts the viewing of spectra from the spectrometer, 
		without saving or evaluating them. */
void CDMSpecView::OnControlViewSpectra(){
	char text[100];
	CString tmpStr;
	CRect rect;

	if(!fRunSpec){
		CDMSpecDoc* pDoc = GetDocument();
		CMeasurement_View *spec = new CMeasurement_View();
		this->m_Spectrometer = (CSpectrometer *)spec;
		m_Spectrometer->m_spectrometerMode = MODE_VIEW;
		memset(text,0,(size_t)100);

		pSpecThread			= AfxBeginThread(CollectSpectra,(LPVOID)(m_Spectrometer),THREAD_PRIORITY_LOWEST,0,0,NULL);
		fRunSpec			= TRUE;
		m_spectrometerMode	= MODE_VIEW;

		// Show the window that makes it possible to change the exposure time
		m_specSettingsDlg.m_Spectrometer = spec;
		m_specSettingsDlg.Create(IDD_SPECTRUM_SETTINGS_DLG, this);
		m_specSettingsDlg.ShowWindow(SW_SHOW);

		// Show the window that makes it possible to change the spectrum-scale 
		m_specScaleDlg.Create(IDD_SPECTRUM_SCALE_DLG, this);
		m_specScaleDlg.ShowWindow(SW_SHOW);
		m_specScaleDlg.GetWindowRect(rect);
		int width = rect.Width();
		int cx	= GetSystemMetrics(SM_CXSCREEN); // the width of the screen
		rect.right = cx - 10;
		rect.left = rect.right - width;
		m_specScaleDlg.MoveWindow(rect);
		
		if(m_realTimeRouteGraph.fVisible){
			m_realTimeRouteGraph.m_spectrometer = m_Spectrometer;
			m_realTimeRouteGraph.DrawRouteGraph();
		}
		if(m_showFitDlg.fVisible){
			m_showFitDlg.m_spectrometer = m_Spectrometer;
			m_showFitDlg.DrawFit();
		}
		
		// Also set the column-plot to only show the measured spectrum
		m_ColumnPlot.SetSecondYUnit("");
		m_ColumnPlot.SetYUnits("Intensity [%]");
		m_ColumnPlot.SetXUnits("Pixel");
		m_ColumnPlot.EnableGridLinesX(false);
		m_ColumnPlot.SetRange(0.0, 2048, 0, 0.0, 100.0, 0);		
	}else{
		MessageBox(TEXT("Spectra are collecting"),"Notice",MB_OK);
	}
}


/** Starts the viewing of spectra from the spectrometer, 
		without saving or evaluating them. */
void CDMSpecView::OnControlCalibrateSpectrometer(){
	char text[100];
	CString tmpStr;
	CRect rect;

	if(!fRunSpec){
		CDMSpecDoc* pDoc = GetDocument();
		CMeasurement_Calibrate *spec = new CMeasurement_Calibrate();
		this->m_calibration	= spec->m_calibration;
		this->m_Spectrometer = (CSpectrometer *)spec;
		m_Spectrometer->m_spectrometerMode = MODE_CALIBRATE;
		memset(text,0,(size_t)100);

		pSpecThread			= AfxBeginThread(CollectSpectra,(LPVOID)(m_Spectrometer),THREAD_PRIORITY_LOWEST,0,0,NULL);
		fRunSpec			= TRUE;
		m_spectrometerMode	= MODE_CALIBRATE;

		// Show the window that makes it possible to change the exposure time
		m_specSettingsDlg.m_Spectrometer = spec;
		m_specSettingsDlg.Create(IDD_SPECTRUM_SETTINGS_DLG, this);
		m_specSettingsDlg.ShowWindow(SW_SHOW);

		// Show the window that makes the calibration
		m_specCalibrationDlg.m_Spectrometer = spec;
		this->m_calibration	= spec->m_calibration;
		m_specCalibrationDlg.Create(IDD_SPECTRUM_CALIBRATION, this);
		m_specCalibrationDlg.ShowWindow(SW_SHOW);
		m_specCalibrationDlg.GetWindowRect(rect);
		int width = rect.Width();
		int cx	= GetSystemMetrics(SM_CXSCREEN); // the width of the screen
		rect.right = cx - 10;
		rect.left = rect.right - width;
		m_specCalibrationDlg.MoveWindow(rect);

		if(m_realTimeRouteGraph.fVisible){
			m_realTimeRouteGraph.m_spectrometer = m_Spectrometer;
			m_realTimeRouteGraph.DrawRouteGraph();
		}
		if(m_showFitDlg.fVisible){
			m_showFitDlg.m_spectrometer = m_Spectrometer;
			m_showFitDlg.DrawFit();
		}

		// Also set the column-plot to only show the measured spectrum
		m_ColumnPlot.SetSecondYUnit("");
		m_ColumnPlot.SetYUnits("Intensity [%]");
		m_ColumnPlot.SetXUnits("Pixel");
		m_ColumnPlot.EnableGridLinesX(false);
		m_ColumnPlot.SetRange(0.0, 2048, 0, 0.0, 100.0, 0);
	}else{
		MessageBox(TEXT("Spectra are collecting"),"Notice",MB_OK);
	}
}


void CDMSpecView::OnControlStartWindMeasurement() 
{
	char text[100];
	CString tmpStr;

	if(!fRunSpec){
		/* Check that the base name does not contain any illegal characters */
		this->GetDlgItemText(IDC_BASEEDIT, tmpStr);
		if(-1 != tmpStr.FindOneOf("\\/:*?\"<>|")){
			tmpStr.Format("The base name is not allowed to contain any of the following characters: \\ / : * ? \" < > | Please choose another basename and try again.");
			MessageBox(tmpStr, "Error", MB_OK);
			return;
		}

		// Initialize a new CSpectromber object, this is the one
		//  which actually does everything...
		m_Spectrometer	= new CMeasurement_Wind();

		// Set the measurement mode to wind-speed measurement
		m_Spectrometer->m_spectrometerMode = MODE_WIND;

		// Copy the settings that the user typed in the dialog
		memset(text,0,(size_t)100);
		if(UpdateData(TRUE)){
			m_BaseEdit.GetWindowText(text,255);
			m_Spectrometer->SetUserParameters(m_WindSpeed, m_WindDirection, text);
		}

		// Start the measurement thread
		pSpecThread			= AfxBeginThread(CollectSpectra_Wind,(LPVOID)(m_Spectrometer),THREAD_PRIORITY_NORMAL,0,0,NULL);
		fRunSpec			= TRUE;
		m_spectrometerMode	= MODE_WIND;

		// If the user wants to see the real-time route then initialize it also
		if(m_realTimeRouteGraph.fVisible){
			m_realTimeRouteGraph.m_spectrometer = m_Spectrometer;
			m_realTimeRouteGraph.DrawRouteGraph();
		}
		if(m_showFitDlg.fVisible){
		m_showFitDlg.m_spectrometer = m_Spectrometer;
		m_showFitDlg.DrawFit();
		}
	}else{
		MessageBox(TEXT("Spectra are collecting"),"Notice",MB_OK);
	}
}

void CDMSpecView::OnControlStop() 
{
	if(fRunSpec)
	{
		fRunSpec = FALSE;

		DWORD dwExitCode;
		HANDLE hThread = pSpecThread->m_hThread;
		if(hThread !=NULL && GetExitCodeThread(hThread, &dwExitCode) && dwExitCode ==STILL_ACTIVE)
		{
			AfxGetApp()->BeginWaitCursor();
			m_Spectrometer->m_wrapper.stopAveraging(m_Spectrometer->m_spectrometerIndex);
			m_Spectrometer->Stop();
			Sleep(500);			
			WaitForSingleObject(hThread,INFINITE);
			m_Spectrometer->serial.Close();
			AfxGetApp()->EndWaitCursor();
			MessageBox(TEXT("Spectrum collection has been stopped"),NULL,MB_OK);
		}
	}
}

void CDMSpecView::DrawSpectrum()
{
	static double *spectrum1 = new double[MAX_SPECTRUM_LENGTH];
	static double *spectrum2 = new double[MAX_SPECTRUM_LENGTH];
	CString lambdaStr;
	
	// if the program is no longer running, then don't try to draw anything more...
	if(!fRunSpec){
		return;
	}
	
	// Get the length of the spectrum
	int spectrumLength = m_Spectrometer->m_detectorSize;

	// Get the maximum intensity of the spectrometer
	double	dynRange_inv = 100.0 / (double)m_Spectrometer->m_spectrometerDynRange;

	// If the length of the 'number2' - parameter does not 
	//	agree with the size of the spectrometer - detector
	if(spectrumLength != number2Length){
		for(int i = 0; i < spectrumLength; i++)
			number2[i] =  (200.0 * i) / spectrumLength;
		number2Length = spectrumLength;
	}

	// Copy the spectrum and transform it into saturation-ratio
	memcpy(spectrum1, m_Spectrometer->GetSpectrum(0), sizeof(double)*spectrumLength);
	for(int k = 0; k < spectrumLength; ++k)
		spectrum1[k] *= dynRange_inv;

	// if we're using a normal measurement mode...
	if(m_spectrometerMode == MODE_TRAVERSE || m_spectrometerMode == MODE_WIND){

		// Plot the spectrum
		m_ColumnPlot.SetPlotColor(m_Spectrum0Color);
		m_ColumnPlot.XYPlot(number2, spectrum1, spectrumLength, Graph::CGraphCtrl::PLOT_SECOND_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);

		// If a second channel is used, then do the same thing with the slave-spectrum
		if(m_Spectrometer->m_NChannels == 1){
			return;
		}else{
			memcpy(spectrum2, m_Spectrometer->GetSpectrum(1), sizeof(double)*spectrumLength);
			for(int k = 0; k < spectrumLength; ++k)
				spectrum2[k] *= dynRange_inv;

			m_ColumnPlot.SetPlotColor(m_Spectrum1Color);
			m_ColumnPlot.SetLineWidth(2);
			m_ColumnPlot.XYPlot(number2, spectrum2, spectrumLength, Graph::CGraphCtrl::PLOT_SECOND_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);
		}
		
		return;
	}else if(m_spectrometerMode == MODE_VIEW){
		// Plot the spectrum
		m_ColumnPlot.SetPlotColor(m_Spectrum0Color);
		m_ColumnPlot.XYPlot(NULL, spectrum1, spectrumLength, Graph::CGraphCtrl::PLOT_CONNECTED);

		// If a second channel is used, then do the same thing with the slave-spectrum
		if(m_Spectrometer->m_NChannels == 1){
			return;
		}else{
			// Get the second spectrum
			memcpy(spectrum2, m_Spectrometer->GetSpectrum(1), sizeof(double)*spectrumLength);
			for(int k = 0; k < spectrumLength; ++k)
				spectrum2[k] *= dynRange_inv;

			m_ColumnPlot.SetPlotColor(m_Spectrum1Color);
			m_ColumnPlot.SetLineWidth(2);
			m_ColumnPlot.XYPlot(NULL, spectrum2, spectrumLength, Graph::CGraphCtrl::PLOT_CONNECTED);
		}
		return;
	}else if(m_spectrometerMode == MODE_CALIBRATE){
		
		// Plot the spectrum
		m_ColumnPlot.SetPlotColor(m_Spectrum0Color);
		m_ColumnPlot.XYPlot(NULL, spectrum1, spectrumLength, Graph::CGraphCtrl::PLOT_CONNECTED);

		// Plot the wavelengths of each of the lines
		if(m_calibration != NULL){
			for(unsigned int k = 0; k < m_calibration->m_lineNum; ++k){
				lambdaStr.Format("%.1lf", m_calibration->m_lines[k].wavelength);
				m_ColumnPlot.DrawLine(Graph::VERTICAL, m_calibration->m_lines[k].pixelNumber, RGB(255, 255, 0), Graph::STYLE_DASHED, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
				m_ColumnPlot.DrawTextBox(m_calibration->m_lines[k].pixelNumber, m_calibration->m_lines[k].pixelNumber + 50, m_calibration->m_lines[k].maxIntensity, m_calibration->m_lines[k].maxIntensity + 20, lambdaStr);
			}
		}

		return;
	}
}

void CDMSpecView::OnConfigurationOperation() 
{
	/** The new configuration dialog */

	CString titles[2];
	CString cfgFile; // <-- the path and filename of the configurationfile to read-in
	cfgFile.Format("%s\\cfg.xml", g_exePath);
	if(!IsExistingFile(cfgFile)){
		cfgFile.Format("%s\\cfg.txt", g_exePath);
	}

	// Initiate the configuration-object
	Configuration::CMobileConfiguration	*configuration = new Configuration::CMobileConfiguration(cfgFile);

	// Initiate the configuration-dialog itself
	Configuration::CConfigurationDialog	confDlg;
	confDlg.Construct("Configure", this, 0);

	// Initiate the pages in the configuration dialog
	Configuration::CConfigure_Spectrometer m_specPage;
	m_specPage.Construct(IDD_CONFIGURE_SPECTROMETER);
	m_specPage.m_conf	= configuration;

	Configuration::CConfigure_GPS	m_gpsPage;
	m_gpsPage.Construct(IDD_CONFIGURE_GPS);
	m_gpsPage.m_conf	= configuration;

	Configuration::CConfigure_Evaluation m_EvalPage;
	m_EvalPage.Construct(IDD_CONFIGURE_EVALUATION);
	m_EvalPage.m_conf		= configuration;

	// Add the pages once they have been constructed
	confDlg.AddPage(&m_specPage);
	confDlg.AddPage(&m_gpsPage);
	confDlg.AddPage(&m_EvalPage);

	// Open the configuration dialog
	confDlg.DoModal();


	// Clean up
	delete configuration;
}

BOOL CDMSpecView::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	::WinHelp((HWND)pHelpInfo->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTENTS,NULL);
	return CFormView::OnHelpInfo(pHelpInfo);
}

void CDMSpecView::ReadMobileLog(){
	char baseNameTxt[256];
	char txt[256];
	char *pt = 0;
	int i, L, d;
	char m_Base[256];
	bool fFoundBaseName = false;

	FILE *f = fopen(g_exePath + "MobileLog.txt", "r");
	if(0 != f){
		while(fgets(txt, sizeof(txt)-1, f)){

			if(pt = strstr(txt,"BASENAME=")){
			/* Find the last basename used */
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%s",&baseNameTxt);
			fFoundBaseName = true;
		}

			if(pt = strstr(txt,"WINDSPEED=")){
				/* Find the last windspeed used */
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%lf", &m_WindSpeed);
		}

		if(pt = strstr(txt,"WINDDIRECTION=")){
			/* Find the last winddirection used */
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%lf", &m_WindDirection);
		}
		}
	}else{
		return;
	}

	if(fFoundBaseName){
		i = L = strlen(baseNameTxt);
		while(baseNameTxt[i-1] >= '0' && baseNameTxt[i-1] <= '9')
		--i;

		if(i == L){
		sprintf(m_Base, "%s%02d", baseNameTxt, 1);
		}else{
		sscanf(baseNameTxt + i, "%d", &d);
		baseNameTxt[i] = 0;
		switch(L - i){
			case 1: sprintf(m_Base, "%s%01d", baseNameTxt, ++d); break;
			case 2: sprintf(m_Base, "%s%02d", baseNameTxt, ++d); break;
			case 3: sprintf(m_Base, "%s%03d", baseNameTxt, ++d); break;
			case 4: sprintf(m_Base, "%s%04d", baseNameTxt, ++d); break;
			case 5: sprintf(m_Base, "%s%05d", baseNameTxt, ++d); break;
			case 6: sprintf(m_Base, "%s%06d", baseNameTxt, ++d); break;
			case 7: sprintf(m_Base, "%s%07d", baseNameTxt, ++d); break;
		}
		}
		m_BaseEdit.SetWindowText(m_Base);
		UpdateData(FALSE);
	}
}

LRESULT CDMSpecView::OnShowInformationDialog(WPARAM wParam,LPARAM lParam){

	Dialogs::CInformationDialog infDlg;
	if(wParam == DARK_DIALOG)
		infDlg.informationString = "The offset level has dropped more than 20 counts since last dark spectrum was measured. Please cover the telescope to collect a new dark spectrum.";
	else if(wParam == INVALID_GPS)
		infDlg.informationString = "Lost contact with GPS-satellites, no valid GPS-data recived.";
	else if(wParam == CHANGED_EXPOSURETIME)
		infDlg.informationString = "Exposuretime has changed, please cover the telescope to get a new dark measurement";

	infDlg.DoModal();

	return 0;
}

void CDMSpecView::OnViewRealtimeroute(){
	if(m_realTimeRouteGraph.fVisible){
		m_realTimeRouteGraph.DestroyWindow();
	}else{
		/* create the real time route graph */
		m_realTimeRouteGraph.Create(IDD_REALTIME_ROUTE_DLG, NULL);

		if(fRunSpec){
		m_realTimeRouteGraph.m_spectrometer		= this->m_Spectrometer;
			m_realTimeRouteGraph.m_intensityLimit = m_Spectrometer->m_spectrometerDynRange * (100 - m_intensitySliderLow.GetPos());
		}else{
		m_realTimeRouteGraph.m_spectrometer		= NULL;
			m_realTimeRouteGraph.m_intensityLimit = 0.25 * 4095;
		}
		m_realTimeRouteGraph.ShowWindow(SW_SHOW);
	}
	m_realTimeRouteGraph.fVisible = !m_realTimeRouteGraph.fVisible;
}

void CDMSpecView::OnUpdateViewRealtimeroute(CCmdUI *pCmdUI){
	if(m_realTimeRouteGraph.fVisible){
		pCmdUI->SetCheck(BST_CHECKED);
	}else{
		pCmdUI->SetCheck(BST_UNCHECKED);
	}
}

void CDMSpecView::OnUpdateViewColumnError(CCmdUI *pCmdUI){
	if(m_showErrorBar){
		pCmdUI->SetCheck(BST_CHECKED);
	}else{
		pCmdUI->SetCheck(BST_UNCHECKED);
	}
}

void CDMSpecView::OnUpdateViewSpectrumFit(CCmdUI *pCmdUI){
	if(m_showFitDlg.fVisible){
		pCmdUI->SetCheck(BST_CHECKED);
	}else{
		pCmdUI->SetCheck(BST_UNCHECKED);
	}
}

void CDMSpecView::OnViewSpectrumFit(){
	if(m_showFitDlg.fVisible){
		m_showFitDlg.DestroyWindow();
	}else{
		/* create the real time route graph */
		m_showFitDlg.Create(IDD_VIEW_FIT_DLG, NULL);

		if(fRunSpec){
		m_showFitDlg.m_spectrometer = this->m_Spectrometer;
		}else{
		m_showFitDlg.m_spectrometer = NULL;
		}
		m_showFitDlg.ShowWindow(SW_SHOW);
	}
	m_showFitDlg.fVisible = !m_showFitDlg.fVisible;
}

void CDMSpecView::OnControlAddComment(){
	Dialogs::CCommentDlg cdlg;

	if(fRunSpec){
		m_Spectrometer->GetCurrentPos(&cdlg.lat, &cdlg.lon, &cdlg.alt);
		cdlg.t = m_Spectrometer->GetCurrentGPSTime();
		cdlg.outputDir.Format(m_Spectrometer->m_subFolder);
	}else{
		cdlg.lat = cdlg.lon = cdlg.alt = 0;
		cdlg.t = 0;
		cdlg.outputDir.Format(g_exePath);
	}

/*  cdlg.Create(IDD_COMMENT_DLG, NULL);
  cdlg.ShowWindow(SW_SHOW);*/
	cdlg.DoModal();
}

void CDMSpecView::OnControlReevaluate(){
	/* the reevaluation dialog */
	CReEvaluationDlg reEvalDlg;
	reEvalDlg.Construct("ReEvaluation", this, 0);

	// open the window
	reEvalDlg.DoModal();
}

void CDMSpecView::OnUpdateControlReevaluate(CCmdUI *pCmdUI){
#ifdef _DEBUG
	pCmdUI->Enable(TRUE);
#else
	pCmdUI->Enable(TRUE);
#endif
}
void CDMSpecView::OnConfigurationChangeexposuretime()
{
	if(m_Spectrometer != NULL)
	  m_Spectrometer->m_adjustIntegrationTime = TRUE;
}
void CDMSpecView::OnMenuAnalysisWindSpeedMeasurement()
{
	CPostWindDlg postWindDlg;

	postWindDlg.DoModal();
}

void CDMSpecView::OnMenuAnalysisPlumeheightmeasurement()
{
	CPostPlumeHeightDlg postPHDlg;
	postPHDlg.DoModal();
}

void CDMSpecView::OnMenuControlTestTheGPS()
{
	CSerialConnection serial;
	CString status;

	// set the baudrate for the connection
	serial.baudrate = 4800;

	for(int port = 1; port < 20; ++port){
		// try this serial-port and see what happens
		sprintf(serial.serialPort, "COM%d", port);

		status.Format("Testing port: %s", serial.serialPort);
		ShowStatusMsg(status);

		// test the serial-port
		if(!serial.Init(serial.baudrate)){
			// could not connect to this serial-port
			continue;
		}else{
			// it was possible to open the serial-port, test if there is a gps on this port
			serial.Close();

			CGPS *gps = new CGPS(serial.serialPort, serial.baudrate);
			for(int i = 0; i < 10; ++i){
				if(1 == gps->ReadGPS()){
					CString msg;
					msg.Format("Found GPS on serialPort: %s using baudrate %d", serial.serialPort, serial.baudrate);
					MessageBox(msg, "Found GPS reciever");

					delete gps;
					return;
				}

				Sleep(10);
			}

			delete gps;
			
		}

	}

	MessageBox("No GPS reciever could be found");
}

void CDMSpecView::UpdateLegend(){
	if(m_Spectrometer == NULL){
		// if the program has not been started yet
		m_colorLabelSpectrum1.ShowWindow(SW_HIDE);
		m_colorLabelSpectrum2.ShowWindow(SW_HIDE);
		m_colorLabelSeries1.ShowWindow(SW_HIDE);
		m_colorLabelSeries2.ShowWindow(SW_HIDE);
		m_legendSpectrum1.ShowWindow(SW_HIDE);
		m_legendSpectrum2.ShowWindow(SW_HIDE);
		m_legendSeries1.ShowWindow(SW_HIDE);
		m_legendSeries2.ShowWindow(SW_HIDE);
	}else{
		if(m_spectrometerMode == MODE_TRAVERSE){
			m_colorLabelSpectrum1.SetBackgroundColor(this->m_Spectrum0Color);
			m_colorLabelSeries1.SetBackgroundColor(this->m_PlotColor[0]);
			m_colorLabelSeries2.SetBackgroundColor(this->m_PlotColor[1]);

			m_legendSpectrum1.ShowWindow(SW_SHOW);
			m_colorLabelSpectrum1.ShowWindow(SW_SHOW);
		}
		if(m_spectrometerMode == MODE_WIND){
			m_colorLabelSpectrum1.SetBackgroundColor(this->m_Spectrum0Color);
			m_colorLabelSpectrum2.SetBackgroundColor(this->m_Spectrum1Color);
			m_colorLabelSeries1.SetBackgroundColor(this->m_PlotColor[0]);
			m_colorLabelSeries2.SetBackgroundColor(this->m_PlotColor[1]);

			m_legendSpectrum1.ShowWindow(SW_SHOW);
			m_legendSpectrum2.ShowWindow(SW_SHOW);
			SetDlgItemText(IDC_LABEL_SPECTRUM, "Spectrum (Master)");
			SetDlgItemText(IDC_LABEL_SPECTRUM2, "Spectrum (Slave)");
			m_colorLabelSpectrum1.ShowWindow(SW_SHOW);
			m_colorLabelSpectrum2.ShowWindow(SW_SHOW);
		}

		if(m_Spectrometer->GetFitRegionNum() == 1){
			m_colorLabelSeries1.ShowWindow(SW_SHOW);
			m_colorLabelSeries2.ShowWindow(SW_HIDE);
			m_legendSeries1.ShowWindow(SW_SHOW);
			m_legendSeries2.ShowWindow(SW_HIDE);
			m_legendSeries1.SetWindowText(m_Spectrometer->GetFitWindowName(0));
		}else{
			m_colorLabelSeries1.ShowWindow(SW_SHOW);
			m_colorLabelSeries2.ShowWindow(SW_SHOW);
			m_legendSeries1.ShowWindow(SW_SHOW);
			m_legendSeries2.ShowWindow(SW_SHOW);
			m_legendSeries1.SetWindowText(m_Spectrometer->GetFitWindowName(0));
			m_legendSeries2.SetWindowText(m_Spectrometer->GetFitWindowName(1));
		}
	}
}

/** Toggles the showing of the column error bars */
void CDMSpecView::OnViewColumnError(){
	CString msg;
	m_showErrorBar = !m_showErrorBar;

	if(m_showErrorBar)
		msg.Format("Showing Error Bars");
	else
		msg.Format("Hiding Error Bars");

	ShowStatusMsg(msg);
}

void CDMSpecView::OnUpdate_EnableOnRun(CCmdUI *pCmdUI){
	if(m_Spectrometer != NULL && m_Spectrometer->fRun){
		// enable
		pCmdUI->Enable(TRUE);
	}else{
		// disable
		pCmdUI->Enable(FALSE);
	}
}

void CDMSpecView::OnUpdate_DisableOnRun(CCmdUI *pCmdUI){
	if(m_Spectrometer != NULL && m_Spectrometer->fRun){
		// disable
		pCmdUI->Enable(FALSE);
	}else{
		// enable
		pCmdUI->Enable(TRUE);
	}
}

void CDMSpecView::OnUpdateWindMeasurement(CCmdUI *pCmdUI){
	// disable
	pCmdUI->Enable(FALSE); // unfortunately does not OmniDriver not yet support dual-beam wind measurements. Disable this in the meantime...
}

void CDMSpecView::OnUpdate_CalibrateSpectrometer(CCmdUI *pCmdUI){
	// this is not fully implemented yet...
#ifdef _DEBUG
	pCmdUI->Enable(TRUE);
#else
	pCmdUI->Enable(FALSE);
#endif
}


void CDMSpecView::OnClose()
{
	// Terminate the collection of spectra
	this->OnControlStop();

	CFormView::OnClose();
}

void CDMSpecView::OnDestroy()
{
	CFormView::OnDestroy();

	// Terminate the collection of spectra
	this->OnControlStop();
}
