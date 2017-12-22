// SpectrumSettingsDlg.cpp : implementation file
//

#undef min
#undef max

#include "stdafx.h"
#include "../DMSpec.h"
#include "SpectrumSettingsDlg.h"
#include <algorithm>

int GetLargestDivisorBelow16(int n);

// CSpectrumSettingsDlg dialog

using namespace Dialogs;

IMPLEMENT_DYNAMIC(CSpectrumSettingsDlg, CDialog)
CSpectrumSettingsDlg::CSpectrumSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpectrumSettingsDlg::IDD, pParent)
{
	m_Spectrometer = NULL;
	m_exptime = 100;
	m_average = 1;
	m_channel = 0;
}

CSpectrumSettingsDlg::~CSpectrumSettingsDlg()
{
	m_Spectrometer = NULL;
}

void CSpectrumSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	
	DDX_Control(pDX, IDC_SPIN_EXPTIME,			m_exptimeSpin);
	DDX_Control(pDX, IDC_SPIN_NAVERAGE,			m_averageSpin);

	DDX_Control(pDX, IDC_EDIT_EXPOSURETIME,		m_exptimeEdit);
	DDX_Control(pDX, IDC_EDIT_NAVERAGE,			m_averageEdit);

	DDX_Text(pDX, IDC_EDIT_EXPOSURETIME,		m_exptime);
	DDX_Text(pDX, IDC_EDIT_NAVERAGE,			m_average);

	DDX_Control(pDX, IDC_COMBO_SPECTROMETERS,	m_comboSpecs);
	
	DDX_Radio(pDX, IDC_RADIO_CHANNEL0,			m_channel);
}


BEGIN_MESSAGE_MAP(CSpectrumSettingsDlg, CDialog)
	ON_MESSAGE(WM_SHOWINTTIME,						UpdateFromSpectrometer)
	ON_MESSAGE(WM_CHANGEDSPEC,						OnChangeSpectrometer)
	ON_COMMAND(IDC_BUTTON_SAVESPEC,					SaveSpectrum)
	
	ON_EN_CHANGE(IDC_EDIT_EXPOSURETIME,				SaveToSpectrometer)
	ON_EN_CHANGE(IDC_EDIT_NAVERAGE,					SaveToSpectrometer)
	
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_EXPTIME,		OnChangeSpinExptime)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_NAVERAGE,		OnChangeSpinAverage)
	
	ON_CBN_SELCHANGE(IDC_COMBO_SPECTROMETERS,		OnUserChangeSpectrometer)
	
	ON_BN_CLICKED(IDC_RADIO_CHANNEL0,				OnUserChangeSpectrometer)
	ON_BN_CLICKED(IDC_RADIO_CHANNEL1,				OnUserChangeSpectrometer)
	
END_MESSAGE_MAP()

// CSpectrumSettingsDlg message handlers

BOOL CSpectrumSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_exptimeSpin.SetBuddy(&m_exptimeEdit);
	m_averageSpin.SetBuddy(&m_averageEdit);

	UpdateFromSpectrometer(0,0);

	return 0;
}

/** Updates the dialog with the data from the CSpectrometer */
LRESULT CSpectrumSettingsDlg::UpdateFromSpectrometer(WPARAM wParam, LPARAM lParam){
	if(this->m_Spectrometer == NULL){
		return 0;
	}

	// Get the parameters from the spectrometer
	this->m_average = m_Spectrometer->totalSpecNum;
	this->m_exptime = m_Spectrometer->integrationTime;
	
	// Update the window
	if(this->m_hWnd != NULL)
		UpdateData(FALSE);
	
	return 0;
}


/** Saves the settings in the dialog to the spectrometer */
void CSpectrumSettingsDlg::SaveToSpectrometer(){

	if(this->m_Spectrometer == NULL){
		return;
	}

	// Get the values from the window...
	UpdateData(TRUE);
	
	// sanity check
	if(m_exptime < 3 || m_exptime > 65536 || m_average < 1 || m_average > 50000){
		return;
	}
		
	// set the parameters
	m_Spectrometer->integrationTime		= m_exptime;
	m_Spectrometer->totalSpecNum		= m_average;
	if(Equals(m_Spectrometer->m_spectrometerModel, "USB2000+")){
		m_Spectrometer->m_sumInSpectrometer = m_average;
		m_Spectrometer->m_sumInComputer = 1;
	}else{
		if(m_average > 15){
			// Get the largest possible number to add together in the spectrometer
			int largestDivisor = GetLargestDivisorBelow16(m_average);
	
			m_Spectrometer->m_sumInSpectrometer	= std::max(largestDivisor, 1); //max(largestDivisor_lo, largestDivisor_hi));
			m_Spectrometer->m_sumInComputer		= m_average / m_Spectrometer->m_sumInSpectrometer;
		}else{
			m_Spectrometer->m_sumInSpectrometer	= m_average;
			m_Spectrometer->m_sumInComputer		= 1;
		}
	}
	m_Spectrometer->totalSpecNum = m_Spectrometer->m_sumInComputer * m_Spectrometer->m_sumInSpectrometer;
}

int GetLargestDivisorBelow16(int n){
	for(int k = 15; k > 0; --k){
		if(n % k == 0)
			return k;
	}
	return 1;
}


/** Saves the last spectrum to file */
void CSpectrumSettingsDlg::SaveSpectrum(){
	CString stdFileName;

	// this is the filter to use in the 'Save As' dialog
	TCHAR filter[512];
	int n = _stprintf(filter, "Spectrum files\0");
	n += _stprintf(filter + n + 1, "*.std;\0");
	filter[n + 2] = 0;
		
	// let the user browse for a place where to store the spectrum
	if(!Common::BrowseForFile_SaveAs(filter, stdFileName))
		return;

	// add the file-ending .std if the user hasn't done so
	if(!Equals(stdFileName.Right(4), ".std"))
		stdFileName.AppendFormat(".std");

	// copy the data from the spectrometer
	static double *spectrum1 = new double[MAX_SPECTRUM_LENGTH];
	static double *spectrum2 = new double[MAX_SPECTRUM_LENGTH];

	int spectrumLength       = m_Spectrometer->m_detectorSize;
	double lat = 0.0;	// <-- this we don't know anything about
	double lon = 0.0;	// <-- this we don't know anything about
	char* startdate = "000000";
	long starttime = 0; // <-- this we don't know anything about
	long stoptime  = 0; // <-- this we don't know anything about

	// Copy the spectrum(-a)
	memcpy(spectrum1, m_Spectrometer->GetSpectrum(0), sizeof(double)*spectrumLength);
	if(m_Spectrometer->m_NChannels > 1){
		memcpy(spectrum2, m_Spectrometer->GetSpectrum(1), sizeof(double)*spectrumLength);
	}

	// write the file
	CSpectrumIO::WriteStdFile(stdFileName, spectrum1, spectrumLength, startdate, starttime, stoptime, lat, lon, m_Spectrometer->integrationTime, m_Spectrometer->spectrometerName, "...", m_Spectrometer->totalSpecNum);
}

// Called when the user has pressed the spin button that controlls the exposure-time
void CSpectrumSettingsDlg::OnChangeSpinExptime(NMHDR *pNMHDR, LRESULT *pResult)
{
	CString str;
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	// Check which direction the user has pressed the spinbutton (which is upside down)
	//  and change the circle radius accordingly
	if(pNMUpDown->iDelta > 0)
		--m_exptime;
	if(pNMUpDown->iDelta < 0)
		++m_exptime;
	  
	// Enforce the limits
	m_exptime = std::max(3,		m_exptime);
	m_exptime = std::min(65536,	m_exptime);

	*pResult = 0;
	
	// Update the screen
	str.Format("%d", m_exptime);
	m_exptimeEdit.SetWindowText(str);

}

// Called when the user has pressed the spin button that controlls the number of spectra to average
void CSpectrumSettingsDlg::OnChangeSpinAverage(NMHDR *pNMHDR, LRESULT *pResult)
{
	CString str;
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	// Check which direction the user has pressed the spinbutton (which is upside down)
	//  and change the circle radius accordingly
	if(pNMUpDown->iDelta > 0)
		--m_average;
	if(pNMUpDown->iDelta < 0)
		++m_average;
	  
	// Enforce the limits
	m_average = std::max(1,		m_average);
	m_average = std::min(20000,	m_average);

	*pResult = 0;

	// Update the screen
	str.Format("%d", m_average);
	m_averageEdit.SetWindowText(str);
}

/** Retrieves the list of spectrometers from the CSpectrometer and 
	updates the combo-box */
void CSpectrumSettingsDlg::UpdateListOfSpectrometers(){
	CList <CString, CString&> spectrometers;

	m_Spectrometer->GetConnectedSpecs(spectrometers);
	
	int nFoundSpecs = spectrometers.GetCount();
	if(nFoundSpecs == 0 || (m_Spectrometer->m_spectrometerIndex < 0 || m_Spectrometer->m_spectrometerIndex >= nFoundSpecs)){
		return;
	}else{
		// build the list 
		m_comboSpecs.ResetContent();
		POSITION p = spectrometers.GetHeadPosition();
		while(p != NULL){
			m_comboSpecs.AddString(spectrometers.GetNext(p));
		}

		// also select the currently used one
		if(m_Spectrometer->m_spectrometerIndex >= 0)
			m_comboSpecs.SetCurSel(m_Spectrometer->m_spectrometerIndex);
			
		// Not least, set the channel to use
		this->m_channel = m_Spectrometer->m_spectrometerChannel;
		
		UpdateData(FALSE);
	}
	
	return;
}

/** Updates the dialog with the list of spectrometers from CSpectrometer */
LRESULT CSpectrumSettingsDlg::OnChangeSpectrometer(WPARAM wParam, LPARAM lParam){
	CList <CString, CString&> spectrometers;

	m_Spectrometer->GetConnectedSpecs(spectrometers);
	
	int nFoundSpecs = spectrometers.GetCount();
	// build the list 
	m_comboSpecs.ResetContent();
	POSITION p = spectrometers.GetHeadPosition();
	while(p != NULL){
		m_comboSpecs.AddString(spectrometers.GetNext(p));
	}

	// also select the currently used one
	if(m_Spectrometer->m_spectrometerIndex >= 0)
		m_comboSpecs.SetCurSel(m_Spectrometer->m_spectrometerIndex);
		
	// Not least, set the channel to use
	this->m_channel = m_Spectrometer->m_spectrometerChannel;

	UpdateData(FALSE);

	return 0;
}

/** Called when the user has changed the spectrometer to use */
void CSpectrumSettingsDlg::OnUserChangeSpectrometer(){
	UpdateData(TRUE);

	int curSel = m_comboSpecs.GetCurSel();
	if(curSel == -1 || (curSel == m_Spectrometer->m_spectrometerIndex && m_channel == m_Spectrometer->m_spectrometerChannel))
		return;

	// change the spectrometer to use...
	m_Spectrometer->ChangeSpectrometer(curSel, m_channel);
}
