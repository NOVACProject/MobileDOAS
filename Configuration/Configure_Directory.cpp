#include "stdafx.h"
#include "Configure_Directory.h"
using namespace Configuration;

IMPLEMENT_DYNAMIC(CConfigure_Directory, CPropertyPage)

CConfigure_Directory::CConfigure_Directory()
	: CPropertyPage(CConfigure_Directory::IDD)
{
	m_conf = nullptr;
}

CConfigure_Directory::~CConfigure_Directory()
{
	m_conf = nullptr;
}

void CConfigure_Directory::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	// The fit-parameters edit boxes
	DDX_Text(pDX, IDC_EDIT_DIRECTORY, m_conf->m_directory);
	DDX_Text(pDX, IDC_EDIT_SLEEP, m_conf->m_sleep);
}

BEGIN_MESSAGE_MAP(CConfigure_Directory, CPropertyPage)
	ON_EN_KILLFOCUS(IDC_EDIT_DIRECTORY, SaveData)
	ON_EN_KILLFOCUS(IDC_EDIT_SLEEP, SaveData)
	ON_BN_CLICKED(IDC_BROWSE_DIRECTORY, &CConfigure_Directory::OnBnClickedBrowseDirectory)
END_MESSAGE_MAP()

BOOL CConfigure_Directory::OnInitDialog() {
	CPropertyPage::OnInitDialog();
	return TRUE;
}
void CConfigure_Directory::SaveData() {
	UpdateData(TRUE);
}

void Configuration::CConfigure_Directory::OnBnClickedBrowseDirectory()
{
	Common common;
	if (common.BrowseForDirectory(m_directory)) {
		m_conf->m_directory = m_directory;
		UpdateData(FALSE);
	}
}
