#pragma once

#include "ReEvaluator.h"
#include "afxwin.h"

// CReEval_SkyDlg dialog
namespace ReEvaluation
{

	class CReEval_SkyDlg : public CPropertyPage
	{
		DECLARE_DYNAMIC(CReEval_SkyDlg)

	public:
		CReEval_SkyDlg();
		virtual ~CReEval_SkyDlg();

	// Dialog Data
		enum { IDD = IDD_REEVAL_SKY };

		CReEvaluator *m_reeval;
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()

	private:
		int       m_numSky;
	public:
		afx_msg void OnBrowseForSky();
		afx_msg void OnBrowseForDark();

		afx_msg void OnChangeSkyOption();
		void UpdateControls();

		afx_msg void SaveData();
		
		/*CEdit m_intensityLow;
		CEdit m_intensityHigh;
		CEdit m_intensityChannel;
		CEdit m_columnLow;
		CEdit m_columnHigh;*/
		CEdit m_editSkySpec, m_editSkySpecDark;
		CButton m_btnBrowseSky, m_btnBrowseDark;
		virtual BOOL OnInitDialog();
		virtual BOOL OnKillActive();
	};
}