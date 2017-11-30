// ReEval_SkyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\DMSpec.h"
#include "ReEval_SkyDlg.h"


// CReEval_SkyDlg dialog
using namespace ReEvaluation;

IMPLEMENT_DYNAMIC(CReEval_SkyDlg, CPropertyPage)
CReEval_SkyDlg::CReEval_SkyDlg()
	: CPropertyPage(CReEval_SkyDlg::IDD)
{
	m_numSky = 0;
}

CReEval_SkyDlg::~CReEval_SkyDlg()
{
}

void CReEval_SkyDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Radio(pDX, IDC_SINGLE_SKY,       (int &)m_reeval->m_settings.m_skySelection);

	//DDX_Text(pDX, IDC_COLUMN_HIGH,       m_reeval->m_settings.m_skyColumnHigh);
	//DDX_Text(pDX, IDC_COLUMN_LOW,        m_reeval->m_settings.m_skyColumnLow);
	//DDX_Text(pDX, IDC_INTENSITY_CHANNEL, m_reeval->m_settings.m_skyIntensityChannel);
	//DDX_Text(pDX, IDC_HIGH_INTENSITY,    m_reeval->m_settings.m_skyIntensityHigh);
	//DDX_Text(pDX, IDC_LOW_INTENSITY,     m_reeval->m_settings.m_skyIntensityLow);

	// The user supplied sky spectrum and the corresponding dark
	DDX_Text(pDX, IDC_EDIT_SKYSPEC,      m_reeval->m_settings.m_skySpectrumFile);
	DDX_Text(pDX, IDC_EDIT_SKYSPECDARK,  m_reeval->m_settings.m_skySpectrumDark);
	
	// The controls in the window
	//DDX_Control(pDX, IDC_LOW_INTENSITY,      m_intensityLow);
	//DDX_Control(pDX, IDC_HIGH_INTENSITY,     m_intensityHigh);
	//DDX_Control(pDX, IDC_INTENSITY_CHANNEL,  m_intensityChannel);
	//DDX_Control(pDX, IDC_COLUMN_LOW,         m_columnLow);
	//DDX_Control(pDX, IDC_COLUMN_HIGH,        m_columnHigh);
	DDX_Control(pDX, IDC_EDIT_SKYSPEC,       m_editSkySpec);
	DDX_Control(pDX, IDC_EDIT_SKYSPECDARK,   m_editSkySpecDark);
	DDX_Control(pDX, IDC_BUTTON_BROWSE_SKY,  m_btnBrowseSky);
	DDX_Control(pDX, IDC_BUTTON_BROWSE_DARK, m_btnBrowseDark);
}


BEGIN_MESSAGE_MAP(CReEval_SkyDlg, CPropertyPage)
	//ON_EN_CHANGE(IDC_COLUMN_HIGH,       SaveData)
	//ON_EN_CHANGE(IDC_COLUMN_LOW,        SaveData)
	//ON_EN_CHANGE(IDC_INTENSITY_CHANNEL, SaveData)
	//ON_EN_CHANGE(IDC_HIGH_INTENSITY,    SaveData)
	//ON_EN_CHANGE(IDC_LOW_INTENSITY,     SaveData)

	ON_BN_CLICKED(IDC_SINGLE_SKY,       OnChangeSkyOption)
	ON_BN_CLICKED(IDC_ALL_SKY,          OnChangeSkyOption)
	//ON_BN_CLICKED(IDC_CONDITIONAL_SKY,  OnChangeSkyOption)
	ON_BN_CLICKED(IDC_USER_SKY,         OnChangeSkyOption)
	

	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SKY,  OnBrowseForSky)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_DARK, OnBrowseForDark)
	
END_MESSAGE_MAP()


// CReEval_SkyDlg message handlers


void CReEval_SkyDlg::OnChangeSkyOption(){
	UpdateData(TRUE);
	UpdateControls();
}

void CReEval_SkyDlg::UpdateControls(){

	if(m_reeval->m_settings.m_skySelection == USE_SKY_FIRST){
		m_columnHigh.EnableWindow(FALSE);
		m_columnLow.EnableWindow(FALSE);
		m_intensityLow.EnableWindow(FALSE);
		m_intensityHigh.EnableWindow(FALSE);
		m_intensityChannel.EnableWindow(FALSE);
		m_editSkySpec.EnableWindow(FALSE);
		m_editSkySpecDark.EnableWindow(FALSE);
		m_btnBrowseSky.EnableWindow(FALSE);
		m_btnBrowseDark.EnableWindow(FALSE);
	}else if(m_reeval->m_settings.m_skySelection == USE_SKY_ALL){
		m_columnHigh.EnableWindow(FALSE);
		m_columnLow.EnableWindow(FALSE);
		m_intensityLow.EnableWindow(FALSE);
		m_intensityHigh.EnableWindow(FALSE);
		m_intensityChannel.EnableWindow(FALSE);
		m_editSkySpec.EnableWindow(FALSE);
		m_editSkySpecDark.EnableWindow(FALSE);
		m_btnBrowseSky.EnableWindow(FALSE);
		m_btnBrowseDark.EnableWindow(FALSE);
	}else if(m_reeval->m_settings.m_skySelection == USE_SKY_CUSTOM){
		m_columnHigh.EnableWindow(TRUE);
		m_columnLow.EnableWindow(TRUE);
		m_intensityLow.EnableWindow(TRUE);
		m_intensityHigh.EnableWindow(TRUE);
		m_intensityChannel.EnableWindow(TRUE);
		m_editSkySpec.EnableWindow(FALSE);
		m_editSkySpecDark.EnableWindow(FALSE);
		m_btnBrowseSky.EnableWindow(FALSE);
		m_btnBrowseDark.EnableWindow(FALSE);
	}else if(m_reeval->m_settings.m_skySelection == USE_SKY_USER){
		m_columnHigh.EnableWindow(FALSE);
		m_columnLow.EnableWindow(FALSE);
		m_intensityLow.EnableWindow(FALSE);
		m_intensityHigh.EnableWindow(FALSE);
		m_intensityChannel.EnableWindow(FALSE);
		m_editSkySpec.EnableWindow(TRUE);
		m_editSkySpecDark.EnableWindow(TRUE);
		m_btnBrowseSky.EnableWindow(TRUE);
		m_btnBrowseDark.EnableWindow(TRUE);
	}
}

BOOL CReEval_SkyDlg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	UpdateControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CReEval_SkyDlg::OnBrowseForSky(){
	CString fileName;

	// Let the user browse for a sky-spectrum file
	if(SUCCESS == Common::BrowseForFile("*.std", fileName)){
		m_reeval->m_settings.m_skySpectrumFile.Format("%s", fileName);
		m_reeval->m_settings.m_skySelection = USE_SKY_USER;
		UpdateData(FALSE); // <-- Fill in the dialog
	}
}
void CReEval_SkyDlg::OnBrowseForDark(){
	CString fileName;

	// Let the user browse for a dark-spectrum file
	if(SUCCESS == Common::BrowseForFile("*.std", fileName)){
		m_reeval->m_settings.m_skySpectrumDark.Format("%s", fileName);
		m_reeval->m_settings.m_skySelection = USE_SKY_USER;
		UpdateData(FALSE); // <-- Fill in the dialog
	}
}
BOOL ReEvaluation::CReEval_SkyDlg::OnKillActive()
{
	CString message;
	
	SaveData();

	if(m_reeval->m_settings.m_skySelection == USE_SKY_USER){
		// check so that the user has supplied a sky spectrum
		if(strlen(m_reeval->m_settings.m_skySpectrumFile) <= 0){
			MessageBox("You must supply a sky spectrum to use", "Error in settings", MB_OK);
			return 0;
		}
		// check so that the user has supplied a dark spectrum
		if(strlen(m_reeval->m_settings.m_skySpectrumDark) <= 0){
			MessageBox("You must supply a dark spectrum to use for the sky spectrum", "Error in settings", MB_OK);
			return 0;
		}
		// check so that the sky spectrum that the user has supplied does exist
		FILE *f = fopen(m_reeval->m_settings.m_skySpectrumFile, "r");
		if(f == NULL){
			message.Format("Cannot open file: %s", m_reeval->m_settings.m_skySpectrumFile);
			MessageBox(message, "Error in settings");
			return 0;
		}
		fclose(f);
		// check so that the dark spectrum that the user has supplied does exist
		f = fopen(m_reeval->m_settings.m_skySpectrumDark, "r");
		if(f == NULL){
			message.Format("Cannot open file: %s", m_reeval->m_settings.m_skySpectrumFile);
			MessageBox(message, "Error in settings");
			return 0;
		}
		fclose(f);
	}

	return CPropertyPage::OnKillActive();
}

void ReEvaluation::CReEval_SkyDlg::SaveData(){
	UpdateData(TRUE);
}
