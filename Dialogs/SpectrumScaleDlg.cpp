// SpectrumScaleDlg.cpp : implementation file
//

#undef min
#undef max

#include "stdafx.h"
#include "../DMSpec.h"
#include "SpectrumScaleDlg.h"
#include "../Spectrometer.h"
#include <algorithm>

using namespace Dialogs;

// CSpectrumScaleDlg dialog

IMPLEMENT_DYNAMIC(CSpectrumScaleDlg, CDialog)
CSpectrumScaleDlg::CSpectrumScaleDlg(CWnd* pParent)
    : CDialog(CSpectrumScaleDlg::IDD, pParent)
    , m_minValue(0)
    , m_maxValue(100)
{
}

CSpectrumScaleDlg::~CSpectrumScaleDlg()
{
}

void CSpectrumScaleDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_SPECSCALE_MIN, m_editMin);
    DDX_Control(pDX, IDC_EDIT_SPECSCALE_MAX, m_editMax);

    DDX_Text(pDX, IDC_EDIT_SPECSCALE_MIN, m_minValue);
    DDX_Text(pDX, IDC_EDIT_SPECSCALE_MAX, m_maxValue);

    DDX_Control(pDX, IDC_SPIN_MINVALUE, m_spinMin);
    DDX_Control(pDX, IDC_SPIN_MAXVALUE, m_spinMax);
}


BEGIN_MESSAGE_MAP(CSpectrumScaleDlg, CDialog)
    ON_EN_CHANGE(IDC_EDIT_SPECSCALE_MIN, OnChangeMin)
    ON_EN_CHANGE(IDC_EDIT_SPECSCALE_MAX, OnChangeMax)

    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MINVALUE, OnChangeSpinMinvalue)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MAXVALUE, OnChangeSpinMaxvalue)
END_MESSAGE_MAP()


// CSpectrumScaleDlg message handlers

void CSpectrumScaleDlg::OnChangeMin()
{
    UpdateData(TRUE);

    // make sure the view updates...
    if (this->m_mainForm != nullptr)
    {
        this->m_mainForm->PostMessage(WM_CHANGEDSPECSCALE, m_minValue, m_maxValue);
    }
}

void CSpectrumScaleDlg::OnChangeMax()
{
    UpdateData(TRUE);

    // make sure the view updates...
    if (this->m_mainForm != nullptr)
    {
        this->m_mainForm->PostMessage(WM_CHANGEDSPECSCALE, m_minValue, m_maxValue);
    }
}

void CSpectrumScaleDlg::OnChangeSpinMinvalue(NMHDR* pNMHDR, LRESULT* pResult)
{
    CString str;
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    int oldValue = m_minValue;

    // Check which direction the user has pressed the spinbutton (which is upside down)
    //  and change the circle radius accordingly
    if (pNMUpDown->iDelta > 0)
        --m_minValue;
    if (pNMUpDown->iDelta < 0)
        ++m_minValue;

    // Enforce the limits
    m_minValue = std::max(0, m_minValue);
    m_minValue = std::min(m_maxValue - 1, m_minValue);

    *pResult = 0;

    // only update the screen if anything has happened
    if (m_minValue != oldValue)
    {
        // Update the screen
        str.Format("%d", m_minValue);
        m_editMin.SetWindowText(str);
    }
}

void CSpectrumScaleDlg::OnChangeSpinMaxvalue(NMHDR* pNMHDR, LRESULT* pResult)
{
    CString str;
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    int oldValue = m_maxValue;

    // Check which direction the user has pressed the spinbutton (which is upside down)
    //  and change the circle radius accordingly
    if (pNMUpDown->iDelta > 0)
        --m_maxValue;
    if (pNMUpDown->iDelta < 0)
        ++m_maxValue;

    // Enforce the limits
    m_maxValue = std::max(m_minValue + 1, m_maxValue);
    m_maxValue = std::min(100, m_maxValue);

    *pResult = 0;

    // only update the screen if anything has happened
    if (m_maxValue != oldValue)
    {
        // Update the screen
        str.Format("%d", m_maxValue);
        m_editMax.SetWindowText(str);
    }
}

BOOL CSpectrumScaleDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_spinMin.SetBuddy(&m_editMin);
    m_spinMax.SetBuddy(&m_editMax);


    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
