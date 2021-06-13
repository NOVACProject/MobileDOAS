#pragma once

#include "Graphs/GraphCtrl.h"

// CCalibrateInstrumentLineShape dialog

class InstrumentLineshapeCalibrationController;

class CCalibrateInstrumentLineShape : public CPropertyPage
{
    DECLARE_DYNAMIC(CCalibrateInstrumentLineShape)

public:
    CCalibrateInstrumentLineShape(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CCalibrateInstrumentLineShape();

    /** Initializes the controls and the dialog */
    virtual BOOL OnInitDialog();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CALIBRATE_LINESHAPE_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:

    CString m_inputSpectrum;
    CString m_darkSpectrum;
    CListBox m_peaksList;
    Graph::CGraphCtrl m_spectrumPlot; // The large plot for the spectrum
    Graph::CGraphCtrl m_minimapPlot; // The minimap for the spectrum, showing where we have zoomed in
    CStatic m_graphHolder;   // Holder for the plot, for easy ui access
    CStatic m_minimapHolder; // Holder for the zoomed out graph (minimap)
    int m_fitFunctionOption = 0; // The option for which function to fit to the line shape. 
    CStatic m_labelSpectrumContainsNoWavelengthCalibration;
    BOOL m_autoDetermineCalibration;

    afx_msg void OnBnClickedButtonBrowseSpectrum();
    afx_msg void OnBnClickedBrowseSpectrumDark();
    afx_msg void OnLbnSelchangeFoundPeak();
    afx_msg void OnBnClickedRadioFitGaussian();
    afx_msg void OnBnClickedSave();
    afx_msg void OnBnClickedToggleCalibrationFromMercuryLines();

private:
    InstrumentLineshapeCalibrationController* m_controller;

    /// <summary>
    /// Updates the controller with a new spectrum or new set of options
    /// </summary>
    void UpdateLineShape();

    /// <summary>
    /// Updates the m_peaksList with the found peaks
    /// </summary>
    void UpdateListOfPeaksFound();

    /// <summary>
    /// Updates m_spectrumPlot with the measured spectrum
    /// </summary>
    void UpdateGraph(bool reset = true);

    /// <summary>
    /// Updates the function fitted to the line shape with the currently selected 
    /// peak and type of function
    /// </summary>
    void UpdateFittedLineShape();

    /// <summary>
    /// Updates the wavelength calibration according to the users choice.
    /// </summary>
    void UpdateWavelengthCalibrationOption();
};
