

#include "stdafx.h"
#include "../DMSpec.h"
#include "Configure_Advanced.h"

using namespace Configuration;

// CConfigure_Advanced dialog

IMPLEMENT_DYNAMIC(CConfigure_Advanced, CPropertyPage)
CConfigure_Advanced::CConfigure_Advanced()
	: CPropertyPage(CConfigure_Advanced::IDD)
{
	m_conf = NULL;
}

CConfigure_Advanced::~CConfigure_Advanced()
{
	m_conf = NULL;
}

void CConfigure_Advanced::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	// The check box.
	DDX_Control(pDX, IDC_CHECK_ADAPTIVEEXPOSURETIME, m_buttonAdaptExpTime);
}


BEGIN_MESSAGE_MAP(CConfigure_Advanced, CPropertyPage)
	ON_BN_CLICKED(IDC_CHECK_ADAPTIVEEXPOSURETIME,			SaveSettings)
END_MESSAGE_MAP()


// CConfigure_Advanced message handlers
BOOL CConfigure_Advanced::OnInitDialog(){

	CPropertyPage::OnInitDialog();

	EnableControls();

	// setup the tool tips
	InitToolTips();

	return TRUE;
}

void CConfigure_Advanced::SaveSettings(){
	UpdateData(TRUE); // <-- save the data in the dialog

}

/** Enables the controls that should be enabled, and disables
		the ones which should be disabled */
void CConfigure_Advanced::EnableControls(){
	UpdateData(TRUE); // <-- save the data in the dialog

}

void CConfigure_Advanced::InitToolTips(){
	// Don't initialize the tool tips twice
	if(m_toolTip.m_hWnd != NULL)
		return;

	// Enable the tool tips
  if(!m_toolTip.Create(this)){
    TRACE0("Failed to create tooltip control\n"); 
  }
	m_toolTip.AddTool(&m_buttonAdaptExpTime,						"Adjust exposure time in real-time");

	m_toolTip.SetMaxTipWidth(SHRT_MAX);
  m_toolTip.Activate(TRUE);
}

BOOL CConfigure_Advanced::PreTranslateMessage(MSG* pMsg){
  m_toolTip.RelayEvent(pMsg);

  return CPropertyPage::PreTranslateMessage(pMsg);
}
