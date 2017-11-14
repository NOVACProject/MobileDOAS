#include "stdafx.h"
#include "../DMSpec.h"
#include "ReEval_ScriptDlg.h"
#include "../Common.h"

using namespace ReEvaluation;

void ScanDirectoryForEvalLogs(const CString &folderName, CList <CString, CString &> &fileList);

IMPLEMENT_DYNAMIC(CReEval_ScriptDlg, CDialog)
CReEval_ScriptDlg::CReEval_ScriptDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CReEval_ScriptDlg::IDD, pParent)
{
}

CReEval_ScriptDlg::~CReEval_ScriptDlg()
{
}

void CReEval_ScriptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	
	DDX_Text(pDX, IDC_EDIT_SETTINGSFILE, m_settingsFile);
	
	DDX_Control(pDX, IDC_BUTTON_INSERTEVALLOG, m_btnInsertEvalLog);
	DDX_Control(pDX, IDC_BUTTON_REMOVEEVALLOG, m_btnRemoveEvalLog);
	DDX_Control(pDX, IDC_BUTTON_SCANFORLOGFILES, m_btnScanEvalLogs);
	
	DDX_Control(pDX, ID_SAVESCRIPT,            m_btnSaveScript);
	
	DDX_Control(pDX, IDC_COMBO_MAX_N_THREADS,  m_comboThreadNum);
	
	DDX_Control(pDX, IDC_LIST_EVALUATIONLOGS,  m_listEvalLogs);
}


BEGIN_MESSAGE_MAP(CReEval_ScriptDlg, CDialog)
	ON_BN_CLICKED(IDC_BROWSE_SETTINGSFILE,		OnBrowseSettingsFile)
	ON_BN_CLICKED(ID_SAVESCRIPT,				OnSaveScript)
	ON_BN_CLICKED(IDC_BUTTON_INSERTEVALLOG,		OnBrowseEvallog)
	ON_BN_CLICKED(IDC_BUTTON_REMOVEEVALLOG,		OnRemoveEvallog)
	ON_BN_CLICKED(IDC_BUTTON_SCANFORLOGFILES,	OnScanForEvallogs)
	ON_BN_CLICKED(ID_FILE_LOADSCRIPT,			OnMenuLoadScript)
	ON_BN_CLICKED(ID_FILE_SAVESCRIPT,			OnMenuSaveScript)
	
	ON_CBN_SELCHANGE(IDC_COMBO_MAX_N_THREADS,	OnChangeMaxThreads)
END_MESSAGE_MAP()

BOOL ReEvaluation::CReEval_ScriptDlg::OnInitDialog()
{
	BOOL bResult = CDialog::OnInitDialog();

	UpdateControls();
	m_comboThreadNum.SetCurSel(0);

	return bResult;
}


// CReEval_ScriptDlg message handlers

void CReEval_ScriptDlg::OnBrowseSettingsFile(){
	m_settingsFile.Format("");

	TCHAR filter[512];
	int n = _stprintf(filter, "ReEvaluation settings\0");
	n += _stprintf(filter + n + 1, "*.rxml;\0");
	filter[n + 2] = 0;
	
	// Browse for the file
	if(Common::BrowseForFile(filter, m_settingsFile)){
		UpdateControls();

		// Update the settings file in all the jobs that are already
		//  configured
		POSITION pos = m_script.m_jobs.GetHeadPosition();
		while(pos != NULL){
			ReEvaluation::CReEvaluation_Script::Job &job = m_script.m_jobs.GetAt(pos);
			job.settingsFile.Format(m_settingsFile);
			m_script.m_jobs.SetAt(pos, job);
			m_script.m_jobs.GetNext(pos);
		}
	}
}

void CReEval_ScriptDlg::OnSaveScript(){
	OnMenuSaveScript();
	CDialog::OnOK();
}

void CReEval_ScriptDlg::OnBrowseEvallog(){
	CString evLog;
	evLog.Format("");

	TCHAR filter[512];
	int n = _stprintf(filter, "Evaluation logs\0");
	n += _stprintf(filter + n + 1, "*.txt;\0");
	filter[n + 2] = 0;
	
	// Browse for the file
	if(Common::BrowseForFile(filter, evLog)){
		CReEvaluation_Script::Job j;
		j.evaluationLog.Format(evLog);
		j.settingsFile.Format(m_settingsFile);	
		
		m_script.m_jobs.AddTail(j);
		
		UpdateEvalLogList();
	}
}

void CReEval_ScriptDlg::OnRemoveEvallog(){
	int index = 0;

	// get the currently selected evaluation log
	int cursel = m_listEvalLogs.GetCurSel();
	if(cursel < 0)
		return; // no log file selected
	
	// Remove the log file from the list
	POSITION pos = m_script.m_jobs.GetHeadPosition();
	while(index < cursel){
		m_script.m_jobs.GetNext(pos);
		++index;
	
		if(pos == NULL)
			return;	 // shouldn't happen...
	}
	m_script.m_jobs.RemoveAt(pos);
	
	// Update the list
	UpdateEvalLogList();
}

void CReEval_ScriptDlg::OnScanForEvallogs(){
	CString folderName;
	CList <CString, CString &> fileList;

	// Start in the same folder as the settings-file is located
	folderName.Format(m_settingsFile);
	Common::GetDirectory(folderName);

	// let the user select a directory
	if(Common::BrowseForDirectory(folderName)){

		// look for evaluation-log files in the given directory
		ScanDirectoryForEvalLogs(folderName, fileList);
		
		// Add the found evaluation log files to the list of jobs
		POSITION pos = fileList.GetHeadPosition();
		while(pos != NULL){
			ReEvaluation::CReEvaluation_Script::Job j;

			CString &str = fileList.GetNext(pos);
			
			j.evaluationLog.Format(str);
			j.settingsFile.Format(m_settingsFile);

			m_script.m_jobs.AddTail(j);
		}
		UpdateControls();
		UpdateEvalLogList();
	}
}

void CReEval_ScriptDlg::OnMenuLoadScript(){
	CString fileName;
	fileName.Format("");

	TCHAR filter[512];
	int n = _stprintf(filter, "ReEvaluation script\0");
	n += _stprintf(filter + n + 1, "*.rs;\0");
	filter[n + 2] = 0;
	
	// Browse for the file
	if(Common::BrowseForFile(filter, fileName)){
		// Read the script
		m_script.ReadFromFile(fileName);
		
		// take the settings file in the first job as the main settings file
		if(m_script.m_jobs.GetCount() > 0){
			ReEvaluation::CReEvaluation_Script::Job &j = m_script.m_jobs.GetHead();
			m_settingsFile.Format(j.settingsFile);
		}
		
		// Update the dialog
		UpdateControls();
		UpdateEvalLogList();
	}
}

void CReEval_ScriptDlg::OnMenuSaveScript(){
	// Get the contents of the dialog
	UpdateData(TRUE);

	CString fileName;
	fileName.Format("");

	TCHAR filter[512];
	int n = _stprintf(filter, "ReEvaluation script\0");
	n += _stprintf(filter + n + 1, "*.rs;\0");
	filter[n + 2] = 0;
	
	// Browse for the file
	if(Common::BrowseForFile_SaveAs(filter, fileName)){
		// Add the file-ending .rs if it's not already there
		if(!Equals(fileName.Right(3), ".rs")){
			fileName.AppendFormat(".rs");
		}

		// Write the script
		m_script.WriteToFile(fileName);
	}
}

void CReEval_ScriptDlg::OnChangeMaxThreads(){
	m_script.m_maxThreadNum = 1 + max(0, m_comboThreadNum.GetCurSel());
}

void CReEval_ScriptDlg::UpdateControls(){
	UpdateData(FALSE);

	if(m_settingsFile.GetLength() <= 0){
		m_btnInsertEvalLog.EnableWindow(FALSE);
		m_btnRemoveEvalLog.EnableWindow(FALSE);
		m_btnScanEvalLogs.EnableWindow(FALSE);
		m_btnSaveScript.EnableWindow(FALSE);
	}else{
		m_btnInsertEvalLog.EnableWindow(TRUE);
		m_btnRemoveEvalLog.EnableWindow(TRUE);
		m_btnScanEvalLogs.EnableWindow(TRUE);
		m_btnSaveScript.EnableWindow(TRUE);
	}
}

void CReEval_ScriptDlg::UpdateEvalLogList(){
	m_listEvalLogs.ResetContent();

	POSITION pos = m_script.m_jobs.GetHeadPosition();
	
	while(pos != NULL){
		ReEvaluation::CReEvaluation_Script::Job &j = m_script.m_jobs.GetNext(pos);

		m_listEvalLogs.AddString(j.evaluationLog);
	}
	
	// Find the longest string in the list box.
	CString      str;
	CSize      sz;
	int      dx = 0;
	TEXTMETRIC   tm;
	CDC*      pDC = m_listEvalLogs.GetDC();
	CFont*      pFont = m_listEvalLogs.GetFont();

	// Select the listbox font, save the old font
	CFont* pOldFont = pDC->SelectObject(pFont);
	// Get the text metrics for avg char width
	pDC->GetTextMetrics(&tm); 

	for (int i = 0; i < m_listEvalLogs.GetCount(); i++)
	{
		m_listEvalLogs.GetText(i, str);
		sz = pDC->GetTextExtent(str);

		// Add the avg width to prevent clipping
		sz.cx += tm.tmAveCharWidth;

		if (sz.cx > dx)
		dx = sz.cx;
	}
	// Select the old font back into the DC
	pDC->SelectObject(pOldFont);
	m_listEvalLogs.ReleaseDC(pDC);

	// Set the horizontal extent so every character of all strings can be scrolled to.
	m_listEvalLogs.SetHorizontalExtent(dx);
}


void ScanDirectoryForEvalLogs(const CString &folderName, CList <CString, CString &> &fileList){
	WIN32_FIND_DATA FindFileData;
	char fileToFind[MAX_PATH];
	CString fileName;

	// --------------------------------------
	// 1.  Search for all directories
	// --------------------------------------

	sprintf(fileToFind, "%s\\*", folderName);
	HANDLE hFile = FindFirstFile(fileToFind, &FindFileData);

	if(hFile == INVALID_HANDLE_VALUE)
		return; // no files found
	do{
		if(Equals(FindFileData.cFileName, ".") || Equals(FindFileData.cFileName, "..")){
			continue;
		}
		
		// Get the name of the file that we've just found...
		fileName.Format("%s\\%s", folderName, FindFileData.cFileName);

		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			// This is a directory that we've found. Enter it..
			ScanDirectoryForEvalLogs(fileName, fileList);
		}
	}while(0 != FindNextFile(hFile, &FindFileData));
	FindClose(hFile);
	
	// --------------------------------------
	// 2. Search for all evaluation log files 
	// --------------------------------------

	sprintf(fileToFind, "%s\\*EvaluationLog*.txt", folderName);
	hFile = FindFirstFile(fileToFind, &FindFileData);

	if(hFile == INVALID_HANDLE_VALUE)
		return; // no files found
	do{
		// don't include ReEvaluation logs
		if(strstr(FindFileData.cFileName, "ReEvaluation"))
			continue;
	
		// Get the name of the file that we've just found...
		fileName.Format("%s\\%s", folderName, FindFileData.cFileName);

		fileList.AddTail(fileName);

	}while(0 != FindNextFile(hFile, &FindFileData));
	FindClose(hFile);
	
	return;
}