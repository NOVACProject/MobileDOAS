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

CSpectrometerCalibrationDlg::CSpectrometerCalibrationDlg(novac::ILogger& log)
    : CPropertySheet()
{
    m_calibrateInstrumentLineShape = new CCalibrateInstrumentLineShape();
    m_calibrateInstrumentLineShape->Construct(IDD_CALIBRATE_LINESHAPE_DIALOG);

    m_calibratePixelToWavelength = new CCalibratePixelToWavelengthDialog(log);
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
    delete m_calibrateReferences;
}

void CSpectrometerCalibrationDlg::DoDataExchange(CDataExchange* pDX)
{
    CPropertySheet::DoDataExchange(pDX);
}

BOOL CSpectrometerCalibrationDlg::OnInitDialog()
{
    BOOL bResult = CPropertySheet::OnInitDialog();

    // ------------ Get the buttons ---------------
    CWnd* pApply = this->GetDlgItem(ID_APPLY_NOW);
    CWnd* pCancel = this->GetDlgItem(IDCANCEL);
    CWnd* pOk = this->GetDlgItem(IDOK);

    // Remove each of the buttons
    if (pApply) {
        pApply->DestroyWindow();
    }
    if (pCancel) {
        pCancel->DestroyWindow();
    }
    if (pOk) {
        pOk->DestroyWindow();
    }

    return bResult;
}

BEGIN_MESSAGE_MAP(CSpectrometerCalibrationDlg, CPropertySheet)
END_MESSAGE_MAP()


// CSpectrometerCalibrationDlg message handlers
