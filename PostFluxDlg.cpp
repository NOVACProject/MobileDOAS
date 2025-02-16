// CPostFluxDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DMSpec.h"
#include "PostFluxDlg.h"
#include "Common.h"
#include "RouteDlg.h"
#include "SourceSelectionDlg.h"
#include "WindFieldDlg.h"
#include "ExportEvLogDlg.h"
#include <MobileDoasLib/Flux/Traverse.h>
#include <MobileDoasLib/File/KMLFileHandler.h>
#include "Dialogs/SelectionDialog.h"
#include <MobileDoasLib/GpsData.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ReEvaluation;
using namespace Graph;

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp


#define DLGLEFT 93
#define DLGTOP 14
#define DLGRIGHT 673
#define DLGBOTTOM 556

#define ORANGE RGB(255,128,0)
#define YELLOW RGB(255,255,0)
#define GREEN RGB(0,255,0)
/////////////////////////////////////////////////////////////////////////////
// CPostFluxDlg dialog


CPostFluxDlg::CPostFluxDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CPostFluxDlg::IDD, pParent)
{
    m_srcLat = 0.0;
    m_srcLon = 0.0;
    m_WindDirect = 0.0;
    m_WindSpeed = 10.0;
    m_lowIndex = 0;
    m_highIndex = 499;
    m_flux = new mobiledoas::CFlux();
    fMovePlot = FALSE;

    m_windDirectionAverage = 0;
    m_windDirectionMax = 0;

    m_GraphToolTip = nullptr;

    m_recordNumber = 0;

    m_XAxisUnit = 1;

    /* by default - show the intensity in the column graph */
    m_showIntensity = true;

    /* by default - don't show the column errors */
    m_showColumnError = false;

    m_additionalLogCheck = false;
}


CPostFluxDlg::~CPostFluxDlg()
{
    if (m_flux != nullptr)
        delete(m_flux);
}


void CPostFluxDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPostFluxDlg)

    // Selecting the offset
    DDX_Control(pDX, IDC_SLIDEROFFSET, m_sliderOffset);
    DDX_Text(pDX, IDC_FLUXDOFFSET, m_flux->m_traverse[m_flux->m_curTraverse]->m_Offset);

    // Selecting the range
    DDX_Control(pDX, IDC_SLIDERTO, m_sliderTo);
    DDX_Control(pDX, IDC_SLIDERFROM, m_sliderFrom);
    DDX_Text(pDX, IDC_FLUXDFROM, m_lowIndex);
    DDV_MinMaxLong(pDX, m_lowIndex, 0, 65535);
    DDX_Text(pDX, IDC_FLUXDTO, m_highIndex);
    DDV_MinMaxLong(pDX, m_highIndex, 0, 65536);

    // The source
    DDX_Text(pDX, IDC_FLUXDLAT, m_srcLat);
    DDV_MinMaxDouble(pDX, m_srcLat, -90., 90.);
    DDX_Text(pDX, IDC_FLUXDLON, m_srcLon);
    DDV_MinMaxDouble(pDX, m_srcLon, -180., 180.);

    // The wind speed and wind direction
    DDX_Text(pDX, IDC_FLUXDWD, m_WindDirect);
    DDX_Text(pDX, IDC_FLUXDWS, m_WindSpeed);
    DDV_MinMaxDouble(pDX, m_WindSpeed, 0., 1000000.);

    // Selecting points based on intensity
    DDX_Control(pDX, IDC_SLIDERINTENSITY_LOW, m_intensitySliderLow);
    DDX_Control(pDX, IDC_SLIDERINTENSITY_HIGH, m_intensitySliderHigh);
    DDX_Control(pDX, IDC_PATH_LIST, m_pathList);
    DDX_Control(pDX, IDC_UNIT_COMBO, m_unitSelection);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPostFluxDlg, CDialog)
    //{{AFX_MSG_MAP(CPostFluxDlg)
    // Opening a evaluation-log
    ON_BN_CLICKED(IDC_BTNCHOOSEFILE, OnBtnOpenLogFile)
    ON_COMMAND(ID_FILE_OPENLOGFILE, OnBtnOpenLogFile)
    ON_COMMAND(ID_RELOAD_LOGFILE, OnReloadLogfile)

    ON_BN_CLICKED(IDC_CALC, OnCalculateFlux)
    ON_BN_CLICKED(IDC_BTNFOR, OnBtnfor)
    ON_BN_CLICKED(IDC_BTNBACK, OnBtnback)

    ON_COMMAND(ID_FILE_CREATEADDITIONALLOGFILE, OnChangeCreateAdditionalLogFile)

    // Showing the route-graph
    ON_BN_CLICKED(IDC_BTNROUTE, OnBtnShowRoute)

    // Deleting points
    ON_BN_CLICKED(IDC_BTN_DEL_INTENSITY, OnBnClickedBtnDelIntensity)
    ON_BN_CLICKED(IDC_BTNDEL, OnBtnDeleteSelected)

    // Showing the sources-list
    ON_BN_CLICKED(IDC_BTN_SOURCE_LAT, OnBnClickedBtnSourceLat)
    ON_BN_CLICKED(IDC_BTN_SOURCE_LONG, OnBnClickedBtnSourceLong)

    // Sliders
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDERFROM, OnReleasedcaptureSliderfrom)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDERTO, OnReleasedcaptureSliderto)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDEROFFSET, OnReleasedcaptureSlideroffset)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDERINTENSITY_LOW, OnNMReleasedcaptureSliderintensityLow)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDERINTENSITY_HIGH, OnNMReleasedcaptureSliderintensityHigh)

    // Changing the source latitude/longitude
    ON_EN_KILLFOCUS(IDC_FLUXDLON, OnChangeSource)
    ON_EN_KILLFOCUS(IDC_FLUXDLAT, OnChangeSource)

    // Tool-tips
    ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)

    // Selecting one of the calculated wind-directions
    ON_STN_CLICKED(IDC_MAXWD, OnStnClickedMaxwd)
    ON_STN_CLICKED(IDC_AVWD, OnStnClickedAvwd)

    // Changing the graph to show
    ON_LBN_SELCHANGE(IDC_PATH_LIST, OnLbnSelchangePathList)

    // Changing what to show
    ON_COMMAND(ID_VIEW_SHOWINTENSITY, OnMenuViewShowIntensity)
    ON_COMMAND(ID_VIEW_SHOWCOLUMNERROR, OnMenuViewShowColumnError)
    ON_COMMAND(ID_VIEW_SHOWVSTIME, OnViewShowVsTime)
    ON_COMMAND(ID_VIEW_SHOWVSDISTANCETRAVELLED, OnViewShowVsDistance)
    ON_COMMAND(ID_VIEW_SHOWVSMEASUREMENTNUMBER, OnViewShowVsNumber)
    ON_COMMAND(ID_COLUMNGRAPH_INCREASELINEWIDTH, OnColumnGraphIncreaseLineWidth)
    ON_COMMAND(ID_COLUMNGRAPH_DECREASELINEWIDTH, OnColumnGraphDecreaseLineWidth)

    // Re-Evaluating
    ON_COMMAND(155, OnReEvaluateThisTraverse)
    ON_COMMAND(ID_FILE_CLOSEALLLOGFILES, OnFileCloseAllLogFiles)
    ON_BN_CLICKED(IDC_BTN_IMPORT_WINDFIELD, OnImportWindField)
    ON_WM_CLOSE()

    // Updating the interface
    ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWINTENSITY, OnUpdateViewShowintensity)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWCOLUMNERROR, OnUpdateViewShowColumnError)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWVSTIME, OnUpdateViewShowVsTime)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWVSDISTANCETRAVELLED, OnUpdateViewShowVsDistance)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWVSMEASUREMENTNUMBER, OnUpdateViewShowVsNumber)
    ON_UPDATE_COMMAND_UI(ID_RELOAD_LOGFILE, OnUpdateItemReloadLogFile)
    ON_UPDATE_COMMAND_UI(IDC_CALC, OnUpdateItemCalculateFlux)
    ON_UPDATE_COMMAND_UI(IDC_BTNROUTE, OnUpdateItemShowRouteDlg)
    ON_UPDATE_COMMAND_UI(155, OnUpdateItemReEvaluateTraverse)
    ON_UPDATE_COMMAND_UI(ID_FILE_CLOSEALLLOGFILES, OnUpdateItemCloseAllLogFiles)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVECOLUMNGRAPH, OnUpdateItemSaveColumnGraph)
    ON_UPDATE_COMMAND_UI(ID_FILE_EXPORTTHISLOGFILE, OnUpdateItemExportLogFile)
    ON_UPDATE_COMMAND_UI(IDC_BTN_DEL_LOW_INTENSITY, OnUpdateItemDeleteLowIntensity)
    ON_UPDATE_COMMAND_UI(IDC_BTNDEL, OnUpdateItemDeleteSelected)
    ON_UPDATE_COMMAND_UI(IDC_BTN_IMPORT_WINDFIELD, OnUpdateItemImportWindField)
    ON_UPDATE_COMMAND_UI(ID_FILE_CREATEADDITIONALLOGFILE, OnUpdateMenuItemCreateAdditionalLogFile)
    ON_UPDATE_COMMAND_UI(ID_FILE_EXPORT_TRAVERSE_TO_KML, OnUpdateItemExportLogFile)

    // Saving the graph
    ON_COMMAND(ID_FILE_SAVECOLUMNGRAPH, OnFileSaveColumnGraph)

    ON_COMMAND(ID_FILE_EXPORTTHISLOGFILE, OnFileExportLogfile)
    ON_COMMAND(ID_FILE_EXPORT_TRAVERSE_TO_KML, OnFileExportToKML)

    // Updating the interface, notice that this has to be here due to a bug in Microsoft MFC
    //	http://support.microsoft.com/kb/242577
    ON_WM_INITMENUPOPUP()

    ON_CBN_SELCHANGE(IDC_UNIT_COMBO, &CPostFluxDlg::ChangeFluxUnit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPostFluxDlg message handlers

void CPostFluxDlg::OnBtnOpenLogFile()
{
    static char szFile[16384];
    szFile[0] = 0;

    OPENFILENAME ofn;       // common dialog box structure
    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = this->m_hWnd;
    ofn.hInstance = AfxGetInstanceHandle();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Evaluation Logs\0*.txt";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT;

    if (GetOpenFileName(&ofn) == TRUE)
    {
        CString fileName, filePath;

        char* pt = szFile;
        if (strlen(pt) == 0)
        {
            return;
        }
        filePath.Format("%s", pt);
        pt += strlen(filePath) + 1;

        /* only one file chosen */
        if (strlen(pt) == 0)
        {
            pt = strrchr(filePath.GetBuffer(), '\\');
            fileName.Format("%s", pt + 1);
            pt[0] = 0;
            OpenLogFile(fileName, filePath);
            return;
        }

        /* several files chosen */
        while (strlen(pt) != 0)
        {
            fileName.Format("%s", pt);
            if (OpenLogFile(fileName, filePath))
                break;

            pt += strlen(pt) + 1;
            if (pt[1] == 0)
                break;
        }
    }
}

int CPostFluxDlg::OpenLogFile(CString& filename, CString& filePath)
{
    int fileType, nChannels;
    double fileVersion;

    memset((void*)columnBuffer, 0, sizeof(double) * MAX_TRAVERSELENGTH);
    memset((void*)colErrBuffer, 0, sizeof(double) * MAX_TRAVERSELENGTH);
    memset((void*)intensityBuffer, 0, sizeof(double) * MAX_TRAVERSELENGTH);
    memset((void*)altitudeBuffer, 0, sizeof(double) * MAX_TRAVERSELENGTH);
    memset((void*)timeBuffer, 0, sizeof(double) * MAX_TRAVERSELENGTH);
    memset((void*)latBuffer, 0, sizeof(double) * MAX_TRAVERSELENGTH);
    memset((void*)lonBuffer, 0, sizeof(double) * MAX_TRAVERSELENGTH);

    if (UpdateData(TRUE))
    {
        // Read the header of the log file and see if it is an ok file
        fileType = m_flux->ReadSettingFile((LPCSTR)filename, nChannels, fileVersion);
        if (fileType != 1)
        {
            MessageBox(TEXT("The file is not evaluation log file with right format.\nPlease choose a right file"), NULL, MB_OK);
            return 1;
        }

        // allocate space for the new channels
        for (int k = 1; k <= nChannels; ++k)
        {
            mobiledoas::CTraverse* tr = new mobiledoas::CTraverse();
            m_flux->m_traverse.push_back(tr);
        }

        // Read the data from the file
        int oldNumberOfFilesOpened = m_flux->m_traverseNum;
        if (0 == m_flux->ReadLogFile((LPCSTR)filePath, (LPCSTR)filename, nChannels, fileVersion))
        {
            MessageBox(TEXT("That file is empty"));
            return 1;
        }

        // Update the list of opened log files
        CString tmpStr;
        for (int k = 0; k < nChannels; ++k)
        {
            tmpStr.Format("%s", m_flux->m_traverse[oldNumberOfFilesOpened + k]->m_fileName.c_str());
            m_pathList.AddString(tmpStr);
        }
        m_pathList.UpdateWidth();
        this->m_pathList.SetCurSel(m_flux->m_curTraverse);

    }

    OnChangeSelectedFile();

    return 0;
}

/* when the user has selected the menu item 'Reload This Log File' */
void CPostFluxDlg::OnReloadLogfile()
{
    //CString filePath, fileName;
    //m_flux->GetCurrentFileName(filePath); // get the full path and filename of the file

    //int fileType = m_flux->ReadSettingFile(filePath, m_flux->m_curTraverse);
    //if(fileType != 1){
     // MessageBox(TEXT("The file is not evaluation log file with right format.\nPlease choose a right file"),NULL,MB_OK);
     // return;
    //}

    //int stringIndex = filePath.ReverseFind('\\');
    //if(stringIndex != -1)
    //  fileName.Format(filePath.Right(strlen(filePath) - stringIndex - 1));
    //filePath.Format(filePath.Left(stringIndex));

    //if(0 == m_flux->ReadLogFile(filePath, fileName, m_flux->m_curTraverse)){
    //  MessageBox(TEXT("That file is empty"));
    //  return;
    //}
    //OnChangeSelectedFile();
}

/* When the button 'Flux=' has been pressed, calculate the flux */
void CPostFluxDlg::OnCalculateFlux()
{
    CString fluxMessage, fluxRangeMessage, strMessage, str1, str2, str3;
    Common* m_common = new Common();
    char timetxt[25];
    if (!UpdateData(TRUE))
    {
        delete m_common;
        return;
    }

    // 1. Make sure that the range of data points is valid
    if (m_lowIndex >= m_highIndex)
    {
        MessageBox(TEXT("It should be from one value to a bigger value.\nPleas input again"),
            TEXT("Input Error"), MB_OK);

        this->SetDlgItemText(IDC_FLUXDFROM, TEXT("0"));
        this->SetDlgItemText(IDC_FLUXDTO, TEXT("1"));
        m_sliderFrom.SetPos(0);
        m_sliderTo.SetPos(1);
        delete m_common;
        return;
    }

    m_flux->SetParams(m_WindSpeed, m_WindDirect, m_srcLat, m_srcLon,
        m_lowIndex, m_highIndex, m_flux->m_traverse[m_flux->m_curTraverse]->m_Offset);
    m_flux->fCreateAdditionalLog = m_additionalLogCheck;

    ShowColumn();

    mobiledoas::GetDateTimeText(timetxt);
    m_flux->SetParams(m_WindSpeed, m_WindDirect, m_srcLat, m_srcLon,
        m_lowIndex, m_highIndex, m_flux->m_traverse[m_flux->m_curTraverse]->m_Offset);

    // Calculate the flux
    double totalFlux = m_flux->GetTotalFlux();

    // Calculate the relative error in flux
    double fluxError_Low = (m_flux->m_totalFlux_Low - m_flux->m_totalFlux) / m_flux->m_totalFlux;
    double fluxError_High = (m_flux->m_totalFlux_High - m_flux->m_totalFlux) / m_flux->m_totalFlux;

    // Convert the unit...
    CString dispFmt, dispUnit;
    double dispFlux, dispLow, dispHigh, dispErrorLow, dispErrorHigh;
    if (m_unitSelection.GetCurSel() == UNIT_KGS)
    {
        dispFmt = "(%.2f - %.2f) [kg/s] <-> \n (%.1lf - %.1lf) [%%]";
        dispUnit = " [kg/s]\n";
        dispFlux = abs(totalFlux);
        dispLow = abs(m_flux->m_totalFlux_Low);
        dispHigh = abs(m_flux->m_totalFlux_High);
        dispErrorLow = 100 * fluxError_Low;
        dispErrorHigh = 100 * fluxError_High;
    }
    else
    {
        dispFmt = "(%.2f - %.2f) [ton/day] <-> \n (%.1lf - %.1lf) [%%]";
        dispUnit = " [ton/day]\n";
        dispFlux = abs(totalFlux) * 3.6 * 24;
        dispLow = abs(m_flux->m_totalFlux_Low) * 3.6 * 24;
        dispHigh = abs(m_flux->m_totalFlux_High) * 3.6 * 24;
        dispErrorLow = 100 * fluxError_Low;
        dispErrorHigh = 100 * fluxError_High;
    }
    fluxMessage.Format(_T("%f"), dispFlux);
    fluxRangeMessage.Format(dispFmt, dispLow, dispHigh, dispErrorLow, dispErrorHigh);
    this->SetDlgItemText(IDC_FLUX_EDIT, fluxMessage);
    this->SetDlgItemText(IDC_LABEL_FLUXRANGE, fluxRangeMessage);
    fluxMessage.AppendFormat(dispUnit);

    // Output the results to the user
    strMessage.Format("PlumeWidth=%ld [m]\r\nTraverseLength=%ld [m]",
        (long)abs(m_flux->plumeWidth), (long)m_flux->traverseLength);
    this->SetDlgItemText(IDC_STATISTICS, strMessage);

    // Output the results to file
    std::string fileName;
    m_flux->GetCurrentFileName(fileName);
    str1.Format("Processed file: \t%s\n", fileName);
    str2.Format("Processed time:\t%s\n", timetxt);
    if (m_flux->m_traverse[m_flux->m_curTraverse]->m_useWindField)
        str3.Format("Wind Speed from imported wind field\nWind Direction from imported wind field\n");
    else
        str3.Format("Wind Speed=%f\nWind Direction=%f\n", m_WindSpeed, m_WindDirect);
    str3.AppendFormat("Source latitude=%f\nSource longitude=%f\nColumn offset=%f\n",
        m_srcLat, m_srcLon, m_flux->m_traverse[m_flux->m_curTraverse]->m_Offset);

    m_common->WriteLogFile(TEXT("fluxCalculation.txt"), fluxMessage + str1 + str2 + str3 + strMessage + "\n");

    delete m_common;

    UpdateMobileLog();
}

BOOL CPostFluxDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    CRect columnRect;
    m_routeDlg = new CRouteDlg();
    columnRect = CRect(DLGLEFT, DLGTOP, DLGRIGHT, DLGBOTTOM);
    m_ColumnPlot.Create(WS_VISIBLE | WS_CHILD, columnRect, this);
    m_PlotColor = RGB(255, 0, 0);
    m_ColumnPlot.SetSecondYUnit(TEXT("Intensity"));
    m_ColumnPlot.SetYUnits("Column");
    m_ColumnPlot.SetXUnits("Number");
    m_ColumnPlot.SetBackgroundColor(RGB(0, 0, 0));
    m_ColumnPlot.SetGridColor(RGB(255, 255, 255));//(192, 192, 255)) ;
    m_ColumnPlot.SetPlotColor(m_PlotColor);
    m_ColumnPlot.SetRange(0, 500, 0, -100.0, 100.0, 1);
    m_ColumnPlot.SetSecondRangeY(0, 100, 0);
    m_ColumnPlot.m_parentWindow = this;

    m_sliderFrom.SetRange(0, 499);
    m_sliderTo.SetRange(1, 500);
    m_sliderOffset.SetRange(-100, 100);
    m_sliderOffset.SetTicFreq(1);
    m_sliderFrom.SetPos(0);
    m_sliderTo.SetPos(499);
    m_sliderOffset.SetPos(100);

    m_intensitySliderLow.SetRange(0, 100);// the intensity slider is in percent
    m_intensitySliderLow.SetPos(95);/* The intensity slider is upside down */
    m_intensitySliderLow.SetTicFreq(25);

    m_intensitySliderHigh.SetRange(0, 100);// the intensity slider is in percent
    m_intensitySliderHigh.SetPos(5);/* The intensity slider is upside down */
    m_intensitySliderHigh.SetTicFreq(25);

    this->SetDlgItemText(IDC_INTENSITY_EDIT_LOW, TEXT("400"));
    this->SetDlgItemText(IDC_INTENSITY_EDIT_HIGH, TEXT("-400"));

    this->SetDlgItemText(IDC_STATISTICS, TEXT(""));

    /* make sure that this window handles the messages from the popup-menu in the path list */
    m_pathList.m_parentWindow = this;

    /* Enable tool tips */
    this->EnableToolTips();
    m_GraphToolTip = new CToolTipCtrl();
    if (!m_GraphToolTip->Create(this))
    {
        TRACE("Unable To create ToolTip\n");
        return TRUE;
    }
    m_GraphToolTip->SetMaxTipWidth(400);
    m_GraphToolTip->AddTool(&m_ColumnPlot, "The Column plot. \r\n Red - The column values (scale to the left) \r\n White - the intensities (in % of max) \r\n Green Line - Peak of plume \r\n Yellow Line - Center of mass of plume ");
    m_GraphToolTip->Activate(TRUE);

    /* set the unit picker */
    this->m_unitSelection.SetCurSel(UNIT_TONDAY);

    /* Read the mobile log to remember what settings was used the last time */
    ReadMobileLog();

    UpdateData(FALSE);

    // Also move the window to the centre of the screen
    CRect rect;
    this->GetWindowRect(rect);
    int windowWidth = rect.Width();
    int windowHeight = rect.Height();
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    rect.left = (screenWidth - windowWidth) / 2;
    rect.right = rect.left + windowWidth;
    rect.top = (screenHeight - windowHeight) / 2;
    rect.bottom = rect.top + windowHeight;
    this->MoveWindow(rect);


    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CPostFluxDlg::OnClose()
{
    if (m_flux != nullptr)
    {
        delete(m_flux);
        m_flux = nullptr;
    }

    if (m_routeDlg != nullptr)
    {
        delete(m_routeDlg);
        m_routeDlg = nullptr;
    }

    if (m_GraphToolTip != nullptr)
    {
        delete(m_GraphToolTip);
        m_GraphToolTip = nullptr;
    }

    CDialog::OnClose();
}

void CPostFluxDlg::ShowColumn()
{
    Common common;
    static double* number = (double*)calloc(MAX_TRAVERSELENGTH, sizeof(double));
    static double* distance = (double*)calloc(MAX_TRAVERSELENGTH, sizeof(double));

    if (number[1] == 0)
    {
        for (int i = 1; i < MAX_TRAVERSELENGTH; ++i)
            number[i] = i;
    }

    if (m_left >= m_right)
        return;

    // Set the buffer to use for the x-axis data
    double* xBuffer = nullptr;
    if (m_XAxisUnit == 2)
    {
        m_ColumnPlot.SetXAxisNumberFormat(Graph::FORMAT_GENERAL);
        m_ColumnPlot.SetXUnits("Distance travelled [km]");
        xBuffer = distance;

        // fill in the distance buffer
        distance[0] = 0.0;
        int k;
        for (k = 1; k < m_right - m_left + 1; ++k)
        {
            distance[k] = distance[k - 1] + 1e-3 * mobiledoas::GPSDistance(latBuffer[k], lonBuffer[k], latBuffer[k - 1], lonBuffer[k - 1]);
        }
        if (distance[k - 1] < 1.0)
        {
            MessageBox("No GPS data found in traverse. Will show measurement against spectrum number");
            m_XAxisUnit = 0;
        }
    }
    if (m_XAxisUnit == 1)
    {
        if (timeBuffer[1] < 1.0)
        {
            MessageBox("No Time data found in traverse. Will show measurement against spectrum number");
            m_XAxisUnit = 0;
        }
        else
        {
            m_ColumnPlot.SetXAxisNumberFormat(Graph::FORMAT_TIME);
            xBuffer = this->timeBuffer;
            m_ColumnPlot.SetXUnits("Time");
        }
    }
    if (m_XAxisUnit == 0)
    {
        m_ColumnPlot.SetXAxisNumberFormat(Graph::FORMAT_GENERAL);
        m_ColumnPlot.SetXUnits("Number");
        xBuffer = number;
    }

    /* Draw the column */
    if (m_showColumnError)
    {
        m_ColumnPlot.XYPlot(xBuffer, columnBuffer, NULL, NULL, colErrBuffer, (int)(m_right - m_left + 1));
    }
    else
    {
        m_ColumnPlot.XYPlot(xBuffer, columnBuffer, (int)(m_right - m_left + 1));
    }

    /* Draw the plume-center lines */
    if (maxBuf[4] < (m_right - m_left + 1))
    {
        m_ColumnPlot.DrawLine(VERTICAL, xBuffer[(int)maxBuf[4]], GREEN, STYLE_DASHED);
    }
    if (avBuf[4] < (m_right - m_left + 1))
    {
        m_ColumnPlot.DrawLine(VERTICAL, xBuffer[(int)avBuf[4]], YELLOW, STYLE_DASHED);
    }

    /* draw the offset line */
    m_ColumnPlot.DrawLine(HORIZONTAL, m_flux->m_traverse[m_flux->m_curTraverse]->m_Offset, ORANGE, STYLE_DASHED);

    /* draw the intensity */
    if (m_showIntensity)
    {
        /* the intensity low limit */
        int selectedIntensityLow = 100 - m_intensitySliderLow.GetPos();
        m_ColumnPlot.DrawLine(HORIZONTAL, selectedIntensityLow, YELLOW, STYLE_DASHED, CGraphCtrl::PLOT_SECOND_AXIS);

        /* the intensity high limit */
        int selectedIntensityHigh = 100 - m_intensitySliderHigh.GetPos();
        m_ColumnPlot.DrawLine(HORIZONTAL, selectedIntensityHigh, YELLOW, STYLE_DASHED, CGraphCtrl::PLOT_SECOND_AXIS);

        /* the circles showing the intensity */
        m_ColumnPlot.DrawCircles(xBuffer, intensityBuffer, (int)(m_right - m_left + 1), CGraphCtrl::PLOT_SECOND_AXIS);
    }

    /* draw the lower and higher limit */
    int from = m_sliderFrom.GetPos();
    int to = m_sliderTo.GetPos();
    m_ColumnPlot.DrawLine(VERTICAL, xBuffer[from], ORANGE, STYLE_DASHED);
    m_ColumnPlot.DrawLine(VERTICAL, xBuffer[to], ORANGE, STYLE_DASHED);

    /* shade the area outside of the lower and higher limit */
    double shade = 0.7;
    if (from != 0)
    {
        m_ColumnPlot.ShadeFilledSquare(xBuffer[1], xBuffer[from], m_ColumnPlot.GetYMin() + 1, m_ColumnPlot.GetYMax() - 1, shade);
    }
    if (to != (int)(m_right - m_left))
    {
        m_ColumnPlot.ShadeFilledSquare(xBuffer[to], xBuffer[(int)(m_right - m_left)], m_ColumnPlot.GetYMin() + 1, m_ColumnPlot.GetYMax() - 1, shade);
    }
}

/* The user has pressed the '>>' button, show the next part of the traverse */
void CPostFluxDlg::OnBtnfor()
{
    if (fMovePlot && (m_right <= m_recordNumber))
    {
        m_left = m_left + 100;
        m_right = m_right + 100;

        ShowColumn();
        m_sliderFrom.SetRange(m_left, m_right);
        m_sliderTo.SetRange(m_left, m_right);
    }
}

/* The user has pressed the '<<' button, show the previous part of the traverse */
void CPostFluxDlg::OnBtnback()
{
    if (fMovePlot && (m_left >= 100))//0))
    {
        m_left = m_left - 100;
        m_right = m_right - 100;
        if (m_left >= 0)
            ShowColumn();

        m_sliderFrom.SetRange(m_left, m_right);
        m_sliderTo.SetRange(m_left, m_right);
    }
}

void CPostFluxDlg::ShowPlumeCenter()
{
    CString strMaxwd, strAvwd;
    m_flux->GetPlumeCenter(m_srcLat, m_srcLon, maxBuf, avBuf);

    strMaxwd.Format("Max Center\t%.2f", maxBuf[3]);
    strAvwd.Format("Average Center\t%.2f", avBuf[3]);

    //if(UpdateData(TRUE))
    {
        this->SetDlgItemText(IDC_MAXWD, strMaxwd);
        this->SetDlgItemText(IDC_AVWD, strAvwd);
    }

    m_windDirectionAverage = avBuf[3];
    m_windDirectionMax = maxBuf[3];
}



void CPostFluxDlg::OnReleasedcaptureSliderfrom(NMHDR* pNMHDR, LRESULT* pResult)
{
    CString str;
    m_flux->m_traverse[m_flux->m_curTraverse]->m_Offset = m_ColumnPlot.GetYMax() - m_sliderOffset.GetPos() + m_ColumnPlot.GetYMin();
    ShowColumn();

    str.Format("%d", m_sliderFrom.GetPos());
    this->SetDlgItemText(IDC_FLUXDFROM, str);

    *pResult = 0;
}

void CPostFluxDlg::OnReleasedcaptureSliderto(NMHDR* pNMHDR, LRESULT* pResult)
{
    CString str;
    m_flux->m_traverse[m_flux->m_curTraverse]->m_Offset = m_ColumnPlot.GetYMax() - m_sliderOffset.GetPos() + m_ColumnPlot.GetYMin();
    ShowColumn();
    m_lowIndex = m_sliderFrom.GetPos();
    m_highIndex = m_sliderTo.GetPos();
    UpdateData(FALSE);
    *pResult = 0;
}

void CPostFluxDlg::OnReleasedcaptureSlideroffset(NMHDR* pNMHDR, LRESULT* pResult)
{
    if (m_flux->m_traverseNum == 0)
        return;

    m_flux->m_traverse[m_flux->m_curTraverse]->m_Offset = m_ColumnPlot.GetYMax() - m_sliderOffset.GetPos() + m_ColumnPlot.GetYMin();
    ShowColumn();   /* draw the graph */

    UpdateData(FALSE);

    *pResult = 0;
}

/**Delete the bad points which are not needed in evaluation
*
*/
void CPostFluxDlg::OnBtnDeleteSelected()
{
    if (UpdateData(TRUE))
    {
        /* if the user has marked more than 50 points for deletion, make an question of
            this really is what the user wants to do */
        if (m_highIndex - m_lowIndex > 50)
        {
            CString message;
            message.Format("This will delete %ld points from the traverse. Continue?", m_highIndex - m_lowIndex);
            int answer = MessageBox(message, "Delete", MB_YESNO);
            if (IDNO == answer)
                return;
        }

        // Delete the points
        m_flux->m_traverse[m_flux->m_curTraverse]->DeletePoints(m_lowIndex, m_highIndex);

        m_recordNumber = m_flux->GetColumn(columnBuffer);
        m_flux->GetColumnError(colErrBuffer);
        m_flux->GetIntensity(intensityBuffer);
        m_flux->GetAltitude(altitudeBuffer);
        m_flux->GetTime(timeBuffer);
        m_flux->GetLat(latBuffer);
        m_flux->GetLon(lonBuffer);

        // Convert the intensities to saturation-ratios
        double	dynRange_inv = 100.0 / (double)m_flux->GetDynamicRange();
        for (int k = 0; k < m_recordNumber; ++k)
        {
            intensityBuffer[k] = intensityBuffer[k] * dynRange_inv;
        }

        fMovePlot = TRUE;
        if (m_recordNumber < MAX_TRAVERSE_SHOWN)
        {
            m_left = 0;
            m_right = m_recordNumber - 1;
        }
        else
        {
            m_left = 0;
            m_right = MAX_TRAVERSE_SHOWN - 1;
        }

        m_sliderFrom.SetRange(0, m_right - 1);
        m_sliderTo.SetRange(0, m_right);
        m_sliderFrom.SetPos(0);
        m_sliderTo.SetPos(m_right);
        ShowPlumeCenter();
        ShowColumn();

        m_sliderOffset.SetRange((int)m_ColumnPlot.GetYMin(), (int)m_ColumnPlot.GetYMax());
        m_sliderOffset.SetPos((int)(m_ColumnPlot.GetYMax() + m_ColumnPlot.GetYMin() - m_flux->m_traverse[m_flux->m_curTraverse]->m_Offset));
        m_lowIndex = 0;
        m_highIndex = m_right;
        m_flux->m_traverse[m_flux->m_curTraverse]->CalculateOffset();
        UpdateData(FALSE);

    }
}

void CPostFluxDlg::OnBnClickedBtnDelIntensity()
{
    long nDeleted;
    if (UpdateData(TRUE))
    {
        // delete the points
        long	dynRange = m_flux->GetDynamicRange();

        long	intensityLimitLow = (long)(dynRange * (100.0 - m_intensitySliderLow.GetPos()) / 100.0);
        nDeleted = m_flux->m_traverse[m_flux->m_curTraverse]->DeleteLowIntensityPoints(intensityLimitLow);

        long	intensityLimitHigh = (long)(dynRange * (100.0 - m_intensitySliderHigh.GetPos()) / 100.0);
        nDeleted += m_flux->m_traverse[m_flux->m_curTraverse]->DeleteHighIntensityPoints(intensityLimitHigh);
        m_recordNumber = m_flux->GetColumn(columnBuffer);
        m_flux->GetColumnError(colErrBuffer);
        m_flux->GetIntensity(intensityBuffer);
        m_flux->GetAltitude(altitudeBuffer);
        m_flux->GetTime(timeBuffer);
        m_flux->GetLat(latBuffer);
        m_flux->GetLon(lonBuffer);

        // Convert the intensities to saturation-ratios
        double	dynRange_inv = 100.0 / (double)m_flux->GetDynamicRange();
        for (int k = 0; k < m_recordNumber; ++k)
        {
            intensityBuffer[k] = intensityBuffer[k] * dynRange_inv;
        }

        fMovePlot = TRUE;
        if (m_recordNumber < MAX_TRAVERSE_SHOWN)
        {
            m_left = 0;
            m_right = m_recordNumber - 1;
        }
        else
        {
            m_left = 0;
            m_right = MAX_TRAVERSE_SHOWN - 1;
        }

        m_sliderFrom.SetRange(0, m_right - 1);
        m_sliderTo.SetRange(0, m_right);
        m_sliderFrom.SetPos(0);
        m_sliderTo.SetPos(m_right);
        ShowPlumeCenter();
        ShowColumn();

        m_sliderOffset.SetRange((int)m_ColumnPlot.GetYMin(), (int)m_ColumnPlot.GetYMax());
        m_sliderOffset.SetPos((int)(m_ColumnPlot.GetYMax() + m_ColumnPlot.GetYMin() - m_flux->m_traverse[m_flux->m_curTraverse]->m_Offset));
        m_lowIndex = 0;
        m_highIndex = m_right;
        m_flux->m_traverse[m_flux->m_curTraverse]->m_Offset = 0;
        UpdateData(FALSE);

        CString tmpStr;
        tmpStr.Format("Deleted %ld points", nDeleted);
        MessageBox(tmpStr);
    }
}

void CPostFluxDlg::OnBtnShowRoute()
{
    UpdateData(TRUE);

    m_routeDlg->m_flux = this->m_flux;
    m_routeDlg->m_srcLat = m_srcLat;
    m_routeDlg->m_srcLon = m_srcLon;
    m_routeDlg->m_initalSelection = m_pathList.GetCurSel();
    m_routeDlg->DoModal();

    // The dialog has closed. Check if the user has changed the lat and long
    //	of the source, if so then change also in this window
    if (m_srcLat != m_routeDlg->m_srcLat || m_srcLon != m_routeDlg->m_srcLon)
    {
        m_srcLat = m_routeDlg->m_srcLat;
        m_srcLon = m_routeDlg->m_srcLon;
        UpdateData(FALSE);
        OnChangeSource();
    }
}

// if the user clicked the 'View'->'Show Intensity' menu item
void CPostFluxDlg::OnMenuViewShowIntensity()
{
    m_showIntensity = !m_showIntensity;
    ShowColumn();
}

// if the user clicked the 'View'->'Show ColumnError' menu item
void CPostFluxDlg::OnMenuViewShowColumnError()
{
    m_showColumnError = !m_showColumnError;
    ShowColumn();
}

void CPostFluxDlg::OnNMReleasedcaptureSliderintensityLow(NMHDR* pNMHDR, LRESULT* pResult)
{
    // TODO: Add your control notification handler code here
    *pResult = 0;

    CString tmpStr;
    tmpStr.Format("%ld", 100 - m_intensitySliderLow.GetPos());
    this->SetDlgItemText(IDC_INTENSITY_EDIT_LOW, tmpStr);

    ShowColumn();
}

void CPostFluxDlg::OnNMReleasedcaptureSliderintensityHigh(NMHDR* pNMHDR, LRESULT* pResult)
{
    // TODO: Add your control notification handler code here
    *pResult = 0;

    CString tmpStr;
    tmpStr.Format("%ld", 100 - m_intensitySliderHigh.GetPos());
    this->SetDlgItemText(IDC_INTENSITY_EDIT_HIGH, tmpStr);

    ShowColumn();
}

/* Read the mobile log - the 'memory' of the program */
void CPostFluxDlg::ReadMobileLog()
{
    double lat, lon;
    char txt[256];
    char* pt = 0;

    /* Find the last flux center used */
    FILE* f = fopen(g_exePath + "MobileLog.txt", "r");

    if (0 != f)
    {
        while (fgets(txt, sizeof(txt) - 1, f))
        {

            if (pt = strstr(txt, "FLUXCENTER="))
            {
                pt = strstr(txt, "=");
                sscanf(&pt[1], "%lf\t%lf", &lat, &lon);
                if (-180 <= lat && 180 >= lat && -180 <= lon && 180 >= lon)
                {
                    m_srcLat = lat;
                    m_srcLon = lon;
                }
                break;
            }
        }
    }
    else
    {
        return;
    }

    fclose(f);

    return;
}

void CPostFluxDlg::UpdateMobileLog()
{
    fpos_t filePos;
    char txt[256];
    char* pt = 0;

    /* Open the mobile log file */
    FILE* f = fopen(g_exePath + "MobileLog.txt", "r+");

    if (0 == f)
    {
        /* File might not exist */
        f = fopen(g_exePath + "MobileLog.txt", "w");
        if (0 == f)
        {
            /* File cannot be opened */
            return;
        }
        else
        {
            fprintf(f, "FLUXCENTER=%lf\t%lf\n", m_srcLat, m_srcLon);
            fclose(f);
            return;
        }
    }
    else
    {
        if (0 != fgetpos(f, &filePos))
        {
            fclose(f);
            return;
        }
        while (fgets(txt, sizeof(txt) - 1, f))
        {
            if (pt = strstr(txt, "FLUXCENTER="))
            {
                /* Update */
                fflush(f);
                fsetpos(f, &filePos);
                fprintf(f, "FLUXCENTER=%lf\t%lf\n", m_srcLat, m_srcLon);
                fclose(f);
                return;
            }
            else
            {
                /* Save the current position before we continue */
                if (0 != fgetpos(f, &filePos))
                {
                    fclose(f);
                    return;
                }
            }
        }

        /* there was no note about the flux center in the mobile log */
        fprintf(f, "FLUXCENTER=%lf\t%lf\n", m_srcLat, m_srcLon);

        fclose(f);
        return;
    }
}

void CPostFluxDlg::OnStnClickedMaxwd()
{
    m_WindDirect = m_windDirectionMax;
    UpdateData(FALSE);
}

void CPostFluxDlg::OnStnClickedAvwd()
{
    m_WindDirect = m_windDirectionAverage;
    UpdateData(FALSE);
}

BOOL CPostFluxDlg::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
    static char string[512];

    TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pNMHDR;
    UINT_PTR nID = pNMHDR->idFrom;
    nID = ::GetDlgCtrlID((HWND)nID);

    int ok = 0;

    if (nID == IDC_BTN_DEL_INTENSITY)
    {
        ok = sprintf(string, "Deletes all spectra with intensity below or above the values indicated by the Intensity sliders above.");
    }
    if (nID == IDC_BTNDEL)
    {
        ok = sprintf(string, "Deletes all spectra in the range indicated by the 'From' and 'To' sliders above.");
    }
    if (nID == IDC_CALC)
    {
        ok = sprintf(string, "Calculates the flux using the spectrum in the selected range and the values of wind speed and wind direction given above");
    }
    if (nID == IDC_SLIDERINTENSITY_LOW)
    {
        ok = sprintf(string, "Intensity slider low: exact value of this slider is shown below. ");
    }
    if (nID == IDC_SLIDERINTENSITY_HIGH)
    {
        ok = sprintf(string, "Intensity slider high: exact value of this slider is shown below. ");
    }
    if (nID == IDC_INTENSITY_EDIT_LOW)
    {
        ok = sprintf(string, "Value of the Intensity (low) slider.");
    }
    if (nID == IDC_INTENSITY_EDIT_HIGH)
    {
        ok = sprintf(string, "Value of the Intensity (high) slider.");
    }
    if (nID == IDC_SLIDEROFFSET)
    {
        ok = sprintf(string, "Offset Slider. Should be set so that the column values outside of the plume varies around zero. Exact value shown below.");
    }
    if (nID == IDC_INTENSITY_CHECK)
    {
        ok = sprintf(string, "If checked, the graph will show the intensity of each collected spectrum as a small white square");
    }
    if (nID == IDC_CHANNEL_COMBO)
    {
        ok = sprintf(string, "The channel selector. If several spectrometer channels were on during the traverse this selects which channel we're currently working on");
    }
    if (nID == IDC_FLUXDOFFSET)
    {
        ok = sprintf(string, "The value of the offset slider");
    }
    if (nID == IDC_FLUXDFROM)
    {
        ok = sprintf(string, "Beginning of the selected range");
    }
    if (nID == IDC_FLUXDTO)
    {
        ok = sprintf(string, "End of the selected range");
    }
    if (nID == IDC_ADDITIONAL_CHECK)
    {
        ok = sprintf(string, "If checked a second, overly detailed, flux log will be created when pressing the 'Flux' button");
    }
    if (nID == IDC_SLIDERFROM)
    {
        ok = sprintf(string, "Slider to select the beginning of the range. Value shown to the right. ");
    }
    if (nID == IDC_SLIDERTO)
    {
        ok = sprintf(string, "Slider to select the end of the range. Value shown to the right. ");
    }
    if (nID == IDC_BTNROUTE)
    {
        ok = sprintf(string, "Shows a map of the traverse");
    }
    if (nID == IDC_BTNCHOOSEFILE)
    {
        ok = sprintf(string, "Click to select the EvaluationLog for the traverse. Only Evaluation logs from MobileDOAS are accepted");
    }
    if (nID == IDC_MAXWD)
    {
        ok = sprintf(string, "Wind Direction calculated using source position given above and the peak value of the plume");
    }
    if (nID == IDC_AVWD)
    {
        ok = sprintf(string, "Wind Direction calculated using source position given above and the center of mass of the plume");
    }

    if (ok)
    {
        pTTT->lpszText = string;
        return TRUE;
    }

    return FALSE;
}

BOOL CPostFluxDlg::PreTranslateMessage(MSG* pMsg)
{
    if (nullptr != m_GraphToolTip)
        m_GraphToolTip->RelayEvent(pMsg);

    return CDialog::PreTranslateMessage(pMsg);
}

void CPostFluxDlg::OnBnClickedBtnSourceLat()
{
    Dialogs::CSourceSelectionDlg sourceDlg;
    INT_PTR modal = sourceDlg.DoModal();
    if (IDOK == modal)
    {
        m_srcLat = sourceDlg.m_selectedLat;
        m_srcLon = sourceDlg.m_selectedLon;

        UpdateData(FALSE);
        ShowPlumeCenter();
    }

    UpdateData(FALSE);
}

void CPostFluxDlg::OnBnClickedBtnSourceLong()
{
    Dialogs::CSourceSelectionDlg sourceDlg;
    INT_PTR modal = sourceDlg.DoModal();
    if (IDOK == modal)
    {
        if (sourceDlg.m_selectedLat > -1)
        {
            this->m_srcLat = sourceDlg.m_selectedLat;
            this->m_srcLon = sourceDlg.m_selectedLon;
        }
        UpdateData(FALSE);
        ShowPlumeCenter();
    }

    UpdateData(FALSE);
}

void CPostFluxDlg::OnLbnSelchangePathList()
{
    OnChangeSelectedFile();
}

int CPostFluxDlg::OnChangeSelectedFile()
{
    // The user has selected another traverse to show

    int curSel = m_pathList.GetCurSel();
    if (curSel < 0)
        return 1;

    m_flux->m_curTraverse = curSel;

    m_recordNumber = m_flux->GetColumn(columnBuffer);
    m_flux->GetColumnError(colErrBuffer);
    m_flux->GetIntensity(intensityBuffer);
    m_flux->GetAltitude(altitudeBuffer);
    m_flux->GetTime(timeBuffer);
    m_flux->GetLat(latBuffer);
    m_flux->GetLon(lonBuffer);

    // Convert the intensities to saturation-ratios
    double	dynRange_inv = 100.0 / (double)m_flux->GetDynamicRange();
    for (int k = 0; k < m_recordNumber; ++k)
    {
        intensityBuffer[k] = intensityBuffer[k] * dynRange_inv;
    }

    if (m_recordNumber < MAX_TRAVERSE_SHOWN)
    {
        m_left = 0;
        m_right = m_recordNumber - 1;
    }
    else
    {
        m_left = 0;
        m_right = MAX_TRAVERSE_SHOWN - 1;
    }

    if (m_right < 0)
        return 1;

    m_sliderFrom.SetRange(0, m_right - 1);
    m_sliderTo.SetRange(0, m_right);
    m_sliderFrom.SetPos(0);
    m_sliderTo.SetPos(m_right);

    m_lowIndex = 0;
    m_highIndex = m_right;

    // update the properties
    UpdatePropertiesWindow();

    ShowPlumeCenter();
    ShowColumn();

    m_sliderOffset.SetRange((int)m_ColumnPlot.GetYMin(), (int)m_ColumnPlot.GetYMax());
    m_sliderOffset.SetPos((int)(m_ColumnPlot.GetYMax() + m_ColumnPlot.GetYMin() - m_flux->m_traverse[m_flux->m_curTraverse]->m_Offset));
    m_highIndex = m_right;

    UpdateData(FALSE);

    return 0;
}

// updating the 'View'->'Show Intensity' menu item
void CPostFluxDlg::OnUpdateViewShowintensity(CCmdUI* pCmdUI)
{
    if (m_showIntensity)
    {
        pCmdUI->SetCheck(1);
    }
    else
    {
        pCmdUI->SetCheck(0);
    }
}
// updating the 'View'->'Show Column Error' menu item
void CPostFluxDlg::OnUpdateViewShowColumnError(CCmdUI* pCmdUI)
{
    if (m_showColumnError)
    {
        pCmdUI->SetCheck(1);
    }
    else
    {
        pCmdUI->SetCheck(0);
    }
}
void CPostFluxDlg::OnUpdateViewShowVsTime(CCmdUI* pCmdUI)
{
    if (m_XAxisUnit == 1)
    {
        pCmdUI->SetCheck(1);
    }
    else
    {
        pCmdUI->SetCheck(0);
    }
}
void CPostFluxDlg::OnUpdateViewShowVsDistance(CCmdUI* pCmdUI)
{
    if (m_XAxisUnit == 2)
    {
        pCmdUI->SetCheck(1);
    }
    else
    {
        pCmdUI->SetCheck(0);
    }
}
void CPostFluxDlg::OnUpdateViewShowVsNumber(CCmdUI* pCmdUI)
{
    if (m_XAxisUnit == 0)
    {
        pCmdUI->SetCheck(1);
    }
    else
    {
        pCmdUI->SetCheck(0);
    }
}
void CPostFluxDlg::OnUpdateMenuItemCreateAdditionalLogFile(CCmdUI* pCmdUI)
{
    if (m_additionalLogCheck)
    {
        pCmdUI->SetCheck(1);
    }
    else
    {
        pCmdUI->SetCheck(0);
    }
}

void CPostFluxDlg::OnUpdateItemReloadLogFile(CCmdUI* pCmdUI)
{
    if (m_recordNumber > 0)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}
void CPostFluxDlg::OnUpdateItemCalculateFlux(CCmdUI* pCmdUI)
{
    if (m_recordNumber > 0)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}
void CPostFluxDlg::OnUpdateItemShowRouteDlg(CCmdUI* pCmdUI)
{
    if (m_recordNumber > 0)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}
void CPostFluxDlg::OnUpdateItemReEvaluateTraverse(CCmdUI* pCmdUI)
{
    if (m_recordNumber > 0)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}
void CPostFluxDlg::OnUpdateItemCloseAllLogFiles(CCmdUI* pCmdUI)
{
    if (m_recordNumber > 0)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}
void CPostFluxDlg::OnUpdateItemSaveColumnGraph(CCmdUI* pCmdUI)
{
    if (m_recordNumber > 0)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}
void CPostFluxDlg::OnUpdateItemExportLogFile(CCmdUI* pCmdUI)
{
    if (m_recordNumber > 0)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}
void CPostFluxDlg::OnUpdateItemDeleteLowIntensity(CCmdUI* pCmdUI)
{
    if (m_recordNumber > 0)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}
void CPostFluxDlg::OnUpdateItemDeleteSelected(CCmdUI* pCmdUI)
{
    if (m_recordNumber > 0)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}
void CPostFluxDlg::OnUpdateItemImportWindField(CCmdUI* pCmdUI)
{
    if (m_recordNumber > 0)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}

void CPostFluxDlg::OnColumnGraphIncreaseLineWidth()
{
    m_ColumnPlot.IncreaseLineWidth();
    ShowColumn();
}

void CPostFluxDlg::OnColumnGraphDecreaseLineWidth()
{
    m_ColumnPlot.DecreaseLineWidth();
    ShowColumn();
}

// update the text in the 'traverse properties' window
void CPostFluxDlg::UpdatePropertiesWindow()
{
    mobiledoas::CTraverse* cur = m_flux->m_traverse[m_flux->m_curTraverse];

    CString tmpStr;
    tmpStr.Format("Exposure Time\t\t%ld [ms]", cur->m_expTime);
    SetDlgItemText(IDC_STATIC_EXPTIME, tmpStr);

    tmpStr.Format("Averaged Spectra\t%ld", cur->m_nSpectra);
    SetDlgItemText(IDC_STATIC_AVERAGE, tmpStr);

    tmpStr.Format("Fit Region [pixels]\t%ld to %ld", cur->m_fitRegion[0], cur->m_fitRegion[1]);
    SetDlgItemText(IDC_STATIC_FITREGION, tmpStr);

    tmpStr.Format("Gas Factor\t\t%.3lf", cur->m_gasFactor);
    SetDlgItemText(IDC_STATIC_GASFACTOR, tmpStr);
}

void CPostFluxDlg::OnReEvaluateThisTraverse()
{
    if (m_flux->m_traverseNum == 0)
        return;

    /* the reevaluation dialog */
    CReEvaluationDlg reEvalDlg;
    reEvalDlg.Construct("ReEvaluation", this, 0);

    std::string filename;
    m_flux->GetCurrentFileName(filename);
    reEvalDlg.m_reeval->m_evalLogFileName.Format("%s", filename.c_str());
    reEvalDlg.m_reeval->m_specFileDir.Format("%s", m_flux->m_traverse[m_flux->m_curTraverse]->m_filePath);
    reEvalDlg.m_reeval->ReadEvaluationLog();

    // open the window
    reEvalDlg.DoModal();

    // reeval->m_mainView should be NULL if there is no page to recieve the messages generated by 'reeval'
    reEvalDlg.m_reeval->m_mainView = nullptr;

    // if the reevaluation is still running, quit it...
    if (reEvalDlg.m_reeval->fRun)
    {
        DWORD dwExitCode;
        HANDLE hThread = nullptr;
        CString messageToUser;

        // find which thread is running
        if (reEvalDlg.m_page5.pReEvalThread != nullptr)
        {
            hThread = reEvalDlg.m_page5.pReEvalThread->m_hThread;
            messageToUser.Format("ReEvaluation has been stopped");
        }

        if (hThread != nullptr && GetExitCodeThread(hThread, &dwExitCode) && dwExitCode == STILL_ACTIVE)
        {
            AfxGetApp()->BeginWaitCursor();
            reEvalDlg.m_reeval->Stop();

            WaitForSingleObject(hThread, INFINITE);
            AfxGetApp()->EndWaitCursor();
            MessageBox(messageToUser, "Info", MB_OK);
        }
        else
        {

        }
    }
    delete reEvalDlg.m_reeval;
}

/* Close all the open log files */
void CPostFluxDlg::OnFileCloseAllLogFiles()
{
    m_flux->m_traverseNum = 0;
    m_flux->m_curTraverse = 0;
    m_flux->m_traverse[0]->m_recordNum = 0;

    m_pathList.ResetContent();

    m_ColumnPlot.CleanPlot();
}

void CPostFluxDlg::OnImportWindField()
{
    CWindFieldDlg dlg;
    dlg.m_flux = m_flux;

    INT_PTR ok = dlg.DoModal();
}



void CPostFluxDlg::OnFileSaveColumnGraph()
{
    CString fileName;

    // Get the fileName
    if (Common::BrowseForFile_SaveAs("*.png;*.bmp;*.gif;*.jpg", fileName))
    {
        if (!Equals(fileName.Right(4), ".png") && !Equals(fileName.Right(4), ".bmp") && !Equals(fileName.Right(4), ".gif") && !Equals(fileName.Right(4), ".jpg"))
            fileName.AppendFormat(".png");

        m_ColumnPlot.SaveGraph(fileName);
    }
}

void CPostFluxDlg::OnChangeSource()
{
    CString strMaxwd, strAvwd;

    // Get the data in the dialog
    if (UpdateData(TRUE))
    {

        m_flux->GetPlumeCenter(m_srcLat, m_srcLon, maxBuf, avBuf);

        strMaxwd.Format("Wind Direction\t%f", maxBuf[3]);
        strAvwd.Format("Wind Direction\t%f", avBuf[3]);

        this->SetDlgItemText(IDC_MAXWD, strMaxwd);
        this->SetDlgItemText(IDC_AVWD, strAvwd);

        m_windDirectionAverage = avBuf[3];
        m_windDirectionMax = maxBuf[3];
    }
}

void CPostFluxDlg::OnFileExportLogfile()
{
    if (m_flux->m_traverseNum == 0)
    {
        MessageBox("Please open an evaluation log file first");
        return;
    }
    Dialogs::CExportEvLogDlg dlg;
    dlg.m_traverse = this->m_flux->m_traverse[m_flux->m_curTraverse];
    dlg.DoModal();
}

/** Exports this evaluation log file in .kml format
    (which can then be read by e.g. Google Earth) */
void CPostFluxDlg::OnFileExportToKML()
{
    CString fileName;

    if (m_flux->m_traverseNum == 0)
    {
        MessageBox("Please open an evaluation log file first");
        return;
    }

    // let the user decide where to store the file
    if (Common::BrowseForFile_SaveAs("*.kml", fileName))
    {

        // if the given file does not end in .kml then add this ending
        if (!Equals(fileName.Right(4), ".kml"))
            fileName.AppendFormat(".kml");

        // Also let the user decide on the scaling height to use
        Dialogs::CSelectionDialog dlg;
        CString selectedScalingHeight;
        dlg.m_windowText.Format("Select which scaling height to use in file");
        int index = 0;
        int numbers[] = { 1, 2, 3, 5, 7 };
        int nNumbers = 5;
        for (int magnitude = 1; magnitude < 4; ++magnitude)
        {
            for (int k = 0; k < nNumbers; ++k)
            {
                double t = numbers[k] * pow(10, magnitude);
                dlg.m_option[index++].Format("%d", (int)t);
            }
        }
        dlg.m_currentSelection = &selectedScalingHeight;
        dlg.DoModal();

        int scalingHeight = atoi(selectedScalingHeight);

        // Store the file
        mobiledoas::CKMLFileHandler::StoreTraverseAsKML(*m_flux->m_traverse[m_flux->m_curTraverse], (LPCSTR)fileName, scalingHeight);
    }
}


void CPostFluxDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
    ASSERT(pPopupMenu != NULL);
    // Check the enabled state of various menu items.

    CCmdUI state;
    state.m_pMenu = pPopupMenu;
    ASSERT(state.m_pOther == NULL);
    ASSERT(state.m_pParentMenu == NULL);

    // Determine if menu is popup in top-level menu and set m_pOther to
    // it if so (m_pParentMenu == NULL indicates that it is secondary popup).
    HMENU hParentMenu;
    if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
        state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
    else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
    {
        CWnd* pParent = this;
        // Child windows don't have menus--need to go to the top!
        if (pParent != NULL &&
            (hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
        {
            int nIndexMax = ::GetMenuItemCount(hParentMenu);
            for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
            {
                if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
                {
                    // When popup is found, m_pParentMenu is containing menu.
                    state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
                    break;
                }
            }
        }
    }

    state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
    for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
        state.m_nIndex++)
    {
        state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
        if (state.m_nID == 0)
            continue; // Menu separator or invalid cmd - ignore it.

        ASSERT(state.m_pOther == nullptr);
        ASSERT(state.m_pMenu != nullptr);
        if (state.m_nID == (UINT)-1)
        {
            // Possibly a popup menu, route to first item of that popup.
            state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
            if (state.m_pSubMenu == nullptr ||
                (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
                state.m_nID == (UINT)-1)
            {
                continue;       // First item of popup can't be routed to.
            }
            state.DoUpdate(this, TRUE);   // Popups are never auto disabled.
        }
        else
        {
            // Normal menu item.
            // Auto enable/disable if frame window has m_bAutoMenuEnable
            // set and command is _not_ a system command.
            state.m_pSubMenu = nullptr;
            state.DoUpdate(this, FALSE);
        }

        // Adjust for menu deletions and additions.
        UINT nCount = pPopupMenu->GetMenuItemCount();
        if (nCount < state.m_nIndexMax)
        {
            state.m_nIndex -= (state.m_nIndexMax - nCount);
            while (state.m_nIndex < nCount && pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
            {
                state.m_nIndex++;
            }
        }
        state.m_nIndexMax = nCount;
    }
}

void CPostFluxDlg::OnChangeCreateAdditionalLogFile()
{
    this->m_additionalLogCheck = !m_additionalLogCheck;
}

void CPostFluxDlg::OnViewShowVsTime()
{
    m_XAxisUnit = 1;

    // Redraw the column graph
    ShowColumn();
}

void CPostFluxDlg::OnViewShowVsDistance()
{
    m_XAxisUnit = 2;

    // Redraw the column graph
    ShowColumn();
}

void CPostFluxDlg::OnViewShowVsNumber()
{
    m_XAxisUnit = 0;

    // Redraw the column graph
    ShowColumn();
}





void CPostFluxDlg::ChangeFluxUnit()
{
    OnCalculateFlux();
}
