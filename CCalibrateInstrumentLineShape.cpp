// CCalibrateInstrumentLineShape.cpp : implementation file
//

#include "stdafx.h"
#include "CCalibrateInstrumentLineShape.h"
#include "afxdialogex.h"
#include "resource.h"
#include "Common.h"
#include "Calibration/InstrumentLineshapeCalibrationController.h"

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
    m_spectrumPlot.SetXUnits("Pixel");
    m_spectrumPlot.SetBackgroundColor(RGB(0, 0, 0));
    m_spectrumPlot.SetGridColor(RGB(255, 255, 255));
    m_spectrumPlot.SetPlotColor(RGB(255, 0, 0));
    // m_spectrumPlot.SetSecondYUnit("Intensity");
    // m_spectrumPlot.SetSecondRangeY(0, 100, 0);
    m_spectrumPlot.CleanPlot();

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
}


BEGIN_MESSAGE_MAP(CCalibrateInstrumentLineShape, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SPECTRUM, &CCalibrateInstrumentLineShape::OnBnClickedButtonBrowseSpectrum)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SPECTRUM_DARK, &CCalibrateInstrumentLineShape::OnBnClickedBrowseSpectrumDark)
    ON_LBN_SELCHANGE(IDC_LIST_FOUND_PEAKS, &CCalibrateInstrumentLineShape::OnLbnSelchangeFoundPeak)
    ON_BN_CLICKED(IDC_RADIO_FIT_GAUSSIAN, &CCalibrateInstrumentLineShape::OnBnClickedRadioFitGaussian)
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
        m_spectrumPlot.SetRangeX(selectedPeak.pixel - 50, selectedPeak.pixel + 50, 1, false);
        m_spectrumPlot.SetRangeY(
            Min(m_controller->m_inputSpectrum.data() + (int)(selectedPeak.pixel - 50), 100),
            Max(m_controller->m_inputSpectrum.data() + (int)(selectedPeak.pixel - 50), 100),
            true);
    }
    else
    {
        // zoom out to show the entire graph
        m_spectrumPlot.SetRangeX(0.0, static_cast<double>(this->m_controller->m_inputSpectrum.size()), 0, true);
    }

    UpdateGraph(false);
}

void CCalibrateInstrumentLineShape::UpdateGraph(bool reset)
{
    m_spectrumPlot.CleanPlot();

    /* Draw the spectrum */
    if (m_controller->m_inputSpectrum.size() > 0)
    {
        int plotOption = (reset) ? Graph::CGraphCtrl::PLOT_CONNECTED : Graph::CGraphCtrl::PLOT_FIXED_AXIS;
        m_spectrumPlot.SetPlotColor(RGB(255, 0, 0));
        m_spectrumPlot.Plot(m_controller->m_inputSpectrum.data(), static_cast<int>(m_controller->m_inputSpectrum.size()), plotOption);
    }

    /* Draw the peaks */
    if (m_controller->m_peaksFound.size() > 0)
    {
        std::vector<double> peakX;
        std::vector<double> peakY;

        for each (auto peak in m_controller->m_peaksFound)
        {
            peakX.push_back(peak.pixel);
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
