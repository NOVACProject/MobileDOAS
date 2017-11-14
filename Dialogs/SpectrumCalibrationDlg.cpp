// SpectrumCalibrationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../DMSpec.h"
#include "SpectrumCalibrationDlg.h"
#include "../BasicMath.h"

using namespace Dialogs;

// CSpectrumCalibrationDlg dialog

IMPLEMENT_DYNAMIC(CSpectrumCalibrationDlg, CDialog)
CSpectrumCalibrationDlg::CSpectrumCalibrationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpectrumCalibrationDlg::IDD, pParent)
{
	memset(m_pixels, 0, 256 * sizeof(double));
	memset(m_wavelengths, 0, 256 * sizeof(double));
	memset(m_intensities, 0, 256 * sizeof(double));
	m_pointNum = 0;
}

CSpectrumCalibrationDlg::~CSpectrumCalibrationDlg()
{
	m_pointNum = 0;
}

void CSpectrumCalibrationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PEAKLIST_FRAME, m_hglistFrame);
	DDX_Control(pDX, IDC_FIT_GRAPH_FRAME, m_graphFrame);
}


BEGIN_MESSAGE_MAP(CSpectrumCalibrationDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_USE_CURRENT_SPECTRUM, OnUseCurrentSpectrum)
	ON_BN_CLICKED(IDC_BUTTON_FIT_POLYNOMIAL, OnBnClickedFitPolynomial)
END_MESSAGE_MAP()


// CSpectrumCalibrationDlg message handlers
BOOL CSpectrumCalibrationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rect;

	// Initialize the graph	
	int margin = 2;
	m_graphFrame.GetWindowRect(&rect);
	rect.bottom -= rect.top + margin;
	rect.right  -= rect.left + margin;
	rect.top    =  margin + 7;
	rect.left   =  margin;
	m_graph.Create(WS_VISIBLE | WS_CHILD, rect, &m_graphFrame);
	m_graph.SetRange(0,500,1,-100.0,100.0,1);
	m_graph.SetYUnits("Intensity [Counts]") ;
	m_graph.SetXUnits("Pixel number") ;
	m_graph.SetBackgroundColor(RGB(0, 0, 0)) ;
	m_graph.SetGridColor(RGB(255,255,255));
	m_graph.CleanPlot();

	// Initialize the list of peaks
	this->m_hglistFrame.GetWindowRect(rect);
	int height = rect.bottom - rect.top;
	int width  = rect.right - rect.left;
	rect.top = 20; rect.bottom = height - 10;
	rect.left = 10; rect.right = width - 10;  
	int columnWidth0 = (int)(0.4*(rect.right - rect.left));
	int columnWidth1 = (int)(0.6*(rect.right - rect.left));

	m_lineList.Create(WS_VISIBLE|WS_BORDER|LVS_REPORT, rect, &m_hglistFrame, 65536);
	m_lineList.InsertColumn(0, "Pixel", LVCFMT_LEFT, columnWidth0);
	m_lineList.InsertColumn(1, "Wavelength [nm]", LVCFMT_LEFT, columnWidth1);

	
	return 0;
}


void CSpectrumCalibrationDlg::OnUseCurrentSpectrum()
{
	for(unsigned int j = 0; j < m_Spectrometer->m_calibration->m_lineNum; ++j){
		if(m_Spectrometer->m_calibration->m_lines[j].use){
			m_pixels[m_pointNum]		= m_Spectrometer->m_calibration->m_lines[j].pixelNumber;
			m_wavelengths[m_pointNum]	= m_Spectrometer->m_calibration->m_lines[j].wavelength;
			m_intensities[m_pointNum]	= m_Spectrometer->m_calibration->m_lines[j].maxIntensity;
			++m_pointNum;
		}
	}
	
	SortLines();
	
	PopulateLineList();
}

void CSpectrumCalibrationDlg::SortLines(){
	double eps = 1e-5;
	double newPixels[256]; // local copy of m_pixels
	double newWavelengths[256]; // local copy of m_wavelengths
	double newIntensities[256]; // local copy of intensities
	int newPointNum = 0;
	int indices[256];
	
	// Sort the list (on wavelength) while searching for duplicates (in wavelength)
	double minLambda = 1.0;
	while(1){
		minLambda = 1e99;
		double pixelValue = 0.0;
		int nRepetitions = 0;

		for(unsigned int j = 0; j < m_pointNum; ++j){
			if(m_wavelengths[j] < minLambda){
				minLambda	= m_wavelengths[j];
				pixelValue	= m_pixels[j];
				nRepetitions= 0;
				indices[0]	= j;
			}else if(fabs(m_wavelengths[j] - minLambda) < eps){
				pixelValue				+= m_pixels[j];
				nRepetitions++;
				indices[nRepetitions]	= j;
			}
		}
		if(minLambda > 1e98)
			break;
		
		newWavelengths[newPointNum] = minLambda;
		newPixels[newPointNum] = pixelValue / (nRepetitions + 1);
		for(int j = 0; j < nRepetitions + 1; ++j){
			m_wavelengths[indices[j]] = 1e99;
		}
		++newPointNum;
	}
	
	// copy the new data back to the original arrays
	memcpy(this->m_wavelengths, newWavelengths, sizeof(double) * newPointNum);
	memcpy(this->m_pixels,		newPixels,		sizeof(double) * newPointNum);
	this->m_pointNum = newPointNum;
	
}

void CSpectrumCalibrationDlg::OnBnClickedFitPolynomial()
{
	CBasicMath mathObj;
	int order = 3;
	double coefficients[4];
	CString polyStr;
	double errorInFit[256];

	// Make the polynomial fit.
	mathObj.PolynomialFit(m_pixels, m_pointNum, m_wavelengths, coefficients, order);

	// Show the user the polynomial
	polyStr.Format("%.3lf + %.3g * x + %.3g * x² + %.3g * x³", 
		coefficients[0], coefficients[1], coefficients[2], coefficients[3]);
	this->SetDlgItemText(IDC_LABEL_POLYNOMIAL, polyStr);

	// Draw the error in the fit...
	for(unsigned int j = 0; j < m_pointNum; ++j){
		double x		= m_pixels[j];
		double polyVal	= coefficients[0] + x * coefficients[1] + x * x * coefficients[2] + x * x * x * coefficients[3];
		errorInFit[j]	= polyVal - m_wavelengths[j];
	}
	m_graph.XYPlot(m_pixels, errorInFit, m_pointNum, Graph::CGraphCtrl::PLOT_CIRCLES);
}

void CSpectrumCalibrationDlg::PopulateLineList(){
	CString pixelStr, wavelengthStr;

	m_lineList.DeleteAllItems();
	
	for(unsigned int j = 0; j < m_pointNum; ++j){
		pixelStr.Format("%.3lf", m_pixels[j]);
		wavelengthStr.Format("%.3lf", m_wavelengths[j]);
	
		m_lineList.InsertItem(j, pixelStr);
		m_lineList.SetItemText(j, 1, wavelengthStr);
	}
}