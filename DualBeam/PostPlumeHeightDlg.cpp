#include "StdAfx.h"
#include "../DMSpec.h"
#include "PostPlumeHeightDlg.h"
#include "../SourceSelectionDlg.h"
#include <MobileDoasLib/Flux/Flux1.h>

#undef min
#undef max

#include <algorithm>
#include <stdexcept>

// CPostPlumeHeightDlg dialog

using namespace mobiledoas;

IMPLEMENT_DYNAMIC(CPostPlumeHeightDlg, CDialog)
CPostPlumeHeightDlg::CPostPlumeHeightDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CPostPlumeHeightDlg::IDD, pParent)
{
    for (int k = 0; k < MAX_N_SERIES; ++k) {
        this->m_OriginalSeries[k] = nullptr;
        this->m_PreparedSeries[k] = nullptr;
    }
    m_automatic = false;
    m_sourceLat = 0.0;
    m_sourceLon = 0.0;
}

CPostPlumeHeightDlg::~CPostPlumeHeightDlg()
{
    for (int k = 0; k < MAX_N_SERIES; ++k) {
        delete m_OriginalSeries[k];
        delete m_PreparedSeries[k];
    }
}

void CPostPlumeHeightDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_FRAME_TIME_SERIES, m_frameColumn);

    // The settings
    DDX_Text(pDX, IDC_EDIT_ANGLESEPARATION, m_settings.angleSeparation);
    DDX_Text(pDX, IDC_EDIT_LATITUDE, m_sourceLat);
    DDX_Text(pDX, IDC_EDIT_LONGITUDE, m_sourceLon);
}


BEGIN_MESSAGE_MAP(CPostPlumeHeightDlg, CDialog)
    // Actions to perform
    ON_BN_CLICKED(IDC_BTN_BROWSE_EVALLOG_SERIES1, OnBrowseEvallogSeries1)
    ON_BN_CLICKED(IDC_BTN_BROWSE_EVALLOG_SERIES2, OnBrowseEvallogSeries2)
    ON_BN_CLICKED(IDC_BUTTON_CALCULATE_CORRELATION, OnCalculatePlumeHeight)

    // Changing the settings
    ON_BN_CLICKED(IDC_BUTTON_SOURCE_LAT, OnBnClickedButtonSourceLat)
    ON_BN_CLICKED(IDC_BUTTON_SOURCE_LON, OnBnClickedButtonSourceLon)
END_MESSAGE_MAP()


// CPostPlumeHeightDlg message handlers
void CPostPlumeHeightDlg::OnBrowseEvallogSeries1()
{
    CString selectedLogFile;
    if (!Common::BrowseForEvaluationLog(selectedLogFile)) {
        return;
    }

    m_evalLog[0].Format("%s", (LPCTSTR)selectedLogFile);

    // Read the newly opened evaluation-log
    if (ReadEvaluationLog(0)) {
        m_automatic = true;

        // Update the text on the screen
        SetDlgItemText(IDC_EDIT_EVALLOG_SERIES1, m_evalLog[0]);

        // Redraw the screen
        DrawColumn();

        m_automatic = false;
    }
}


void CPostPlumeHeightDlg::OnBrowseEvallogSeries2()
{
    CString selectedLogFile;
    if (!Common::BrowseForEvaluationLog(selectedLogFile)) {
        return;
    }

    m_evalLog[1].Format("%s", (LPCTSTR)selectedLogFile);

    // Read the newly opened evaluation-log
    if (ReadEvaluationLog(1)) {
        m_automatic = true;

        // Update the text on the screen
        SetDlgItemText(IDC_EDIT_EVALLOG_SERIES2, m_evalLog[1]);

        // Redraw the screen
        DrawColumn();

        m_automatic = false;
    }
}

bool CPostPlumeHeightDlg::ReadEvaluationLog(int channelIndex) {
    if (channelIndex < 0 || channelIndex > 1) { throw std::invalid_argument("Invalid channel index passed to ReadEvaluationLog!"); }

    // the index in the traverse which we should read. Setting this to zero always assumes that we will always use
    //	the first species in the traverse to evaluate the windspeed...
    const int specieIndex = 0;
    mobiledoas::CFlux flux;		// The CFlux object helps with reading the data from the eval-log
    int nChannels = 1;		// the number of channels in the data-file
    double fileVersion = 0;	// the file-version of the evalution-log file

    // Read the header of the log file and see if it is an ok file
    std::string filenameStr = (LPCSTR)m_evalLog[channelIndex];
    int fileType = flux.ReadSettingFile(filenameStr, nChannels, fileVersion);
    if (fileType != 1) {
        MessageBox(TEXT("The file is not evaluation log file with right format.\nPlease choose a right file"), nullptr, MB_OK);
        return FAIL;
    }

    if (nChannels != 1) {
        MessageBox(TEXT("The evaluation log does not contain evaluation data from one single channel. Please select another file in a correct format."), nullptr, MB_OK);
        return FAIL;
    }

    // Read the data from the file
    std::string filePath = "";
    if (0 == flux.ReadLogFile(filePath, filenameStr, nChannels, fileVersion)) {
        MessageBox(TEXT("That file is empty"));
        return FAIL;
    }

    // Copy the data to the local variables 'm_originalSeries'
    const mobiledoas::CTraverse* traverse = flux.m_traverse[specieIndex];
    const long traverseLength = traverse->m_recordNum;

    // create a new data series
    m_OriginalSeries[channelIndex] = new CDualBeamCalculator::CMeasurementSeries(traverseLength);
    if (m_OriginalSeries[channelIndex] == nullptr)
        return FAIL; // <-- failed to allocate enough memory

    mobiledoas::Time startTime = traverse->time[0];
    for (int specIndex = 0; specIndex < traverseLength; ++specIndex) {

        // Get the column of this spectrum
        m_OriginalSeries[channelIndex]->column[specIndex] = traverse->columnArray[specIndex];

        // Get the latitude 
        m_OriginalSeries[channelIndex]->lat[specIndex] = traverse->latitude[specIndex];

        // Get the longitude
        m_OriginalSeries[channelIndex]->lon[specIndex] = traverse->longitude[specIndex];

        // Get the start time of this spectrum
        mobiledoas::Time t = traverse->time[specIndex];

        // Save the time difference
        m_OriginalSeries[channelIndex]->time[specIndex] =
            3600 * (t.hour - startTime.hour) +
            60 * (t.minute - startTime.minute) +
            (t.second - startTime.second);
    }

    // Correct the newly read in time-series for some common problems
    CorrectTimeSeries(channelIndex);

    return SUCCESS;
}

BOOL CPostPlumeHeightDlg::OnInitDialog()
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

    // Setup the colors to use for the different time-series
    m_colorSeries[0] = RGB(255, 0, 0);
    m_colorSeries[1] = RGB(0, 0, 255);
    // m_colorSeries[2] = RGB(0, 255, 0);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}


void CPostPlumeHeightDlg::DrawColumn() {
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

int	CPostPlumeHeightDlg::LowPassFilter(int seriesNo) {
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
        m_PreparedSeries[seriesNo] = new CDualBeamCalculator::CMeasurementSeries();

    if (SUCCESS != CPlumeHeightCalculator::LowPassFilter(m_OriginalSeries[seriesNo], m_PreparedSeries[seriesNo], m_settings.lowPassFilterAverage))
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


void CPostPlumeHeightDlg::CorrectTimeSeries(int seriesIndex) {
    for (int spec = 1; spec < m_OriginalSeries[seriesIndex]->length - 1; ++spec) {
        double* lat = m_OriginalSeries[seriesIndex]->lat;
        double* lon = m_OriginalSeries[seriesIndex]->lon;
        double* time = m_OriginalSeries[seriesIndex]->time;

        // If this latitude is same as for the last spectrum, then interpolate
        if (lat[spec] == lat[spec - 1]) {
            lat[spec] = 0.5 * (lat[spec - 1] + lat[spec + 1]);
        }
        // If this longitude is same as for the last spectrum, then interpolate
        if (lon[spec] == lon[spec - 1]) {
            lon[spec] = 0.5 * (lon[spec - 1] + lon[spec + 1]);
        }
        // If this starttime is unreasonable, then interpolate
        if (time[spec] < time[spec - 1] || time[spec] < 0) {
            time[spec] = 0.5 * (time[spec - 1] + time[spec + 1]);
        }
    }
}

void CPostPlumeHeightDlg::OnBnClickedButtonSourceLat()
{
    Dialogs::CSourceSelectionDlg sourceDlg;
    INT_PTR modal = sourceDlg.DoModal();
    if (IDOK == modal) {
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
