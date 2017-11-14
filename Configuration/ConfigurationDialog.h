#pragma once

#include "MobileConfiguration.h"

namespace Configuration{
	class CConfigurationDialog :	public CPropertySheet
	{
	public:
		CConfigurationDialog(void);
		~CConfigurationDialog(void);

	protected:
		DECLARE_MESSAGE_MAP()

		/** Called when the dialog is to be shown */
		virtual BOOL OnInitDialog();

	};
}