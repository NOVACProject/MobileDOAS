// CCalibrationWavelength.cpp : implementation file
//

#include "stdafx.h"
#include "CCalibrationWavelength.h"
#include "afxdialogex.h"
#include "resource.h"
#include "Common.h"


// CCalibrationWavelength dialog

IMPLEMENT_DYNAMIC(CCalibrationWavelength, CPropertyPage)

CCalibrationWavelength::CCalibrationWavelength(CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_CALIBRATE_WAVELENGTH_DIALOG)
    , m_inputSpectrum(_T(""))
    , m_solarSpectrum(_T(""))
{

}

CCalibrationWavelength::~CCalibrationWavelength()
{
}

void CCalibrationWavelength::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_SPECTRUM, m_inputSpectrum);
    DDX_Text(pDX, IDC_EDIT_SOLAR_SPECTRUM, m_solarSpectrum);
}


BEGIN_MESSAGE_MAP(CCalibrationWavelength, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SPECTRUM, &CCalibrationWavelength::OnBnClickedButtonBrowseSpectrum)
    ON_BN_CLICKED(IDC_BUTTON_RUN, &CCalibrationWavelength::OnClickedButtonRun)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SOLAR_SPECTRUM, &CCalibrationWavelength::OnClickedButtonBrowseSolarSpectrum)
END_MESSAGE_MAP()


// CCalibrationWavelength message handlers


void CCalibrationWavelength::OnBnClickedButtonBrowseSpectrum()
{
    Common common;
    if (!common.BrowseForFile("Spectrum Files\0*.std\0", m_inputSpectrum))
    {
        return;
    }
    UpdateData(FALSE);
}

void CCalibrationWavelength::OnClickedButtonBrowseSolarSpectrum()
{
    Common common;
    if (!common.BrowseForFile("Spectrum Files\0*.std;*.txt;*.xs\0", m_solarSpectrum))
    {
        return;
    }
    UpdateData(FALSE);
}


void CCalibrationWavelength::OnClickedButtonRun()
{
    // TODO: Add your control notification handler code here
}

