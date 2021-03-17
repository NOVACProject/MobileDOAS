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
    Graph::CGraphCtrl m_spectrumPlot; // The plot for the spectrum
    CStatic m_graphHolder; // Holder for the plot, for easy ui access
    int m_fitFunctionOption; // The option for which function to fit to the line shape. 

    afx_msg void OnBnClickedButtonBrowseSpectrum();
    afx_msg void OnBnClickedBrowseSpectrumDark();
    afx_msg void OnLbnSelchangeFoundPeak();
    afx_msg void OnBnClickedRadioFitGaussian();

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
};
