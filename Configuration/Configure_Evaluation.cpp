#include "stdafx.h"
#include "../DMSpec.h"
#include "Configure_Evaluation.h"
#include "../Dialogs/ReferencePropertiesDlg.h"
#include "../Dialogs/ReferencePlotDlg.h"
#include <vector>

using namespace Configuration;
using namespace Evaluation;

// CConfigure_Evaluation dialog

IMPLEMENT_DYNAMIC(CConfigure_Evaluation, CPropertyPage)
CConfigure_Evaluation::CConfigure_Evaluation()
	: CPropertyPage(CConfigure_Evaluation::IDD)
{
	m_conf = nullptr;
}

CConfigure_Evaluation::~CConfigure_Evaluation()
{
	m_conf = nullptr;
}

void CConfigure_Evaluation::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	// The fit-parameters edit boxes
	DDX_Text(pDX,	IDC_EDIT_FITFROM,				this->m_fitLow);
	DDX_Text(pDX,	IDC_EDIT_FITTO,					this->m_fitHigh);
	DDX_Text(pDX,	IDC_REEVAL_WINDOW_POLYNOM2,		this->m_polyOrder);
	DDX_CBIndex(pDX,	IDC_COMBO_CHANNEL,			this->m_fitChannel);

	DDX_Control(pDX,	IDC_REFERENCE_FRAME,		m_referenceFrame);
	DDX_Control(pDX,	IDC_REEVAL_FITWINDOWLIST,	m_windowList);
}

BEGIN_MESSAGE_MAP(CConfigure_Evaluation, CPropertyPage)
	// the user has selected another fit window
	ON_LBN_SELCHANGE(IDC_REEVAL_FITWINDOWLIST,	OnChangeFitWindow)
	
	// The user has pressed the 'insert' item on the reference-grid context menu
	ON_COMMAND(ID__INSERT,						OnInsertReference)
	ON_BN_CLICKED(IDC_INSERT_REFERENCE,			OnInsertReference)

	// The user has pressed the 'remove' item on the reference-grid context menu
	ON_COMMAND(ID__REMOVE,						OnRemoveReference)
	ON_BN_CLICKED(IDC_REMOVE_REFERENCE,			OnRemoveReference)

	// The user wants to see the properties or graph of the references
	ON_BN_CLICKED(IDC_REFERENCE_PROPERTIES,		OnShowProperties)
	ON_BN_CLICKED(IDC_REFERENCE_VIEW,			OnShowReferenceGraph)

	ON_EN_KILLFOCUS(IDC_EDIT_FITFROM,	SaveData)
	ON_EN_KILLFOCUS(IDC_EDIT_FITTO,	SaveData)
	ON_EN_KILLFOCUS(IDC_REEVAL_WINDOW_POLYNOM2,	SaveData)
	ON_LBN_SELCHANGE(IDC_COMBO_CHANNEL,			SaveData)
END_MESSAGE_MAP()


// CConfigure_Evaluation message handlers

// CConfigure_Spectrometer message handlers
BOOL CConfigure_Evaluation::OnInitDialog(){
	CPropertyPage::OnInitDialog();

	// Populate the fit window - list 
	m_windowList.m_conf = this->m_conf;
	m_windowList.PopulateList();
	OnChangeFitWindow(); // Update

	// Initialize the reference grid control
	InitReferenceFileControl();

	// Populate the reference grid control
	PopulateReferenceFileControl();

	// setup the tool tips
	InitToolTips();

	return TRUE;
}

void CConfigure_Evaluation::InitReferenceFileControl(){

	// Get the dimensions of the reference frame
	CRect rect;
	m_referenceFrame.GetWindowRect(&rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	rect.top = 20;
	rect.left = 10;
	rect.right = width - 20;
	rect.bottom = height - 10;

	// Create the grid
	m_referenceGrid.Create(rect, &m_referenceFrame, 0);
	m_referenceGrid.parent = this;

	// Set the columns of the grid
	m_referenceGrid.InsertColumn("Name");
	m_referenceGrid.SetColumnWidth(0, (int)(rect.right / 9));
	m_referenceGrid.InsertColumn("Reference File");
	m_referenceGrid.SetColumnWidth(1, (int)(rect.right * 5 / 8));
	m_referenceGrid.InsertColumn("Shift");
	m_referenceGrid.SetColumnWidth(2, (int)(rect.right / 6));
	m_referenceGrid.InsertColumn("Squeeze");
	m_referenceGrid.SetColumnWidth(3, (int)(rect.right / 6));

	// Makes sure that the user cannot edit the titles of the grid
	m_referenceGrid.SetFixedRowCount(1);

	// make sure the user can edit items in the grid
	//m_referenceGrid.SetEditable(TRUE); 

	// Disable the small title tips
	m_referenceGrid.EnableTitleTips(FALSE);
}

void CConfigure_Evaluation::PopulateReferenceFileControl(){
	int curSel = m_windowList.GetCurSel();
	if(curSel == -1){
		m_windowList.SetCurSel(0);
		curSel = 0;
	}

	m_referenceGrid.DeleteNonFixedRows();

	Evaluation::CFitWindow &window	= m_conf->m_fitWindow[curSel];
	m_referenceGrid.m_window				= &m_conf->m_fitWindow[curSel];


	// make sure that there's always one empty line at the end
	m_referenceGrid.SetRowCount(window.nRef + 2);

	//// if there's no references defined
	//if(window.nRef == 0){
	//	m_btnRefProp.EnableWindow(FALSE);
	//	m_btnRemoveRef.EnableWindow(FALSE);
	//}else{
	//	m_btnRefProp.EnableWindow(TRUE);
	//	m_btnRemoveRef.EnableWindow(TRUE);
	//}
	int i;
	for(i = 0; i < window.nRef; i++){

		CReferenceFile &ref = window.ref[i];

		m_referenceGrid.SetItemTextFmt(1 + i, 0, ref.m_specieName);
		m_referenceGrid.SetItemTextFmt(1 + i, 1, ref.m_path);

		if(ref.m_shiftOption == SHIFT_FREE)
		m_referenceGrid.SetItemTextFmt(1 + i, 2, "free");

		if(ref.m_shiftOption == SHIFT_FIX)
		m_referenceGrid.SetItemTextFmt(1 + i, 2, "fixed to %.2lf", ref.m_shiftValue);

		if(ref.m_shiftOption == SHIFT_LINK)
		m_referenceGrid.SetItemTextFmt(1 + i, 2, "linked to %.2lf", ref.m_shiftValue);

		//if(ref.m_shiftOption == SHIFT_LIMIT)
		//  m_referenceGrid.SetItemTextFmt(1 + i, 2, "limited to %.2lf", ref.m_shiftValue);

		if(ref.m_squeezeOption == SHIFT_FREE)
		m_referenceGrid.SetItemTextFmt(1 + i, 3, "free");

		if(ref.m_squeezeOption == SHIFT_FIX)
		m_referenceGrid.SetItemTextFmt(1 + i, 3, "fixed to %.2lf", ref.m_squeezeValue);

		if(ref.m_squeezeOption == SHIFT_LINK)
		m_referenceGrid.SetItemTextFmt(1 + i, 3, "linked to %.2lf", ref.m_squeezeValue);

		//if(ref.m_squeezeOption == SHIFT_LIMIT)
	//    m_referenceGrid.SetItemTextFmt(1 + i, 3, "limited to %.2lf", ref.m_squeezeValue);
	}

	// make sure that the last line is clear
	m_referenceGrid.SetItemTextFmt(1 + i, 0, "");
	m_referenceGrid.SetItemTextFmt(1 + i, 1, "");
	m_referenceGrid.SetItemTextFmt(1 + i, 2, "");
	m_referenceGrid.SetItemTextFmt(1 + i, 3, "");

	// Update the grid
	if(m_referenceGrid.m_hWnd != nullptr)
		m_referenceGrid.UpdateData(FALSE);
}

/** Called when the user wants to remove a reference file */
void CConfigure_Evaluation::OnRemoveReference(){

	// save the data in the dialog
	UpdateData(TRUE);

	// Get the currently selected fit window
	int curSel = m_windowList.GetCurSel();
	if (curSel < 0)
		return;
	Evaluation::CFitWindow &window = m_conf->m_fitWindow[curSel];

	// if there's no reference file, then there's nothing to remove
	if (window.nRef <= 0)
		return;

	// Get the selected reference files
	CCellRange cellRange = m_referenceGrid.GetSelectedCellRange();

	int minRow = cellRange.GetMinRow() - 1;
	int nRows = cellRange.GetRowSpan();
	
	if (nRows <= 0) {
		return;
	}
	// move every reference file in the list down one step
	for(int i = minRow; i < window.nRef; i++){
		window.ref[i] = window.ref[i+nRows];
	}

	// reduce the number of references by number deleted
	window.nRef -= nRows;

	// Update the reference grid
	PopulateReferenceFileControl();
}

/** Called when the user wants to insert a new reference file */
void CConfigure_Evaluation::OnInsertReference(){

	// save the data in the dialog
	UpdateData(TRUE);

	// Get the currently selected fit window
	int curSel = m_windowList.GetCurSel();
	if(curSel < 0){
		curSel = 0;
		if(m_windowList.GetCount() == 0){
			// insert a new fit-window (this makes the interface much more intuitive...)
			m_conf->m_fitWindow[0].name.Format("NEW");
			m_conf->m_fitWindow[0].nRef = 0;
			m_conf->m_nFitWindows = 1;
		}
	}

	Evaluation::CFitWindow &window = m_conf->m_fitWindow[curSel];

	// Let the user browse for the reference files
	Common common;
	std::vector<CString> filenames = common.BrowseForFiles();
	for (int i = 0; i < filenames.size(); i++) {
		const CString fileName = filenames[i];

		// The user has selected a new reference file, insert it into the list

		// 1. Set the path
		window.ref[window.nRef].m_path.Format("%s", fileName);

		// 2. make a guess of the specie name
		CString specie;
		Common::GuessSpecieName(fileName, specie);
		if (strlen(specie) != 0) {
			window.ref[window.nRef].m_specieName.Format("%s", specie);

			if (Equals(specie, "NO2")) {
				window.ref[window.nRef].m_gasFactor = GASFACTOR_NO2;
			}
			else if (Equals(specie, "O3")) {
				window.ref[window.nRef].m_gasFactor = GASFACTOR_O3;
			}
			else if (Equals(specie, "HCHO")) {
				window.ref[window.nRef].m_gasFactor = GASFACTOR_HCHO;
			}
		}

		// 3. Set the shift and squeeze options for this reference
		if (window.nRef == 0) {
			// If it is the first one, select 'optimal' 
			window.ref[window.nRef].m_shiftOption = Evaluation::SHIFT_FIX;
			window.ref[window.nRef].m_squeezeOption = Evaluation::SHIFT_FIX;
		}
		else {
			window.ref[window.nRef].m_shiftOption = Evaluation::SHIFT_LINK;
			window.ref[window.nRef].m_shiftValue = 0;
			window.ref[window.nRef].m_squeezeOption = Evaluation::SHIFT_LINK;
			window.ref[window.nRef].m_squeezeValue = 1;
		}

		// 4. update the number of references
		window.nRef += 1;

		// If this is the first reference inserted, also make a guess for the window name; if there isn't already a window name
		if (window.nRef == 1 && strlen(specie) != 0 && window.name == "NEW") {
			window.name.Format("%s", specie);
			PopulateWindowList();
		}
	}

	// Update the grid
	PopulateReferenceFileControl();

}

void CConfigure_Evaluation::PopulateWindowList(){
	this->m_windowList.ResetContent(); // clear the list

	for(int i = 0; i < m_conf->m_nFitWindows; ++i){
		m_windowList.AddString(m_conf->m_fitWindow[i].name);
	}

	//m_windowList.SetCurSel(0);
}

void CConfigure_Evaluation::SaveData(){
	UpdateData(TRUE);

	int curSel = this->m_windowList.GetCurSel();
	if(curSel < 0)
		return;

	Evaluation::CFitWindow &window = m_conf->m_fitWindow[curSel];

	window.fitLow		= this->m_fitLow;
	window.fitHigh		= this->m_fitHigh;
	window.polyOrder	= this->m_polyOrder;
	window.channel		= this->m_fitChannel;
}

void CConfigure_Evaluation::OnChangeFitWindow(){
	int curWindow = m_windowList.GetCurSel();
	if(curWindow < 0)
		curWindow = 0;

	// update the reference grid
	m_referenceGrid.m_window = &m_conf->m_fitWindow[curWindow];
	PopulateReferenceFileControl();

	// Update the controls
	this->m_fitHigh		= m_conf->m_fitWindow[curWindow].fitHigh;
	this->m_fitLow		= m_conf->m_fitWindow[curWindow].fitLow;
	this->m_polyOrder	= m_conf->m_fitWindow[curWindow].polyOrder;
	this->m_fitChannel	= m_conf->m_fitWindow[curWindow].channel;

	// Update the rest of the controls on the screen
	UpdateData(FALSE);
}

void CConfigure_Evaluation::InitToolTips(){
	// Don't initialize the tool tips twice
	if(m_toolTip.m_hWnd != nullptr)
		return;

	// Enable the tool tips
	if(!m_toolTip.Create(this)){
		TRACE0("Failed to create tooltip control\n"); 
	}
	//m_toolTip.AddTool(&m_labelVolcano,							IDC_EDIT_VOLCANO);
	//m_toolTip.AddTool(&m_comboVolcano,							IDC_EDIT_VOLCANO);
	//m_toolTip.AddTool(&m_labelSite,									IDC_EDIT_SITE);
	//m_toolTip.AddTool(&m_editSite,									IDC_EDIT_SITE);
	//m_toolTip.AddTool(&m_labelObservatory,					IDC_EDIT_OBSERVATORY);
	////m_toolTip.AddTool(&m_editObservatory,						IDC_EDIT_OBSERVATORY);
	//m_toolTip.AddTool(&m_comboSpectrometerModel,		IDC_COMBO_SPECTROMETERMODEL);
	//m_toolTip.AddTool(&m_comboSpectrometerChannels,	IDC_COMBO_CHANNELS);
	//m_toolTip.AddTool(&m_editSerial,								IDC_EDIT_SERIALNUMBER);

	m_toolTip.SetMaxTipWidth(SHRT_MAX);
	m_toolTip.Activate(TRUE);
}

BOOL CConfigure_Evaluation::PreTranslateMessage(MSG* pMsg){
	m_toolTip.RelayEvent(pMsg);

	return CPropertyPage::PreTranslateMessage(pMsg);
}

/** Called when the user wants to see the
properties of one reference */
void CConfigure_Evaluation::OnShowProperties() {

	// save the data in the dialog
	UpdateData(TRUE);

	// Get the currently selected fit window
	int curSel = m_windowList.GetCurSel();
	if (curSel < 0)
		return;
	Evaluation::CFitWindow &window = m_conf->m_fitWindow[curSel];

	// if there's no reference file, then there's nothing to remove
	if (window.nRef <= 0)
		return;

	// Get the selected reference file
	CCellRange cellRange = m_referenceGrid.GetSelectedCellRange();
	int minRow = cellRange.GetMinRow() - 1;
	int nRows = cellRange.GetRowSpan();

	if (nRows <= 0 || nRows > 1) { /* nothing selected or several lines selected */
		MessageBox("Please select a reference file.", "Properties");
		return;
	}

	// Show the properties dialog
	Dialogs::CReferencePropertiesDlg dlg;
	dlg.m_ref = &window.ref[minRow];
	dlg.DoModal();

	// Update the reference grid
	PopulateReferenceFileControl();
}

/** Called when the user wants to see the 
	size of the references */
void CConfigure_Evaluation::OnShowReferenceGraph(){
	// save the data in the dialog
	UpdateData(TRUE);

	// Get the currently selected fit window
	int curSel = m_windowList.GetCurSel();
	if(curSel < 0)
		return;
	Evaluation::CFitWindow &window = m_conf->m_fitWindow[curSel];

	// if there's no reference file, then there's nothing to show
	if(window.nRef <= 0)
		return;

	// Open the dialog
	Dialogs::CReferencePlotDlg dlg;
	dlg.m_window = &m_conf->m_fitWindow[curSel];
	dlg.DoModal();
}