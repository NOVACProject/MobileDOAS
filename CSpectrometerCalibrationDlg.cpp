// CSpectrometerCalibrationDlg.cpp : implementation file
//
#include "StdAfx.h"
#include "CSpectrometerCalibrationDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include "CCalibrateInstrumentLineShape.h"
#include "CCalibratePixelToWavelengthDialog.h"
#include "CCalibrateReferencesDialog.h"

// CSpectrometerCalibrationDlg dialog

CSpectrometerCalibrationDlg::CSpectrometerCalibrationDlg()
    : CPropertySheet()
{
    m_calibrateInstrumentLineShape = new CCalibrateInstrumentLineShape();
    m_calibrateInstrumentLineShape->Construct(IDD_CALIBRATE_LINESHAPE_DIALOG);

    m_calibratePixelToWavelength = new CCalibratePixelToWavelengthDialog();
    m_calibratePixelToWavelength->Construct(IDD_CALIBRATE_WAVELENGTH_DIALOG);

    m_calibrateReferences = new CCalibrateReferencesDialog();
    m_calibrateReferences->Construct(IDD_CALIBRATE_REFERENCES);

    AddPage(m_calibrateInstrumentLineShape);
    AddPage(m_calibratePixelToWavelength);
    AddPage(m_calibrateReferences);
}

CSpectrometerCalibrationDlg::~CSpectrometerCalibrationDlg()
{
    delete m_calibratePixelToWavelength;
    delete m_calibrateInstrumentLineShape;
}

void CSpectrometerCalibrationDlg::DoDataExchange(CDataExchange* pDX)
{
    CPropertySheet::DoDataExchange(pDX);
}

BOOL CSpectrometerCalibrationDlg::OnInitDialog()
{
    BOOL bResult = CPropertySheet::OnInitDialog();

    CRect rectAppl, rectCancel, rectWindow;

    // Add the menu
    // CMenu* pMenu = new CMenu();
    // pMenu->LoadMenu(IDR_REEVAL_DLG_MENU);
    // this->SetMenu(pMenu);

    // Make the window a little bit bigger, this is needed since
    //	adding the menu will destry the layout...
    GetWindowRect(rectWindow);
    rectWindow.bottom = rectWindow.bottom + 20;
    MoveWindow(rectWindow);


    // ------------ Get the buttons ---------------
    CWnd* pApply = this->GetDlgItem(ID_APPLY_NOW);
    CWnd* pCancel = this->GetDlgItem(IDCANCEL);
    CWnd* pOk = this->GetDlgItem(IDOK);

    // Get the position of the 'Apply'-button, and then remove it
    if (pApply) {
        pApply->GetWindowRect(rectAppl);
        ScreenToClient(rectAppl);
        pApply->DestroyWindow();
    }

    // Get the position of the 'Cancel'-button and then remove it
    if (pCancel) {
        pCancel->GetWindowRect(rectCancel);
        ScreenToClient(rectCancel);
        pCancel->DestroyWindow();
    }

    // Change the 'OK'-button to a 'Save'-button and move it to where
    //	the 'apply'-button was
    if (pOk) {
        pOk->SetWindowText("Close");
        pOk->MoveWindow(rectAppl);
    }

    return bResult;
}

BEGIN_MESSAGE_MAP(CSpectrometerCalibrationDlg, CPropertySheet)
END_MESSAGE_MAP()


// CSpectrometerCalibrationDlg message handlers
