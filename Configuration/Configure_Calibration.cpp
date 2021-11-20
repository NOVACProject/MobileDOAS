#include "stdafx.h"
#include "../DMSpec.h"
#include "Configure_Calibration.h"
#include "../OpenInstrumentCalibrationDialog.h"
#include <SpectralEvaluation/Calibration/StandardCrossSectionSetup.h>

// CConfigure_Calibration dialog

using namespace Configuration;

IMPLEMENT_DYNAMIC(CConfigure_Calibration, CPropertyPage)

CConfigure_Calibration::CConfigure_Calibration(CWnd* pParent /*=nullptr*/)
    : CPropertyPage(CConfigure_Calibration::IDD)
{
}

CConfigure_Calibration::~CConfigure_Calibration()
{
}

void CConfigure_Calibration::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);

    DDX_Radio(pDX, IDC_RADIO_INSTRUMENT_LINE_SHAPE_FIT_NOTHING, m_conf->m_calibration.m_instrumentLineShapeFitOption);
    DDX_Text(pDX, IDC_EDIT_SOLAR_SPECTRUM_SETTING, m_conf->m_calibration.m_solarSpectrumFile);
    DDX_Text(pDX, IDC_EDIT_INITIAL_CALIBRATION_SETTING, m_conf->m_calibration.m_initialCalibrationFile);
    DDX_Text(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_LOW, m_conf->m_calibration.m_instrumentLineShapeFitRegion.low);
    DDX_Text(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_HIGH, m_conf->m_calibration.m_instrumentLineShapeFitRegion.high);

    DDX_Check(pDX, IDC_CHECK_REPLACE_USER_DEFINED_REFERENCES, m_conf->m_calibration.m_generateReferences);
    DDX_Check(pDX, IDC_CHECK_FILTER_REFERENCES, m_conf->m_calibration.m_filterReferences);

    DDX_Control(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_LOW, m_fitRegionEditLow);
    DDX_Control(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_HIGH, m_fitRegionEditHigh);
}

BEGIN_MESSAGE_MAP(CConfigure_Calibration, CPropertyPage)
    ON_EN_KILLFOCUS(IDC_RADIO_INSTRUMENT_LINE_SHAPE_FIT_NOTHING, SaveData)
    ON_EN_KILLFOCUS(IDC_RADIO_INSTRUMENT_LINE_SHAPE_FIT_SUPER_GAUSS, SaveData)
    ON_EN_KILLFOCUS(IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_LOW, SaveData)
    ON_EN_KILLFOCUS(IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_HIGH, SaveData)

    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SOLAR_SPECTRUM_SETTING, &CConfigure_Calibration::OnClickedButtonBrowseSolarSpectrum)
    ON_BN_CLICKED(IDC_BUTTON_SELECT_INITIAL_CALIBRATION_SETTING, &CConfigure_Calibration::OnButtonSelectInitialCalibration)

    ON_BN_CLICKED(IDC_RADIO_INSTRUMENT_LINE_SHAPE_FIT_NOTHING, &CConfigure_Calibration::OnBnClickedRadioInstrumentLineShapeFitOption)
    ON_BN_CLICKED(IDC_RADIO_INSTRUMENT_LINE_SHAPE_FIT_SUPER_GAUSS, &CConfigure_Calibration::OnBnClickedRadioInstrumentLineShapeFitOption)

END_MESSAGE_MAP()


BOOL CConfigure_Calibration::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    LoadDefaultSetup();

    BOOL enableInstrumentLineShapeFit = m_conf->m_calibration.m_instrumentLineShapeFitOption == 1;
    m_fitRegionEditLow.EnableWindow(enableInstrumentLineShapeFit);
    m_fitRegionEditHigh.EnableWindow(enableInstrumentLineShapeFit);

    UpdateData(FALSE); // Update the UI

    return TRUE;
}

void CConfigure_Calibration::LoadDefaultSetup()
{
    Common common;
    common.GetExePath();

    // See if there a Fraunhofer reference in the standard cross section setup.
    std::string exePath = common.m_exePath;
    m_standardCrossSections = new novac::StandardCrossSectionSetup{ exePath };

    if (m_conf->m_calibration.m_solarSpectrumFile.GetLength() < 3)
    {
        const auto solarCrossSection = m_standardCrossSections->FraunhoferReferenceFileName();

        if (solarCrossSection.size() > 0)
        {
            m_conf->m_calibration.m_solarSpectrumFile = CString(solarCrossSection.c_str());
        }
    }
}

void CConfigure_Calibration::SaveData()
{
    UpdateData(TRUE); // Save the contents of the dialog
}

BOOL CConfigure_Calibration::OnSetActive()
{

    return CPropertyPage::OnSetActive();
}

void CConfigure_Calibration::OnButtonSelectInitialCalibration()
{
    OpenInstrumentCalibrationDialog dlg;
    dlg.m_state.initialCalibrationFile = m_conf->m_calibration.m_initialCalibrationFile;
    dlg.m_state.instrumentLineshapeFile = m_conf->m_calibration.m_instrumentLineshapeFile;
    dlg.m_state.calibrationOption = (InstrumentCalibrationInputOption)m_conf->m_calibration.m_initialCalibrationSetupOption;

    if (IDOK == dlg.DoModal())
    {
        m_conf->m_calibration.m_initialCalibrationFile = dlg.m_state.initialCalibrationFile;
        m_conf->m_calibration.m_instrumentLineshapeFile = dlg.m_state.instrumentLineshapeFile;
        m_conf->m_calibration.m_initialCalibrationSetupOption = (int)dlg.m_state.calibrationOption;

        UpdateData(FALSE);
    }
}

void CConfigure_Calibration::OnClickedButtonBrowseSolarSpectrum()
{
    if (!Common::BrowseForFile("Spectrum Files\0*.std;*.txt;*.xs\0", m_conf->m_calibration.m_solarSpectrumFile))
    {
        return;
    }
    UpdateData(FALSE);
}

void CConfigure_Calibration::OnBnClickedRadioInstrumentLineShapeFitOption()
{
    UpdateData(TRUE); // get the selections from the user interface

    BOOL enableInstrumentLineShapeFit = m_conf->m_calibration.m_instrumentLineShapeFitOption == 1;
    m_fitRegionEditLow.EnableWindow(enableInstrumentLineShapeFit);
    m_fitRegionEditHigh.EnableWindow(enableInstrumentLineShapeFit);
}

// CConfigure_Calibration message handlers
