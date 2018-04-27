#include "stdafx.h"
#include "../DMSpec.h"
#include "PostWindDlg.h"

// CPostWindDlg dialog

using namespace DualBeamMeasurement;

IMPLEMENT_DYNAMIC(CPostWindDlg, CDialog)
CPostWindDlg::CPostWindDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPostWindDlg::IDD, pParent)
{
	m_flux = nullptr;
	m_showOption = 0;

	for (int k = 0; k < MAX_N_SERIES; ++k) {
		this->m_OriginalSeries[k] = nullptr;
		this->m_PreparedSeries[k] = nullptr;
	}

	corr = delay = ws = nullptr;
}

CPostWindDlg::~CPostWindDlg()
{
	if (m_flux != nullptr) {
		delete m_flux;
		m_flux = nullptr;
	}
	for (int k = 0; k < MAX_N_SERIES; ++k) {
		delete m_OriginalSeries[k];
		delete m_PreparedSeries[k];
	}

	delete[] corr;
	delete[] delay;
	delete[] ws;
}

void CPostWindDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FRAME_TIME_SERIES, m_frameColumn);
	DDX_Control(pDX, IDC_FRAME_RESULT, m_frameResult);

	// The settings
	DDX_Text(pDX, IDC_EDIT_LPITERATIONS, m_settings.lowPassFilterAverage);
	DDX_Text(pDX, IDC_EDIT_MAXSHIFTTIME, m_settings.shiftMax);
	DDX_Text(pDX, IDC_EDIT_TESTLENGTH, m_settings.testLength);
	DDX_Text(pDX, IDC_EDIT_PLUMEHEIGHT, m_settings.plumeHeight);

	// Deciding what to show
	DDX_Radio(pDX, IDC_RADIO_SHOW_CORR, m_showOption);
}


BEGIN_MESSAGE_MAP(CPostWindDlg, CDialog)
	// Actions to perform
	ON_BN_CLICKED(IDC_BTN_BROWSE_EVALLOG, OnBrowseEvallog)
	ON_BN_CLICKED(IDC_BUTTON_CALCULATE_CORRELATION, OnCalculateWindspeed)
	//ON_EN_CHANGE(IDC_EDIT_EVALLOG,									OnChangeEvalLog)

	// Changing the settings
	ON_EN_CHANGE(IDC_EDIT_LPITERATIONS, OnChangeLPIterations)
	ON_EN_CHANGE(IDC_EDIT_PLUMEHEIGHT, OnChangePlumeHeight)

	// Changing what to show
	ON_BN_CLICKED(IDC_RADIO_SHOW_CORR, DrawResult)
	ON_BN_CLICKED(IDC_RADIO_SHOW_DELAY, DrawResult)
	ON_BN_CLICKED(IDC_RADIO_SHOW_WS, DrawResult)

END_MESSAGE_MAP()


// CPostWindDlg message handlers

void CPostWindDlg::OnBrowseEvallog()
{
	CString evLog;
	evLog.Format("");
	TCHAR filter[512];
	int n = _stprintf(filter, "Evaluation Logs\0");
	n += _stprintf(filter + n + 1, "*.txt;\0");
	filter[n + 2] = 0;

	// let the user browse for an evaluation log file and if one is selected, read it
	if (Common::BrowseForFile(filter, evLog)) {
		m_evalLog.Format("%s", evLog);

		if (ReadEvaluationLog()) {
			// Update the text on the screen
			SetDlgItemText(IDC_EDIT_EVALLOG, m_evalLog);
			// Redraw the screen
			DrawColumn();
		}
	}

}

void CPostWindDlg::OnChangeEvalLog() {
	CString evalLog;
	// Get the name of the eval-log
	GetDlgItemText(IDC_EDIT_EVALLOG, evalLog);

	// Check if the file exists
	if (strlen(evalLog) <= 3 || !Equals(evalLog.Right(4), ".txt"))
		return;

	FILE *f = fopen(evalLog, "r");
	if (f == nullptr)
		return;
	fclose(f);

	// Read the evaluation - log
	m_evalLog.Format(evalLog);
	ReadEvaluationLog();

	// Redraw the screen
	DrawColumn();
}

bool CPostWindDlg::ReadEvaluationLog() {
	// Completely reset the old data
	if (m_flux != nullptr)
		delete m_flux;
	m_flux = new Flux::CFlux();

	int nChannels;			// the number of channels in the data-file
	double fileVersion;	// the file-version of the evalution-log file

						// Read the header of the log file and see if it is an ok file
	int fileType = m_flux->ReadSettingFile(m_evalLog, nChannels, fileVersion);
	if (fileType != 1) {
		MessageBox(TEXT("The file is not evaluation log file with right format.\nPlease choose a right file"), NULL, MB_OK);
		return FAIL;
	}

	if (nChannels < 2) {
		MessageBox(TEXT("The evaluation log does not contain 2 channels."), NULL, MB_OK);
		return FAIL;
	}

	// Read the data from the file
	int oldNumberOfFilesOpened = m_flux->m_traverseNum;
	if (0 == m_flux->ReadLogFile("", m_evalLog, nChannels, fileVersion)) {
		MessageBox(TEXT("That file is empty"));
		return FAIL;
	}

	// Copy the data to the local variables 'm_originalSeries'
	for (int chnIndex = 0; chnIndex < nChannels; ++chnIndex) {
		// to get less dereferencing
		Flux::CTraverse *traverse = m_flux->m_traverse[chnIndex];

		long length = traverse->m_recordNum; // the length of the traverse

											 // create a new data series
		m_OriginalSeries[chnIndex] = new DualBeamMeasurement::CDualBeamCalculator::CMeasurementSeries(length);
		if (m_OriginalSeries[chnIndex] == nullptr)
			return FAIL; // <-- failed to allocate enough memory

		Time &startTime = traverse->time[0];
		for (int specIndex = 0; specIndex < length; ++specIndex) {

			// Get the column of this spectrum
			m_OriginalSeries[chnIndex]->column[specIndex] = traverse->columnArray[specIndex];

			// Get the start time of this spectrum
			Time &t = traverse->time[specIndex];

			// Save the time difference
			m_OriginalSeries[chnIndex]->time[specIndex] =
				3600 * (t.hour - startTime.hour) +
				60 * (t.minute - startTime.minute) +
				(t.second - startTime.second);
		}
	}

	return SUCCESS;
}

BOOL CPostWindDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Initialize the column graph
	CRect rect;
	m_frameColumn.GetWindowRect(rect);
	int height = rect.bottom - rect.top;
	int width = rect.right - rect.left;
	rect.top = 20; rect.bottom = height - 10;
	rect.left = 10; rect.right = width - 10;
	m_columnGraph.Create(WS_VISIBLE | WS_CHILD, rect, &m_frameColumn);
	m_columnGraph.SetXUnits("Time [s]");
	m_columnGraph.SetYUnits("Column [ppmm]");
	m_columnGraph.SetBackgroundColor(RGB(0, 0, 0));
	m_columnGraph.SetPlotColor(RGB(255, 0, 0));
	m_columnGraph.SetGridColor(RGB(255, 255, 255));
	m_columnGraph.SetRange(0, 600, 0, 0, 100, 1);

	// Initialize the results graph
	m_frameResult.GetWindowRect(rect);
	height = rect.bottom - rect.top;
	width = rect.right - rect.left;
	rect.top = 20; rect.bottom = height - 10;
	rect.left = 10; rect.right = width - 10;
	m_resultGraph.Create(WS_VISIBLE | WS_CHILD, rect, &m_frameResult);
	m_resultGraph.SetXUnits("Time [s]");
	m_resultGraph.SetYUnits("Wind Speed [m/s]");
	m_resultGraph.SetBackgroundColor(RGB(0, 0, 0));
	m_resultGraph.SetPlotColor(RGB(255, 0, 0));
	m_resultGraph.SetGridColor(RGB(255, 255, 255));
	m_resultGraph.SetRange(0, 600, 0, 0, 100, 1);

	// Setup the colors to use for the different time-series
	m_colorSeries[0] = RGB(255, 0, 0);
	m_colorSeries[1] = RGB(0, 0, 255);
	m_colorSeries[2] = RGB(0, 255, 0);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CPostWindDlg::DrawColumn() {
	double minT = 1e16, maxT = -1e16, minC = 1e16, maxC = -1e16;
	int nSeries = 0;

	// get the range for the plot
	for (int k = 0; k < MAX_N_SERIES; ++k) {
		if (m_OriginalSeries[k] != nullptr) {
			minT = min(minT, m_OriginalSeries[k]->time[0]);
			maxT = max(maxT, m_OriginalSeries[k]->time[m_OriginalSeries[k]->length - 1]);

			minC = min(minC, Min(m_OriginalSeries[k]->column, m_OriginalSeries[k]->length));
			maxC = max(maxC, Max(m_OriginalSeries[k]->column, m_OriginalSeries[k]->length));

			++nSeries;
		}
	}

	if (nSeries == 0)
		return; // <-- nothing to plot

				// Set the range for the plot
	m_columnGraph.SetRange(minT, maxT, 0, minC, maxC, 1);

	// Draw the time series
	for (int k = 0; k < MAX_N_SERIES; ++k) {
		if (m_OriginalSeries[k] != nullptr) {

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
			if (m_settings.lowPassFilterAverage > 0) {
				m_columnGraph.SetLineWidth(2);
				m_columnGraph.SetPlotColor(RGB(255, 255, 255));
				// perform the low pass filtering
				LowPassFilter(k);

				// draw the series
				m_columnGraph.XYPlot(
					m_PreparedSeries[k]->time,
					m_PreparedSeries[k]->column,
					m_PreparedSeries[k]->length,
					Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);

				//				m_columnGraph.SetLineWidth(1);
			}

		}

	}
}

/** Draws the result */
void	CPostWindDlg::DrawResult() {
	static const int BUFFER_SIZE = 1024;
	if (corr == nullptr) {
		corr = new double[BUFFER_SIZE];
		delay = new double[BUFFER_SIZE];
		ws = new double[BUFFER_SIZE];
	}

	if (m_calc.m_length == 0 || m_calc.corr == 0)
		return;

	UpdateData(TRUE); // <-- get the options for how to plot the data

					  /** Copy the values to the local buffers */
	int length = 0;
	double distance = m_settings.plumeHeight * tan(DEGREETORAD * m_settings.angleSeparation);
	for (int k = 0; k < m_calc.m_length; ++k) {
		if (length > BUFFER_SIZE)
			break;
		if (m_calc.used[k]) {
			corr[length] = m_calc.corr[k];
			delay[length] = m_calc.delays[k];
			if (fabs(m_calc.delays[k]) < 1e-5)
				ws[length] = 0.0;
			else
				ws[length] = distance / m_calc.delays[k];
			++length;
		}
	}

	if (m_showOption == 0) {
		/** Draw the correlation */
		m_resultGraph.SetYUnits("Correlation [-]");
		m_resultGraph.XYPlot(NULL, corr, length);
	}
	else if (m_showOption == 1) {
		/** Draw the delay */
		m_resultGraph.SetYUnits("Temporal delay [s]");
		m_resultGraph.XYPlot(NULL, delay, length);
	}
	else if (m_showOption == 2) {
		/** Draw the resulting wind speed */
		m_resultGraph.SetYUnits("Wind speed [m/s]");
		m_resultGraph.XYPlot(NULL, ws, length);
	}
}

/** Performes a low pass filtering procedure on series number 'seriesNo'.
The number of iterations is taken from 'm_settings.lowPassFilterAverage'
The treated series is m_OriginalSeries[seriesNo]
The result is saved as m_PreparedSeries[seriesNo]	*/
int	CPostWindDlg::LowPassFilter(int seriesNo) {
	if (m_settings.lowPassFilterAverage <= 0)
		return 0;

	if (seriesNo < 0 || seriesNo > MAX_N_SERIES)
		return 0;

	if (m_OriginalSeries[seriesNo] == nullptr)
		return 0;

	int length = m_OriginalSeries[seriesNo]->length;
	if (length <= 0)
		return 0;

	if (m_PreparedSeries[seriesNo] == nullptr)
		m_PreparedSeries[seriesNo] = new CWindSpeedCalculator::CMeasurementSeries();

	if (SUCCESS != CWindSpeedCalculator::LowPassFilter(m_OriginalSeries[seriesNo], m_PreparedSeries[seriesNo], m_settings.lowPassFilterAverage))
		return 0;

	return 1;
}

void CPostWindDlg::OnChangeLPIterations()
{
	UpdateData(TRUE); // <-- save the data in the dialog
	if (m_settings.lowPassFilterAverage >= 0)
		DrawColumn();
}

void CPostWindDlg::OnCalculateWindspeed()
{
	UpdateData(TRUE); // <-- start by saving the data in the dialog

	double delay; // <-- the calculated delay

				  // 1. Perform the correlation - calculations...
	if (SUCCESS != m_calc.CalculateDelay(delay, m_OriginalSeries[0], m_OriginalSeries[1], m_settings)) {
		switch (m_calc.m_lastError) {
		case CWindSpeedCalculator::ERROR_TOO_SHORT_DATASERIES:
			MessageBox("The time series is too short to be used with these settings. Please check the settings and try again"); break;
		case CWindSpeedCalculator::ERROR_LOWPASSFILTER:
			MessageBox("Failed to low-pass filter the time-series. Please check settings and try again"); break;
		case CWindSpeedCalculator::ERROR_DIFFERENT_SAMPLEINTERVALS:
			MessageBox("Failed to correlate the time-series. They have different sample-intervals"); break;
		default:
			MessageBox("Faile to correlate the time-series. Unknown error"); break;
		}
		return;
	}

	// 2. Calculate the average value of the correlation
	double	 avgCorr1 = Average(m_calc.corr, m_calc.m_length);

	// 3. Perform the correlation - calculations with the second series as upwind
	if (SUCCESS != m_calc.CalculateDelay(delay, m_OriginalSeries[1], m_OriginalSeries[0], m_settings)) {
		switch (m_calc.m_lastError) {
		case CWindSpeedCalculator::ERROR_TOO_SHORT_DATASERIES:
			MessageBox("The time series is too short to be used with these settings. Please check the settings and try again"); break;
		case CWindSpeedCalculator::ERROR_LOWPASSFILTER:
			MessageBox("Failed to low-pass filter the time-series. Please check settings and try again"); break;
		case CWindSpeedCalculator::ERROR_DIFFERENT_SAMPLEINTERVALS:
			MessageBox("Failed to correlate the time-series. They have different sample-intervals"); break;
		default:
			MessageBox("Faile to correlate the time-series. Unknown error"); break;
		}
		return;
	}

	// 4. Calculate the average correlation for the second case
	double	avgCorr2 = Average(m_calc.corr, m_calc.m_length);

	// 5. Use the results which gave the highest average correlation
	if (avgCorr1 > avgCorr2) {
		m_calc.CalculateDelay(delay, m_OriginalSeries[0], m_OriginalSeries[1], m_settings);
	}

	// 6. Display the results on the screen
	DrawResult();

	// 7. Save the results to an log file
	SaveResult();
}

void CPostWindDlg::OnChangePlumeHeight()
{
	// 1. save the data in the dialog
	UpdateData(TRUE);

	// 2. Update the result-graph, if necessary
	if (m_showOption == 2)
		DrawResult();

}

void CPostWindDlg::SaveResult() {
	// 1. Get the directory where the evaluation log-file is. The
	//		wind-speed log-file will be saved in the same directory
	CString directory;
	directory.Format(m_evalLog);
	Common::GetDirectory(directory);

	// 2. Make a new name for the wind-speed log-file
	CString fileName, timeNow;
	Common::GetDateTimeTextPlainFormat(timeNow);
	fileName.Format("%sWindCalculation_%s.txt", directory, timeNow);

	// 3. Open the log-file for writing
	FILE *f = fopen(fileName, "w");
	if (f == nullptr) {
		// Could not open the file
		MessageBox("Could not open log-file for writing, no data saved");
		return;
	}

	// 4. Write the header, containing the settings for the calculation
	fprintf(f, "PlumeHeight [m]\t%lf\n", m_settings.plumeHeight);
	fprintf(f, "AngleSeparation [deg]\t%lf\n", m_settings.angleSeparation);
	fprintf(f, "LowPassFilterAverage\t%d\n", m_settings.lowPassFilterAverage);
	fprintf(f, "MaxShift [s]\t%d\n", m_settings.shiftMax);
	fprintf(f, "TestLength [s]\t%d\n", m_settings.testLength);

	// 5. Write the result
	double distance = m_settings.plumeHeight * tan(DEGREETORAD * m_settings.angleSeparation);
	fprintf(f, "\nCorrelation\tDelay[s]\tWindSpeed[m/s]\n");
	for (int k = 0; k < m_calc.m_length; ++k) {
		if (m_calc.used[k]) {
			fprintf(f, "%lf\t", m_calc.corr[k]);
			fprintf(f, "%lf\t", m_calc.delays[k]);
			if (fabs(m_calc.delays[k]) < 1e-5) {
				fprintf(f, "%%0.0\n");
			}
		}
		else {
			fprintf(f, "%lf\n", distance / m_calc.delays[k]);
		}
	}

	// 6. Close the file
	fclose(f);
}