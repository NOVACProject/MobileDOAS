// CCalibratePixelToWavelengthSetupDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CCalibratePixelToWavelengthSetupDialog.h"
#include "CCalibratePixelToWavelengthDialog.h"
#include "Common.h"
#include "afxdialogex.h"
#include "resource.h"


// CCalibratePixelToWavelengthSetupDialog dialog

IMPLEMENT_DYNAMIC(CCalibratePixelToWavelengthSetupDialog, CDialog)

CCalibratePixelToWavelengthSetupDialog::CCalibratePixelToWavelengthSetupDialog(CalibratePixelToWavelengthDialogSetup* setup, CWnd* pParent /*=nullptr*/)
    : m_setup(setup),
    CDialog(IDD_CALIBRATE_WAVELENGTH_SETTINGS_DIALOG, pParent)
{
}

CCalibratePixelToWavelengthSetupDialog::~CCalibratePixelToWavelengthSetupDialog()
{
    m_setup = nullptr;
}

BOOL CCalibratePixelToWavelengthSetupDialog::OnInitDialog() {
    CDialog::OnInitDialog();

    UpdateData(FALSE);

    OnBnClickedRadioInstrumentLineShapeFitOption();

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CCalibratePixelToWavelengthSetupDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_INITIAL_CALIBRATION_SETTING, m_setup->m_initialCalibrationFile);
    DDX_Text(pDX, IDC_EDIT_SOLAR_SPECTRUM_SETTING, m_setup->m_solarSpectrumFile);
    DDX_Control(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_LOW, m_fitRegionEditLow);
    DDX_Control(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_HIGH, m_fitRegionEditHigh);
    DDX_Text(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_LOW, m_setup->m_fitInstrumentLineShapeRegionStart);
    DDX_Text(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_HIGH, m_setup->m_fitInstrumentLineShapeRegionStop);
    DDX_Radio(pDX, IDC_RADIO_INSTRUMENT_LINE_SHAPE_FIT_NOTHING, m_setup->m_fitInstrumentLineShapeOption);
}

BEGIN_MESSAGE_MAP(CCalibratePixelToWavelengthSetupDialog, CDialog)
    ON_BN_CLICKED(IDC_RADIO_INSTRUMENT_LINE_SHAPE_FIT_NOTHING, &CCalibratePixelToWavelengthSetupDialog::OnBnClickedRadioInstrumentLineShapeFitOption)
    ON_BN_CLICKED(IDC_RADIO_INSTRUMENT_LINE_SHAPE_FIT_SUPER_GAUSS, &CCalibratePixelToWavelengthSetupDialog::OnBnClickedRadioInstrumentLineShapeFitOption)
    ON_BN_CLICKED(IDOK, &CCalibratePixelToWavelengthSetupDialog::OnBnClickedOk)
END_MESSAGE_MAP()

// CCalibratePixelToWavelengthSetupDialog message handlers

void CCalibratePixelToWavelengthSetupDialog::OnBnClickedRadioInstrumentLineShapeFitOption()
{
    UpdateData(TRUE); // get the selections from the user interface

    BOOL enableRegion = m_setup->m_fitInstrumentLineShapeOption == 1;
    m_fitRegionEditLow.EnableWindow(enableRegion);
    m_fitRegionEditHigh.EnableWindow(enableRegion);
}

void CCalibratePixelToWavelengthSetupDialog::OnBnClickedOk()
{
    // Validate the input data..
    if (!IsExistingFile(m_setup->m_solarSpectrumFile))
    {
        MessageBox("Please select a high resolved solar spectrum to use in the calibration", "Missing input", MB_OK);
        return;
    }
    if (!IsExistingFile(m_setup->m_initialCalibrationFile))
    {
        MessageBox("Please select a file which contains an initial guess for the wavelength calibration of the spectrometer", "Missing input", MB_OK);
        return;
    }
    if (m_setup->m_fitInstrumentLineShapeOption == 1)
    {
        // If we are to fit an instrument line shape, check the region.
        double fitRegionStart = std::atof(m_setup->m_fitInstrumentLineShapeRegionStart);
        double fitRegionStop = std::atof(m_setup->m_fitInstrumentLineShapeRegionStart);

        if (std::abs(fitRegionStart) < std::numeric_limits<float>::epsilon())
        {
            MessageBox("Failed to interpret the fit from as a numeric value. Please check the value and try again.", "Cannot parse input", MB_OK);
            return;
        }
        if (std::abs(fitRegionStop) < std::numeric_limits<float>::epsilon())
        {
            MessageBox("Failed to interpret the fit to as a numeric value. Please check the value and try again.", "Cannot parse input", MB_OK);
            return;
        }
        if (fitRegionStart > fitRegionStop)
        {
            MessageBox("The 'fit from' is larger than 'fit to'. Please make sure the values are correct and try again.", "Cannot parse input", MB_OK);
            return;
        }
    }

    CDialog::OnOK();
}
