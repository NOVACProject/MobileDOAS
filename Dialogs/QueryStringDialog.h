#pragma once

namespace Dialogs
{

    // CQueryStringDialog dialog

    class CQueryStringDialog : public CDialog
    {
	    DECLARE_DYNAMIC(CQueryStringDialog)

    public:
	    CQueryStringDialog(CWnd* pParent = nullptr);   // standard constructor
	    virtual ~CQueryStringDialog();

      virtual BOOL OnInitDialog();

			// Dialog Data
	    enum { IDD = IDD_QUERY_STRING_DIALOG };

      /** The window text */
      CString   m_windowText;

      /** The returned string */
      CString   *m_inputString;

    protected:
	    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	    DECLARE_MESSAGE_MAP()

			/** The input edit-box */
			CEdit m_editBox;

			/** When the user presses the 'ok' button */
			virtual void OnOK();
    };

}