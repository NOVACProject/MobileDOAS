#include "stdafx.h"
#include "RouteGraph.h"

using namespace Graph;

CRouteGraph::CRouteGraph()
{
    parentWnd = 0;
    m_zooming = false;
    m_zoomRect.minLon = 0.0;
    m_zoomRect.maxLon = 0.0;
    m_zoomRect.minLat = 0.0;
    m_zoomRect.maxLat = 0.0;
}

CRouteGraph::~CRouteGraph()
{
}

BEGIN_MESSAGE_MAP(CRouteGraph, CGraphCtrl)
    ON_WM_MOUSEMOVE()
    ON_WM_PAINT()
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

void CRouteGraph::OnMouseMove(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default

    CGraphCtrl::OnMouseMove(nFlags, point);

    double alpha = (point.x - m_rectPlot.left) / (double)(m_rectPlot.right - m_rectPlot.left);

    curLong = alpha * m_axisOptions.first.right + (1 - alpha) * m_axisOptions.first.left;

    alpha = (point.y - m_rectPlot.top) / (double)(m_rectPlot.bottom - m_rectPlot.top);
    curLat = alpha * m_axisOptions.first.bottom + (1 - alpha) * m_axisOptions.first.top;

    parentWnd->PostMessage(WM_SHOW_LATLONG);

    // If the user is dragging with the left mouse button pressed
    if (nFlags & MK_LBUTTON)
    {
        if (!m_zooming)
            m_zooming = true;

        // in case we haven't established the memory dc's
        CBitmap memBitmap;

        // Copy the m_dcRoute to m_dcPlot!
        m_dcPlot.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, &m_dcRoute, 0, 0, SRCCOPY);

        DrawShadedRect(lbdLat, lbdLong, curLat, curLong);
    }
}

void CRouteGraph::OnLButtonUp(UINT nFlags, CPoint point)
{
    CGraphCtrl::OnLButtonUp(nFlags, point);

    double alpha = (point.x - m_rectPlot.left) / (double)(m_rectPlot.right - m_rectPlot.left);

    curLong = alpha * m_axisOptions.first.right + (1 - alpha) * m_axisOptions.first.left;

    alpha = (point.y - m_rectPlot.top) / (double)(m_rectPlot.bottom - m_rectPlot.top);
    curLat = alpha * m_axisOptions.first.bottom + (1 - alpha) * m_axisOptions.first.top;

    if (m_zooming)
    {
        // Remove any previously drawn squares...
        m_dcPlot.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, &m_dcRoute, 0, 0, SRCCOPY);
        Invalidate(); // <-- Redraw everything
        m_zooming = false;

        m_zoomRect.minLon = min(curLong, lbdLong);
        m_zoomRect.maxLon = max(curLong, lbdLong);
        m_zoomRect.minLat = min(curLat, lbdLat);
        m_zoomRect.maxLat = max(curLat, lbdLat);

        parentWnd->PostMessage(WM_ZOOM_IN_GPSPLOT);
    }
    else
    {
        parentWnd->PostMessage(WM_LBU_IN_GPSPLOT);
    }

}

void CRouteGraph::OnLButtonDown(UINT nFlags, CPoint point)
{
    CGraphCtrl::OnLButtonDown(nFlags, point);

    double alpha = (point.x - m_rectPlot.left) / (double)(m_rectPlot.right - m_rectPlot.left);

    curLong = alpha * m_axisOptions.first.right + (1 - alpha) * m_axisOptions.first.left;

    alpha = (point.y - m_rectPlot.top) / (double)(m_rectPlot.bottom - m_rectPlot.top);
    curLat = alpha * m_axisOptions.first.bottom + (1 - alpha) * m_axisOptions.first.top;

    // Remember where the left mouse button was pressed the last time
    lbdLat = curLat;
    lbdLong = curLong;

    // if we don't have one yet, set up a memory dc for the plot
    if (m_dcRoute.GetSafeHdc() == nullptr)
    {
        CClientDC dc(this);
        m_dcRoute.CreateCompatibleDC(&dc);
        m_bitmapRoute.CreateCompatibleBitmap(&dc, m_nClientWidth, m_nClientHeight);
        m_pbitmapOldRoute = m_dcRoute.SelectObject(&m_bitmapRoute);
    }
    // Copy the m_dcPlot to m_dcRoute!
    m_dcRoute.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, &m_dcPlot, 0, 0, SRCCOPY);

    parentWnd->PostMessage(WM_LBD_IN_GPSPLOT);
}

void CRouteGraph::OnRButtonDown(UINT nFlags, CPoint point)
{
    CGraphCtrl::OnRButtonDown(nFlags, point);

    // zoom back to default values
    m_zooming = false;

    m_zoomRect.minLon = 0.0;
    m_zoomRect.maxLon = 0.0;
    m_zoomRect.minLat = 0.0;
    m_zoomRect.maxLat = 0.0;

    parentWnd->PostMessage(WM_ZOOM_IN_GPSPLOT);

}

BOOL CRouteGraph::Create(DWORD dwStyle, const RECT& rect,
    CWnd* pParentWnd, UINT nID)
{
    BOOL result;
    static CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);

    result = CWnd::CreateEx(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE,
        className, NULL, dwStyle,
        rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
        pParentWnd->GetSafeHwnd(), (HMENU)nID);

    if (result != 0)
        InvalidateCtrl();

    return result;

} // Create


void CRouteGraph::OnPaint()
{
    CGraphCtrl::OnPaint();
}

/** Draws a shaded rectangle between with the two given points
        as corners */
void CRouteGraph::DrawShadedRect(double lat1, double lon1, double lat2, double lon2)
{
    double x[5], y[5];
    double maxX, minX, maxY, minY;
    double xFactor, yFactor, offsLeft, offsBottom;
    int curX, curY, prevX, prevY;
    double left = (double)m_rectPlot.left;
    double bottom = (double)m_rectPlot.bottom;
    AxisOptions::FloatRect curAxis; // The current axis (either first or second axis)

    // Copy the current pen
    LOGPEN logpen;
    m_penPlot.GetLogPen(&logpen);

    // Get the opposite of the grid color
    BYTE R = (BYTE)255 - GetRValue(m_colors.background);
    BYTE G = (BYTE)255 - GetGValue(m_colors.background);
    BYTE B = (BYTE)255 - GetBValue(m_colors.background);
    COLORREF complement = RGB(R, G, B);

    // Make a new pen with this color
    m_penPlot.DeleteObject();
    m_penPlot.CreatePen(PS_DOT, 1, complement);

    // Draw the rectangle
    x[4] = x[0] = x[1] = lon1;
    x[2] = x[3] = lon2;
    y[4] = y[0] = y[3] = lat1;
    y[1] = y[2] = lat2;

    // make sure there's a memory dc for the plot
    if (m_dcRoute.GetSafeHdc() == nullptr)
        return;

    GetDataRange(x, y, 5, maxX, minX, maxY, minY);

    // Get the current axis
    curAxis = m_axisOptions.first;

    // ------------ CALCULATE THE TRANSFORM FROM DATA POINT TO PIXELS ---------------
    GetTransform(offsLeft, offsBottom, xFactor, yFactor, curAxis);

    m_dcPlot.SelectObject(&m_penPlot);

    // The starting point
    prevX = (int)(left + (x[0] - offsLeft) * xFactor);
    prevY = (int)(bottom - (y[0] - offsBottom) * yFactor);

    for (int i = 0; i < 5; ++i)
    {
        // Calculate the next point...
        curX = (int)(left + xFactor * (x[i] - offsLeft));
        curY = (int)(bottom - (y[i] - offsBottom) * yFactor);

        // Draw connected lines
        m_dcPlot.MoveTo(prevX, prevY);
        m_dcPlot.LineTo(curX, curY);

        prevX = curX;
        prevY = curY;
    }

    // Restore the pen
    m_penPlot.DeleteObject();
    m_penPlot.CreatePenIndirect(&logpen);

    FinishPlot();

    // Invalidate the active region
    // TODO: Use InvalidateRect instead!!
    Invalidate();

}

/** Gets the coordinate values that the user wants to zoom into.
        If the user does not want to zoom, the values in 'rect' will be zero. */
void CRouteGraph::GetZoomRect(struct plotRange& range)
{
    if (m_zoomRect.minLat == m_zoomRect.maxLat && m_zoomRect.minLat == 0)
    {
        range.maxLat = range.maxLon = range.minLat = range.minLon = 0.0;
        return;
    }
    else
    {
        range.maxLat = m_zoomRect.maxLat;
        range.maxLon = m_zoomRect.maxLon;
        range.minLat = m_zoomRect.minLat;
        range.minLon = m_zoomRect.minLon;
        return;
    }
}
