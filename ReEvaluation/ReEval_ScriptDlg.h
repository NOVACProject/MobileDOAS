#pragma once

#include "ReEvaluation_Script.h"

namespace ReEvaluation{

	class CReEval_ScriptDlg : public CDialog
	{
		DECLARE_DYNAMIC(CReEval_ScriptDlg)

	public:
		CReEval_ScriptDlg(CWnd* pParent = NULL);   // standard constructor
		virtual ~CReEval_ScriptDlg();

	// Dialog Data
		enum { IDD = IDD_REEVAL_SCRIPT_DLG };

		virtual BOOL OnInitDialog();
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
		
		/** The script that we're working on */
		CReEvaluation_Script m_script;
		
		// The settings file, this should be used in every job that we create
		CString m_settingsFile;
		
		// --------- ITEMS IN THE DIALOG --------------
		CButton m_btnInsertEvalLog, m_btnRemoveEvalLog, m_btnScanEvalLogs, m_btnSaveScript;
		CListBox m_listEvalLogs;
		CComboBox m_comboThreadNum;

		// ---------- HANDLING THE INTERFACE ----------
		void UpdateControls();
		void UpdateEvalLogList();

		// ----------- HANDLING THE USER ------------	
		afx_msg void OnBrowseSettingsFile();
		afx_msg void OnSaveScript();
		afx_msg void OnBrowseEvallog();
		afx_msg void OnRemoveEvallog();
		afx_msg void OnScanForEvallogs();
		afx_msg void OnMenuLoadScript();
		afx_msg void OnMenuSaveScript();
		afx_msg void OnChangeMaxThreads();
	
	
	
	};
}