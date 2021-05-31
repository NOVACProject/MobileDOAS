// CCalibrateInstrumentLineShape.cpp : implementation file
//

#include "stdafx.h"
#include "CCalibrateInstrumentLineShape.h"
#include "afxdialogex.h"
#include "resource.h"
#include "Common.h"
#include "Calibration/InstrumentLineshapeCalibrationController.h"
#include <SpectralEvaluation/File/File.h>
#include <algorithm>

#undef min
#undef max

// CCalibrateInstrumentLineShape dialog

IMPLEMENT_DYNAMIC(CCalibrateInstrumentLineShape, CPropertyPage)

CCalibrateInstrumentLineShape::CCalibrateInstrumentLineShape(CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_CALIBRATE_LINESHAPE_DIALOG)
    , m_inputSpectrum(_T(""))
{
    this->m_controller = new InstrumentLineshapeCalibrationController();
}

CCalibrateInstrumentLineShape::~CCalibrateInstrumentLineShape()
{
    delete this->m_controller;
}

BOOL CCalibrateInstrumentLineShape::OnInitDialog() {
    CPropertyPage::OnInitDialog();

    CRect rect;
    int margin = 2;
    m_graphHolder.GetWindowRect(&rect);
    rect.bottom -= rect.top + margin;
    rect.right -= rect.left + margin;
    rect.top = margin + 7;
    rect.left = margin;
    m_spectrumPlot.Create(WS_VISIBLE | WS_CHILD, rect, &m_graphHolder);
    m_spectrumPlot.SetRange(0, 500, 1, -100.0, 100.0, 1);
    m_spectrumPlot.SetYUnits("Intensity");
    m_spectrumPlot.SetXUnits("Wavelength");
    m_spectrumPlot.SetBackgroundColor(RGB(0, 0, 0));
    m_spectrumPlot.SetGridColor(RGB(255, 255, 255));
    m_spectrumPlot.SetPlotColor(RGB(255, 0, 0));
    m_spectrumPlot.CleanPlot();

    m_labelSpectrumContainsNoWavelengthCalibration.ShowWindow(SW_HIDE);

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CCalibrateInstrumentLineShape::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_SPECTRUM, m_inputSpectrum);
    DDX_Text(pDX, IDC_EDIT_SPECTRUM_DARK, m_darkSpectrum);
    DDX_Control(pDX, IDC_LIST_FOUND_PEAKS, m_peaksList);
    DDX_Control(pDX, IDC_STATIC_GRAPH_HOLDER, m_graphHolder);
    DDX_Radio(pDX, IDC_RADIO_FIT_GAUSSIAN, m_fitFunctionOption);
    DDX_Control(pDX, IDC_LABEL_MISSING_CALIBRATION, m_labelSpectrumContainsNoWavelengthCalibration);
    // DDX_Radio(pDX, IDC_CALIBRATION_FROM_FILE, m_calibrationOption); // TODO: Try to uncomment! This line gives a assert failure during setup. Cannot find why atm
}


BEGIN_MESSAGE_MAP(CCalibrateInstrumentLineShape, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SPECTRUM, &CCalibrateInstrumentLineShape::OnBnClickedButtonBrowseSpectrum)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SPECTRUM_DARK, &CCalibrateInstrumentLineShape::OnBnClickedBrowseSpectrumDark)
    ON_LBN_SELCHANGE(IDC_LIST_FOUND_PEAKS, &CCalibrateInstrumentLineShape::OnLbnSelchangeFoundPeak)
    ON_BN_CLICKED(IDC_RADIO_FIT_GAUSSIAN, &CCalibrateInstrumentLineShape::OnBnClickedRadioFitGaussian)
    ON_BN_CLICKED(IDC_RADIO_FIT_SUPER_GAUSSIAN, &CCalibrateInstrumentLineShape::OnBnClickedRadioFitGaussian)
    ON_BN_CLICKED(IDC_RADIO_FIT_NOTHING, &CCalibrateInstrumentLineShape::OnBnClickedRadioFitGaussian)
    ON_BN_CLICKED(IDC_BUTTON_SAVE, &CCalibrateInstrumentLineShape::OnBnClickedSave)
    ON_BN_CLICKED(IDC_CALIBRATION_FROM_FILE, &CCalibrateInstrumentLineShape::OnBnClickedRadioCalibrationFromFile)
    ON_BN_CLICKED(IDC_CALIBRATION_FROM_MERCURY_LINES, &CCalibrateInstrumentLineShape::OnBnClickedRadioCalibrationFromMercuryLines)
END_MESSAGE_MAP()

// CCalibrateInstrumentLineShape message handlers

void CCalibrateInstrumentLineShape::OnBnClickedButtonBrowseSpectrum()
{
    if (!Common::BrowseForFile("Spectrum Files\0*.std;*.txt\0All Files\0*.*\0", m_inputSpectrum))
    {
        return;
    }

    this->m_controller->m_inputSpectrumPath = this->m_inputSpectrum;

    UpdateLineShape();
}

void CCalibrateInstrumentLineShape::OnBnClickedBrowseSpectrumDark()
{
    if (!Common::BrowseForFile("Spectrum Files\0*.std;*.txt\0All Files\0*.*\0", m_darkSpectrum))
    {
        return;
    }
    this->m_controller->m_darkSpectrumPath = this->m_darkSpectrum;

    UpdateLineShape();
}

void CCalibrateInstrumentLineShape::UpdateLineShape()
{
    try
    {
        m_controller->Update();
    }
    catch (std::invalid_argument& e)
    {
        MessageBox(e.what(), "Invalid input", MB_OK);
    }
    catch (std::exception& e)
    {
        MessageBox(e.what(), "Invalid input", MB_OK);
    }

    // Update the User interface
    UpdateData(FALSE);
    UpdateListOfPeaksFound();
    UpdateGraph();
}

void CCalibrateInstrumentLineShape::UpdateListOfPeaksFound()
{
    this->m_peaksList.ResetContent();

    // the elements need to be added in reversed order as new elements are added to the top, not to the bottom of the control
    for each (auto peak in m_controller->m_peaksFound)
    {
        CString fmt;
        if (peak.wavelength > 0)
        {
            fmt.AppendFormat("%.1lf nm", peak.wavelength);
        }
        else
        {
            fmt.AppendFormat("px: %.0lf", peak.pixel);
        }
        this->m_peaksList.AddString(fmt);
    }

    this->m_peaksList.AddString("--");
}

InstrumentLineshapeCalibrationController::LineShapeFunction OptionToLineShapeFunction(int radioButtonOption)
{
    switch (radioButtonOption)
    {
    case 0: return InstrumentLineshapeCalibrationController::LineShapeFunction::Gaussian;
    case 1: return InstrumentLineshapeCalibrationController::LineShapeFunction::SuperGauss;
    default: return InstrumentLineshapeCalibrationController::LineShapeFunction::None;
    }
}

void CCalibrateInstrumentLineShape::UpdateFittedLineShape()
{
    try
    {
        UpdateData(TRUE); // get the selections from the user interface

        const int selectedPeak = this->m_peaksList.GetCurSel();
        const auto selectedFunctionToFit = OptionToLineShapeFunction(this->m_fitFunctionOption);
        m_controller->FitFunctionToLineShape(selectedPeak, selectedFunctionToFit);
    }
    catch (std::invalid_argument& e)
    {
        MessageBox(e.what(), "Invalid input", MB_OK);
    }
    catch (std::exception& e)
    {
        MessageBox(e.what(), "Invalid input", MB_OK);
    }
}

void CCalibrateInstrumentLineShape::OnLbnSelchangeFoundPeak()
{
    const int selectedElement = this->m_peaksList.GetCurSel();

    UpdateFittedLineShape();

    if (selectedElement >= 0 && selectedElement < static_cast<int>(this->m_controller->m_peaksFound.size()))
    {
        // zoom in on the selected peak
        const novac::SpectrumDataPoint selectedPeak = this->m_controller->m_peaksFound[selectedElement];
        const int firstPixel = std::max(static_cast<int>(selectedPeak.pixel - 50), 0);
        const int lastPixel = std::min(static_cast<int>(selectedPeak.pixel + 50), static_cast<int>(this->m_controller->m_inputSpectrumWavelength.size()) - 1);
        const double lambdaMin = this->m_controller->m_inputSpectrumWavelength[firstPixel];
        const double lambdaMax = this->m_controller->m_inputSpectrumWavelength[lastPixel];

        m_spectrumPlot.SetRangeX(lambdaMin, lambdaMax, 1, false);
        m_spectrumPlot.SetRangeY(
            Min(m_controller->m_inputSpectrum.data() + firstPixel, lastPixel - firstPixel),
            Max(m_controller->m_inputSpectrum.data() + firstPixel, lastPixel - firstPixel),
            true);
    }
    else
    {
        // zoom out to show the entire graph
        m_spectrumPlot.SetRangeX(
            this->m_controller->m_inputSpectrumWavelength.front(),
            this->m_controller->m_inputSpectrumWavelength.back(),
            0,
            false);
        m_spectrumPlot.SetRangeY(
            Min(m_controller->m_inputSpectrum.data(), static_cast<long>(m_controller->m_inputSpectrum.size())),
            Max(m_controller->m_inputSpectrum.data(), static_cast<long>(m_controller->m_inputSpectrum.size())),
            true);
    }

    UpdateGraph(false);
}

void CCalibrateInstrumentLineShape::UpdateGraph(bool reset)
{
    m_spectrumPlot.CleanPlot();

    /* Draw the spectrum */
    if (m_controller->m_inputSpectrum.size() > 0)
    {
        int plotOption = (reset) ? Graph::CGraphCtrl::PLOT_CONNECTED : Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED;
        m_spectrumPlot.SetPlotColor(RGB(255, 0, 0));
        m_spectrumPlot.XYPlot(m_controller->m_inputSpectrumWavelength.data(), m_controller->m_inputSpectrum.data(), static_cast<int>(m_controller->m_inputSpectrum.size()), plotOption);
    }

    /* Draw the peaks */
    if (m_controller->m_peaksFound.size() > 0)
    {
        std::vector<double> peakX;
        std::vector<double> peakY;

        for each (auto peak in m_controller->m_peaksFound)
        {
            peakX.push_back(peak.wavelength);
            peakY.push_back(peak.intensity);
        }
        m_spectrumPlot.DrawCircles(peakX.data(), peakY.data(), static_cast<int>(m_controller->m_peaksFound.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS);
    }

    /* Draw the fitted line shape (if any) */
    if (m_controller->m_sampledLineShapeFunction != nullptr)
    {
        m_spectrumPlot.SetPlotColor(RGB(0, 255, 0));
        m_spectrumPlot.XYPlot(
            m_controller->m_sampledLineShapeFunction->m_wavelength.data(),
            m_controller->m_sampledLineShapeFunction->m_data,
            m_controller->m_sampledLineShapeFunction->m_length,
            Graph::CGraphCtrl::PLOT_CONNECTED | Graph::CGraphCtrl::PLOT_FIXED_AXIS);
    }
}

void CCalibrateInstrumentLineShape::OnBnClickedRadioFitGaussian()
{
    UpdateFittedLineShape();

    UpdateGraph(false);
}

void CCalibrateInstrumentLineShape::OnBnClickedSave()
{
    try
    {
        // Extract the currently selected line shape
        const int selectedPeak = this->m_peaksList.GetCurSel();
        if (selectedPeak < 0 || selectedPeak >= static_cast<int>(m_controller->m_peaksFound.size()))
        {
            MessageBox("Please select a peak to save in the list box to the left", "No peak selected", MB_OK);
            return;
        }

        const auto hgLine = m_controller->GetMercuryPeak(selectedPeak);

        // Differentiate the wavelengths against the center value
        const double centerWavelength = m_controller->m_peaksFound[selectedPeak].wavelength;
        for (double& lambda : hgLine->m_wavelength)
        {
            lambda -= centerWavelength;
        }

        CString destinationFileName = L"";
        if (Common::BrowseForFile_SaveAs("Instrument Line Shape Files\0*.slf\0", destinationFileName))
        {
            std::string dstFileName = novac::EnsureFilenameHasSuffix(std::string(destinationFileName), "slf");
            novac::SaveCrossSectionFile(dstFileName, *hgLine);
        }
    }
    catch (std::exception& e)
    {
        MessageBox(e.what(), "Failed to save instrument line shape", MB_OK);
    }
}

void CCalibrateInstrumentLineShape::UpdateWavelengthCalibrationOption()
{
    try
    {
        UpdateData(TRUE); // get the selections from the user interface

        if (this->m_controller->m_inputSpectrum.size() == 0)
        {
            return;
        }

        this->m_controller->m_readWavelengthCalibrationFromFile = this->m_calibrationOption == 0;

        this->m_controller->Update();

        int showCalibrationWarning = (this->m_calibrationOption == 0 && this->m_controller->m_inputSpectrumContainsWavelength) ? SW_SHOW : SW_HIDE;
        m_labelSpectrumContainsNoWavelengthCalibration.ShowWindow(showCalibrationWarning);
    }
    catch (std::invalid_argument& e)
    {
        MessageBox(e.what(), "Invalid input", MB_OK);
    }
    catch (std::exception& e)
    {
        MessageBox(e.what(), "Invalid input", MB_OK);
    }
}

void CCalibrateInstrumentLineShape::OnBnClickedRadioCalibrationFromFile()
{
    UpdateWavelengthCalibrationOption();
}

void CCalibrateInstrumentLineShape::OnBnClickedRadioCalibrationFromMercuryLines()
{
    UpdateWavelengthCalibrationOption();
}
