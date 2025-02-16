#pragma once

#include "afxwin.h"
#include "Graphs/GraphCtrl.h"

namespace Graph
{

#define WM_SHOW_LATLONG           WM_USER +18
#define WM_LBU_IN_GPSPLOT         WM_USER +19
#define WM_LBD_IN_GPSPLOT         WM_USER +20
#define WM_ZOOM_IN_GPSPLOT        WM_USER +21

class CRouteGraph : public CGraphCtrl
{

public:
    CRouteGraph();
    ~CRouteGraph();

    /** the size of this graph */
    CRect rect;

    /** handle to the parent window, for message handling */
    CWnd* parentWnd;

    /* current lat and long */
    double curLat, curLong;

    /** The lat and long where the left mouse button was pressed
            down the last time. */
    double lbdLat, lbdLong;

    /** true if the user is right now trying to zoom into the graph */
    bool m_zooming;

    /** The coordinates (lat & long) into which the user wants to zoom. */
    struct plotRange m_zoomRect;

    /** A DC to draw something on... */
    CDC     m_dcRoute;
    CBitmap* m_pbitmapOldRoute;
    CBitmap m_bitmapRoute;

    DECLARE_MESSAGE_MAP()

    /** Redraws the graph */
    afx_msg	void OnPaint();

    /** Draws a shaded rectangle between with the two given points
            as corners */
    void DrawShadedRect(double lat1, double lon1, double lat2, double lon2);

public:
    /** Called when the user moves the mouse over the graph */
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);

    /* distance calculations */
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

    /* distance calculations */
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

    /* zooming out */
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

    /// ---------------------- OVERLOADED FUNCTIONS ----------------------
/** Creates the graph */
    virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = NULL);

    // ------------------ Communication ------------------------

    /** Gets the coordinate values that the user wants to zoom into.
            If the user does not want to zoom, the values in 'rect' will be zero. */
    void GetZoomRect(struct plotRange& range);
};
}
