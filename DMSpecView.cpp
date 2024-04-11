// DMSpecView.cpp : implementation of the CDMSpecView class

#undef min
#undef max

#include "stdafx.h"
#include <afxdlgs.h>
#include <memory>
#include "DMSpec.h"

#include "DMSpecDoc.h"
#include "DMSpecView.h"
#include "MainFrm.h"
#include <MobileDoasLib/DateTime.h>

#include "PostFluxDlg.h"
#include <MobileDoasLib/Flux/Flux1.h>
#include "DualBeam/PostWindDlg.h"
#include "DualBeam/PostPlumeHeightDlg.h"
#include "CSpectrometerCalibrationDlg.h"
#include "CommentDlg.h"
#include "InformationDialog.h"
#include "Dialogs/SpectrumInspectionDlg.h"

#include "ReEvaluation\ReEvaluationDlg.h"

#include "Configuration/ConfigurationDialog.h"
#include "Configuration/Configure_Evaluation.h"
#include "Configuration/Configure_GPS.h"
#include "Configuration/Configure_Spectrometer.h"
#include "Configuration/Configure_Directory.h"
#include "Configuration/Configure_Calibration.h"

#include "MeasurementSetup.h"
#include <algorithm>
#include <Mmsystem.h>	// used for PlaySound

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
CFormView* pView; // <-- The main window

/////////////////////////////////////////////////////////////////////////////
// CDMSpecView

IMPLEMENT_DYNCREATE(CDMSpecView, CFormView)

BEGIN_MESSAGE_MAP(CDMSpecView, CFormView)
    // Menu commands
    // Starting and stopping the program
    ON_BN_CLICKED(IDC_BTNSTART, OnControlStart) // <-- the toolbar button
    ON_BN_CLICKED(ID_CONTROL_STARTWINDMEASUREMENT, OnControlStartWindMeasurement)
    ON_COMMAND(ID_CONTROL_STOP, OnControlStop)

    // Just view the spectra from the spectrometer without evaluations
    ON_COMMAND(ID_CONTROL_VIEWSPECTRAFROMSPECTROMETER, OnControlViewSpectra) // <-- view the output from the spectrometer
    ON_COMMAND(ID_CONTROL_VIEWSPECTRAFROMDIRECTORY, OnControlProcessSpectraFromDirectory) // <-- view latest spectra file in directory

    ON_COMMAND(ID_ANALYSIS_POSTFLUX, OnMenuShowPostFluxDialog)

    ON_COMMAND(ID_ANALYSIS_VIEWMEASUREDSPECTRA, OnMenuShowSpectrumInspectionDialog)

    ON_COMMAND(ID_CONTROL_COUNTFLUX, OnControlCountflux)

    // Changing the plot
    ON_COMMAND(ID_CONFIGURATION_PLOT_CHANGEBACKGROUND, OnConfigurationPlotChangebackground)
    ON_COMMAND(ID_CONFIGURATION_PLOT_CHANGEPLOTCOLOR, OnConfigurationPlotChangeplotcolor)
    ON_COMMAND(ID_CONFIGURATION_PLOT_CHANGEPLOTCOLOR_SLAVE, OnConfigurationPlotChangeplotcolor_Slave)

    // dual-beam
    ON_COMMAND(ID_ANALYSIS_PLUMEHEIGHTMEASUREMENT, OnMenuAnalysisPlumeheightmeasurement)
    ON_COMMAND(ID_ANALYSIS_WINDSPEEDMEASUREMENT, OnMenuAnalysisWindSpeedMeasurement)

    ON_COMMAND(ID_CONFIGURATION_OPERATION, OnConfigurationOperation)
    ON_MESSAGE(WM_DRAWCOLUMN, OnDrawColumn)
    ON_MESSAGE(WM_STATUSMSG, OnShowStatus)
    ON_MESSAGE(WM_READGPS, OnReadGPS)
    ON_MESSAGE(WM_SHOWINTTIME, OnShowIntTime)
    ON_MESSAGE(WM_CHANGEDSPEC, OnChangeSpectrometer)
    ON_MESSAGE(WM_DRAWSPECTRUM, OnDrawSpectrum)
    ON_MESSAGE(WM_CHANGEDSPECSCALE, OnChangedSpectrumScale)
    ON_MESSAGE(WM_SHOWDIALOG, OnShowInformationDialog)
    ON_COMMAND(ID_VIEW_REALTIMEROUTE, OnViewRealtimeroute)
    ON_COMMAND(ID_VIEW_SPECTRUMFIT, OnViewSpectrumFit)
    ON_UPDATE_COMMAND_UI(ID_VIEW_REALTIMEROUTE, OnUpdateViewRealtimeroute)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SPECTRUMFIT, OnUpdateViewSpectrumFit)
    ON_COMMAND(ID_CONTROL_ADDCOMMENT, OnControlAddComment)
    ON_COMMAND(ID_CONTROL_REEVALUATE, OnControlReevaluate)
    ON_UPDATE_COMMAND_UI(ID_CONTROL_REEVALUATE, OnUpdateControlReevaluate)
    ON_COMMAND(ID_CONFIGURATION_CHANGEEXPOSURETIME, OnConfigurationChangeexposuretime)
    ON_WM_HELPINFO()
    ON_COMMAND(ID_CONTROL_TESTTHEGPS, OnMenuControlTestTheGPS)
    ON_COMMAND(ID_CONTROL_STARTTHEGPS, OnMenuControlRunTheGPS)
    ON_COMMAND(ID_ANALYSIS_CALIBRATESPECTROMETER, OnAnalysisCalibratespectrometer)

    ON_COMMAND(ID_VIEW_COLUMNERROR, OnViewColumnError)
    ON_UPDATE_COMMAND_UI(ID_VIEW_COLUMNERROR, OnUpdateViewColumnError)

    ON_UPDATE_COMMAND_UI(ID_CONTROL_TESTTHEGPS, OnUpdate_DisableOnRun)
    ON_UPDATE_COMMAND_UI(IDC_BTNSTART, OnUpdate_DisableOnRun)
    ON_UPDATE_COMMAND_UI(ID_CONTROL_STOP, OnUpdate_EnableOnRun)
    ON_UPDATE_COMMAND_UI(ID_CONTROL_VIEWSPECTRAFROMSPECTROMETER, OnUpdate_DisableOnRun)
    ON_UPDATE_COMMAND_UI(ID_CONTROL_STARTWINDMEASUREMENT, OnUpdateWindMeasurement)
    ON_UPDATE_COMMAND_UI(ID_CONTROL_STARTTHEGPS, OnUpdate_StartTheGps)
    ON_UPDATE_COMMAND_UI(ID_CONTROL_ADDCOMMENT, OnUpdate_EnableOnRun)
    ON_UPDATE_COMMAND_UI(ID_CONFIGURATION_CHANGEEXPOSURETIME, OnUpdate_EnableOnRun)

    ON_WM_CLOSE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDMSpecView construction/destruction

// boolean flag to help us determine if the spectrometer thread is running. s_ stands for statics...
static bool s_spectrometerAcquisitionThreadIsRunning = false;

// label colors

COLORREF warning = RGB(255, 75, 75);
COLORREF normal = RGB(236, 233, 216);

CDMSpecView::CDMSpecView()
    : CFormView(CDMSpecView::IDD)
{
    m_WindDirection = 0.0;
    m_WindSpeed = 8.0;
    pView = this;
    s_spectrometerAcquisitionThreadIsRunning = false;

    m_columnChartXAxisValues.resize(200);
    for (int i = 0; i < 200; i++)
    {
        m_columnChartXAxisValues[i] = i;
    }

    m_spectrumChartXAxisValues.resize(MAX_SPECTRUM_LENGTH);
    for (int i = 0; i < MAX_SPECTRUM_LENGTH; i++)
    {
        m_spectrumChartXAxisValues[i] = (200.0 * i) / MAX_SPECTRUM_LENGTH;
    }

    m_Spectrometer = nullptr;
    m_showErrorBar = FALSE;
}

CDMSpecView::~CDMSpecView()
{
    if (m_Spectrometer != nullptr)
    {
        delete(m_Spectrometer);
        m_Spectrometer = nullptr;
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
    DDX_Control(pDX, IDC_LAT, m_gpsLatLabel);
    DDX_Control(pDX, IDC_LON, m_gpsLonLabel);
    DDX_Control(pDX, IDC_GPSTIME, m_gpsTimeLabel);
    DDX_Control(pDX, IDC_NGPSSAT, m_gpsNSatLabel);

    // Spectrometer Info labels
    DDX_Control(pDX, IDC_INTTIME, m_expLabel);
    DDX_Control(pDX, IDC_SCANNO, m_scanNoLabel);
    DDX_Control(pDX, IDC_CONCENTRATION, m_colLabel);
    DDX_Control(pDX, IDC_SPECNO, m_noSpecLabel);
    DDX_Control(pDX, IDC_SH, m_shiftLabel);
    DDX_Control(pDX, IDC_SQ, m_squeezeLabel);
    DDX_Control(pDX, IDC_TEMPERATURE, m_tempLabel);

    // The legend
    DDX_Control(pDX, IDC_LABEL_COLOR_SPECTRUM, m_colorLabelSpectrum1);
    DDX_Control(pDX, IDC_LABEL_COLOR_SPECTRUM2, m_colorLabelSpectrum2);
    DDX_Control(pDX, IDC_LABEL_COLOR_SERIES1, m_colorLabelSeries1);
    DDX_Control(pDX, IDC_LABEL_COLOR_SERIES2, m_colorLabelSeries2);
    DDX_Control(pDX, IDC_LABEL_SPECTRUM, m_legendSpectrum1);
    DDX_Control(pDX, IDC_LABEL_SPECTRUM2, m_legendSpectrum2);
    DDX_Control(pDX, IDC_LABEL_SERIES1, m_legendSeries1);
    DDX_Control(pDX, IDC_LABEL_SERIES2, m_legendSeries2);
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
    int left = 50;
    int top = 14;
    int right = 970;
    int bottom = 520;

    // Get the resolution of the screen
    int cx = GetSystemMetrics(SM_CXSCREEN);

    // rescale the graph to fit the width of the window
    right = cx * right / 1024;

    // Also move the slider to the right of the window
    m_intensitySliderLow.GetWindowRect(rect);
    int diff = rect.right - rect.left;
    rect = CRect(right, TOP, right + diff + 1, BOTTOM);
    m_intensitySliderLow.MoveWindow(rect);

    rect = CRect(LEFT, TOP, right, BOTTOM);
    m_ColumnPlot.Create(WS_VISIBLE | WS_CHILD, rect, this);

    // customize the control
    m_columnLimit = 100.0;
    m_PlotColor[0] = RGB(255, 0, 0);
    m_PlotColor[1] = RGB(0, 0, 255);
    m_ColumnPlot.SetSecondYUnit(TEXT("Intensity [%]"));
    m_ColumnPlot.SetYUnits("Column [ppmm]");
    m_ColumnPlot.SetXUnits("Number");
    m_ColumnPlot.EnableGridLinesX(false);
    m_ColumnPlot.SetBackgroundColor(RGB(0, 0, 0));
    m_ColumnPlot.SetGridColor(RGB(255, 255, 255));
    m_ColumnPlot.SetPlotColor(m_PlotColor[0]);
    m_ColumnPlot.SetRange(0, 200, 1, 0.0, 100.0, 1);
    m_ColumnPlot.SetMinimumRangeX(200.0f);
    m_ColumnPlot.SetSecondRange(0.0, 200, 0, 0.0, 100.0, 0);

    m_BaseEdit.LimitText(99);
    ReadMobileLog();

    /* the intensity slider */
    m_intensitySliderLow.SetRange(0, 100); /** The scale of the intensity slider is in percent */
    m_intensitySliderLow.SetPos(100 - 25); /* The slider is upside down - i.e. the real value is "100 - m_intensitySlider.GetPos()"*/
    m_intensitySliderLow.SetTicFreq(25);

    /* The colors for the spectrum plots */
    m_Spectrum0Color = RGB(0, 255, 0);
    m_Spectrum0FitColor = RGB(0, 150, 0);
    m_Spectrum1Color = RGB(255, 0, 255);
    m_Spectrum1FitColor = RGB(150, 0, 150);
    m_SpectrumLineWidth = 1;

    // Fix the legend
    UpdateLegend();

    // set background color for spectrometer info labels
    m_expLabel.SetBackgroundColor(normal);
    m_scanNoLabel.SetBackgroundColor(normal);
    m_colLabel.SetBackgroundColor(normal);
    m_noSpecLabel.SetBackgroundColor(normal);
    m_shiftLabel.SetBackgroundColor(normal);
    m_squeezeLabel.SetBackgroundColor(normal);
    m_tempLabel.SetBackgroundColor(normal);

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
LRESULT CDMSpecView::OnDrawColumn(WPARAM wParam, LPARAM lParam)
{

    CString cCon;		// the concentration str
    CString cShift;		//shift str
    CString cSqueeze;	//squeeze str
    CString cScanNo;	//scanned spectra number after sky and dark spectra
    CString cTemp;		// detector temperature

    // if the program is no longer running, then don't try to draw anything more...
    if (!s_spectrometerAcquisitionThreadIsRunning)
    {
        return 0;
    }

    const long dynRange = m_Spectrometer->m_spectrometerDynRange;
    const int intensityLimit = (100 - m_intensitySliderLow.GetPos());

    // Get the last value and the total number of values
    mobiledoas::ReferenceFitResult lastEvaluationResult;
    m_Spectrometer->GetLastColumn(lastEvaluationResult);
    const long scanNo = m_Spectrometer->GetNumberOfSpectraAcquired() - 2;

    // Get the number of channels used and the number of fit-regions used
    const int nChannels = m_Spectrometer->m_NChannels;
    const int fitRegionNum = m_Spectrometer->GetFitRegionNum();

    // --- Update the column, shift and squeeze ---
    cCon.Format("%.2lf \u00B1 %.2lf", lastEvaluationResult.m_column, lastEvaluationResult.m_columnError);
    cShift.Format("%.1f", lastEvaluationResult.m_shift);
    cSqueeze.Format("%.1f", lastEvaluationResult.m_squeeze);
    cScanNo.Format("%d", scanNo);
    this->SetDlgItemText(IDC_CONCENTRATION, cCon);
    this->SetDlgItemText(IDC_SH, cShift);
    this->SetDlgItemText(IDC_SQ, cSqueeze);
    this->SetDlgItemText(IDC_SCANNO, cScanNo);

    // update the temperature
    double temp = m_Spectrometer->detectorTemperature;
    if (!std::isnan(temp))
    {
        cTemp.Format("%.1f", temp);
        if (m_Spectrometer->detectorTemperatureIsSetPointTemp)
        {
            m_tempLabel.SetBackgroundColor(normal);
        }
        else
        {
            m_tempLabel.SetBackgroundColor(warning);

        }
    }
    else
    {
        cTemp = "N/A";
        m_tempLabel.SetBackgroundColor(normal);
    }
    this->SetDlgItemText(IDC_TEMPERATURE, cTemp);

    // --- Get the data, but plot mo more than 199 values (to keep the graph readable) ---
    const long size = std::min(long(199), m_Spectrometer->GetColumnNumber());

    std::vector<double> intensity(size);
    m_Spectrometer->GetIntensity(intensity, size);

    std::vector<double> masterChannelColumns(size);
    std::vector<double> masterChannelColumnErrors(size);
    m_Spectrometer->GetColumns(masterChannelColumns, size, 0);
    m_Spectrometer->GetColumnErrors(masterChannelColumnErrors, size, 0);


    std::vector<double> slaveChannelColumns;
    std::vector<double> slaveChannelColumnErrors;
    if (fitRegionNum > 1)
    {
        slaveChannelColumns.resize(size);
        slaveChannelColumnErrors.resize(size);
        m_Spectrometer->GetColumns(slaveChannelColumns, size, 1);
        m_Spectrometer->GetColumnErrors(slaveChannelColumnErrors, size, 1);
    }

    // -- Convert the intensity to saturation ratio
    for (int k = 0; k < size; ++k)
    {
        intensity[k] = intensity[k] * 100.0 / dynRange;
    }

    double maxColumn = 0.0;
    double minColumn = 0.0;

    // -- Get the limits for the data ---
    for (int i = 0; i < size; i++)
    {
        if (intensity[i] > intensityLimit)
        {
            maxColumn = std::max(maxColumn, std::abs(masterChannelColumns[i]));
            minColumn = std::min(minColumn, masterChannelColumns[i]);
            if (fitRegionNum > 1)
            {
                maxColumn = std::max(maxColumn, std::abs(slaveChannelColumns[i]));
                minColumn = std::min(minColumn, slaveChannelColumns[i]);
            }
        }
    }
    const double lowLimit = (-1.25) * std::abs(minColumn);
    m_columnLimit = 1.25 * maxColumn;

    if (m_columnLimit == 0)
    {
        m_columnLimit = 0.1;
    }

    // Set the range for the plot
    m_ColumnPlot.SetRange(0.0, 199.0, 0, lowLimit, m_columnLimit, 1);
    m_ColumnPlot.SetSecondRange(0.0, 200, 0, m_minSaturationRatio, m_maxSaturationRatio, 0);

    // Draw the columns (don't change the scale again here...)
    if (m_spectrometerMode == MODE_TRAVERSE || m_spectrometerMode == MODE_DIRECTORY)
    {
        if (fitRegionNum == 1)
        {
            m_ColumnPlot.SetPlotColor(m_PlotColor[0]);
            if (m_showErrorBar)
            {
                m_ColumnPlot.BarChart(m_columnChartXAxisValues.data(), masterChannelColumns.data(), masterChannelColumnErrors.data(), size, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
            }
            else
            {
                m_ColumnPlot.BarChart(m_columnChartXAxisValues.data(), masterChannelColumns.data(), size, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
            }
        }
        else
        {
            m_ColumnPlot.SetPlotColor(m_PlotColor[0]);
            if (m_showErrorBar)
            {
                m_ColumnPlot.BarChart2(m_columnChartXAxisValues.data(), masterChannelColumns.data(), slaveChannelColumns.data(), masterChannelColumnErrors.data(), slaveChannelColumnErrors.data(), m_PlotColor[1], size, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
            }
            else
            {
                m_ColumnPlot.BarChart2(m_columnChartXAxisValues.data(), masterChannelColumns.data(), slaveChannelColumns.data(), m_PlotColor[1], size, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
            }
        }
    }
    else if (m_spectrometerMode == MODE_WIND)
    {
        if (m_showErrorBar)
        {
            m_ColumnPlot.SetPlotColor(m_PlotColor[0]);
            m_ColumnPlot.XYPlot(m_columnChartXAxisValues.data(), masterChannelColumns.data(), NULL, NULL, masterChannelColumnErrors.data(), size, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);

            m_ColumnPlot.SetPlotColor(m_PlotColor[1]);
            m_ColumnPlot.XYPlot(m_columnChartXAxisValues.data(), slaveChannelColumns.data(), NULL, NULL, slaveChannelColumnErrors.data(), size, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);
        }
        else
        {
            m_ColumnPlot.SetPlotColor(m_PlotColor[0]);
            m_ColumnPlot.XYPlot(m_columnChartXAxisValues.data(), masterChannelColumns.data(), size, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);

            m_ColumnPlot.SetPlotColor(m_PlotColor[1]);
            m_ColumnPlot.XYPlot(m_columnChartXAxisValues.data(), slaveChannelColumns.data(), size, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);
        }
    }

    // Draw the intensities
    m_ColumnPlot.DrawCircles(m_columnChartXAxisValues.data(), intensity.data(), size, Graph::CGraphCtrl::PLOT_SECOND_AXIS);

    // Draw the spectrum
    DrawSpectrum();

    if (m_realTimeRouteGraph.fVisible)
    {
        m_realTimeRouteGraph.m_intensityLimit = dynRange * (100 - m_intensitySliderLow.GetPos());
        m_realTimeRouteGraph.DrawRouteGraph();
    }
    if (m_showFitDlg.m_isVisible)
    {
        m_showFitDlg.DrawFit();
    }

    return 0;
}

LRESULT CDMSpecView::OnDrawSpectrum(WPARAM wParam, LPARAM lParam)
{
    // to not overload the computer, make sure that we don't draw too often...
    static double secondsBetweenDraw = 0.05;
    static clock_t cLastCall = 0;
    clock_t now = clock();
    double secondsSinceLastDraw = (double)(now - cLastCall) / (double)CLOCKS_PER_SEC;
    if (secondsSinceLastDraw < secondsBetweenDraw)
    {
        return 0;
    }
    cLastCall = now;

    m_ColumnPlot.CleanPlot();

    // set the ranges for the plot
    if (m_spectrometerMode == MODE_VIEW || m_spectrometerMode == MODE_DIRECTORY)
    {
        m_ColumnPlot.SetRange(0.0, 2048, 0, m_minSaturationRatio, m_maxSaturationRatio, 0);
    }
    else
    {
        m_ColumnPlot.SetSecondRange(0.0, 200, 0, m_minSaturationRatio, m_maxSaturationRatio, 0);
    }

    DrawSpectrum();

    // also update the integration time
    OnShowIntTime(wParam, lParam);

    return 0;
}

/** Changes the saturation-ratio scale of the spectrum-view */
LRESULT CDMSpecView::OnChangedSpectrumScale(WPARAM wParam, LPARAM lParam)
{
    this->m_minSaturationRatio = (int)wParam;
    this->m_maxSaturationRatio = (int)lParam;

    return OnDrawSpectrum(wParam, lParam);
}

LRESULT CDMSpecView::OnShowIntTime(WPARAM wParam, LPARAM lParam)
{
    CString expTime, nAverage;
    int averageInSpectrometer = 0;
    int averageInComputer = 0;

    // if the program is no longer running, then don't try to draw anything more...
    if (!s_spectrometerAcquisitionThreadIsRunning)
    {
        return 0;
    }

    expTime.Format("%d  ms", m_Spectrometer->GetCurrentIntegrationTime());
    this->SetDlgItemText(IDC_INTTIME, expTime);

    if (m_spectrometerMode == MODE_DIRECTORY)
    {
        nAverage.Format("%d", m_Spectrometer->NumberOfSpectraToAverage());
    }
    else
    {
        m_Spectrometer->GetNSpecAverage(averageInSpectrometer, averageInComputer);
        nAverage.Format("%dx%d", averageInSpectrometer, averageInComputer);
    }
    this->SetDlgItemText(IDC_SPECNO, nAverage);

    // Update the legend
    UpdateLegend();

    // forward the message to the spectrum-settings dialog(if any);
    if (this->m_specSettingsDlg.m_hWnd != nullptr)
    {
        m_specSettingsDlg.PostMessage(WM_SHOWINTTIME);
    }

    return 0;
}

LRESULT CDMSpecView::OnChangeSpectrometer(WPARAM wParam, LPARAM lParam)
{
    // forward the message to the spectrum-settings dialog(if any);
    if (this->m_specSettingsDlg.m_hWnd != nullptr)
    {
        m_specSettingsDlg.PostMessage(WM_CHANGEDSPEC);
    }
    return 0;
}

void CDMSpecView::OnMenuShowPostFluxDialog()
{
    CPostFluxDlg fluxDlg;
    fluxDlg.DoModal();
}

void CDMSpecView::OnMenuShowSpectrumInspectionDialog()
{
    Dialogs::CSpectrumInspectionDlg dlg;
    dlg.DoModal();
}

/** This function is the thread function to start running spectrometer
**
*/
UINT CollectSpectra(LPVOID pParam)
{
    CSpectrometer* spec = (CSpectrometer*)pParam;
    spec->Run();
    s_spectrometerAcquisitionThreadIsRunning = false;
    return 0;
}

LRESULT CDMSpecView::OnShowStatus(WPARAM wParam, LPARAM lParam)
{
    if (s_spectrometerAcquisitionThreadIsRunning)
    {
        CString str;
        str = m_Spectrometer->m_statusMsg;
        ShowStatusMsg(str);
    }
    return 0;
}

LRESULT CDMSpecView::OnReadGPS(WPARAM wParam, LPARAM lParam)
{
    mobiledoas::GpsData data;
    static int latNSat = 10;

    // if the program is no longer running, then don't try to draw anything more...
    if (!s_spectrometerAcquisitionThreadIsRunning)
    {
        return 0;
    }

    m_Spectrometer->GetGpsPos(data);

    CString lat, lon, tim, strHr, strMin, strSec, nSat;
    int hr, min, sec;
    ExtractTime(data, hr, min, sec);

    if (data.latitude >= 0.0)
    {
        lat.Format("%f  degree N", data.latitude);
    }
    else
    {
        lat.Format("%f  degree S", -1.0 * data.latitude);
    }

    if (data.longitude >= 0.0)
    {
        lon.Format("%f  degree E", data.longitude);
    }
    else
    {
        lon.Format("%f  degree W", -1.0 * data.longitude);
    }

    if (hr < 10)
    {
        strHr.Format("0%d:", hr);
    }
    else
    {
        strHr.Format("%d:", hr);
    }

    if (min < 10)
    {
        strMin.Format("0%d:", min);
    }
    else
    {
        strMin.Format("%d:", min);
    }

    if (sec < 10)
    {
        strSec.Format("0%d", sec);
    }
    else
    {
        strSec.Format("%d", sec);
    }

    nSat.Format("%d", (long)data.nSatellitesTracked);

    tim = strHr + strMin + strSec;
    this->SetDlgItemText(IDC_GPSTIME, tim);
    this->SetDlgItemText(IDC_LAT, lat);
    this->SetDlgItemText(IDC_LON, lon);
    this->SetDlgItemText(IDC_NGPSSAT, nSat);


    if (!(m_spectrometerMode == MODE_DIRECTORY) && !m_Spectrometer->GpsGotContact())
    {
        // If the communication with the GPS is broken (e.g. device unplugged)
        COLORREF warning = RGB(255, 75, 75);
        // Set the background color to red
        m_gpsLatLabel.SetBackgroundColor(warning);
        m_gpsLonLabel.SetBackgroundColor(warning);
        m_gpsTimeLabel.SetBackgroundColor(warning);
        m_gpsNSatLabel.SetBackgroundColor(warning);

        SoundAlarm();
    }
    else if (latNSat != 0 && data.nSatellitesTracked == 0)
    {
        COLORREF warning = RGB(255, 75, 75);

        // Set the background color to red
        m_gpsLatLabel.SetBackgroundColor(warning);
        m_gpsLonLabel.SetBackgroundColor(warning);
        m_gpsTimeLabel.SetBackgroundColor(warning);
        m_gpsNSatLabel.SetBackgroundColor(warning);
    }
    else
    {
        // Set the background color to normal
        m_gpsLatLabel.SetBackgroundColor(normal);
        m_gpsLonLabel.SetBackgroundColor(normal);
        m_gpsTimeLabel.SetBackgroundColor(normal);
        m_gpsNSatLabel.SetBackgroundColor(normal);
    }

    // Remember the number of satelites
    latNSat = (int)data.nSatellitesTracked;

    return 0;
}



void CDMSpecView::ShowStatusMsg(CString& str)
{
    CMainFrame* pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
    CStatusBar* pStatus = &pFrame->m_wndStatusBar;
    if (pStatus)
    {
        pStatus->SetPaneText(0, str);

    }
}


void CDMSpecView::OnControlCountflux()
{
    if (s_spectrometerAcquisitionThreadIsRunning)
    {
        double flux = m_Spectrometer->GetFlux();
        // m_Spectrometer->WriteFluxLog();

        CString str;
        str.Format("By now the flux is %f", flux);

        MessageBox(str, "Flux", MB_OK);
    }
    else
    {
        MessageBox(TEXT("The spectrometer hasn't been started.\nStart it first,\nthen you can use this function"), "Notice", MB_OK);
    }
}

void CDMSpecView::OnConfigurationPlotChangebackground()
{
    CColorDialog dlg;

    if (dlg.DoModal() == IDOK)
    {
        m_bkColor = dlg.m_cc.rgbResult;
        m_ColumnPlot.SetBackgroundColor(m_bkColor);
    }
}

void CDMSpecView::OnConfigurationPlotChangeplotcolor()
{
    CColorDialog dlg;
    if (dlg.DoModal() == IDOK)
    {
        m_PlotColor[0] = dlg.m_cc.rgbResult;
        m_ColumnPlot.SetPlotColor(m_PlotColor[0]);
    }

    // Update the legend
    UpdateLegend();
}

void CDMSpecView::OnConfigurationPlotChangeplotcolor_Slave()
{
    CColorDialog dlg;
    if (dlg.DoModal() == IDOK)
    {
        m_PlotColor[1] = dlg.m_cc.rgbResult;
    }

    // Update the legend
    UpdateLegend();
}

std::unique_ptr<Configuration::CMobileConfiguration> ReadConfiguration()
{
    CString cfgFile = g_exePath + TEXT("cfg.xml");
    std::unique_ptr<Configuration::CMobileConfiguration> conf;
    conf.reset(new Configuration::CMobileConfiguration(cfgFile));
    return conf;
}

void CDMSpecView::OnControlStart()
{
    if (!s_spectrometerAcquisitionThreadIsRunning)
    {
        /* Check that the base name does not contain any illegal characters */
        CString tmpStr;
        this->GetDlgItemText(IDC_BASEEDIT, tmpStr);
        if (-1 != tmpStr.FindOneOf("\\/:*?\"<>|"))
        {
            tmpStr.Format("The base name is not allowed to contain any of the following characters: \\ / : * ? \" < > | Please choose another basename and try again.");
            MessageBox(tmpStr, "Error", MB_OK);
            return;
        }

        auto conf = ReadConfiguration();
        if (conf->m_spectrometerConnection == conf->CONNECTION_DIRECTORY)
        {
            OnControlProcessSpectraFromDirectory();
            return;
        }

        // Initialize a new CSpectromber object, this is the one which actually does everything...
        m_Spectrometer = CreateSpectrometer(MODE_TRAVERSE, *this, std::move(conf));

        // Copy the settings that the user typed in the dialog
        char text[100];
        memset(text, 0, (size_t)100);
        if (UpdateData(TRUE))
        {
            m_BaseEdit.GetWindowText(text, 255);
            m_Spectrometer->SetUserParameters(m_WindSpeed, m_WindDirection, text);
        }

        // Start the measurement thread
        pSpecThread = AfxBeginThread(CollectSpectra, (LPVOID)(m_Spectrometer), THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        s_spectrometerAcquisitionThreadIsRunning = true;
        m_spectrometerMode = MODE_TRAVERSE;

        // If the user wants to see the real-time route then initialize it also
        if (m_realTimeRouteGraph.fVisible)
        {
            m_realTimeRouteGraph.m_spectrometer = m_Spectrometer;
            m_realTimeRouteGraph.DrawRouteGraph();
        }

        if (m_showFitDlg.m_isVisible)
        {
            m_showFitDlg.m_spectrometer = m_Spectrometer;
            m_showFitDlg.DrawFit();
        }
    }
    else
    {
        MessageBox(TEXT("Spectra are collecting"), "Notice", MB_OK);
    }
}

/** Starts the viewing of spectra from the spectrometer,
        without saving or evaluating them. */
void CDMSpecView::OnControlViewSpectra()
{
    char text[100];
    CString tmpStr;
    CRect rect;

    if (!s_spectrometerAcquisitionThreadIsRunning)
    {
        CDMSpecDoc* pDoc = GetDocument();

        auto conf = ReadConfiguration();

        m_Spectrometer = CreateSpectrometer(MODE_VIEW, *this, std::move(conf));

        memset(text, 0, (size_t)100);

        pSpecThread = AfxBeginThread(CollectSpectra, (LPVOID)(m_Spectrometer), THREAD_PRIORITY_LOWEST, 0, 0, NULL);
        s_spectrometerAcquisitionThreadIsRunning = true;
        m_spectrometerMode = MODE_VIEW;

        // Show the window that makes it possible to change the exposure time
        m_specSettingsDlg.m_Spectrometer = m_Spectrometer;
        if (!IsWindow(m_specSettingsDlg))
        {
            m_specSettingsDlg.Create(IDD_SPECTRUM_SETTINGS_DLG, this);
        }
        m_specSettingsDlg.ShowWindow(SW_SHOW);

        // Show the window that makes it possible to change the spectrum-scale 
        if (!IsWindow(m_specScaleDlg))
        {
            m_specScaleDlg.Create(IDD_SPECTRUM_SCALE_DLG, this);
        }
        m_specScaleDlg.SetMainForm(this);
        m_specScaleDlg.ShowWindow(SW_SHOW);
        m_specScaleDlg.GetWindowRect(rect);
        int width = rect.Width();
        int cx = GetSystemMetrics(SM_CXSCREEN); // the width of the screen
        rect.right = cx - 10;
        rect.left = rect.right - width;
        m_specScaleDlg.MoveWindow(rect);

        if (m_realTimeRouteGraph.fVisible)
        {
            m_realTimeRouteGraph.m_spectrometer = m_Spectrometer;
            m_realTimeRouteGraph.DrawRouteGraph();
        }

        if (m_showFitDlg.m_isVisible)
        {
            m_showFitDlg.m_spectrometer = m_Spectrometer;
            m_showFitDlg.DrawFit();
        }

        // Also set the column-plot to only show the measured spectrum
        m_ColumnPlot.SetSecondYUnit("");
        m_ColumnPlot.SetYUnits("Intensity [%]");
        m_ColumnPlot.SetXUnits("Pixel");
        m_ColumnPlot.EnableGridLinesX(false);
        m_ColumnPlot.SetRange(0.0, 2048, 0, 0.0, 100.0, 0);
    }
    else
    {
        MessageBox(TEXT("Spectra are collecting"), "Notice", MB_OK);
    }
}

/** Starts the viewing of latest spectra in a directory specified by config file. */
void CDMSpecView::OnControlProcessSpectraFromDirectory()
{

    auto conf = ReadConfiguration();

    m_Spectrometer = CreateSpectrometer(MODE_DIRECTORY, *this, std::move(conf));

    pSpecThread = AfxBeginThread(CollectSpectra, (LPVOID)(m_Spectrometer), THREAD_PRIORITY_LOWEST, 0, 0, NULL);
    s_spectrometerAcquisitionThreadIsRunning = true;
    m_spectrometerMode = MODE_DIRECTORY;

    m_ColumnPlot.SetYUnits("Column [ppmm]");
    m_ColumnPlot.SetSecondYUnit("Intensity [%]");
    m_ColumnPlot.SetXUnits("Number");
    m_ColumnPlot.EnableGridLinesX(false);
    m_ColumnPlot.SetBackgroundColor(RGB(0, 0, 0));
    m_ColumnPlot.SetGridColor(RGB(255, 255, 255));
    m_ColumnPlot.SetPlotColor(m_PlotColor[0]);
    m_ColumnPlot.SetRange(0, 200, 1, 0.0, 100.0, 1);
    m_ColumnPlot.SetMinimumRangeX(200.0f);
    m_ColumnPlot.SetSecondRange(0.0, 200, 0, 0.0, 100.0, 0);
}

void CDMSpecView::OnControlStartWindMeasurement()
{
    char text[100];
    CString tmpStr;

    if (!s_spectrometerAcquisitionThreadIsRunning)
    {
        /* Check that the base name does not contain any illegal characters */
        this->GetDlgItemText(IDC_BASEEDIT, tmpStr);
        if (-1 != tmpStr.FindOneOf("\\/:*?\"<>|"))
        {
            tmpStr.Format("The base name is not allowed to contain any of the following characters: \\ / : * ? \" < > | Please choose another basename and try again.");
            MessageBox(tmpStr, "Error", MB_OK);
            return;
        }

        auto conf = ReadConfiguration();

        // Initialize a new CSpectrometer object, this is the one which actually does everything...
        m_Spectrometer = CreateSpectrometer(MODE_WIND, *this, std::move(conf));

        // Copy the settings that the user typed in the dialog
        memset(text, 0, (size_t)100);
        if (UpdateData(TRUE))
        {
            m_BaseEdit.GetWindowText(text, 255);
            m_Spectrometer->SetUserParameters(m_WindSpeed, m_WindDirection, text);
        }

        // Start the measurement thread
        pSpecThread = AfxBeginThread(CollectSpectra, (LPVOID)(m_Spectrometer), THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        s_spectrometerAcquisitionThreadIsRunning = true;
        m_spectrometerMode = MODE_WIND;

        // If the user wants to see the real-time route then initialize it also
        if (m_realTimeRouteGraph.fVisible)
        {
            m_realTimeRouteGraph.m_spectrometer = m_Spectrometer;
            m_realTimeRouteGraph.DrawRouteGraph();
        }

        if (m_showFitDlg.m_isVisible)
        {
            m_showFitDlg.m_spectrometer = m_Spectrometer;
            m_showFitDlg.DrawFit();
        }
    }
    else
    {
        MessageBox(TEXT("Spectra are collecting"), "Notice", MB_OK);
    }
}

void CDMSpecView::OnControlStop()
{
    if (s_spectrometerAcquisitionThreadIsRunning)
    {
        s_spectrometerAcquisitionThreadIsRunning = false;

        DWORD dwExitCode;
        HANDLE hThread = pSpecThread->m_hThread;
        if (hThread != nullptr && GetExitCodeThread(hThread, &dwExitCode) && dwExitCode == STILL_ACTIVE)
        {
            AfxGetApp()->BeginWaitCursor();
            m_Spectrometer->Stop();
            Sleep(500);
            WaitForSingleObject(hThread, INFINITE);
            AfxGetApp()->EndWaitCursor();
            MessageBox(TEXT("Spectrum collection has been stopped"), NULL, MB_OK);
        }
    }
}

void CDMSpecView::DrawSpectrum()
{
    CString lambdaStr;

    // if the program is no longer running, then don't try to draw anything more...
    if (!s_spectrometerAcquisitionThreadIsRunning)
    {
        return;
    }

    // Get the length of the spectrum
    const int spectrumLength = m_Spectrometer->m_detectorSize;

    // Get the maximum intensity of the spectrometer
    const double conversionSpectrometerIntensityToSaturationInPercent = 100.0 / (double)m_Spectrometer->m_spectrometerDynRange;

    // If the length of the 'm_spectrumChartXAxisValues' - parameter does not 
    //	agree with the size of the spectrometer - detector
    if (spectrumLength != m_spectrumChartXAxisValues.size())
    {
        m_spectrumChartXAxisValues.resize(spectrumLength);
        for (int i = 0; i < spectrumLength; i++)
        {
            m_spectrumChartXAxisValues[i] = (200.0 * i) / spectrumLength;
        }
    }

    // Copy the spectrum and transform it into saturation-ratio (in percent)
    std::vector<double> spectrum1 = m_Spectrometer->GetSpectrum(0);
    for (int k = 0; k < spectrumLength; ++k)
    {
        spectrum1[k] *= conversionSpectrometerIntensityToSaturationInPercent;
    }

    // if we're using a normal measurement mode...
    if (m_spectrometerMode == MODE_TRAVERSE
        || m_spectrometerMode == MODE_WIND
        || m_spectrometerMode == MODE_DIRECTORY)
    {
        // Plot the spectrum
        m_ColumnPlot.SetPlotColor(m_Spectrum0Color);
        m_ColumnPlot.XYPlot(m_spectrumChartXAxisValues.data(), spectrum1.data(), spectrumLength, Graph::CGraphCtrl::PLOT_SECOND_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);

#ifdef _DEBUG

        // Plot the spectrum intensity region
        {
            auto pixelX = m_Spectrometer->GetIntensityRegion();
            std::vector<double> partialSpectrum;
            std::vector<double> scaledXAxisValue;
            partialSpectrum.reserve(pixelX.size());
            for each (double x in pixelX)
            {
                int pixel = std::max(0, std::min(spectrumLength, static_cast<int>(std::round(x))));
                partialSpectrum.push_back(spectrum1[pixel]);
                scaledXAxisValue.push_back(m_spectrumChartXAxisValues[pixel]);
            }

            m_ColumnPlot.SetPlotColor(m_Spectrum1Color);
            m_ColumnPlot.XYPlot(scaledXAxisValue.data(), partialSpectrum.data(), partialSpectrum.size(), Graph::CGraphCtrl::PLOT_SECOND_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);
        }

#endif // _DEBUG


        // If a second channel is used, then do the same thing with the slave-spectrum
        if (m_Spectrometer->m_NChannels > 1)
        {
            // Copy the spectrum of the second channel into saturation-ratio (in percent)
            std::vector<double> spectrum2 = m_Spectrometer->GetSpectrum(1);
            for (int k = 0; k < spectrumLength; ++k)
            {
                spectrum2[k] *= conversionSpectrometerIntensityToSaturationInPercent;
            }

            m_ColumnPlot.SetPlotColor(m_Spectrum1Color);
            m_ColumnPlot.SetLineWidth(2);
            m_ColumnPlot.XYPlot(m_spectrumChartXAxisValues.data(), spectrum2.data(), spectrumLength, Graph::CGraphCtrl::PLOT_SECOND_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);
        }

        return;
    }
    else if (m_spectrometerMode == MODE_VIEW)
    {
        if (m_Spectrometer->m_spectrometerChannel == 0)
        {
            // Plot master spectrum
            m_ColumnPlot.SetPlotColor(m_Spectrum0Color);
            m_ColumnPlot.XYPlot(NULL, spectrum1.data(), spectrumLength, Graph::CGraphCtrl::PLOT_CONNECTED);
        }
        else
        {
            // Plot slave spectrum
            if (m_Spectrometer->m_NChannels < 2)
            {
                return;
            }
            std::vector<double> spectrum2 = m_Spectrometer->GetSpectrum(1);
            for (int k = 0; k < spectrumLength; ++k)
            {
                spectrum2[k] *= conversionSpectrometerIntensityToSaturationInPercent;
            }
            m_ColumnPlot.SetPlotColor(m_Spectrum1Color);
            m_ColumnPlot.SetLineWidth(2);
            m_ColumnPlot.XYPlot(NULL, spectrum2.data(), spectrumLength, Graph::CGraphCtrl::PLOT_CONNECTED);
        }
        return;
    }
}

void CDMSpecView::OnConfigurationOperation()
{
    CString cfgFile; // <-- the path and filename of the configurationfile to read-in
    cfgFile.Format("%s\\cfg.xml", (LPCSTR)g_exePath);
    if (!IsExistingFile(cfgFile))
    {
        cfgFile.Format("%s\\cfg.txt", (LPCSTR)g_exePath);
    }

    // Initiate the configuration-object
    std::shared_ptr<Configuration::CMobileConfiguration> configuration;
    configuration.reset(new Configuration::CMobileConfiguration{ cfgFile });

    // Initiate the configuration-dialog itself
    Configuration::CConfigurationDialog confDlg;
    confDlg.Construct("Configure", this, 0);

    // Initiate the pages in the configuration dialog
    Configuration::CConfigure_Spectrometer m_specPage;
    m_specPage.Construct(IDD_CONFIGURE_SPECTROMETER);
    m_specPage.m_conf = configuration;

    Configuration::CConfigure_GPS m_gpsPage;
    m_gpsPage.Construct(IDD_CONFIGURE_GPS);
    m_gpsPage.m_conf = configuration;

    Configuration::CConfigure_Evaluation m_EvalPage;
    m_EvalPage.Construct(IDD_CONFIGURE_EVALUATION);
    m_EvalPage.m_conf = configuration;

    Configuration::CConfigure_Directory m_DirectoryPage;
    m_DirectoryPage.Construct(IDD_CONFIGURE_DIRECTORY);
    m_DirectoryPage.m_conf = configuration;

    Configuration::CConfigure_Calibration m_ConfigurationPage;
    m_ConfigurationPage.Construct(IDD_CONFIGURE_CALIBRATION);
    m_ConfigurationPage.m_conf = configuration;

    // Add the pages once they have been constructed
    confDlg.AddPage(&m_specPage);
    confDlg.AddPage(&m_gpsPage);
    confDlg.AddPage(&m_EvalPage);
    confDlg.AddPage(&m_DirectoryPage);
    confDlg.AddPage(&m_ConfigurationPage);

    // Open the configuration dialog
    confDlg.DoModal();
}

BOOL CDMSpecView::OnHelpInfo(HELPINFO* pHelpInfo)
{
    ::WinHelp((HWND)pHelpInfo->hItemHandle,
        AfxGetApp()->m_pszHelpFilePath,
        HELP_CONTENTS, NULL);
    return CFormView::OnHelpInfo(pHelpInfo);
}

void CDMSpecView::ReadMobileLog()
{
    char baseNameTxt[256] = "run";
    char txt[256];
    char* pt = 0;
    size_t i, L, d;
    char m_Base[256];
    bool fFoundBaseName = false;

    FILE* f = fopen(g_exePath + "MobileLog.txt", "r");
    if (0 != f)
    {
        while (fgets(txt, sizeof(txt) - 1, f))
        {

            if (pt = strstr(txt, "BASENAME="))
            {
                /* Find the last basename used */
                pt = strstr(txt, "=");
                sscanf(&pt[1], "%255s", &baseNameTxt);
                fFoundBaseName = true;
            }

            if (pt = strstr(txt, "WINDSPEED="))
            {
                /* Find the last windspeed used */
                pt = strstr(txt, "=");
                sscanf(&pt[1], "%lf", &m_WindSpeed);
            }

            if (pt = strstr(txt, "WINDDIRECTION="))
            {
                /* Find the last winddirection used */
                pt = strstr(txt, "=");
                sscanf(&pt[1], "%lf", &m_WindDirection);
            }
        }
    }
    else
    {
        return;
    }
    fclose(f);

    if (fFoundBaseName)
    {
        i = L = strlen(baseNameTxt);
        while (baseNameTxt[i - 1] >= '0' && baseNameTxt[i - 1] <= '9')
        {
            --i;
        }

        if (i == L)
        {
            sprintf(m_Base, "%s%02d", baseNameTxt, 1);
        }
        else
        {
            sscanf(baseNameTxt + i, "%zu", &d);
            baseNameTxt[i] = 0;
            switch (L - i)
            {
            case 1: sprintf(m_Base, "%s%01zu", baseNameTxt, ++d); break;
            case 2: sprintf(m_Base, "%s%02zu", baseNameTxt, ++d); break;
            case 3: sprintf(m_Base, "%s%03zu", baseNameTxt, ++d); break;
            case 4: sprintf(m_Base, "%s%04zu", baseNameTxt, ++d); break;
            case 5: sprintf(m_Base, "%s%05zu", baseNameTxt, ++d); break;
            case 6: sprintf(m_Base, "%s%06zu", baseNameTxt, ++d); break;
            case 7: sprintf(m_Base, "%s%07zu", baseNameTxt, ++d); break;
            }
        }
        m_BaseEdit.SetWindowText(m_Base);
        UpdateData(FALSE);
    }
}

LRESULT CDMSpecView::OnShowInformationDialog(WPARAM wParam, LPARAM lParam)
{

    Dialogs::CInformationDialog infDlg;
    if (wParam == DARK_DIALOG)
        infDlg.informationString = "The offset level has dropped more than 20 counts since last dark spectrum was measured. Please consider restarting the measurements when appropriate.";
    else if (wParam == INVALID_GPS)
        infDlg.informationString = "Lost contact with GPS-satellites, no valid GPS-data recived.";
    else if (wParam == CHANGED_EXPOSURETIME)
        infDlg.informationString = "Exposuretime has changed, please cover the telescope to get a new dark measurement";

    infDlg.DoModal();

    return 0;
}

void CDMSpecView::OnViewRealtimeroute()
{
    if (m_realTimeRouteGraph.fVisible)
    {
        m_realTimeRouteGraph.DestroyWindow();
    }
    else
    {
        /* create the real time route graph */
        m_realTimeRouteGraph.Create(IDD_REALTIME_ROUTE_DLG, NULL);

        if (s_spectrometerAcquisitionThreadIsRunning)
        {
            m_realTimeRouteGraph.m_spectrometer = this->m_Spectrometer;
            m_realTimeRouteGraph.m_intensityLimit = m_Spectrometer->m_spectrometerDynRange * (100 - m_intensitySliderLow.GetPos());
        }
        else
        {
            m_realTimeRouteGraph.m_spectrometer = nullptr;
            m_realTimeRouteGraph.m_intensityLimit = 0.25 * 4095;
        }
        m_realTimeRouteGraph.ShowWindow(SW_SHOW);
    }
    m_realTimeRouteGraph.fVisible = !m_realTimeRouteGraph.fVisible;
}

void CDMSpecView::OnUpdateViewRealtimeroute(CCmdUI* pCmdUI)
{
    if (m_realTimeRouteGraph.fVisible)
    {
        pCmdUI->SetCheck(BST_CHECKED);
    }
    else
    {
        pCmdUI->SetCheck(BST_UNCHECKED);
    }
}

void CDMSpecView::OnUpdateViewColumnError(CCmdUI* pCmdUI)
{
    if (m_showErrorBar)
    {
        pCmdUI->SetCheck(BST_CHECKED);
    }
    else
    {
        pCmdUI->SetCheck(BST_UNCHECKED);
    }
}

void CDMSpecView::OnUpdateViewSpectrumFit(CCmdUI* pCmdUI)
{
    if (m_showFitDlg.m_isVisible)
    {
        pCmdUI->SetCheck(BST_CHECKED);
    }
    else
    {
        pCmdUI->SetCheck(BST_UNCHECKED);
    }
}

void CDMSpecView::OnViewSpectrumFit()
{
    if (m_showFitDlg.m_isVisible)
    {
        m_showFitDlg.DestroyWindow();
    }
    else
    {
        /* create the real time route graph */
        m_showFitDlg.Create(IDD_VIEW_FIT_DLG, NULL);

        if (s_spectrometerAcquisitionThreadIsRunning)
        {
            m_showFitDlg.m_spectrometer = this->m_Spectrometer;
        }
        else
        {
            m_showFitDlg.m_spectrometer = nullptr;
        }
        m_showFitDlg.ShowWindow(SW_SHOW);
    }
    m_showFitDlg.m_isVisible = !m_showFitDlg.m_isVisible;
}

void CDMSpecView::OnControlAddComment()
{
    Dialogs::CCommentDlg cdlg;

    if (s_spectrometerAcquisitionThreadIsRunning)
    {
        mobiledoas::GpsData gps;
        m_Spectrometer->GetGpsPos(gps);
        cdlg.lat = gps.latitude;
        cdlg.lon = gps.longitude;
        cdlg.alt = gps.altitude;
        cdlg.t = gps.time;
        cdlg.outputDir = m_Spectrometer->CurrentOutputDirectory();
    }
    else
    {
        cdlg.lat = cdlg.lon = cdlg.alt = 0;
        cdlg.t = 0;
        cdlg.outputDir.Format(g_exePath);
    }

    /*  cdlg.Create(IDD_COMMENT_DLG, NULL);
      cdlg.ShowWindow(SW_SHOW);*/
    cdlg.DoModal();
}

void CDMSpecView::OnControlReevaluate()
{
    /* the reevaluation dialog */
    CReEvaluationDlg reEvalDlg;
    reEvalDlg.Construct("ReEvaluation", this, 0);

    // open the window
    reEvalDlg.DoModal();
}

void CDMSpecView::OnUpdateControlReevaluate(CCmdUI* pCmdUI)
{
#ifdef _DEBUG
    pCmdUI->Enable(TRUE);
#else
    pCmdUI->Enable(TRUE);
#endif
}
void CDMSpecView::OnConfigurationChangeexposuretime()
{
    if (m_Spectrometer != nullptr)
    {
        m_Spectrometer->RequestIntegrationTimeChange();
    }
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

void CDMSpecView::OnMenuControlRunTheGPS()
{
    CString status;

    // set the baudrate for the connection
    const std::vector<long> baudrate{ 4800, 9600 };

    for (int port = 1; port < 256; ++port)
    {
        for (long baudrateToTest : baudrate)
        {
            // try this serial-port and see what happens
            status.Format("Testing port: COM%d Baud rate: %d", port, baudrateToTest);
            ShowStatusMsg(status);

            // test the serial-port
            mobiledoas::CSerialConnection serial;
            if (!serial.Init(port, baudrateToTest))
            {
                // could not connect to this serial-port
                continue;
            }
            // it was possible to open the serial-port, test if there is a gps on this port

            mobiledoas::CGPS gps(std::move(serial));
            for (int i = 0; i < 10; ++i)
            {
                if (SUCCESS == gps.ReadGPS())
                {
                    mobiledoas::GpsAsyncReader gpsReader(std::move(gps));
                    return;
                }
                Sleep(10);
            }
        }
    }
    status = "No GPS reciever could be found";
    ShowStatusMsg(status);
    MessageBox(status);
}

void CDMSpecView::OnMenuControlTestTheGPS()
{
    CString status;

    // set the baudrate for the connection
    const std::vector<long> baudrate{ 4800, 9600 };

    for (int port = 1; port < 256; ++port)
    {
        for (long baudrateToTest : baudrate)
        {
            // try this serial-port and see what happens
            status.Format("Testing port: COM%d Baud rate: %d", port, baudrateToTest);
            ShowStatusMsg(status);

            // test the serial-port
            mobiledoas::CSerialConnection serial;
            if (!serial.Init(port, baudrateToTest))
            {
                // could not connect to this serial-port
                continue;
            }
            // it was possible to open the serial-port, test if there is a gps on this port

            mobiledoas::CGPS gps(std::move(serial));
            for (int i = 0; i < 10; ++i)
            {
                if (SUCCESS == gps.ReadGPS())
                {
                    status.Format("Found GPS on serialPort: COM%d using baud rate %d", port, baudrateToTest);
                    ShowStatusMsg(status);
                    MessageBox(status, "Found GPS reciever");

                    return;
                }

                Sleep(10);
            }
        }
    }
    status = "No GPS reciever could be found";
    ShowStatusMsg(status);
    MessageBox(status);
}

void CDMSpecView::UpdateLegend()
{
    if (m_Spectrometer == nullptr)
    {
        // if the program has not been started yet
        m_colorLabelSpectrum1.ShowWindow(SW_HIDE);
        m_colorLabelSpectrum2.ShowWindow(SW_HIDE);
        m_colorLabelSeries1.ShowWindow(SW_HIDE);
        m_colorLabelSeries2.ShowWindow(SW_HIDE);
        m_legendSpectrum1.ShowWindow(SW_HIDE);
        m_legendSpectrum2.ShowWindow(SW_HIDE);
        m_legendSeries1.ShowWindow(SW_HIDE);
        m_legendSeries2.ShowWindow(SW_HIDE);
    }
    else
    {
        if (m_spectrometerMode == MODE_TRAVERSE)
        {
            m_colorLabelSpectrum1.SetBackgroundColor(this->m_Spectrum0Color);
            m_colorLabelSeries1.SetBackgroundColor(this->m_PlotColor[0]);
            m_colorLabelSeries1.ShowWindow(SW_SHOW);
            m_legendSeries1.SetWindowText(m_Spectrometer->GetFitWindowName(0));
            m_legendSeries1.ShowWindow(SW_SHOW);
            m_legendSpectrum1.ShowWindow(SW_SHOW);
            m_colorLabelSpectrum1.ShowWindow(SW_SHOW);

            if (m_Spectrometer->m_NChannels > 1)
            {
                m_legendSpectrum2.ShowWindow(SW_SHOW);
                m_colorLabelSpectrum2.SetBackgroundColor(this->m_Spectrum1Color);
                m_colorLabelSpectrum2.ShowWindow(SW_SHOW);
                m_colorLabelSeries2.SetBackgroundColor(this->m_PlotColor[1]);
                m_colorLabelSeries2.ShowWindow(SW_SHOW);
                m_legendSeries2.ShowWindow(SW_SHOW);
                m_legendSeries2.SetWindowText(m_Spectrometer->GetFitWindowName(1));
            }
        }
        if (m_spectrometerMode == MODE_WIND)
        {
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
        if (m_spectrometerMode == MODE_VIEW)
        {

            if (m_Spectrometer->m_spectrometerChannel == 0)
            {
                m_colorLabelSpectrum1.SetBackgroundColor(this->m_Spectrum0Color);
                m_colorLabelSeries1.SetBackgroundColor(this->m_PlotColor[0]);
                SetDlgItemText(IDC_LABEL_SPECTRUM, "Spectrum (Master)");
                m_legendSpectrum2.ShowWindow(SW_HIDE);
                m_colorLabelSpectrum2.ShowWindow(SW_HIDE);
                m_legendSpectrum1.ShowWindow(SW_SHOW);
                m_colorLabelSpectrum1.ShowWindow(SW_SHOW);
            }
            else
            {
                m_colorLabelSpectrum2.SetBackgroundColor(this->m_Spectrum1Color);
                m_colorLabelSeries2.SetBackgroundColor(this->m_PlotColor[1]);
                SetDlgItemText(IDC_LABEL_SPECTRUM2, "Spectrum (Slave)");
                m_legendSpectrum1.ShowWindow(SW_HIDE);
                m_colorLabelSpectrum1.ShowWindow(SW_HIDE);
                m_legendSpectrum2.ShowWindow(SW_SHOW);
                m_colorLabelSpectrum2.ShowWindow(SW_SHOW);
            }
        }

    }
}

/** Toggles the showing of the column error bars */
void CDMSpecView::OnViewColumnError()
{
    CString msg;
    m_showErrorBar = !m_showErrorBar;

    if (m_showErrorBar)
        msg.Format("Showing Error Bars");
    else
        msg.Format("Hiding Error Bars");

    ShowStatusMsg(msg);
}

void CDMSpecView::OnUpdate_EnableOnRun(CCmdUI* pCmdUI)
{
    if (m_Spectrometer != nullptr && m_Spectrometer->m_isRunning)
    {
        // enable
        pCmdUI->Enable(TRUE);
    }
    else
    {
        // disable
        pCmdUI->Enable(FALSE);
    }
}

void CDMSpecView::OnUpdate_DisableOnRun(CCmdUI* pCmdUI)
{
    if (m_Spectrometer != nullptr && m_Spectrometer->m_isRunning)
    {
        // disable
        pCmdUI->Enable(FALSE);
    }
    else
    {
        // enable
        pCmdUI->Enable(TRUE);
    }
}

void CDMSpecView::OnUpdateWindMeasurement(CCmdUI* pCmdUI)
{
    // disable
    pCmdUI->Enable(FALSE); // unfortunately does not OmniDriver not yet support dual-beam wind measurements. Disable this in the meantime...
}

void CDMSpecView::OnUpdate_StartTheGps(CCmdUI* pCmdUI)
{
    // only available for debugging...
#ifdef _DEBUG
    pCmdUI->Enable(TRUE);
#else
    pCmdUI->Enable(FALSE);
#endif // _DEBUG
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


void CDMSpecView::SoundAlarm()
{
    CString fileToPlay;
    TCHAR windowsDir[MAX_PATH + 1];
    GetWindowsDirectory(windowsDir, MAX_PATH + 1);

    DWORD volume = (DWORD)(0xFFFF);
    MMRESULT res = waveOutSetVolume(0, volume);
    fileToPlay.Format("%s\\Media\\Windows Error.wav", windowsDir);

    PlaySound(fileToPlay, 0, SND_SYNC);
}

void CDMSpecView::OnAnalysisCalibratespectrometer()
{
    CSpectrometerCalibrationDlg dlg;
    dlg.DoModal();
}
