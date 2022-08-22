
// AvantesTestDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "AvantesTest.h"
#include "AvantesTestDlg.h"
#include "afxdialogex.h"
#include "AvantesSpectrometerInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAvantesTestDlg dialog



CAvantesTestDlg::CAvantesTestDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_AVANTESTEST_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAvantesTestDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_SPECTROMETERS, m_spectrometerCombo);
    DDX_Control(pDX, IDC_LIST_SPECTRUM, m_spectrumList);
    DDX_Control(pDX, IDC_EDIT_SPECTRA_TO_AVERAGE, m_numSpectraEdit);
    DDX_Control(pDX, IDC_EDIT_INTEGRATION_TIME, m_integrationTimeEdit);
}

BEGIN_MESSAGE_MAP(CAvantesTestDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_SEARCH_FOR_DEVICES, &CAvantesTestDlg::OnBnClickedSearchForDevices)
    ON_BN_CLICKED(IDC_BUTTON_ACQUIRE_SPECTRA, &CAvantesTestDlg::OnBnClickedAcquireSpectra)
    ON_CBN_SELCHANGE(IDC_COMBO_SPECTROMETERS, &CAvantesTestDlg::OnCbnSelchangeComboSpectrometers)
    ON_EN_CHANGE(IDC_EDIT_SPECTRA_TO_AVERAGE, &CAvantesTestDlg::OnChangeSpectraToAverage)
    ON_EN_CHANGE(IDC_EDIT_INTEGRATION_TIME, &CAvantesTestDlg::OnChangeIntegrationTime)
END_MESSAGE_MAP()


// CAvantesTestDlg message handlers

BOOL CAvantesTestDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // TODO: Add extra initialization here

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAvantesTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAvantesTestDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAvantesTestDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CAvantesTestDlg::OnBnClickedSearchForDevices()
{
    if (m_spectrometer != nullptr)
    {
        delete m_spectrometer;
    }

    m_spectrometer = new mobiledoas::AvantesSpectrometerInterface();
    const auto spectrometerSerials = m_spectrometer->ScanForDevices();

    // Update the UI
    m_spectrometerCombo.ResetContent();
    for (const auto& serial : spectrometerSerials)
    {
        CString str;
        str.Format("%s", serial.c_str());
        m_spectrometerCombo.AddString(str);
    }
    m_spectrometerCombo.SetCurSel(-1);
    m_spectrometerCombo.SetCurSel(0);
}

void CAvantesTestDlg::OnCbnSelchangeComboSpectrometers()
{
    if (m_spectrometer == nullptr)
    {
        return;
    }

    int index = m_spectrometerCombo.GetCurSel();

    if (index < 0)
    {
        m_spectrometer->Close();
        return;
    }

    if (!m_spectrometer->SetSpectrometer(index))
    {
        SetDlgItemText(IDC_STATIC_LASTERROR, m_spectrometer->GetLastError().c_str());
        return;
    }

    // Update the parameters from the device
    CString text;
    text.Format("%d", m_spectrometer->GetIntegrationTime() / 1000); // us to ms
    SetDlgItemText(IDC_EDIT_INTEGRATION_TIME, text);

    text.Format("%d", m_spectrometer->GetScansToAverage());
    SetDlgItemText(IDC_EDIT_SPECTRA_TO_AVERAGE, text);

    SetDlgItemText(IDC_STATIC_LASTERROR, m_spectrometer->GetLastError().c_str());
}

void CAvantesTestDlg::OnBnClickedAcquireSpectra()
{
    if (m_spectrometer == nullptr)
    {
        return;
    }

    std::vector<std::vector<double>> data;
    int spectrumLength = m_spectrometer->GetNextSpectrum(data);

    if (spectrumLength == 0)
    {
        MessageBox("Failed to read out spectrum", "Error");
        return;
    }

    m_spectrumList.ResetContent();
    std::vector<double>& spectrum = data[0];
    CString valueStr;
    for (double value : spectrum)
    {
        valueStr.Format("%lf", value);
        m_spectrumList.AddString(valueStr);
    }

    SetDlgItemText(IDC_STATIC_LASTERROR, m_spectrometer->GetLastError().c_str());
}

void CAvantesTestDlg::OnChangeSpectraToAverage()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialogEx::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    UpdateData(TRUE); // Get the values from the dialog

    CString editText;
    m_numSpectraEdit.GetWindowTextA(editText);

    int numberOfSpectraToAverage = std::atoi((LPCSTR)editText);
    if (numberOfSpectraToAverage <= 0)
    {
        MessageBox("The number of spectra to average must be at least one.");
        m_numSpectraEdit.SetWindowTextA("1");
        return;
    }

    m_spectrometer->SetScansToAverage(numberOfSpectraToAverage);
}


void CAvantesTestDlg::OnChangeIntegrationTime()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialogEx::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    UpdateData(TRUE); // Get the values from the dialog

    CString editText;
    m_integrationTimeEdit.GetWindowTextA(editText);

    int interationTime = std::atoi((LPCSTR)editText);
    if (interationTime <= 0)
    {
        MessageBox("The integration time must be at least one ms.");
        m_integrationTimeEdit.SetWindowTextA("1");
        return;
    }

    m_spectrometer->SetIntegrationTime(interationTime * 1000); // ms to us
}
