#pragma once

#include <SpectralEvaluation/Evaluation/ReferenceFile.h>

// CReferencePropertiesDlg dialog
namespace Dialogs
{
    class CReferencePropertiesDlg : public CDialog
    {
        DECLARE_DYNAMIC(CReferencePropertiesDlg)

    public:
        CReferencePropertiesDlg(CWnd* pParent = nullptr);   // standard constructor
        virtual ~CReferencePropertiesDlg();

        // Dialog Data
        enum { IDD = IDD_REFERENCE_PROPERTIES_DLG };

        // The reference file that we're modifying
        novac::CReferenceFile* m_ref;

        /** The tooltip control */
        CToolTipCtrl m_toolTip;

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        DECLARE_MESSAGE_MAP()

        /** Called when the user presses the 'OK' button*/
        virtual void OnOK();

        /** Saves the contents of the dialog */
        afx_msg void SaveData();

        /** Updates the contents in the dialog */
        afx_msg void UpdateDlg();

        /** Lets the user browse for a reference file */
        afx_msg void BrowseForReference();

        /** The option for the shift */
        int  m_shiftOption;

        /** The option for the squeeze */
        int  m_squeezeOption;

        /** The value for the shift, if the 'fix' is selected */
        double m_shiftFixValue;

        /** The value for the shift, if the 'link' is selected */
        double m_shiftLinkValue;

        /** The value for the squeeze, if the 'fix' is selected */
        double m_squeezeFixValue;

        /** The value for the squeeze, if the 'link' is selected */
        double m_squeezeLinkValue;

        /** The name of the reference file */
        CString m_referenceName;

        /** The path to  the reference file */
        CString m_referencePath;

    public:
        virtual BOOL OnInitDialog();

        /** Setup the tool tips */
        void InitToolTips();

        /** Handling the tool tips */
        virtual BOOL PreTranslateMessage(MSG* pMsg);

        // -------------------------- DIALOG COMPONENTS ---------------------
        CStatic m_editSqueezeLink, m_editShiftLink;
        CStatic m_editSqueezeFix, m_editShiftFix;
    };
}