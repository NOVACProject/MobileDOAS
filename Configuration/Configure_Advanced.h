#pragma once

#include "MobileConfiguration.h"

// CConfigure_Advanced dialog

namespace Configuration{
	class CConfigure_Advanced : public CPropertyPage
	{
		DECLARE_DYNAMIC(CConfigure_Advanced)

	public:
		CConfigure_Advanced();
		virtual ~CConfigure_Advanced();

	// Dialog Data
		enum { IDD = IDD_CONFIGURE_ADVANCED };

		/** The local handle to the configuration that we're changing */
		CMobileConfiguration	*m_conf;

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

		/** Setup the tool tips */
		void InitToolTips();

		/** Handling the tool tips */
		virtual BOOL PreTranslateMessage(MSG* pMsg); 

		// -------------- DIALOG COMPONENTS -----------------

		/** The auto-exptime button */
		CButton m_buttonAdaptExpTime;

		/** The tooltip control */
		CToolTipCtrl m_toolTip;

		// -------------------- PROTECTED DATA --------------------------
	};
}