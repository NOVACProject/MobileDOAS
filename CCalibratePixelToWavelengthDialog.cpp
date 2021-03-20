// CCalibratePixelToWavelengthDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CCalibratePixelToWavelengthDialog.h"
#include "afxdialogex.h"
#include "resource.h"
#include "Common.h"
#include "Calibration/WavelengthCalibrationController.h"

// CCalibratePixelToWavelengthDialog dialog

IMPLEMENT_DYNAMIC(CCalibratePixelToWavelengthDialog, CPropertyPage)

CCalibratePixelToWavelengthDialog::CCalibratePixelToWavelengthDialog(CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_CALIBRATE_WAVELENGTH_DIALOG)
    , m_inputSpectrumFile(_T(""))
    , m_solarSpectrumFile(_T(""))
    , m_initialCalibrationFile(_T(""))
    , m_instrumentLineshapeFile(_T(""))
    , m_darkSpectrumFile(_T(""))
{

}

CCalibratePixelToWavelengthDialog::~CCalibratePixelToWavelengthDialog()
{
}

void CCalibratePixelToWavelengthDialog::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_SPECTRUM, m_inputSpectrumFile);
    DDX_Text(pDX, IDC_EDIT_SOLAR_SPECTRUM, m_solarSpectrumFile);
    DDX_Text(pDX, IDC_EDIT_INITIAL_CALIBRATION, m_initialCalibrationFile);
    DDX_Text(pDX, IDC_EDIT_INITIAL_CALIBRATION2, m_instrumentLineshapeFile);
    DDX_Text(pDX, IDC_EDIT_SPECTRUM_DARK2, m_darkSpectrumFile);
}


BEGIN_MESSAGE_MAP(CCalibratePixelToWavelengthDialog, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SPECTRUM, &CCalibratePixelToWavelengthDialog::OnBnClickedButtonBrowseSpectrum)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SPECTRUM_DARK2, &CCalibratePixelToWavelengthDialog::OnBnClickedButtonBrowseSpectrumDark)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SOLAR_SPECTRUM, &CCalibratePixelToWavelengthDialog::OnClickedButtonBrowseSolarSpectrum)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_INITIAL_CALIBRATION2, &CCalibratePixelToWavelengthDialog::OnClickedButtonBrowseInitialCalibration)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_LINE_SHAPE, &CCalibratePixelToWavelengthDialog::OnBnClickedButtonBrowseLineShape)
    ON_BN_CLICKED(IDC_BUTTON_RUN, &CCalibratePixelToWavelengthDialog::OnClickedButtonRun)
END_MESSAGE_MAP()


// CCalibratePixelToWavelengthDialog message handlers


void CCalibratePixelToWavelengthDialog::OnBnClickedButtonBrowseSpectrum()
{
    if (!Common::BrowseForFile("Spectrum Files\0*.std;*.txt\0", this->m_inputSpectrumFile))
    {
        return;
    }
    UpdateData(FALSE);
}

void CCalibratePixelToWavelengthDialog::OnClickedButtonBrowseSolarSpectrum()
{
    if (!Common::BrowseForFile("Spectrum Files\0*.std;*.txt;*.xs\0", this->m_solarSpectrumFile))
    {
        return;
    }
    UpdateData(FALSE);
}

void CCalibratePixelToWavelengthDialog::OnClickedButtonBrowseInitialCalibration()
{
    if (!Common::BrowseForFile("Spectrum Files\0*.txt;*.xs\0", this->m_initialCalibrationFile))
    {
        return;
    }
    UpdateData(FALSE);
}

void CCalibratePixelToWavelengthDialog::OnBnClickedButtonBrowseSpectrumDark()
{
    if (!Common::BrowseForFile("Spectrum Files\0*.std;*.txt\0", this->m_darkSpectrumFile))
    {
        return;
    }
    UpdateData(FALSE);
}

void CCalibratePixelToWavelengthDialog::OnBnClickedButtonBrowseLineShape()
{
    if (!Common::BrowseForFile("Spectrum Files\0*.txt;*.xs\0", this->m_instrumentLineshapeFile))
    {
        return;
    }
    UpdateData(FALSE);
}


void CCalibratePixelToWavelengthDialog::OnClickedButtonRun()
{
    if (!IsExistingFile(m_inputSpectrumFile))
    {
        MessageBox("Please select a spectrum to calibrate", "Missing input", MB_OK);
        return;
    }
    if (!IsExistingFile(m_solarSpectrumFile))
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
    controller.m_inputSpectrumFile = this->m_inputSpectrumFile;
    controller.m_darkSpectrumFile = this->m_darkSpectrumFile;
    controller.m_solarSpectrumFile = this->m_solarSpectrumFile;
    controller.m_initialWavelengthCalibrationFile = this->m_initialCalibrationFile;
    controller.m_initialLineShapeFile = this->m_instrumentLineshapeFile;

    try
    {
        controller.RunCalibration();

    }
    catch (std::exception& e)
    {
        MessageBox(e.what(), "Failed to calibrate", MB_OK);
    }
}
