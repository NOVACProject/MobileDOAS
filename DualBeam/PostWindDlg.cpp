#include "stdafx.h"
#include "../DMSpec.h"
#include "PostWindDlg.h"
#include <MobileDoasLib/Flux/Flux1.h>
#include <stdexcept>

#undef max
#undef min

// CPostWindDlg dialog


IMPLEMENT_DYNAMIC(CPostWindDlg, CDialog)
CPostWindDlg::CPostWindDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CPostWindDlg::IDD, pParent)
{
    m_showOption = 0;

    for (int k = 0; k < MAX_N_SERIES; ++k) {
        this->m_OriginalSeries[k] = nullptr;
        this->m_PreparedSeries[k] = nullptr;
    }

    correlations.resize(MAX_CORRELATION_NUM);
    timeDelay.resize(MAX_CORRELATION_NUM);
    windspeeds.resize(MAX_CORRELATION_NUM);
}

CPostWindDlg::~CPostWindDlg()
{
    for (int k = 0; k < MAX_N_SERIES; ++k) {
        delete m_OriginalSeries[k];
        delete m_PreparedSeries[k];
    }
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
    DDX_Text(pDX, IDC_EDIT_WS_ANGLESEPARATION, m_settings.angleSeparation);

    // Deciding what to show
    DDX_Radio(pDX, IDC_RADIO_SHOW_CORR, m_showOption);
}


BEGIN_MESSAGE_MAP(CPostWindDlg, CDialog)
    // Actions to perform
    ON_BN_CLICKED(IDC_BTN_BROWSE_EVALLOG_SERIES1, OnBrowseEvallogSeries1)
    ON_BN_CLICKED(IDC_BTN_BROWSE_EVALLOG_SERIES2, OnBrowseEvallogSeries2)
    ON_BN_CLICKED(IDC_BUTTON_CALCULATE_CORRELATION, OnCalculateWindspeed)

    // Changing the settings
    ON_EN_KILLFOCUS(IDC_EDIT_LPITERATIONS, OnChangeLPIterations)
    ON_EN_KILLFOCUS(IDC_EDIT_PLUMEHEIGHT, OnChangePlumeHeight)

    // Changing what to show
    ON_BN_CLICKED(IDC_RADIO_SHOW_CORR, DrawResult)
    ON_BN_CLICKED(IDC_RADIO_SHOW_DELAY, DrawResult)
    ON_BN_CLICKED(IDC_RADIO_SHOW_WS, DrawResult)

END_MESSAGE_MAP()


// CPostWindDlg message handlers

void CPostWindDlg::OnBrowseEvallogSeries1()
{
    CString selectedLogFile;
    if (!Common::BrowseForEvaluationLog(selectedLogFile)) {
        return;
    }

    m_evalLog[0].Format("%s", (LPCTSTR)selectedLogFile);

    if (ReadEvaluationLog(0)) {
        // Update the text on the screen
        SetDlgItemText(IDC_EDIT_EVALLOG_SERIES1, m_evalLog[0]);
        // Redraw the screen
        DrawColumn();
    }
}

void CPostWindDlg::OnBrowseEvallogSeries2()
{
    CString selectedLogFile;
    if (!Common::BrowseForEvaluationLog(selectedLogFile)) {
        return;
    }

    m_evalLog[1].Format("%s", (LPCTSTR)selectedLogFile);

    if (ReadEvaluationLog(1)) {
        // Update the text on the screen
        SetDlgItemText(IDC_EDIT_EVALLOG_SERIES2, m_evalLog[1]);
        // Redraw the screen
        DrawColumn();
    }
}

bool CPostWindDlg::ReadEvaluationLog(int channelIndex) {
    if (channelIndex < 0 || channelIndex > 1) { throw std::invalid_argument("Invalid channel index passed to ReadEvaluationLog!"); }

    // the index in the traverse which we should read. Setting this to zero always assumes that we will always use
    //	the first species in the traverse to evaluate the windspeed...
    const int specieIndex = 0;
    mobiledoas::CFlux flux;		// The CFlux object helps with reading the data from the eval-log
    int nChannels = 1;		// the number of channels in the data-file
    double fileVersion = 0;	// the file-version of the evalution-log file

    // Read the header of the log file and see if it is an ok file
    int fileType = flux.ReadSettingFile((LPCSTR)m_evalLog[channelIndex], nChannels, fileVersion);
    if (fileType != 1) {
        MessageBox(TEXT("The file is not evaluation log file with right format.\nPlease choose a right file"), NULL, MB_OK);
        return FAIL;
    }

    if (nChannels != 1) {
        MessageBox(TEXT("The evaluation log does not contain evaluation data from one single channel. Please select another file in a correct format."), NULL, MB_OK);
        return FAIL;
    }

    // Read the data from the file
    if (0 == flux.ReadLogFile("", (LPCSTR)m_evalLog[channelIndex], nChannels, fileVersion)) {
        MessageBox(TEXT("That file is empty"));
        return FAIL;
    }

    // Copy the data to the local variables 'm_originalSeries'
    const mobiledoas::CTraverse* traverse = flux.m_traverse[specieIndex];
    const long traverseLength = traverse->m_recordNum;

    // create a new data series
    m_OriginalSeries[channelIndex] = new mobiledoas::CDualBeamCalculator::CMeasurementSeries(traverseLength);
    if (m_OriginalSeries[channelIndex] == nullptr) {
        return FAIL; // <-- failed to allocate enough memory
    }

    mobiledoas::Time startTime = traverse->time[0];
    for (int spectrumIndex = 0; spectrumIndex < traverseLength; ++spectrumIndex) {

        // Get the column of this spectrum
        m_OriginalSeries[channelIndex]->column[spectrumIndex] = traverse->columnArray[spectrumIndex];

        // Get the start time of this spectrum
        mobiledoas::Time t = traverse->time[spectrumIndex];

        // Save the time difference
        m_OriginalSeries[channelIndex]->time[spectrumIndex] =
            3600 * (t.hour - startTime.hour) +
            60 * (t.minute - startTime.minute) +
            (t.second - startTime.second);
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
    // m_colorSeries[2] = RGB(0, 255, 0);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}


void CPostWindDlg::DrawColumn() {
    double minT = 1e16, maxT = -1e16, minC = 1e16, maxC = -1e16;
    int nSeries = 0;

    // get the range for the plot
    for (int k = 0; k < MAX_N_SERIES; ++k) {
        if (m_OriginalSeries[k] != nullptr) {
            minT = std::min(minT, m_OriginalSeries[k]->time[0]);
            maxT = std::max(maxT, m_OriginalSeries[k]->time[m_OriginalSeries[k]->length - 1]);

            minC = std::min(minC, MinValue(m_OriginalSeries[k]->column, m_OriginalSeries[k]->length));
            maxC = std::max(maxC, MaxValue(m_OriginalSeries[k]->column, m_OriginalSeries[k]->length));

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

void CPostWindDlg::DrawResult() {

    if (m_calc.m_length == 0 || m_calc.corr == 0) {
        return;
    }

    UpdateData(TRUE); // <-- get the options for how to plot the data

    /** Copy the values to the local buffers */
    int length = 0;
    const double distance = m_settings.plumeHeight * tan(DEGREETORAD * m_settings.angleSeparation);
    for (int k = 0; k < m_calc.m_length; ++k) {
        if (length > MAX_CORRELATION_NUM) {
            break;
        }
        if (m_calc.used[k]) {
            correlations[length] = m_calc.corr[k];
            timeDelay[length] = m_calc.delays[k];
            if (fabs(m_calc.delays[k]) < 1e-5) {
                windspeeds[length] = 0.0;
            }
            else {
                windspeeds[length] = distance / m_calc.delays[k];
            }
            ++length;
        }
    }

    if (m_showOption == 0) {
        /** Draw the correlation */
        m_resultGraph.SetYUnits("Correlation [-]");
        m_resultGraph.XYPlot(nullptr, correlations.data(), length);
    }
    else if (m_showOption == 1) {
        /** Draw the timeDelay */
        m_resultGraph.SetYUnits("Temporal delay [s]");
        m_resultGraph.XYPlot(nullptr, timeDelay.data(), length);
    }
    else if (m_showOption == 2) {
        /** Draw the resulting wind speed */
        m_resultGraph.SetYUnits("Wind speed [m/s]");
        m_resultGraph.XYPlot(nullptr, windspeeds.data(), length);
    }
}

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
        m_PreparedSeries[seriesNo] = new mobiledoas::CWindSpeedCalculator::CMeasurementSeries();

    if (SUCCESS != mobiledoas::CWindSpeedCalculator::LowPassFilter(m_OriginalSeries[seriesNo], m_PreparedSeries[seriesNo], m_settings.lowPassFilterAverage))
        return 0;

    return 1;
}

void CPostWindDlg::OnChangeLPIterations()
{
    UpdateData(TRUE); // <-- save the data in the dialog
    if (m_settings.lowPassFilterAverage >= 0) {
        DrawColumn();
    }
}

void CPostWindDlg::OnCalculateWindspeed()
{
    UpdateData(TRUE); // <-- start by saving the data in the dialog

    double delay; // <-- the calculated delay

                  // 1. Perform the correlation - calculations...
    if (SUCCESS != m_calc.CalculateDelay(delay, m_OriginalSeries[0], m_OriginalSeries[1], m_settings)) {
        switch (m_calc.m_lastError) {
        case mobiledoas::CWindSpeedCalculator::ERROR_TOO_SHORT_DATASERIES:
            MessageBox("The time series is too short to be used with these settings. Please check the settings and try again"); break;
        case mobiledoas::CWindSpeedCalculator::ERROR_LOWPASSFILTER:
            MessageBox("Failed to low-pass filter the time-series. Please check settings and try again"); break;
        case mobiledoas::CWindSpeedCalculator::ERROR_DIFFERENT_SAMPLEINTERVALS:
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
        case mobiledoas::CWindSpeedCalculator::ERROR_TOO_SHORT_DATASERIES:
            MessageBox("The time series is too short to be used with these settings. Please check the settings and try again"); break;
        case mobiledoas::CWindSpeedCalculator::ERROR_LOWPASSFILTER:
            MessageBox("Failed to low-pass filter the time-series. Please check settings and try again"); break;
        case mobiledoas::CWindSpeedCalculator::ERROR_DIFFERENT_SAMPLEINTERVALS:
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
    if (m_showOption == 2) {
        DrawResult();
    }
}

void CPostWindDlg::SaveResult() {
    // 1. Get the directory where the evaluation log-file is. The
    //		wind-speed log-file will be saved in the same directory
    CString directory;
    directory.Format(m_evalLog[0]);
    Common::GetDirectory(directory);

    // 2. Make a new name for the wind-speed log-file
    CString fileName;
    std::string timeNow = mobiledoas::GetDateTimeTextPlainFormat();
    fileName.Format("%sWindCalculation_%s.txt", (LPCTSTR)directory, timeNow.c_str());

    // 3. Open the log-file for writing
    FILE* f = fopen(fileName, "w");
    if (f == nullptr) {
        // Could not open the file
        MessageBox("Could not open log-file for writing, no data saved");
        return;
    }

    // 4. Write the header, containing the settings for the calculation
    fprintf(f, "PlumeHeight [m]\t%lf\n", m_settings.plumeHeight);
    fprintf(f, "AngleSeparation [deg]\t%lf\n", m_settings.angleSeparation);
    fprintf(f, "LowPassFilterAverage\t%ud\n", m_settings.lowPassFilterAverage);
    fprintf(f, "MaxShift [s]\t%ud\n", m_settings.shiftMax);
    fprintf(f, "TestLength [s]\t%ud\n", m_settings.testLength);

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