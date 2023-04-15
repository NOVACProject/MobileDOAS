#pragma once
#include "afxwin.h"
#include <MobileDoasLib/Flux/Traverse.h>

// CExportEvLogDlg dialog

namespace Dialogs {

    class CExportEvLogDlg : public CDialog
    {
        DECLARE_DYNAMIC(CExportEvLogDlg)

    public:
        CExportEvLogDlg(CWnd* pParent = nullptr);   // standard constructor
        virtual ~CExportEvLogDlg();

        static const int MAX_N_EXPORT_COLUMNS = 9;

        // Dialog Data
        enum { IDD = IDD_EXPORT_EVLOG_DIALOG };

        /** Intitializing the controls */
        virtual BOOL OnInitDialog();

        // -------------------- PUBLIC DIALOG DATA -------------------
        /** The CTraverse - object holds all traverse data */
        mobiledoas::CTraverse* m_traverse;

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        virtual void OnOK();
        virtual void OnCancel();

        DECLARE_MESSAGE_MAP()

            // -------------------- DIALOG COMPONENTS -------------------------

            /** The Labels saying 'Column 1', 'Column 2', etc.. */
            CStatic m_labels[MAX_N_EXPORT_COLUMNS];

        /** The combo-boxes */
        CComboBox m_combo[MAX_N_EXPORT_COLUMNS];

        /** The border 'frame' around the column-combo-boxes */
        CStatic m_columnFrame;

        /** These are the models for the labels and the combo-boxes */
        CStatic m_labelModel;
        CComboBox m_comboModel;

        // -------------------- DIALOG DATA ------------------------

        /** The file to export data to */
        CString m_exportFileName;

        /** The items to choose from */
        int m_nStrings;
        CString* m_strings;

    public:
        afx_msg void OnBrowseOutputFile();
    };
}