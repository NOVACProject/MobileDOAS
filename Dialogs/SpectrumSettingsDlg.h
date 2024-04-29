#pragma once

// CSpectrumSettingsDlg dialog

class CSpectrometer;

namespace Dialogs {

    class CSpectrumSettingsDlg : public CDialog
    {
        DECLARE_DYNAMIC(CSpectrumSettingsDlg)

    public:
        CSpectrumSettingsDlg(CWnd* pParent = nullptr);   // standard constructor
        virtual ~CSpectrumSettingsDlg();

        // Dialog Data
        enum { IDD = IDD_SPECTRUM_SETTINGS_DLG };

        // A handle to the spectrometer
        CSpectrometer* m_Spectrometer = nullptr;

        virtual BOOL OnInitDialog();

        /** Updates the dialog with the data from the CSpectrometer */
        LRESULT UpdateDialogWithDataFromSpectrometer(WPARAM wParam, LPARAM lParam);

        /** Updates the dialog with the list of spectrometers from CSpectrometer.
            This is called from the CSpectrometer when the selected spec has changed */
        LRESULT OnChangeSpectrometer(WPARAM wParam, LPARAM lParam);

        /** Saves the settings in the dialog to the spectrometer */
        void SaveDialogDataToSpectrometer();

        /** Saves the last spectrum to file */
        void SaveSpectrum();

        /** Called when the user has pressed the spin button that controlls the exposure-time */
        afx_msg void OnChangeSpinExptime(NMHDR* pNMHDR, LRESULT* pResult);

        /** Called when the user has pressed the spin button that controlls the number of spectra to average */
        afx_msg void OnChangeSpinAverage(NMHDR* pNMHDR, LRESULT* pResult);

        /** Called when the user has changed the spectrometer to use */
        afx_msg void OnUserChangeSpectrometer();

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        DECLARE_MESSAGE_MAP()

    private:
        // The values in the edit boxes
        int	m_integrationTimeMs = 100;
        int m_average = 1;

        // The edit-boxes
        CEdit m_exptimeEdit, m_averageEdit;

        // The spin-controls
        CSpinButtonCtrl m_exptimeSpin;
        CSpinButtonCtrl m_averageSpin;

        // The list of spectrometers
        CComboBox m_comboSpecs;

        // The selected channel on the spectrometer
        int m_channel = 0;

        /** Retrieves the list of spectrometers from the CSpectrometer and
            updates the combo-box */
        void UpdateListOfSpectrometers();
    };
}