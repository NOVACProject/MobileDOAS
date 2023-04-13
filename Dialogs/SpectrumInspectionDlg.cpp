// SpectrumInspectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../DMSpec.h"
#include "../Common/SpectrumIO.h"
#include <MobileDoasLib/Flux/Flux1.h>
#include "SpectrumInspectionDlg.h"


using namespace Dialogs;

// CSpectrumInspectionDlg dialog

IMPLEMENT_DYNAMIC(CSpectrumInspectionDlg, CDialog)
CSpectrumInspectionDlg::CSpectrumInspectionDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CSpectrumInspectionDlg::IDD, pParent)
{
    m_selectedSpectrum = -2;
    m_evaluationLog.Format("");
    m_spectrumPath.Format("");

    m_number = new double[4096];
    for (int k = 0; k < 4096; k += 4) {
        m_number[k] = k;
        m_number[k + 1] = k + 1;
        m_number[k + 2] = k + 2;
        m_number[k + 3] = k + 3;
    }

    m_spectrumColor = RGB(0, 255, 0);

    m_totalSpecNum = 0;

    m_timer = 0;
}

CSpectrumInspectionDlg::~CSpectrumInspectionDlg()
{
    if (m_number != nullptr) {
        delete m_number;
        m_number = nullptr;
    }
}

void CSpectrumInspectionDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SPECTRUM_GRAPH_FRAME, m_graphFrame);
    DDX_Control(pDX, IDC_SPECTRUMPROPERITES_FRAME, m_propertiesFrame);

    DDX_Control(pDX, IDC_SPECTRUM_SCROLLBAR, m_spectrumScrollBar);

    DDX_Text(pDX, IDC_LABEL_EVALUATIONLOG, m_evaluationLog);
}


BEGIN_MESSAGE_MAP(CSpectrumInspectionDlg, CDialog)
    ON_BN_CLICKED(IDC_BROWSE_EVALLOG, OnBrowseEvallog)
    ON_MESSAGE(WM_ZOOM, OnZoomGraph)

    ON_WM_HSCROLL()
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CSpectrumInspectionDlg message handlers

BOOL CSpectrumInspectionDlg::OnInitDialog() {
    CDialog::OnInitDialog();
    CRect rect;

    int margin = 2;
    m_graphFrame.GetWindowRect(&rect);
    rect.bottom -= rect.top + margin;
    rect.right -= rect.left + margin;
    rect.top = margin + 7;
    rect.left = margin;
    m_spectrumGraph.Create(WS_VISIBLE | WS_CHILD, rect, &m_graphFrame);
    m_spectrumGraph.SetRange(0, 500, 1, -100.0, 100.0, 1);
    m_spectrumGraph.SetYUnits("Intensity [Counts]");
    m_spectrumGraph.SetXUnits("Pixel number");
    m_spectrumGraph.SetBackgroundColor(RGB(0, 0, 0));
    m_spectrumGraph.SetGridColor(RGB(255, 255, 255));
    m_spectrumGraph.SetPlotColor(m_spectrumColor);
    m_spectrumGraph.CleanPlot();
    m_spectrumGraph.parentWnd = this;
    m_spectrumGraph.ResetZoomRect();

    /** Show the first spectrum */
    m_selectedSpectrum = -2;

    // set the properties for the scroll-bar
    SCROLLINFO ScrollInfo;
    ScrollInfo.cbSize = sizeof(ScrollInfo);     // size of this structure
    ScrollInfo.fMask = SIF_ALL;                 // parameters to set
    ScrollInfo.nMin = 0;                        // minimum scrolling position
    ScrollInfo.nPage = 20;                      // the page size of the scroll box
    ScrollInfo.nMax = m_totalSpecNum + 1 + ScrollInfo.nPage;		// maximum scrolling position
    ScrollInfo.nPos = 0;                        // initial position of the scroll box
    ScrollInfo.nTrackPos = 0;                   // immediate position of a scroll box that the user is dragging
    m_spectrumScrollBar.SetScrollInfo(&ScrollInfo);

    // initialize 
    InitPropertiesList();

    // Draw the spectrum (if we have any)
    DrawSpectrum();

    return TRUE;
}

/** Called when the user browses an evaluation log */
void CSpectrumInspectionDlg::OnBrowseEvallog()
{
    CString fileName;
    Common common;
    TCHAR filter[512];
    int n = _stprintf(filter, "Evaluation Logs\0");
    n += _stprintf(filter + n + 1, "*.txt\0");
    filter[n + 2] = 0;

    // 0. Turn off the zooming temporarily, this since if the user double-clicks
    //		in the dialog we want to show then this can be percieved as a zoom-click by the graph...
    m_spectrumGraph.m_userZoomableGraph = false;

    if (!common.BrowseForFile(filter, fileName)) {
        m_spectrumGraph.m_userZoomableGraph = true;
        return;
    }

    // read the evaluation-log
    if (0 == ReadEvaluationLog(fileName)) {

        /** Save the directory where the evaluation log (and also the spectra) are stored */
        m_spectrumPath.Format(fileName);
        Common::GetDirectory(m_spectrumPath);

        m_evaluationLog.Format(fileName);

        // update the interface
        UpdateData(FALSE);

        // set the properties for the scroll-bar
        SCROLLINFO ScrollInfo;
        ScrollInfo.cbSize = sizeof(ScrollInfo);     // size of this structure
        ScrollInfo.fMask = SIF_ALL;                 // parameters to set
        ScrollInfo.nMin = 0;                        // minimum scrolling position
        ScrollInfo.nPage = 20;                      // the page size of the scroll box
        ScrollInfo.nMax = m_totalSpecNum + 1 + ScrollInfo.nPage;		// maximum scrolling position
        ScrollInfo.nPos = 0;                        // initial position of the scroll box
        ScrollInfo.nTrackPos = 0;                   // immediate position of a scroll box that the user is dragging
        m_spectrumScrollBar.SetScrollInfo(&ScrollInfo);

        /** Show the first spectrum */
        m_selectedSpectrum = -2;

        DrawSpectrum();
    }
    else {
        MessageBox("Cannot parse that evaluation log");
    }

    // Turn on the zoom again in 0.5 second
    m_timer = this->SetTimer(0, 500, NULL);
}

void CSpectrumInspectionDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // Get the minimum and maximum scroll-bar positions.
    int minpos;
    int maxpos;
    pScrollBar->GetScrollRange(&minpos, &maxpos);

    // Get the current position of scroll box.
    int curpos = pScrollBar->GetScrollPos();

    // Determine the new position of scroll box.
    switch (nSBCode)
    {
    case SB_LEFT:      // Scroll to far left.
        curpos = minpos;
        break;

    case SB_RIGHT:      // Scroll to far right.
        curpos = maxpos;
        break;

    case SB_ENDSCROLL:   // End scroll.
        break;

    case SB_LINELEFT:      // Scroll left.
        if (curpos > minpos)
            curpos--;
        break;

    case SB_LINERIGHT:   // Scroll right.
        if (curpos < maxpos)
            curpos++;
        break;

    case SB_PAGELEFT:    // Scroll one page left.
    {
        // Get the page size. 
        SCROLLINFO   info;
        pScrollBar->GetScrollInfo(&info, SIF_ALL);

        if (curpos > minpos)
            curpos = max(minpos, curpos - (int)info.nPage);
    }
    break;

    case SB_PAGERIGHT:      // Scroll one page right.
    {
        // Get the page size. 
        SCROLLINFO   info;
        pScrollBar->GetScrollInfo(&info, SIF_ALL);

        if (curpos < maxpos)
            curpos = min(maxpos, curpos + (int)info.nPage);
    }
    break;

    case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
        curpos = nPos;      // of the scroll box at the end of the drag operation.
        break;

    case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
        curpos = nPos;     // position that the scroll box has been dragged to.
        break;
    }

    // Set the new position of the thumb (scroll box).
    pScrollBar->SetScrollPos(curpos);

    // redraw the spectrum
    this->m_selectedSpectrum = curpos - 2;
    DrawSpectrum();

    CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


/** Zooming in the graph */
LRESULT CSpectrumInspectionDlg::OnZoomGraph(WPARAM wParam, LPARAM lParam) {
    this->DrawSpectrum();

    return 0;
}

/** Draws the currently selected spectrum */
void CSpectrumInspectionDlg::DrawSpectrum() {
    CString fileName, fullFileName, userMessage;
    CSpectrum spectrum;
    Graph::CSpectrumGraph::plotRange range;

    if (m_selectedSpectrum >= m_totalSpecNum || m_spectrumPath.GetLength() <= 3)
        return; // this spectrum does not exist don't even try to draw it...

    // begin by clearing the graph
    m_spectrumGraph.CleanPlot();

    // the name of the spectrum to open
    fullFileName.Format(m_spectrumPath);

    if (m_selectedSpectrum == -3) {
        fileName.Format("offset_0.STD");
    }
    else if (m_selectedSpectrum == -2) {
        fileName.Format("dark_0.STD");
    }
    else if (m_selectedSpectrum == -1) {
        fileName.Format("sky_0.STD");
    }
    else {
        fileName.Format("%05d_0.STD", m_selectedSpectrum);
    }
    fullFileName.AppendFormat(fileName);
    userMessage.Format("Showing spectrum: %s (out of %d spectra)", (LPCTSTR)fileName, m_totalSpecNum);

    // Tell the user which spectrum he's seeing 
    SetDlgItemText(IDC_LABEL_SPECTRUMNUMBER, userMessage);

    // if this file does not exist, then return
    if (!IsExistingFile(fullFileName) || 0 != CSpectrumIO::readSTDFile(fullFileName, &spectrum)) {
        GetPlotRange(range);

        double xmin = (range.maxLambda - range.minLambda) * 0.333;
        double xmax = (range.maxLambda - range.minLambda) * 0.667;
        double ymin = (range.maxIntens - range.minIntens) * 0.333;
        double ymax = (range.maxIntens - range.minIntens) * 0.667;

        userMessage.Format("Cannot read spectrum %s", (LPCTSTR)fileName);

        m_spectrumGraph.DrawTextBox(xmin, xmax, ymin, ymax, userMessage);
    }
    else {
        GetPlotRange(range, &spectrum);

        m_spectrumGraph.SetPlotColor(m_spectrumColor);
        m_spectrumGraph.SetRange(range.minLambda, range.maxLambda, 0, range.minIntens, range.maxIntens, 0);
        m_spectrumGraph.XYPlot(m_number, spectrum.I, spectrum.length, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);

        FillInSpectrumPropertiesList(&spectrum);
    }
}

/** Gets the range of the plot */
void CSpectrumInspectionDlg::GetPlotRange(Graph::CSpectrumGraph::plotRange& range, const CSpectrum* spectrum) {
    Graph::CSpectrumGraph::plotRange rect;

    // See if the user has determined any range...
    m_spectrumGraph.GetZoomRect(rect);
    if (fabs(rect.maxLambda) > 0.1) {
        range = rect;
        return;
    }
    else if (spectrum != nullptr) {
        long maxV = (long)spectrum->GetMax();
        range.minIntens = 0.0;
        range.maxIntens = maxV;
        range.minLambda = 0.0;
        range.maxLambda = spectrum->length;

        return;
    }
    else {
        range.minIntens = 0.0;
        range.maxIntens = 4095;
        range.minLambda = 0.0;
        range.maxLambda = 2048;

        return;
    }
}

void CSpectrumInspectionDlg::InitPropertiesList() {
    // Put out the list of properties...
    CRect rect;
    this->m_propertiesFrame.GetWindowRect(rect);
    int height = rect.bottom - rect.top;
    int width = rect.right - rect.left;
    rect.top = 20; rect.bottom = height - 10;
    rect.left = 10; rect.right = width - 10;
    int columnWidth0 = (int)(0.6 * (rect.right - rect.left));
    int columnWidth1 = (int)(0.4 * (rect.right - rect.left));

    m_propertyList.Create(WS_VISIBLE | WS_BORDER | LVS_REPORT, rect, &m_propertiesFrame, 65536);
    m_propertyList.InsertColumn(0, "", LVCFMT_LEFT, columnWidth0);
    m_propertyList.InsertColumn(1, "", LVCFMT_LEFT, columnWidth1);

    // Fill the list with labels...
    int index = 0;
    m_propertyList.InsertItem(index++, "Spectrometer");
    m_propertyList.InsertItem(index++, "Spectrum length");
    m_propertyList.InsertItem(index++, "Maximum intensity");
    m_propertyList.InsertItem(index++, "Offset");

    m_propertyList.InsertItem(index++, "Expsure time [ms]");
    m_propertyList.InsertItem(index++, "# of exposures");

    m_propertyList.InsertItem(index++, "Started at:");
    m_propertyList.InsertItem(index++, "Stopped at:");
    m_propertyList.InsertItem(index++, "Latitude:");
    m_propertyList.InsertItem(index++, "Longitude:");
    m_propertyList.InsertItem(index++, "Altitude:");
    m_propertyList.InsertItem(index++, "GPS Status:");
    m_propertyList.InsertItem(index++, "Speed [m/s]:");
    m_propertyList.InsertItem(index++, "Course [deg]:");

}

/** Fills in the property list with information about the current spectrum
    if spectrum is NULL then the list is just cleared */
void CSpectrumInspectionDlg::FillInSpectrumPropertiesList(const CSpectrum* spec) {
    CString str;
    int index = 0;

    if (spec != nullptr) {
        // ---- Show the information to the user... ----
        str.Format("%s", (LPCTSTR)spec->spectrometerSerial);
        m_propertyList.SetItemText(index++, 1, str);

        str.Format("%d", spec->length);
        m_propertyList.SetItemText(index++, 1, str);

        str.Format("%.2lf", spec->GetMax());
        m_propertyList.SetItemText(index++, 1, str);

        str.Format("%.2lf", spec->GetAverage(2, 20));
        m_propertyList.SetItemText(index++, 1, str);

        str.Format("%d", spec->exposureTime);
        m_propertyList.SetItemText(index++, 1, str);

        str.Format("%d", spec->scans);
        m_propertyList.SetItemText(index++, 1, str);

        str.Format("%02d:%02d:%02d", spec->startTime[0], spec->startTime[1], spec->startTime[2]);
        m_propertyList.SetItemText(index++, 1, str);

        str.Format("%02d:%02d:%02d", spec->stopTime[0], spec->stopTime[1], spec->stopTime[2]);
        m_propertyList.SetItemText(index++, 1, str);

        str.Format("%.05lf", spec->lat);
        m_propertyList.SetItemText(index++, 1, str);

        str.Format("%.05lf", spec->lon);
        m_propertyList.SetItemText(index++, 1, str);

        str.Format("%.1lf", spec->altitude);
        m_propertyList.SetItemText(index++, 1, str);

        str.Format("Not Available");
        if (spec->gpsStatus == "A") {
            str.Format("%s", "Active");
        }
        if (spec->gpsStatus == "V") {
            str.Format("%s", "Void");
        }
        m_propertyList.SetItemText(index++, 1, str);

        str.Format("%.1lf", spec->speed);
        m_propertyList.SetItemText(index++, 1, str);

        str.Format("%.1lf", spec->course);
        m_propertyList.SetItemText(index++, 1, str);

    }
    else {
        str.Format("");
        m_propertyList.SetItemText(index++, 1, str);
        m_propertyList.SetItemText(index++, 1, str);
        m_propertyList.SetItemText(index++, 1, str);
        m_propertyList.SetItemText(index++, 1, str);
        m_propertyList.SetItemText(index++, 1, str);
        m_propertyList.SetItemText(index++, 1, str);
        m_propertyList.SetItemText(index++, 1, str);
        m_propertyList.SetItemText(index++, 1, str);
        m_propertyList.SetItemText(index++, 1, str);
        m_propertyList.SetItemText(index++, 1, str);
        m_propertyList.SetItemText(index++, 1, str);
        m_propertyList.SetItemText(index++, 1, str);
        m_propertyList.SetItemText(index++, 1, str);
        m_propertyList.SetItemText(index++, 1, str);
    }
}

/** Reads the given evaluation log file.
    @return 0 on success. */
int CSpectrumInspectionDlg::ReadEvaluationLog(const CString& fullFileName) {
    mobiledoas::CFlux flux;
    int nChannels;	// the number of channels that was used in this measurement
    double fileVersion; // the version number of the evaluation-log file
    CString fileName, filePath;

    // Get the path and name of the file
    fileName.Format(fullFileName);	Common::GetFileName(fileName);
    filePath.Format(fullFileName);	Common::GetDirectory(filePath);

    // Read the header of the log file and see if it is an ok file
    int fileType = flux.ReadSettingFile((LPCSTR)fileName, nChannels, fileVersion);
    if (fileType != 1) {
        return 1;
    }

    // allocate space for the new channels
    for (int k = 1; k <= nChannels; ++k) {
        mobiledoas::CTraverse* tr = new mobiledoas::CTraverse();
        flux.m_traverse.push_back(tr);
    }

    // Read the data from the file
    if (0 == flux.ReadLogFile((LPCSTR)filePath, (LPCSTR)fileName, nChannels, fileVersion)) {
        MessageBox(TEXT("That file is empty"));
        return 1;
    }

    m_totalSpecNum = flux.m_traverse[0]->m_recordNum;

    // Delete the data we don't need
    for (int k = 0; k <= nChannels; ++k) {
        mobiledoas::CTraverse* tr = flux.m_traverse[k];
        delete tr;
    }
    flux.m_traverse.clear();

    return 0;
}

void CSpectrumInspectionDlg::OnTimer(UINT_PTR nIDEvent)
{
    m_spectrumGraph.m_userZoomableGraph = true;

    m_timer = KillTimer(m_timer);

    CDialog::OnTimer(nIDEvent);
}
