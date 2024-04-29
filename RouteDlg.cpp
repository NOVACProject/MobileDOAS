// RouteDlg.cpp : implementation file
//

#undef min
#undef max

#include "stdafx.h"
#include "DMSpec.h"
#include "RouteDlg.h"
#include <MobileDoasLib/Flux/Traverse.h>
#include <MobileDoasLib/Flux/WindField.h>
#include "WindFieldDlg.h"
#include "SourceSelectionDlg.h"
#include <algorithm>
#include <MobileDoasLib/GpsData.h>

#include <Math.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRouteDlg dialog


CRouteDlg::CRouteDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CRouteDlg::IDD, pParent)
    , m_curLatitude(0)
    , m_selectedDisplay(0)
{
    m_showSource = 0;
    m_showColumnOption = 0;
    for (int i = 0; i < MAX_FLUX_LOGFILES; ++i)
        m_useWindField[i] = false;

    m_pointSize = 3;

    m_initalSelection = 0;

    m_windFieldColor = RGB(255, 255, 255);
    m_circlesColor = RGB(0, 255, 0);

    m_showScale = 1;
}


void CRouteDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CRouteDlg)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    DDX_Text(pDX, IDC_SRC_LAT_EDIT, m_srcLat);
    DDX_Text(pDX, IDC_SRC_LON_EDIT, m_srcLon);

    DDX_Control(pDX, IDC_CHECK_SHOW_SOURCE, m_showSourceCheck);
    DDX_Control(pDX, IDC_CHECK_CALC_DISTANCE, m_calcDistance);

    DDX_Control(pDX, IDC_FRAME_LANDMARKS, m_frameLandmarks);

    DDX_Radio(pDX, IDC_SHOW_COLUMN, m_selectedDisplay);
    DDX_Control(pDX, IDC_EVALLOG_LIST, m_evalLogList);

    DDX_Control(pDX, IDC_ROUTEDLG_POINTSIZESPIN, m_pointSizeSpin);
    DDX_Control(pDX, IDC_ROUTDLG_POINTSIZEEDIT, m_pointSizeEdit);

    DDX_Control(pDX, IDC_CHECK_SHOW_INTERPOLATED_WINDFIELD, m_showInterpolatedWindField);

    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRouteDlg, CDialog)
    //{{AFX_MSG_MAP(CRouteDlg)
    ON_MESSAGE(WM_SHOW_LATLONG, OnShowLatLong)
    ON_MESSAGE(WM_LBU_IN_GPSPLOT, OnLBUInGPSGraph)
    ON_MESSAGE(WM_ZOOM_IN_GPSPLOT, OnZoomGPSGraph)

    ON_MESSAGE(WM_END_EDIT, OnEndEditLandMarkPosition) // defined in CMeasGrid

    ON_BN_CLICKED(IDC_CHECK_SHOW_SOURCE, OnBnClickedCheckShowSource)
    ON_EN_CHANGE(IDC_SRC_LAT_EDIT, OnEnChangeSrcLatEdit)
    ON_EN_CHANGE(IDC_SRC_LON_EDIT, OnEnChangeSrcLonEdit)
    ON_BN_CLICKED(IDC_CHECK_CALC_DISTANCE, OnBnClickedCheckCalcDistance)
    ON_WM_LBUTTONUP()
    ON_BN_CLICKED(IDC_SHOW_COLUMN, OnBnClickedShowColumn)
    ON_BN_CLICKED(IDC_SHOW_ALTITUDE, OnBnClickedShowAltitude)
    ON_BN_CLICKED(IDC_SHOW_INTENSITY, OnBnClickedShowIntensity)
    ON_LBN_SELCHANGE(IDC_EVALLOG_LIST, OnLbnSelchangeEvallogList)
    ON_BN_CLICKED(IDC_CHECK_SHOW_INTERPOLATED_WINDFIELD, OnBnClickedCheckShowInterpolatedWindfield)
    ON_BN_CLICKED(IDC_BTN_IMPORT_WINDFIELD, OnBnClickedBtnImportWindfield)
    ON_NOTIFY(UDN_DELTAPOS, IDC_ROUTEDLG_POINTSIZESPIN, OnDeltaposRoutedlgPointsizespin)
    ON_EN_CHANGE(IDC_ROUTDLG_POINTSIZEEDIT, OnEnChangeRoutdlgPointsizeedit)

    // Changing the colors
    ON_COMMAND(ID_COLORS_BACKGROUND, OnChangeBackgroundColor)
    ON_COMMAND(ID_CHANGECOLORS_GRID, OnChangeGridColor)
    ON_COMMAND(ID_CHANGECOLORS_WINDFIELD, OnChangeWindFieldColor)
    ON_COMMAND(ID_CHANGECOLORS_CIRCLES, OnChangeCirclesColor)

    // Setting the color-scale
    ON_COMMAND(ID_CHANGECOLORS_COLORSCALEBW, OnChangeColorScale_BW)
    ON_COMMAND(ID_CHANGECOLORS_COLORSCALEDEFAULT, OnChangeColorScale_Default)

    // Changing the look of the grid
    ON_COMMAND(ID_GRID_INCREASE, OnGridIncrease)
    ON_COMMAND(ID_GRID_DECREASE, OnGridDecrease)

    // Showing or not showing the scale of the plot
    ON_COMMAND(ID_SHOWHIDESCALE, OnChangeShowScale)

    // Saving the route-graph
    ON_COMMAND(ID_FILE_SAVEGRAPHASIMAGE, OnFileSaveGraphAsImage)

    // How the column-values should be shown, as different colors or as different point-sizes
    ON_COMMAND(ID_INDICATECOLUMNVALUEBY_COLOR, OnMenuViewShowColumnByColor)
    ON_COMMAND(ID_INDICATECOLUMNVALUEBY_SIZEOFCIRCLE, OnMenuViewShowColumnBySize)
    ON_UPDATE_COMMAND_UI(ID_INDICATECOLUMNVALUEBY_COLOR, OnUpdateMenuShowColumnByColor)
    ON_UPDATE_COMMAND_UI(ID_INDICATECOLUMNVALUEBY_SIZEOFCIRCLE, OnUpdateMenuShowColumnBySize)

    // Showing or hiding the source
    ON_BN_CLICKED(IDC_BTN_SOURCE_LAT, OnBnClickedSourceLat)
    ON_BN_CLICKED(IDC_BTN_SOURCE_LON, OnBnClickedSourceLon)

    // Updating the interface, notice that this has to be here due to a bug in Microsoft MFC
    //	http://support.microsoft.com/kb/242577
    ON_WM_INITMENUPOPUP()

    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRouteDlg message handlers


BOOL CRouteDlg::OnInitDialog() {
    CString str, errorMessage;

    CDialog::OnInitDialog();

    CRect rect;
    rect = CRect(20, 20, 700, 700);
    m_gpsPlot.Create(WS_VISIBLE | WS_CHILD, rect, this);
    memcpy(&m_gpsPlot.rect, &rect, sizeof(CRect));
    m_gpsPlot.parentWnd = this;
    //	m_gpsPlot.SetBackgroundColor(RGB(0, 0, 0)) ;
    //	m_gpsPlot.SetGridColor(RGB(255,255,255));
    m_gpsPlot.EnableGridLinesX(true);
    m_gpsPlot.SetMarginSpace(0.05f);

    // Initialize the list of landmarks
    m_frameLandmarks.GetWindowRect(&rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    rect.top = 20;
    rect.left = 10;
    rect.right = width - 20;
    rect.bottom = height - 10;

    m_gridLandMarks.Create(rect, &m_frameLandmarks, 0);
    m_gridLandMarks.parent = this;
    while (m_gridLandMarks.GetColumnCount() > 0)
        m_gridLandMarks.DeleteColumn(0);
    m_gridLandMarks.InsertColumn("Lat (dd.dd)");
    m_gridLandMarks.InsertColumn("Long (dd.dd)");
    m_gridLandMarks.SetFixedRowCount(1);
    m_gridLandMarks.SetFixedColumnCount(0);
    for (int i = 0; i < 2; ++i)
        m_gridLandMarks.SetColumnWidth(i, (int)(rect.right / 2.5));

    m_gridLandMarks.SetEditable(TRUE); /* make sure the user can edit the positions */
    m_gridLandMarks.SetRowCount(10);

    // customize the control
    this->m_showSourceCheck.SetCheck(m_showSource);

    m_showInterpolatedWindField.SetCheck(0);

    this->SetDlgItemText(IDC_BEARING_LABEL, "");
    this->SetDlgItemText(IDC_DISTANCE_LABEL, "");

    this->m_selectedDisplay = SHOW_COLUMN;

    str.Format("%d", m_pointSize);
    m_pointSizeEdit.SetWindowText(str);
    m_pointSizeSpin.SetBuddy(&m_pointSizeEdit);

    // check so that there are not too many traveses...
    if (m_flux->m_traverseNum >= MAX_FLUX_LOGFILES) {
        errorMessage.Format("Maximum number of traverses that can be plotted simultaneously is %d. Will only show the first %d measurements.", MAX_FLUX_LOGFILES, MAX_FLUX_LOGFILES);
        MessageBox(errorMessage);
        m_traversesToShow = MAX_FLUX_LOGFILES;
    }
    else {
        m_traversesToShow = m_flux->m_traverseNum;
    }

    /* the list of evaluation logs */
    for (int i = 0; i < m_traversesToShow; ++i) {
        this->m_evalLogList.AddString(m_flux->m_traverse[i]->m_fileName.c_str());
    }
    m_evalLogList.SetSel(m_initalSelection, TRUE);

    InitBuffers();

    SetWidthOfLogList();

    DrawRouteGraph();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CRouteDlg::DrawRouteGraph() {
    double marginSpace;
    struct plotRange range;
    CString scaleStr, tmpStr;
    static double u[MAX_TRAVERSELENGTH], v[MAX_TRAVERSELENGTH];
    int i;

    GetPlotRange(range);

    /* Set the width of the margins around the plot */
    marginSpace = 1e-2 + 5e-2 * std::max(range.maxLon - range.minLon, range.maxLat - range.minLat);

    /* Fix the settings of the plot */
    m_gpsPlot.SetYUnits("Latitude");
    m_gpsPlot.SetXUnits("Longitude");
    m_gpsPlot.SetAxisEqual();
    m_gpsPlot.SetMarginSpace(marginSpace);
    m_gpsPlot.SetRange(range.minLon, range.maxLon, 3, range.minLat, range.maxLat, 3);

    /* draw the traverse */
    for (i = 0; i < m_traversesToShow; ++i) {
        if (!m_evalLogList.GetSel(i))
            continue;

        /** Draw the wind field, if wanted */
        if (m_showInterpolatedWindField.GetCheck() && m_useWindField[i]) {
            Common common;

            m_gpsPlot.SetPlotColor(m_windFieldColor);

            double avgWind = Average(m_ws[i], m_recordLen[i]);
            double scaling = 0.05 * (range.maxLat - range.minLat) / avgWind;

            for (int k = 0; k < m_recordLen[i]; ++k) {
                u[k] = scaling * m_ws[i][k] * sin(DEGREETORAD * m_wd[i][k]);
                v[k] = scaling * m_ws[i][k] * cos(DEGREETORAD * m_wd[i][k]);
            }
            m_gpsPlot.DrawVectorField(m_lon[i], m_lat[i], u, v, m_recordLen[i]);
        }

        m_gpsPlot.SetCircleRadius(m_pointSize);
        m_gpsPlot.SetCircleColor(m_circlesColor);

        // The options for the plot
        int plotOption = Graph::CGraphCtrl::PLOT_FIXED_AXIS;
        if (m_showColumnOption == 1)
            plotOption |= Graph::CGraphCtrl::PLOT_SCALE_CIRCLE;

        switch (m_selectedDisplay) {
        case SHOW_INTENSITY:
            m_gpsPlot.DrawCircles(m_lon[i], m_lat[i], m_int[i], m_recordLen[i], plotOption);
            break;
        case SHOW_ALTITUDE:
            m_gpsPlot.DrawCircles(m_lon[i], m_lat[i], m_alt[i], m_recordLen[i], plotOption);
            break;
        case SHOW_COLUMN:
            m_gpsPlot.DrawCircles(m_lon[i], m_lat[i], m_col[i], m_recordLen[i], plotOption);
            break;
        default:
            m_gpsPlot.DrawCircles(m_lon[i], m_lat[i], m_col[i], m_recordLen[i], plotOption);
        }

        /* draw the start of the traverse*/
        m_gpsPlot.SetCircleColor(RGB(0, 255, 0));
        m_gpsPlot.SetCircleRadius(6);
        m_gpsPlot.DrawCircles(m_lon[i], m_lat[i], 1, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
        m_gpsPlot.SetPlotColor(RGB(255, 255, 255), false);
    }

    /* draw the source */
    if (1 == m_showSourceCheck.GetCheck()) {
        m_gpsPlot.SetCircleColor(RGB(255, 0, 0));
        m_gpsPlot.SetCircleRadius(7);
        m_gpsPlot.DrawCircles(&m_srcLon, &m_srcLat, 1, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
        m_gpsPlot.SetPlotColor(RGB(255, 255, 255), false);
    }

    /* Draw the landmarks */
    for (i = 0; i < m_gridLandMarks.GetRowCount() - 1; ++i) {
        double lat, lon;
        tmpStr = m_gridLandMarks.GetCell(i + 1, 0)->GetText();
        if (0 >= sscanf(tmpStr, "%lf", &lat))
            continue;

        tmpStr = m_gridLandMarks.GetCell(i + 1, 1)->GetText();
        if (0 >= sscanf(tmpStr, "%lf", &lon))
            continue;

        m_gpsPlot.SetCircleColor(RGB(0, 0, 255));
        m_gpsPlot.DrawCircles(&lon, &lat, 1, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
    }

    /* Draw the scale of the plot */
    if (1 == m_showScale) {
        double	sLat[2], sLong[2]; // the latitudes and longitudes for the 'scale'
        double plotwidth = 1e-3 * mobiledoas::GPSDistance(m_gpsPlot.GetYMin(), m_gpsPlot.GetXMin(), m_gpsPlot.GetYMin(), m_gpsPlot.GetXMax());
        double scaleWidth = 1e3 * round(plotwidth / 4); // the length of the scale
        sLat[0] = sLat[1] = m_gpsPlot.GetYMin() + 0.1 * (m_gpsPlot.GetYMax() - m_gpsPlot.GetYMin());
        sLong[0] = m_gpsPlot.GetXMin() + 0.1 * (m_gpsPlot.GetXMax() - m_gpsPlot.GetXMin());
        mobiledoas::CalculateDestination(sLat[0], sLong[0], scaleWidth, 90, sLat[1], sLong[1]);
        // draw the distance line...
        m_gpsPlot.SetPlotColor(m_gpsPlot.GetGridColor());
        m_gpsPlot.DrawVector(sLong[0], sLat[0], sLong[1] - sLong[0], sLat[1] - sLat[0], false);

        // Print the number...
        scaleStr.Format("%.0lf km", scaleWidth * 1e-3);
        m_gpsPlot.DrawTextBox(sLong[0], sLong[1], sLat[0], sLat[0] + 0.03 * (m_gpsPlot.GetYMax() - m_gpsPlot.GetYMin()), scaleStr);
    }

}

LRESULT CRouteDlg::OnShowLatLong(WPARAM wParam, LPARAM lParam) {
    CString tmpStrLat, tmpStrLon;

    tmpStrLat.Format("Lat: %lf", m_gpsPlot.curLat);
    tmpStrLon.Format("Long: %lf", m_gpsPlot.curLong);

    this->SetDlgItemText(IDC_STATIC_LATITUDE, tmpStrLat);
    this->SetDlgItemText(IDC_STATIC_LONGITUDE, tmpStrLon);

    UpdateData(FALSE);
    return 0;
}

void CRouteDlg::OnBnClickedCheckShowSource()
{
    // TODO: Add your control notification handler code here
    char txt[256];

    GetDlgItemText(IDC_SRC_LAT_EDIT, txt, 256);
    sscanf(txt, "%lf", &m_srcLat);

    GetDlgItemText(IDC_SRC_LON_EDIT, txt, 256);
    sscanf(txt, "%lf", &m_srcLon);

    m_showSource = m_showSourceCheck.GetCheck();

    DrawRouteGraph();

}
void CRouteDlg::OnEnChangeSrcLatEdit()
{
    char txt[256];
    this->GetDlgItemText(IDC_SRC_LAT_EDIT, txt, 256);
    if (strlen(txt) != 0) {
        UpdateData(TRUE);
        if (1 == m_showSourceCheck.GetCheck())
            DrawRouteGraph();
    }
}

void CRouteDlg::OnEnChangeSrcLonEdit()
{
    char txt[256];
    this->GetDlgItemText(IDC_SRC_LON_EDIT, txt, 256);
    if (strlen(txt) != 0) {
        UpdateData(TRUE);
        if (1 == m_showSourceCheck.GetCheck())
            DrawRouteGraph();
    }
}

void CRouteDlg::OnBnClickedCheckCalcDistance() {
    if (1 == m_calcDistance.GetCheck()) {
        /* start calculating distance */
        this->SetDlgItemText(IDC_DISTANCE_LABEL, "Click on position 1");
        this->SetDlgItemText(IDC_BEARING_LABEL, "");
    }
    else {
        /* clear the distance calculations */

    }
    nClicks = 0;
    lat1 = lat2 = lon1 = lon2 = 0;
}

LRESULT CRouteDlg::OnLBUInGPSGraph(WPARAM wParam, LPARAM lParam) {
    CString tmpStr1, tmpStr2;
    double bearing, distance;

    if (1 == m_calcDistance.GetCheck()) {
        /* If we should calculate distance */

        if (nClicks == 0) {
            lat1 = m_gpsPlot.curLat;
            lon1 = m_gpsPlot.curLong;
            this->SetDlgItemText(IDC_DISTANCE_LABEL, "Click on position 2");
            this->SetDlgItemText(IDC_BEARING_LABEL, "");
        }

        if (nClicks == 1) {
            lat2 = m_gpsPlot.curLat;
            lon2 = m_gpsPlot.curLong;

            bearing = mobiledoas::GPSBearing(lat1, lon1, lat2, lon2);
            distance = mobiledoas::GPSDistance(lat1, lon1, lat2, lon2);
            if (distance < 1000)
                tmpStr1.Format("Dist.: %.0lf [m]", distance);
            else
                tmpStr1.Format("Dist.: %.2lf [km]", distance / 1000);

            tmpStr2.Format("Bearing: %.3lf [deg]", bearing);

            this->SetDlgItemText(IDC_BEARING_LABEL, tmpStr2);
            this->SetDlgItemText(IDC_DISTANCE_LABEL, tmpStr1);
            nClicks = -1;
        }

        ++nClicks;
    }

    return 0;
}

LRESULT CRouteDlg::OnZoomGPSGraph(WPARAM wparam, LPARAM lparam) {

    // Redraw the graph
    DrawRouteGraph();


    return 0;
}

void CRouteDlg::OnBnClickedShowColumn() {
    UpdateData(TRUE);
    DrawRouteGraph();
}

void CRouteDlg::OnBnClickedShowAltitude() {
    UpdateData(TRUE);
    DrawRouteGraph();
}

void CRouteDlg::OnBnClickedShowIntensity() {
    UpdateData(TRUE);
    DrawRouteGraph();
}

void CRouteDlg::GetPlotRange(struct plotRange& range) {
    CString tmpStr;
    double latRange, lonRange;
    Common common;
    int i;
    struct plotRange rect;

    // See if the user has determined any range...
    m_gpsPlot.GetZoomRect(rect);
    if (fabs(rect.maxLat) > 0.1) {
        range = rect;
        return;
    }

    range.maxLat = range.maxLon = -1e10;
    range.minLat = range.minLon = -1e10;
    for (i = 0; i < m_traversesToShow; ++i) {
        if (!m_evalLogList.GetSel(i))
            continue;

        if (range.maxLat < -180) {
            range.maxLat = m_lat[i][0];
            range.minLat = m_lat[i][0];
            range.maxLon = m_lon[i][0];
            range.minLon = m_lon[i][0];
        }

        if (m_flux->hasValidGPS()) {
            range.maxLat = std::max(MaxValue(m_lat[i], m_recordLen[i]), range.maxLat);
            range.maxLon = std::max(MaxValue(m_lon[i], m_recordLen[i]), range.maxLon);
            range.minLat = std::min(MinValue(m_lat[i], m_recordLen[i]), range.minLat);
            range.minLon = std::min(MinValue(m_lon[i], m_recordLen[i]), range.minLon);
        }
    }

    if (1 == this->m_showSourceCheck.GetCheck()) {
        /* show the source */

        if (range.maxLat < -180) {
            range.maxLat = m_srcLat;
            range.minLat = m_srcLat;
            range.maxLon = m_srcLon;
            range.minLon = m_srcLon;
        }
        range.minLat = std::min(range.minLat, m_srcLat);
        range.maxLat = std::max(range.maxLat, m_srcLat);
        range.minLon = std::min(range.minLon, m_srcLon);
        range.maxLon = std::max(range.maxLon, m_srcLon);
    }
    else {
        if (range.maxLat < -180) {
            range.maxLat = 1;
            range.minLat = -1;
            range.maxLon = 1;
            range.minLon = -1;
        }
    }

    // The landmarks...
    for (i = 0; i < m_gridLandMarks.GetRowCount() - 1; ++i) {
        double lat, lon;
        tmpStr = m_gridLandMarks.GetCell(i + 1, 0)->GetText();
        if (0 >= sscanf(tmpStr, "%lf", &lat))
            continue;

        tmpStr = m_gridLandMarks.GetCell(i + 1, 1)->GetText();
        if (0 >= sscanf(tmpStr, "%lf", &lon))
            continue;

        range.minLat = std::min(range.minLat, lat);
        range.maxLat = std::max(range.maxLat, lat);
        range.minLon = std::min(range.minLon, lon);
        range.maxLon = std::max(range.maxLon, lon);
    }

    /* make sure that the plot is square */
    latRange = range.maxLat - range.minLat;
    lonRange = range.maxLon - range.minLon;
    if (latRange > lonRange) {
        double half = (range.maxLon + range.minLon) / 2;
        range.minLon = half - latRange / 2;
        range.maxLon = half + latRange / 2;
    }
    else {
        double half = (range.maxLat + range.minLat) / 2;
        range.minLat = half - lonRange / 2;
        range.maxLat = half + lonRange / 2;
    }

    return;
}

void CRouteDlg::GetColorRange(double& minValue, double& maxValue) {
    int i;
    Common common;

    minValue = 0;
    maxValue = 1;

    for (i = 0; i < m_traversesToShow; ++i) {
        if (!m_evalLogList.GetSel(i))
            continue;

        switch (m_selectedDisplay) {
        case SHOW_INTENSITY:
            minValue = std::min(MinValue(m_int[i], m_recordLen[i]), minValue);
            maxValue = std::max(MaxValue(m_int[i], m_recordLen[i]), maxValue);
            break;
        case SHOW_ALTITUDE:
            minValue = std::min(MinValue(m_alt[i], m_recordLen[i]), minValue);
            maxValue = std::max(MaxValue(m_alt[i], m_recordLen[i]), maxValue);
            break;
        case SHOW_COLUMN:
            minValue = std::min(MinValue(m_col[i], m_recordLen[i]), minValue);
            maxValue = std::max(MaxValue(m_col[i], m_recordLen[i]), maxValue);
            break;
        default:
            minValue = std::min(MinValue(m_col[i], m_recordLen[i]), minValue);
            maxValue = std::max(MaxValue(m_col[i], m_recordLen[i]), maxValue);
        }
    }
}

void CRouteDlg::InitBuffers() {
    int i, j, k, sum;

    for (i = 0; i < m_traversesToShow; ++i) {
        mobiledoas::CTraverse* tr = m_flux->m_traverse[i];
        memcpy(m_lon[i], tr->longitude, tr->m_recordNum * sizeof(double));
        memcpy(m_lat[i], tr->latitude, tr->m_recordNum * sizeof(double));
        memcpy(m_alt[i], tr->altitude, tr->m_recordNum * sizeof(double));
        memcpy(m_col[i], tr->columnArray.data(), tr->m_recordNum * sizeof(double));
        memcpy(m_int[i], tr->intensArray, tr->m_recordNum * sizeof(double));
        memcpy(m_ws[i], tr->m_windSpeed, tr->m_recordNum * sizeof(double));
        memcpy(m_wd[i], tr->m_windDirection, tr->m_recordNum * sizeof(double));

        m_useWindField[i] = tr->m_useWindField;

        /* delete bad points */
        sum = tr->m_recordNum;
        for (j = 0; j < sum; ++j) {
            if (m_lat[i][j] == 0 && m_lon[i][j] == 0) {
                for (k = j; k < sum; ++k) {
                    m_lat[i][k] = m_lat[i][k + 1];
                    m_lon[i][k] = m_lon[i][k + 1];
                    m_ws[i][k] = m_ws[i][k + 1];
                    m_wd[i][k] = m_wd[i][k + 1];
                }
                --j;
                --sum;
            }
        }
        m_recordLen[i] = sum;
    }
}

void CRouteDlg::OnLbnSelchangeEvallogList() {
    DrawRouteGraph();
}


void CRouteDlg::OnBnClickedCheckShowInterpolatedWindfield() {
    DrawRouteGraph();
}

void CRouteDlg::OnBnClickedBtnImportWindfield()
{
    int nSelected = 0, curSel = -1;
    for (int i = 0; i < m_traversesToShow; ++i) {
        if (!m_evalLogList.GetSel(i))
            continue;
        ++nSelected;
        curSel = i;
    }
    if (nSelected > 1) {
        MessageBox("Can only import windfield for one traverse. Please select only one traverse and try again", "Error", MB_OK);
        return;
    }
    if (nSelected == 0) {
        MessageBox("Please select one traverse and try again", "Error", MB_OK);
        return;
    }

    // import windfield
    CWindFieldDlg dlg;
    dlg.m_flux = m_flux;

    INT_PTR ok = dlg.DoModal();

    // update the route dialog
    this->m_useWindField[curSel] = true;
    this->InitBuffers();
}

// Called when the user has pressed the spin button that controlls the
//  radius of the circles which make up the traverse
void CRouteDlg::OnDeltaposRoutedlgPointsizespin(NMHDR* pNMHDR, LRESULT* pResult)
{
    CString str;
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    // Check which direction the user has pressed the spinbutton (which is upside down)
    //  and change the circle radius accordingly
    if (pNMUpDown->iDelta > 0)
        --m_pointSize;
    if (pNMUpDown->iDelta < 0)
        ++m_pointSize;

    // Enforce the limits
    m_pointSize = std::max(1, m_pointSize);

    // Update the screen
    str.Format("%d", m_pointSize);
    m_pointSizeEdit.SetWindowText(str);

    *pResult = 0;
}

// Called when the number in the 'point size' edit has changed
void CRouteDlg::OnEnChangeRoutdlgPointsizeedit()
{
    DrawRouteGraph();
}

void CRouteDlg::OnChangeBackgroundColor()
{
    CColorDialog dlg;
    COLORREF bkColor;

    if (dlg.DoModal() == IDOK) {
        bkColor = dlg.m_cc.rgbResult;
        m_gpsPlot.SetBackgroundColor(bkColor);
    }
    DrawRouteGraph();
}

void CRouteDlg::OnChangeGridColor()
{
    CColorDialog dlg;
    COLORREF gColor;

    if (dlg.DoModal() == IDOK) {
        gColor = dlg.m_cc.rgbResult;
        m_gpsPlot.SetGridColor(gColor);
    }
    DrawRouteGraph();
}

void CRouteDlg::OnChangeWindFieldColor()
{
    CColorDialog dlg;

    if (dlg.DoModal() == IDOK) {
        m_windFieldColor = dlg.m_cc.rgbResult;
    }
    DrawRouteGraph();
}

void CRouteDlg::OnChangeCirclesColor()
{
    CColorDialog dlg;

    if (dlg.DoModal() == IDOK) {
        m_circlesColor = dlg.m_cc.rgbResult;
    }
    DrawRouteGraph();
}

void CRouteDlg::OnChangeColorScale_BW() {
    m_circlesColor = RGB(0, 0, 0);
    m_windFieldColor = RGB(10, 10, 10);

    m_gpsPlot.SetGridColor(RGB(0, 0, 0));
    m_gpsPlot.SetBackgroundColor(RGB(255, 255, 255));

    DrawRouteGraph();
}
void CRouteDlg::OnChangeColorScale_Default() {
    m_circlesColor = RGB(0, 255, 0);
    m_windFieldColor = RGB(255, 255, 255);

    m_gpsPlot.SetGridColor(RGB(255, 255, 255));
    m_gpsPlot.SetBackgroundColor(RGB(0, 0, 0));

    DrawRouteGraph();
}


void CRouteDlg::OnGridIncrease()
{
    //m_gpsPlot.IncreaseGridDensity();
    DrawRouteGraph();
}

void CRouteDlg::OnGridDecrease()
{
    //  m_gpsPlot.DecreaseGridDensity();
    DrawRouteGraph();
}

void CRouteDlg::OnFileSaveGraphAsImage() {
    CString fileName;

    // Get the fileName
    if (Common::BrowseForFile_SaveAs("*.png;*.bmp;*.gif;*.jpg", fileName)) {
        if (!Equals(fileName.Right(4), ".png") && !Equals(fileName.Right(4), ".bmp") && !Equals(fileName.Right(4), ".gif") && !Equals(fileName.Right(4), ".jpg"))
            fileName.AppendFormat(".png");

        m_gpsPlot.SaveGraph(fileName);
    }
}

void CRouteDlg::SetWidthOfLogList() {

    // Find the longest string in the list box.
    CString      str;
    CSize      sz;
    int					dx = 0;
    TEXTMETRIC   tm;
    CDC* pDC = m_evalLogList.GetDC();
    CFont* pFont = m_evalLogList.GetFont();

    // Select the listbox font, save the old font
    CFont* pOldFont = pDC->SelectObject(pFont);
    // Get the text metrics for avg char width
    pDC->GetTextMetrics(&tm);

    for (int i = 0; i < m_evalLogList.GetCount(); i++)
    {
        m_evalLogList.GetText(i, str);
        sz = pDC->GetTextExtent(str);

        // Add the avg width to prevent clipping
        sz.cx += tm.tmAveCharWidth;

        if (sz.cx > dx)
            dx = sz.cx;
    }
    // Select the old font back into the DC
    pDC->SelectObject(pOldFont);
    m_evalLogList.ReleaseDC(pDC);

    // Set the horizontal extent so every character of all strings 
    // can be scrolled to.
    m_evalLogList.SetHorizontalExtent(dx);
}

/** Turns on or off the showing of the scale */
void CRouteDlg::OnChangeShowScale() {

    // change the current status of the showing of the scale
    m_showScale = !m_showScale;


    DrawRouteGraph();
}

LRESULT CRouteDlg::OnEndEditLandMarkPosition(WPARAM wParam, LPARAM lParam) {
    DrawRouteGraph();

    return 0;
}
void CRouteDlg::OnBnClickedSourceLat()
{
    Dialogs::CSourceSelectionDlg sourceDlg;
    INT_PTR modal = sourceDlg.DoModal();
    if (IDOK == modal) {
        m_srcLat = sourceDlg.m_selectedLat;
        m_srcLon = sourceDlg.m_selectedLon;

        m_showSourceCheck.SetCheck(1);

        UpdateData(FALSE);

        DrawRouteGraph();
    }

    UpdateData(FALSE);
}

void CRouteDlg::OnBnClickedSourceLon()
{
    OnBnClickedSourceLat();
}


void CRouteDlg::OnMenuViewShowColumnByColor() {
    m_showColumnOption = 0;
    DrawRouteGraph(); // <-- update the graph
}
void CRouteDlg::OnMenuViewShowColumnBySize() {
    m_showColumnOption = 1;
    DrawRouteGraph(); // <-- update the graph
}
void CRouteDlg::OnUpdateMenuShowColumnByColor(CCmdUI* pCmdUI) {
    // set the check status of this item to the same as the check status of the 'intensity' check box
    if (m_showColumnOption == 0) {
        pCmdUI->SetCheck(1);
    }
    else {
        pCmdUI->SetCheck(0);
    }
}
void CRouteDlg::OnUpdateMenuShowColumnBySize(CCmdUI* pCmdUI) {
    // set the check status of this item to the same as the check status of the 'intensity' check box
    if (m_showColumnOption == 1) {
        pCmdUI->SetCheck(1);
    }
    else {
        pCmdUI->SetCheck(0);
    }
}

void CRouteDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
    ASSERT(pPopupMenu != nullptr);
    // Check the enabled state of various menu items.

    CCmdUI state;
    state.m_pMenu = pPopupMenu;
    ASSERT(state.m_pOther == nullptr);
    ASSERT(state.m_pParentMenu == nullptr);

    // Determine if menu is popup in top-level menu and set m_pOther to
    // it if so (m_pParentMenu == NULL indicates that it is secondary popup).
    HMENU hParentMenu;
    if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
        state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
    else if ((hParentMenu = ::GetMenu(m_hWnd)) != nullptr)
    {
        CWnd* pParent = this;
        // Child windows don't have menus--need to go to the top!
        if (pParent != nullptr &&
            (hParentMenu = ::GetMenu(pParent->m_hWnd)) != nullptr)
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
            while (state.m_nIndex < nCount && pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID) {
                state.m_nIndex++;
            }
        }
        state.m_nIndexMax = nCount;
    }
}
