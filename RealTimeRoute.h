#pragma once
#ifndef _REALTIMEROUTE_
#define _REALTIMEROUTE_

#include "RouteGraph.h"
#include "afxwin.h"

class CSpectrometer;

// CRealTimeRoute dialog

namespace Dialogs
{
class CRealTimeRoute : public CDialog
{
    DECLARE_DYNAMIC(CRealTimeRoute)

public:
    CRealTimeRoute(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CRealTimeRoute();

    // Dialog Data
    enum { IDD = IDD_REALTIME_ROUTE_DLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    /* the plot */
    ::Graph::CRouteGraph   m_gpsPlot;

    DECLARE_MESSAGE_MAP()
public:

    bool fVisible;

    double m_intensityLimit; // points with intensity below 'm_intensityLimit' will not be shown

    CSpectrometer* m_spectrometer;

    virtual BOOL OnInitDialog();

    /* Plotting the data */
    void DrawRouteGraph();

    void ReadData();

    afx_msg void OnSize(UINT nType, int cx, int cy);

    virtual BOOL Create(UINT nID, CWnd* pParentWnd = nullptr);
    afx_msg void OnClose();

private:

    long  m_pointNum;
    std::vector<double>  m_lon;
    std::vector<double>  m_lat;

    std::vector<double> m_col;
    std::vector<double> m_int;

    struct plotRange m_range;
    double m_srcLat;
    double m_srcLon;

    CStatic m_plotArea;
    int m_legendWidth;

};
}
#endif
