// CCalibratePixelToWavelengthDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CCalibratePixelToWavelengthDialog.h"
#include "afxdialogex.h"
#include "resource.h"
#include "Common.h"
#include "Calibration/WavelengthCalibrationController.h"
#include <fstream>

// CCalibratePixelToWavelengthDialog dialog

IMPLEMENT_DYNAMIC(CCalibratePixelToWavelengthDialog, CPropertyPage)

CCalibratePixelToWavelengthDialog::CCalibratePixelToWavelengthDialog(CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_CALIBRATE_WAVELENGTH_DIALOG)
    , m_inputSpectrumFile(_T(""))
    , m_darkSpectrumFile(_T(""))
{
    LoadSetup();
}

CCalibratePixelToWavelengthDialog::~CCalibratePixelToWavelengthDialog()
{
}

BOOL CCalibratePixelToWavelengthDialog::OnInitDialog() {
    CPropertyPage::OnInitDialog();

    CRect rect;
    int margin = 2;
    m_graphHolder.GetWindowRect(&rect);
    rect.bottom -= rect.top + margin;
    rect.right -= rect.left + margin;
    rect.top = margin + 7;
    rect.left = margin;
    m_graph.Create(WS_VISIBLE | WS_CHILD, rect, &m_graphHolder);
    m_graph.SetRange(0, 500, 1, -100.0, 100.0, 1);
    m_graph.SetYUnits("Intensity");
    m_graph.SetXUnits("Pixel");
    m_graph.SetBackgroundColor(RGB(0, 0, 0));
    m_graph.SetGridColor(RGB(255, 255, 255));
    m_graph.SetPlotColor(RGB(255, 0, 0));
    // m_graph.SetSecondYUnit("Intensity");
    // m_graph.SetSecondRangeY(0, 100, 0);
    m_graph.CleanPlot();

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CCalibratePixelToWavelengthDialog::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_SPECTRUM, m_inputSpectrumFile);
    DDX_Text(pDX, IDC_EDIT_SOLAR_SPECTRUM, m_setup.m_solarSpectrumFile);
    DDX_Text(pDX, IDC_EDIT_INITIAL_CALIBRATION, m_setup.m_initialCalibrationFile);
    DDX_Text(pDX, IDC_EDIT_INITIAL_CALIBRATION2, m_setup.m_instrumentLineshapeFile);
    DDX_Text(pDX, IDC_EDIT_SPECTRUM_DARK2, m_darkSpectrumFile);
    DDX_Control(pDX, IDC_STATIC_GRAPH_HOLDER_PANEL, m_graphHolder);
}

BEGIN_MESSAGE_MAP(CCalibratePixelToWavelengthDialog, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SPECTRUM, &CCalibratePixelToWavelengthDialog::OnBnClickedButtonBrowseSpectrum)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SPECTRUM_DARK2, &CCalibratePixelToWavelengthDialog::OnBnClickedButtonBrowseSpectrumDark)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SOLAR_SPECTRUM, &CCalibratePixelToWavelengthDialog::OnClickedButtonBrowseSolarSpectrum)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_INITIAL_CALIBRATION2, &CCalibratePixelToWavelengthDialog::OnClickedButtonBrowseInitialCalibration)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_LINE_SHAPE, &CCalibratePixelToWavelengthDialog::OnBnClickedButtonBrowseLineShape)
    ON_BN_CLICKED(IDC_BUTTON_RUN, &CCalibratePixelToWavelengthDialog::OnClickedButtonRun)
END_MESSAGE_MAP()

// Persisting the setup to file
std::string CCalibratePixelToWavelengthDialog::SetupFilePath()
{
    Common common;
    common.GetExePath();
    CString path;
    path.Format("%sCalibrateWavelengthDlg.xml", common.m_exePath);
    return std::string(path);
}

void CCalibratePixelToWavelengthDialog::SaveSetup()
{
    try
    {
        std::ofstream dst(this->SetupFilePath(), std::ios::out);
        dst << "<CalibrateWavelengthDlg>" << std::endl;
        dst << "\t<SolarSpectrum>" << this->m_setup.m_solarSpectrumFile << "</SolarSpectrum>" << std::endl;
        dst << "\t<InitialCalibrationFile>" << this->m_setup.m_initialCalibrationFile << "</InitialCalibrationFile>" << std::endl;
        dst << "\t<LineShapeFile>" << this->m_setup.m_instrumentLineshapeFile << "</LineShapeFile>" << std::endl;
        dst << "</CalibrateWavelengthDlg>" << std::endl;
    }
    catch (std::exception&)
    {
    }
}

CString ParseXmlString(const char* startTag, const char* stopTag, const std::string& line)
{
    const size_t start = line.find(startTag) + strlen(startTag);
    const size_t stop = line.find(stopTag);
    if (stop > start)
    {
        return CString(line.c_str() + start, stop - start);
    }
    return CString(); // parse failure, return empty string.
}

void CCalibratePixelToWavelengthDialog::LoadSetup()
{
    try
    {
        // Super basic xml parsing
        std::ifstream file(this->SetupFilePath(), std::ios::in);
        std::string line;
        while (std::getline(file, line))
        {
            if (line.find("SolarSpectrum") != std::string::npos)
            {
                this->m_setup.m_solarSpectrumFile = ParseXmlString("<SolarSpectrum>", "</SolarSpectrum>", line);
            }
            else if (line.find("InitialCalibrationFile") != std::string::npos)
            {
                this->m_setup.m_initialCalibrationFile = ParseXmlString("<InitialCalibrationFile>", "</InitialCalibrationFile>", line);
            }
            else if (line.find("LineShapeFile") != std::string::npos)
            {
                this->m_setup.m_instrumentLineshapeFile = ParseXmlString("<LineShapeFile>", "</LineShapeFile>", line);
            }
        }
    }
    catch (std::exception&)
    {
    }
}

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
    if (!Common::BrowseForFile("Spectrum Files\0*.std;*.txt;*.xs\0", this->m_setup.m_solarSpectrumFile))
    {
        return;
    }
    UpdateData(FALSE);
}

void CCalibratePixelToWavelengthDialog::OnClickedButtonBrowseInitialCalibration()
{
    if (!Common::BrowseForFile("Spectrum Files\0*.txt;*.xs\0", this->m_setup.m_initialCalibrationFile))
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
    if (!Common::BrowseForFile("Spectrum Files\0*.txt;*.xs\0", this->m_setup.m_instrumentLineshapeFile))
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
    if (!IsExistingFile(m_setup.m_solarSpectrumFile))
    {
        MessageBox("Please select a high resolved solar spectrum to use in the calibration", "Missing input", MB_OK);
        return;
    }
    if (!IsExistingFile(m_setup.m_initialCalibrationFile))
    {
        MessageBox("Please select a file which contains an initial guess for the wavelength calibration of the spectrometer", "Missing input", MB_OK);
        return;
    }

    WavelengthCalibrationController controller;
    controller.m_inputSpectrumFile = this->m_inputSpectrumFile;
    controller.m_darkSpectrumFile = this->m_darkSpectrumFile;
    controller.m_solarSpectrumFile = this->m_setup.m_solarSpectrumFile;
    controller.m_initialWavelengthCalibrationFile = this->m_setup.m_initialCalibrationFile;
    controller.m_initialLineShapeFile = this->m_setup.m_instrumentLineshapeFile;

    try
    {
        controller.RunCalibration();

        SaveSetup();
    }
    catch (std::exception& e)
    {
        MessageBox(e.what(), "Failed to calibrate", MB_OK);
    }
}
