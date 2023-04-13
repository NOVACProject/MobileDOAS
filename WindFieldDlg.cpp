// WindFieldDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DMSpec.h"
#include "WindFieldDlg.h"
#include "Common.h"

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

// CWindFieldDlg dialog

IMPLEMENT_DYNAMIC(CWindFieldDlg, CDialog)
CWindFieldDlg::CWindFieldDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CWindFieldDlg::IDD, pParent)
{
    m_flux = nullptr;
    m_windfile.Format("");
    m_windField = new mobiledoas::CWindField();
    m_useTimeShift = 0;
    m_timeShift = 0;
}

CWindFieldDlg::~CWindFieldDlg()
{
    m_flux = nullptr;
    if (m_windField != nullptr) {
        delete(m_windField);
        m_windField = nullptr;
    }
}

void CWindFieldDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ALTITUDE_COMBO, m_altitudeCombo);
    DDX_Control(pDX, IDC_WIND_GRAPH_FRAME, m_graphFrame);
    DDX_Control(pDX, IDC_HOUR_COMBO2, m_hourCombo);
    DDX_Control(pDX, IDC_PATH_WINDFIELDFILE, m_windFileCombo);

    DDX_Radio(pDX, IDC_RADIO_TIME1, m_useTimeShift);
    DDX_Text(pDX, IDC_EDIT_TIMESHIFT, m_timeShift);
}


BEGIN_MESSAGE_MAP(CWindFieldDlg, CDialog)
    ON_BN_CLICKED(IDC_BTN_BROWSE_WINDFIELDFILE, OnBrowseWindFieldFile)
    ON_CBN_SELCHANGE(IDC_ALTITUDE_COMBO, OnChangeAltitude)
    ON_CBN_SELCHANGE(IDC_HOUR_COMBO2, OnChangeHour)
    ON_CBN_SELCHANGE(IDC_PATH_WINDFIELDFILE, OnChangeWindFieldFile)
    ON_EN_CHANGE(IDC_PATH_WINDFIELDFILE, OnChangeWindFieldFile)
END_MESSAGE_MAP()


// CWindFieldDlg message handlers

void CWindFieldDlg::OnBrowseWindFieldFile()
{
    int index;

    CString fileName;
    if (SUCCESS == Common::BrowseForFile("*.txt", fileName)) {
        // look if this filename is already in the combo-box
        index = m_windFileCombo.FindString(-1, fileName);
        if (index > 0) {
            m_windFileCombo.SetCurSel(index);
        }
        else {
            m_windFileCombo.InsertString(0, fileName);
            m_windFileCombo.SetCurSel(0);
        }

        m_windfile.Format("%s", (LPCTSTR)fileName);
        m_windField->ReadWindField(fileName);
        FillCombos();
        DrawField();
    }
}

BOOL CWindFieldDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    // initialize the graph
    CRect rect;
    int margin = 2;
    m_graphFrame.GetWindowRect(&rect);
    rect.bottom -= rect.top + margin;
    rect.right -= rect.left + margin;
    rect.top = margin + 7;
    rect.left = margin;
    m_graph.Create(WS_VISIBLE | WS_CHILD, rect, &m_graphFrame);
    m_graph.SetYUnits("Latitude");
    m_graph.SetXUnits("Longitude");
    m_graph.SetBackgroundColor(RGB(0, 0, 0));
    m_graph.SetGridColor(RGB(255, 255, 255));
    m_graph.SetPlotColor(RGB(255, 0, 0));
    m_graph.SetRange(-10, 10, 1, -10.0, 10.0, 1);


    // Read the mobile log, to see which wind field file was used last time.
    ReadMobileLog();

    return TRUE;
}

void CWindFieldDlg::FillCombos() {
    FillCombo_Altitude();
    FillCombo_Hour();
}

void CWindFieldDlg::FillCombo_Altitude() {
    int nAlt = m_windField->GetLayerNum();
    CString str;

    // clear the combo box if necessary
    if (m_altitudeCombo.GetCount() > 0)
        m_altitudeCombo.ResetContent();

    // insert the layers as strings
    for (int i = 0; i < nAlt; ++i) {
        str.Format("%.2lf", m_windField->GetLayerAltitude(i));
        m_altitudeCombo.AddString(str);
    }

    // select the first layer
    if (nAlt > 0)
        m_altitudeCombo.SetCurSel(0);
}

void CWindFieldDlg::FillCombo_Hour() {
    CString str;

    // get the currently selected hour
    int curSel = m_hourCombo.GetCurSel();

    // clear the combo box if necessary
    if (m_hourCombo.GetCount() > 0)
        m_hourCombo.ResetContent();

    int nHours = m_windField->GetHours(0, m_hours);

    // insert the layers as strings
    for (int i = 0; i < nHours; ++i) {
        str.Format("%d", m_hours[i]);
        m_hourCombo.AddString(str);
    }

    // select the same hour as before
    if (curSel > 0)
        m_hourCombo.SetCurSel(curSel);
    else
        m_hourCombo.SetCurSel(0);

}

void CWindFieldDlg::DrawField() {
    static const int BUFSIZE = 2048;
    static double lat[BUFSIZE], lon[BUFSIZE], u[BUFSIZE], v[BUFSIZE], r[BUFSIZE], t[BUFSIZE];
    static int nDataPoints = 0;
    Common common;

    // get the currently selected altitude layer
    int curLayer = m_altitudeCombo.GetCurSel();

    // get the currently selected hour
    int curHour = m_hours[m_hourCombo.GetCurSel()];

    // get the wind field
    int nPoints = m_windField->GetWindField(curLayer, curHour, MAX_SPECTRUM_LENGTH, lat, lon, r, t);

    // get the ranges for the data
    double minLat = MinValue(lat, nPoints);
    double maxLat = MaxValue(lat, nPoints);
    double minLon = MinValue(lon, nPoints);
    double maxLon = MaxValue(lon, nPoints);
    double marginSpace = 0.05 * max(maxLon - minLon, maxLat - minLat);
    m_graph.SetRange(minLon - marginSpace, maxLon + marginSpace, 3, minLat - marginSpace, maxLat + marginSpace, 3);

    // get the scaling factor for the data
    double avgWind = Average(r, nPoints);
    double scaling = 0.05 * (maxLat - minLat) / avgWind;

    // change the field from (radius, angle) to (x-component, y-component)
    for (int i = 0; i < nPoints; ++i) {
        u[i] = scaling * r[i] * sin(DEGREETORAD * t[i]);
        v[i] = scaling * r[i] * cos(DEGREETORAD * t[i]);
    }

    // draw the wind field
    m_graph.DrawVectorField(lon, lat, u, v, nPoints);

}

void CWindFieldDlg::OnChangeAltitude()
{
    FillCombo_Hour();

    DrawField();
}

void CWindFieldDlg::OnChangeHour()
{
    DrawField();
}

void CWindFieldDlg::OnOK()
{
    UpdateData(TRUE); // <-- Save the data in the dialog

    if (m_flux != nullptr) {
        int layer = m_altitudeCombo.GetCurSel();
        if (m_flux->m_windField != nullptr)
            delete(m_flux->m_windField);

        m_flux->m_useWindField = true;
        m_flux->m_windField = new mobiledoas::CWindField(*m_windField);
        if (m_useTimeShift) {
            m_flux->m_windField->UseTimeShift(true);
            m_flux->m_windField->SetTimeShift(m_timeShift);
        }
        m_flux->InterpolateWindField(layer);
    }

    UpdateMobileLog();

    CDialog::OnOK();
}

void CWindFieldDlg::ReadMobileLog() {
    char windfieldfile[512];
    char txt[256];
    char* pt = 0;

    FILE* f = fopen(g_exePath + "MobileLog.txt", "r");
    if (0 != f) {
        while (fgets(txt, sizeof(txt) - 1, f)) {

            if (pt = strstr(txt, "WINDFIELDFILE=")) {
                /* Find the last windfieldfile used */
                pt = strstr(txt, "=");
                sprintf(windfieldfile, "%s", &pt[1]);
                if (pt = strstr(windfieldfile, "\n"))
                    pt[0] = 0; // remove new-line characters
                m_windfile.Format("%s", windfieldfile);
                m_windFileCombo.AddString(m_windfile);
            }
        }
    }
    else {
        return;
    }

    fclose(f);
}

void CWindFieldDlg::UpdateMobileLog() {
    char txt[256];

    if (strlen(m_windfile) <= 0)
        return;

    /* Open the mobile log file */
    FILE* f = fopen(g_exePath + "MobileLog.txt", "r");

    if (0 == f) {
        /* File might not exist */
        f = fopen(g_exePath + "MobileLog.txt", "w");
        if (0 == f) {
            /* File cannot be opened */
            return;
        }
        else {
            fprintf(f, "WINDFIELDFILE=%s", (LPCTSTR)m_windfile);
            fclose(f);
            return;
        }
    }
    else {
        CString tmpStr;
        /* read in all the funny strings we dont understand */
        while (fgets(txt, sizeof(txt) - 1, f)) {
            if ((0 == strstr(txt, "WINDFIELDFILE="))) {
                tmpStr.AppendFormat("%s", txt);
            }
        }
        fclose(f);
        f = fopen(g_exePath + "MobileLog.txt", "w");
        fprintf(f, "%s", (LPCTSTR)tmpStr);
        for (int i = 0; i < min(4, m_windFileCombo.GetCount()); ++i) {
            m_windFileCombo.GetLBText(i, tmpStr);
            fprintf(f, "WINDFIELDFILE=%s\n", (LPCTSTR)tmpStr);
        }

        fclose(f);
    }
}

void CWindFieldDlg::OnChangeWindFieldFile() {
    CString str;
    int curSel = m_windFileCombo.GetCurSel();
    if (curSel < 0)
        return;
    // get the string...
    m_windFileCombo.GetLBText(curSel, str);

    // test if the file can be read
    FILE* f = fopen(str, "r");
    if (0 != f) {
        // Set the wind file
        m_windfile.Format("%s", (LPCTSTR)str);

        m_windField->ReadWindField(m_windfile);
        FillCombos();
        DrawField();
    }

    fclose(f);
}