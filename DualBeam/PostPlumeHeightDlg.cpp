#undef min
#undef max

#include "stdafx.h"
#include "../DMSpec.h"
#include "PostPlumeHeightDlg.h"
#include "../SourceSelectionDlg.h"
#include <algorithm>

// CPostPlumeHeightDlg dialog

using namespace DualBeamMeasurement;

IMPLEMENT_DYNAMIC(CPostPlumeHeightDlg, CDialog)
CPostPlumeHeightDlg::CPostPlumeHeightDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPostPlumeHeightDlg::IDD, pParent)
{
	m_flux = NULL;

	for(int k = 0; k < MAX_N_SERIES; ++k){
		this->m_OriginalSeries[k] = NULL;
		this->m_PreparedSeries[k] = NULL;
	}	
	m_automatic = false;
	m_nChannels = 0;
	m_sourceLat = 0.0;
	m_sourceLon = 0.0;
}

CPostPlumeHeightDlg::~CPostPlumeHeightDlg()
{
	if(m_flux != NULL){
		delete m_flux;
		m_flux = NULL;
	}
	for(int k = 0; k < MAX_N_SERIES; ++k){
		delete m_OriginalSeries[k];
		delete m_PreparedSeries[k];
	}
}

void CPostPlumeHeightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FRAME_TIME_SERIES, m_frameColumn);

	// The settings
	DDX_Text(pDX, IDC_EDIT_ANGLESEPARATION,	m_settings.angleSeparation);
	DDX_Text(pDX, IDC_EDIT_LATITUDE,				m_sourceLat);
	DDX_Text(pDX, IDC_EDIT_LONGITUDE,				m_sourceLon);
}


BEGIN_MESSAGE_MAP(CPostPlumeHeightDlg, CDialog)
	// Actions to perform
	ON_BN_CLICKED(IDC_BTN_BROWSE_EVALLOG,						OnBrowseEvallog)
	ON_BN_CLICKED(IDC_BUTTON_CALCULATE_CORRELATION,	OnCalculatePlumeHeight)
	//ON_EN_CHANGE(IDC_EDIT_EVALLOG,									OnChangeEvalLog)

	// Changing the settings
	ON_BN_CLICKED(IDC_BUTTON_SOURCE_LAT, OnBnClickedButtonSourceLat)
	ON_BN_CLICKED(IDC_BUTTON_SOURCE_LON, OnBnClickedButtonSourceLon)
END_MESSAGE_MAP()


// CPostPlumeHeightDlg message handlers
void CPostPlumeHeightDlg::OnBrowseEvallog()
{
	CString evLog;
	evLog.Format("");
	TCHAR filter[512];
	int n = _stprintf(filter, "Evaluation Logs\0");
	n += _stprintf(filter + n + 1, "*.txt;\0");
	filter[n + 2] = 0;

	// let the user browse for an evaluation log file and if one is selected, read it
	if(Common::BrowseForFile(filter, evLog)){
		m_evalLog.Format("%s", evLog);

		// Read the newly opened evaluation-log
		if (ReadEvaluationLog()) {
			m_automatic = true;

			// Update the text on the screen
			SetDlgItemText(IDC_EDIT_EVALLOG, m_evalLog);

			// Redraw the screen
			DrawColumn();

			m_automatic = false;
		}
	}


}

void CPostPlumeHeightDlg::OnChangeEvalLog(){
	CString evalLog;
	// Get the name of the eval-log
	GetDlgItemText(IDC_EDIT_EVALLOG, evalLog);

	// Check if the file exists
	if(strlen(evalLog) <= 3 || !Equals(evalLog.Right(4), ".txt") || m_automatic)
		return;

	FILE *f = fopen(evalLog, "r");
	if(f == NULL)
		return;
	fclose(f);

	// Read the evaluation - log
	m_evalLog.Format(evalLog);
	ReadEvaluationLog();

	// Redraw the screen
	DrawColumn();
}

bool CPostPlumeHeightDlg::ReadEvaluationLog(){
	// Completely reset the old data
	if(m_flux != NULL)
		delete m_flux;
	m_flux = new Flux::CFlux();

	double fileVersion;	// the file-version of the evalution-log file

  // Read the header of the log file and see if it is an ok file
  int fileType = m_flux->ReadSettingFile(m_evalLog, m_nChannels, fileVersion);
  if(fileType != 1){
	  MessageBox(TEXT("The file is not evaluation log file with right format.\nPlease choose a right file"),NULL,MB_OK);
		return FAIL;
  }
  if (m_nChannels < 2) {
	  MessageBox(TEXT("The evaluation log does not contain 2 channels."), NULL, MB_OK);
	  return FAIL;
  }

  // Read the data from the file
  int oldNumberOfFilesOpened = m_flux->m_traverseNum;
  if(0 == m_flux->ReadLogFile("", m_evalLog, m_nChannels, fileVersion)){
    MessageBox(TEXT("That file is empty"));
    return FAIL;
  }

	// Make sure that there are not more channels then the program can handle
	m_nChannels = std::min(m_nChannels, MAX_N_SERIES);

	// Copy the data to the local variables 'm_originalSeries'
	for(int chnIndex = 0; chnIndex < m_nChannels; ++chnIndex){
		// to get less dereferencing
		Flux::CTraverse *traverse = m_flux->m_traverse[chnIndex];

		long length = traverse->m_recordNum; // the length of the traverse

		// create a new data series
		m_OriginalSeries[chnIndex] = new CDualBeamCalculator::CMeasurementSeries(length);
		if(m_OriginalSeries[chnIndex] == NULL)
			return FAIL; // <-- failed to allocate enough memory

		Time &startTime = traverse->time[0];
		for(int specIndex = 0; specIndex < length; ++specIndex){

			// Get the column of this spectrum
			m_OriginalSeries[chnIndex]->column[specIndex] = traverse->columnArray[specIndex];

			// Get the latitude 
			m_OriginalSeries[chnIndex]->lat[specIndex] = traverse->latitude[specIndex];

			// Get the longitude
			m_OriginalSeries[chnIndex]->lon[specIndex] = traverse->longitude[specIndex];

			// Get the start time of this spectrum
			Time &t = traverse->time[specIndex];

			// Save the time difference
			m_OriginalSeries[chnIndex]->time[specIndex]		= 
				3600 * (t.hour - startTime.hour) + 
				60	 * (t.minute - startTime.minute) + 
				(t.second - startTime.second);
		}
	}

	// Correct the newly read in time-series for some common problems
	CorrectTimeSeries();

	return SUCCESS;
}

BOOL CPostPlumeHeightDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Initialize the column graph
  CRect rect;
	m_frameColumn.GetWindowRect(rect);
  int height = rect.bottom - rect.top;
  int width  = rect.right - rect.left;
  rect.top = 20; rect.bottom = height - 10;
  rect.left = 10; rect.right = width - 10;
  m_columnGraph.Create(WS_VISIBLE | WS_CHILD, rect, &m_frameColumn);
  m_columnGraph.SetXUnits("Time [s]");
  m_columnGraph.SetYUnits("Column [ppmm]");
  m_columnGraph.SetBackgroundColor(RGB(0, 0, 0));
  m_columnGraph.SetPlotColor(RGB(255, 0, 0));
  m_columnGraph.SetGridColor(RGB(255, 255, 255));
  m_columnGraph.SetRange(0, 600, 0, 0, 100, 1);

	// Setup the colors to use for the different time-series
	m_colorSeries[0] = RGB(255, 0, 0);
	m_colorSeries[1] = RGB(0, 0, 255);
	m_colorSeries[2] = RGB(0, 255, 0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CPostPlumeHeightDlg::DrawColumn(){
	double minT = 1e16, maxT = -1e16, minC = 1e16, maxC = -1e16;
	int nSeries = 0;
		
	// get the range for the plot
	for(int k = 0; k < m_nChannels; ++k){
		if(m_OriginalSeries[k] != NULL){
			minT = std::min(minT, m_OriginalSeries[k]->time[0]);
			maxT = std::max(maxT, m_OriginalSeries[k]->time[m_OriginalSeries[k]->length-1]);

			minC = std::min(minC, Min(m_OriginalSeries[k]->column, m_OriginalSeries[k]->length));
			maxC = std::max(maxC, Max(m_OriginalSeries[k]->column, m_OriginalSeries[k]->length));

			++nSeries;
		}
	}

	if(nSeries == 0)
		return; // <-- nothing to plot

	// Set the range for the plot
	m_columnGraph.SetRange(minT, maxT, 0,		minC, maxC, 1);

	// Draw the time series
	for(int k = 0; k < m_nChannels; ++k){
		if(m_OriginalSeries[k] != NULL){

			// ---------- Draw the original time series -----------
			// set the color
			m_columnGraph.SetPlotColor(m_colorSeries[k % 3]);

			// draw the series
			m_columnGraph.XYPlot(
				m_OriginalSeries[k]->time, 
				m_OriginalSeries[k]->column, 
				m_OriginalSeries[k]->length,
				Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);

			// ---------- Draw the filtered time series -----------
//			if(m_settings.lowPassFilterAverage > 0){
//				m_columnGraph.SetLineWidth(2);
//				m_columnGraph.SetPlotColor(RGB(255,255,255));
//				// perform the low pass filtering
//				LowPassFilter(k);
//
//				// draw the series
//				m_columnGraph.XYPlot(
//					m_PreparedSeries[k]->time, 
//					m_PreparedSeries[k]->column, 
//					m_PreparedSeries[k]->length,
//					Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);
//
////				m_columnGraph.SetLineWidth(1);
//			}

		}

	}
}

/** Performes a low pass filtering procedure on series number 'seriesNo'.
		The number of iterations is taken from 'm_settings.lowPassFilterAverage'
		The treated series is m_OriginalSeries[seriesNo]
		The result is saved as m_PreparedSeries[seriesNo]	*/
int	CPostPlumeHeightDlg::LowPassFilter(int seriesNo){
	if(m_settings.lowPassFilterAverage <= 0)
		return 0;

	if(seriesNo < 0 || seriesNo > MAX_N_SERIES)
		return 0;

	if(m_OriginalSeries[seriesNo] == NULL)
		return 0;

	int length = m_OriginalSeries[seriesNo]->length;
	if(length <= 0)
		return 0;

	if(m_PreparedSeries[seriesNo] == NULL)
		m_PreparedSeries[seriesNo] = new CDualBeamCalculator::CMeasurementSeries();

	if(SUCCESS != CPlumeHeightCalculator::LowPassFilter(m_OriginalSeries[seriesNo], m_PreparedSeries[seriesNo], m_settings.lowPassFilterAverage))
		return 0;

	return 1;
}


void CPostPlumeHeightDlg::OnCalculatePlumeHeight()
{
	UpdateData(TRUE); // <-- start by saving the data in the dialog

	// Calculate the distance through the displacement of the centre of masses
	double windDirection;
	double plumeHeight = m_calc.GetPlumeHeight_CentreOfMass(m_OriginalSeries[0], m_OriginalSeries[1], m_sourceLat, m_sourceLon, m_settings.angleSeparation, &windDirection);

	// Give the user the answer
	CString msg;
	msg.Format("%.1lf", plumeHeight);
	SetDlgItemText(IDC_EDIT_PLUMEHEIGHT, msg);
	msg.Format("%.1lf", windDirection);
	SetDlgItemText(IDC_EDIT_WINDDIRECTION, msg);
	
	DrawColumn();
}


/** Corrects the time-series m_OriginalSeries for some common problems */
void CPostPlumeHeightDlg::CorrectTimeSeries(){
	for(int chn = 0; chn < m_nChannels; ++chn){

		for(int spec = 1; spec < m_OriginalSeries[chn]->length-1; ++spec){
			double *lat = m_OriginalSeries[chn]->lat;
			double *lon = m_OriginalSeries[chn]->lon;
			double *time = m_OriginalSeries[chn]->time;

			// If this latitude is same as for the last spectrum, then interpolate
			if(lat[spec] == lat[spec-1]){
				lat[spec] = 0.5*(lat[spec-1] + lat[spec + 1]);
			}
			// If this longitude is same as for the last spectrum, then interpolate
			if(lon[spec] == lon[spec-1]){
				lon[spec] = 0.5*(lon[spec-1] + lon[spec + 1]);
			}
			// If this starttime is unreasonable, then interpolate
			if(time[spec] < time[spec-1] || time[spec] < 0){
				time[spec] = 0.5 * (time[spec-1] + time[spec+1]);
			}
		}
	}
}
void CPostPlumeHeightDlg::OnBnClickedButtonSourceLat()
{
  Dialogs::CSourceSelectionDlg sourceDlg;
  INT_PTR modal = sourceDlg.DoModal();
  if(IDOK == modal){
    m_sourceLat = sourceDlg.m_selectedLat;
    m_sourceLon = sourceDlg.m_selectedLon;

		UpdateData(FALSE);
  }

  UpdateData(FALSE);
}

void CPostPlumeHeightDlg::OnBnClickedButtonSourceLon()
{
	OnBnClickedButtonSourceLat();
}
