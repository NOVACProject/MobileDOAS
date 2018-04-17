#pragma once

namespace Dialogs{
	// CInformationDialog dialog

	class CInformationDialog : public CDialog
	{
		DECLARE_DYNAMIC(CInformationDialog)

	public:
		CInformationDialog(CWnd* pParent = nullptr);   // standard constructor
		virtual ~CInformationDialog();

	// Dialog Data
		enum { IDD = IDD_INFORMATIONDIALOG };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	public:
		CString informationString;
	};
}