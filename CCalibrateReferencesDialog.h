#pragma once


// CCalibrateReferenes dialog

class CCalibrateReferencesDialog : public CPropertyPage
{
	DECLARE_DYNAMIC(CCalibrateReferencesDialog)

public:
	CCalibrateReferencesDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CCalibrateReferencesDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CALIBRATE_REFERENCES };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
