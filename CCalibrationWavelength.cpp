// CCalibrationWavelength.cpp : implementation file
//

#include "stdafx.h"
#include "CCalibrationWavelength.h"
#include "afxdialogex.h"
#include "resource.h"
#include "Common.h"
#include "Calibration/WavelengthCalibrationController.h"

// CCalibrationWavelength dialog

IMPLEMENT_DYNAMIC(CCalibrationWavelength, CPropertyPage)

CCalibrationWavelength::CCalibrationWavelength(CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_CALIBRATE_WAVELENGTH_DIALOG)
    , m_inputSpectrum(_T(""))
    , m_solarSpectrum(_T(""))
    , m_initialCalibrationFile(_T(""))
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
    DDX_Text(pDX, IDC_EDIT_INITIAL_CALIBRATION, m_initialCalibrationFile);
}


BEGIN_MESSAGE_MAP(CCalibrationWavelength, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SPECTRUM, &CCalibrationWavelength::OnBnClickedButtonBrowseSpectrum)
    ON_BN_CLICKED(IDC_BUTTON_RUN, &CCalibrationWavelength::OnClickedButtonRun)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SOLAR_SPECTRUM, &CCalibrationWavelength::OnClickedButtonBrowseSolarSpectrum)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_INITIAL_CALIBRATION, &CCalibrationWavelength::OnClickedButtonBrowseInitialCalibration)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_INITIAL_CALIBRATION, &CCalibrationWavelength::OnClickedButtonBrowseInitialCalibration)
END_MESSAGE_MAP()


// CCalibrationWavelength message handlers


void CCalibrationWavelength::OnBnClickedButtonBrowseSpectrum()
{
    if (!Common::BrowseForFile("Spectrum Files\0*.std\0", m_inputSpectrum))
    {
        return;
    }
    UpdateData(FALSE);
}

void CCalibrationWavelength::OnClickedButtonBrowseSolarSpectrum()
{
    if (!Common::BrowseForFile("Spectrum Files\0*.std;*.txt;*.xs\0", m_solarSpectrum))
    {
        return;
    }
    UpdateData(FALSE);
}

void CCalibrationWavelength::OnClickedButtonBrowseInitialCalibration()
{
    if (!Common::BrowseForFile("Spectrum Files\0*.txt;*.xs\0", m_initialCalibrationFile))
    {
        return;
    }
    UpdateData(FALSE);
}


void CCalibrationWavelength::OnClickedButtonRun()
{
    if (!IsExistingFile(m_inputSpectrum))
    {
        MessageBox("Please select a spectrum to calibrate", "Missing input", MB_OK);
        return;
    }
    if (!IsExistingFile(m_solarSpectrum))
    {
        MessageBox("Please select a high resolved solar spectrum to use in the calibration", "Missing input", MB_OK);
        return;
    }
    if (!IsExistingFile(m_initialCalibrationFile))
    {
        MessageBox("Please select a file which contains an initial guess for the wavelength calibration of the spectrometer", "Missing input", MB_OK);
        return;
    }

    WavelengthCalibrationController controller;
    controller.m_solarSpectrum = m_solarSpectrum;
    controller.m_inputSpectrum = m_inputSpectrum;
    controller.m_initialWavelengthCalibration = m_initialCalibrationFile;

    try
    {
        controller.RunCalibration();

    }
    catch (std::exception& e)
    {
        MessageBox(e.what(), "Failed to calibrate", MB_OK);
    }
}
