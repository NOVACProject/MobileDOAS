/* The following two definitions are necesarry since
	VC6 (and VC7) does not handle the long long data-type 
	used by the OceanOptics OmniDriver
*/
#define HIGHRESTIMESTAMP_H
//#define SPECTROMETERCHANNEL_H

#include "stdafx.h"
#include "../DMSpec.h"
#include "Configure_GPS.h"

using namespace Configuration;

// CConfigure_GPS dialog

IMPLEMENT_DYNAMIC(CConfigure_GPS, CPropertyPage)
CConfigure_GPS::CConfigure_GPS()
	: CPropertyPage(CConfigure_GPS::IDD)
{
	m_conf = NULL;
	m_availableBaudrates[0] = 4800;
	m_availableBaudrates[1] = 9600;
	m_availableBaudrates[2] = 19200;
	m_availableBaudrates[3] = 38400;
	m_availableBaudrates[4] = 57600;
	m_availableBaudrates[5] = 115200;
}

CConfigure_GPS::~CConfigure_GPS()
{
	m_conf = NULL;
}

void CConfigure_GPS::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Check(pDX,	IDC_CHECK_USEGPS,		m_conf->m_useGPS);

	// The combo-boxes
	DDX_Control(pDX,		IDC_COMBO_GPS_PORT,			m_gpsPort);
	DDX_Control(pDX,		IDC_COMBO_GPS_BAUDRATE,	m_gpsBaudrate);
}


BEGIN_MESSAGE_MAP(CConfigure_GPS, CPropertyPage)
	// Changing the contents of the radio-boxes:
	ON_LBN_SELCHANGE(IDC_COMBO_PORT,			SaveSettings)
	ON_LBN_SELCHANGE(IDC_COMBO_BAUDRATE,	SaveSettings)

	// Changing wheather we should use the gps or not
	ON_BN_CLICKED(IDC_CHECK_USEGPS,	SaveSettings)
END_MESSAGE_MAP()


// CConfigure_GPS message handlers
BOOL CConfigure_GPS::OnInitDialog(){

	CPropertyPage::OnInitDialog();

	int k;
	CString str;

	// Initialize the serial-port combo-box
	int toSelect = -1;
	for(k = 1; k < 22; ++k){
		str.Format("COM%d", k);
		m_gpsPort.AddString(str);
		if(Equals(str, m_conf->m_gpsPort))
			toSelect = k - 1;
	}
	m_gpsPort.SetCurSel(toSelect);

	// Initialize the baudrate-combo-box
	toSelect = -1;
	for(k = 0; k < 6; ++k){
		str.Format("%d", m_availableBaudrates[k]);
		m_gpsBaudrate.AddString(str);
		if(m_availableBaudrates[k] == m_conf->m_gpsBaudrate)
			toSelect = k;
	}
	m_gpsBaudrate.SetCurSel(toSelect);

	EnableControls();

	return TRUE;
}

void CConfigure_GPS::SaveSettings(){
	UpdateData(TRUE); // <-- save the data in the dialog
	int curPort = m_gpsPort.GetCurSel();
	int curBaud = m_gpsBaudrate.GetCurSel();

	// The port
	m_conf->m_gpsPort.Format("COM%d", curPort + 1);

	// The baudrate
	if(curBaud == -1){
		m_conf->m_gpsBaudrate	= 9600;
		m_gpsBaudrate.SetCurSel(1);
	}else{
		m_conf->m_gpsBaudrate	=	m_availableBaudrates[curBaud];
	}

	EnableControls();
}

/** Enables the controls that should be enabled, and disables
		the ones which should be disabled */
void	CConfigure_GPS::EnableControls(){
	UpdateData(TRUE); // <-- save the data in the dialog

	if(m_conf->m_useGPS){
		m_gpsPort.EnableWindow(TRUE);
		m_gpsBaudrate.EnableWindow(TRUE);
	}else{
		m_gpsPort.EnableWindow(FALSE);
		m_gpsBaudrate.EnableWindow(FALSE);
	}

}