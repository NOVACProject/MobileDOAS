// CCalibrateInstrumentLineShape.cpp : implementation file
//

#include "stdafx.h"
#include "CCalibrateInstrumentLineShape.h"
#include "afxdialogex.h"
#include "resource.h"
#include "Common.h"

// CCalibrateInstrumentLineShape dialog

IMPLEMENT_DYNAMIC(CCalibrateInstrumentLineShape, CPropertyPage)

CCalibrateInstrumentLineShape::CCalibrateInstrumentLineShape(CWnd* pParent /*=nullptr*/)
	: CPropertyPage(IDD_CALIBRATE_LINESHAPE_DIALOG)
    , m_inputSpectrum(_T(""))
{

}

CCalibrateInstrumentLineShape::~CCalibrateInstrumentLineShape()
{
}

void CCalibrateInstrumentLineShape::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_SPECTRUM, m_inputSpectrum);
}


BEGIN_MESSAGE_MAP(CCalibrateInstrumentLineShape, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SPECTRUM, &CCalibrateInstrumentLineShape::OnBnClickedButtonBrowseSpectrum)
END_MESSAGE_MAP()


// CCalibrateInstrumentLineShape message handlers


void CCalibrateInstrumentLineShape::OnBnClickedButtonBrowseSpectrum()
{
    Common common;
    if (!common.BrowseForFile("Spectrum Files\0*.std\0", m_inputSpectrum))
    {
        return;
    }
    UpdateData(FALSE);
}
