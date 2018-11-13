#pragma once

#include <memory>
#include "MobileConfiguration.h"

// CConfigure_GPS dialog

namespace Configuration{

	class CConfigure_GPS : public CPropertyPage
	{
		DECLARE_DYNAMIC(CConfigure_GPS)

	public:
		CConfigure_GPS();
		virtual ~CConfigure_GPS();

	// Dialog Data
		enum { IDD = IDD_CONFIGURE_GPS };

		/** The local handle to the configuration that we're changing */
		std::shared_ptr<CMobileConfiguration> m_conf;

		// ----------------------- EVENT HANDLERS -------------------------
		/** Saves the settings in the dialog */
		afx_msg void SaveSettings();

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()

		/** Called when the dialog is to be shown */
		virtual BOOL OnInitDialog();

		/** Enables the controls that should be enabled, and disables
				the ones which should be disabled */
		afx_msg void	EnableControls();

		// -------------- DIALOG COMPONENTS -----------------
		/** The GPS's serial-port */
		CComboBox	m_gpsPort;

		/** The GPS's baudrate */
		CComboBox	m_gpsBaudrate;

		// -------------------- PROTECTED DATA --------------------------

		/** The baudrates that the user can choose from */
		long	m_availableBaudrates[6];

	};
}