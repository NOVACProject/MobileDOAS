// Evaluation.cpp: implementation of the CEvaluation class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "DbSpec.h"
#include "Evaluation.h"
#include <iostream>
#include <conio.h>
// include all required fit objects
#include "../Fit/ReferenceSpectrumFunction.h"
#include "../Fit/SimpleDOASFunction.h"
#include "../Fit/StandardMetricFunction.h"
#include "../Fit/StandardFit.h"
#include "../Fit/ExpFunction.h"
#include "../Fit/LnFunction.h"
#include "../Fit/PolynomialFunction.h"
#include "../Fit/NegateFunction.h"
#include "../Fit/MulFunction.h"
#include "../Fit/DivFunction.h"
#include "../Fit/GaussFunction.h"
#include "../Fit/DiscreteFunction.h"
#include "../Fit/DOASVector.h"
#include "../Fit/NonlinearParameterFunction.h"

#include "../Common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// use the MathFit namespace, since all fit objects are contained in this namespace
using namespace MathFit;

using namespace Evaluation;



/////////////////////////////////////////////////////////////////////////////
// The one and only application object


using namespace std;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEvaluation::CEvaluation()
{
    //	iNumSpec = 1;
    m_lowPassFiltering = 0;

    m_subtractDarkFromSky = true;
}

CEvaluation::~CEvaluation()
{

}

/** Evaluate in multiple reference spectra and 2 modes(evaluate 1024 or 2048 channels)
** @darkArray - dark reference
** @skyArray - sky reference
** @specMem - measured spectrum
*/
void CEvaluation::Evaluate(const double* darkSpectrum, const double* skySpectrum, const double* measSpectrum, long numSteps)
{

    CString szOut, szTmp;
    int iNumSpec = m_window.nRef;
    int sumChn = m_window.specLength;
    //////////////////////////////////////////////////////////////////
    // Define the number of used reference spectra
    //	const int iNumSpec = 1;//5;

    CVector vXData, vMeas;

    // Copy the spectra to local variables
    double* measArray = new double[sumChn];
    memcpy(measArray, measSpectrum, sumChn * sizeof(double));

    double* skyArray = new double[sumChn];
    memcpy(skyArray, skySpectrum, sumChn * sizeof(double));

    double* darkArray = new double[sumChn];
    memcpy(darkArray, darkSpectrum, sumChn * sizeof(double));

    // calculate the 'wavelength' column
    vXData.SetSize(sumChn);
    for (int i = 0; i < sumChn; ++i)
    {
        vXData.SetAt(i, (TFitData)(1.0f + (double)i));
    }

    //----------------------------------------------------------------
    // --------- prepare the spectrum for evaluation -----------------
    //----------------------------------------------------------------
    PrepareSpectra(darkArray, skyArray, measArray, m_window); // why not pass in CSpectrum instead of array?

    // Copy the highpass-filtered spectrum to the designated storage
    m_filteredSpectrum = std::vector<double>(measArray, measArray + sumChn);

    // low pass filter
    if (m_lowPassFiltering)
    {
        LowPassBinomial(measArray, sumChn, 5);
    }

    //----------------------------------------------------------------

    vMeas.Copy(measArray, sumChn, 1);
    /////////////////////////////////////////////////////////////////////////////
    // in order to perform the fit on a certain range within the spectrum, we need to extract the
    // appropriate wavelength information from the existing vXData vector. Actually its just a subvector
    // that holds the wavelength values of the fit range
    //CVector vXSec(iFitHigh - iFitLow);
    //vXSec.Copy(vXData.SubVector(iFitLow, iFitHigh - iFitLow));
    CVector vXSec(m_window.fitHigh - m_window.fitLow);
    vXSec.Copy(vXData.SubVector(m_window.fitLow, m_window.fitHigh - m_window.fitLow));

    ////////////////////////////////////////////////////////////////////////////
    // now we start building the model function needed for fitting.
    //
    // First we create a function object that represents our measured spectrum. Since we do not
    // need any interpolation on the measured data its enough to use a CDiscreteFunction object.
    CDiscreteFunction dataTarget;

    // now set the data of the measured spectrum in regard to the wavelength information
    dataTarget.SetData(vXData, vMeas);

    // since the DOAS model function consists of the sum of all reference spectra and a polynomial,
    // we first create a summation object
    CSimpleDOASFunction cRefSum;

    // now we create the required CReferecneSpectrumFunction objects that actually represent the 
    // reference spectra used in the DOAS model function
    //	CReferenceSpectrumFunction ref[iNumSpec];
    CReferenceSpectrumFunction ref[20];
    for (int i = 0; i < iNumSpec; i++)
    {
        // reset all reference's parameters
        ref[i].ResetLinearParameter();
        ref[i].ResetNonlinearParameter();

        // enable amplitude normalization. This should normally be done in order to avoid numerical
        // problems during fitting.
        ref[i].SetNormalize(true);

        // set the spectral data of the reference spectrum to the object. This also causes an internal
        // transformation of the spectral data into a B-Spline that will be used to interpolate the 
        // reference spectrum during shift and squeeze operations
        if (!ref[i].SetData(vXData.SubVector(0, vnYData[i].GetSize()), vnYData[i]))
        {
            Error0("Error initializing spline object!");
        }

        // Set the column (if wanted)
        switch (m_window.ref[i].m_columnOption)
        {
        case novac::SHIFT_TYPE::SHIFT_FIX:   ref[i].FixParameter(CReferenceSpectrumFunction::CONCENTRATION, m_window.ref[i].m_columnValue * ref[i].GetAmplitudeScale()); break;
        case novac::SHIFT_TYPE::SHIFT_LINK:  ref[(int)m_window.ref[i].m_columnValue].LinkParameter(CReferenceSpectrumFunction::CONCENTRATION, ref[i], CReferenceSpectrumFunction::CONCENTRATION); break;
        }

        // Set the shift
        switch (m_window.ref[i].m_shiftOption)
        {
        case novac::SHIFT_TYPE::SHIFT_FIX:   ref[i].FixParameter(CReferenceSpectrumFunction::SHIFT, m_window.ref[i].m_shiftValue); break;
        case novac::SHIFT_TYPE::SHIFT_LINK:  ref[(int)m_window.ref[i].m_shiftValue].LinkParameter(CReferenceSpectrumFunction::SHIFT, ref[i], CReferenceSpectrumFunction::SHIFT); break;
        default:          ref[i].SetDefaultParameter(CReferenceSpectrumFunction::SHIFT, (TFitData)0.0);
            ref[i].SetParameterLimits(CReferenceSpectrumFunction::SHIFT, (TFitData)-5.0, (TFitData)5.0, (TFitData)1e2);
        }

        // Set the squeeze
        switch (m_window.ref[i].m_squeezeOption)
        {
        case novac::SHIFT_TYPE::SHIFT_FIX:   ref[i].FixParameter(CReferenceSpectrumFunction::SQUEEZE, m_window.ref[i].m_squeezeValue); break;
        case novac::SHIFT_TYPE::SHIFT_LINK:  ref[(int)m_window.ref[i].m_squeezeValue].LinkParameter(CReferenceSpectrumFunction::SQUEEZE, ref[i], CReferenceSpectrumFunction::SQUEEZE); break;
        default:          ref[i].SetDefaultParameter(CReferenceSpectrumFunction::SQUEEZE, (TFitData)1.0);
            ref[i].SetParameterLimits(CReferenceSpectrumFunction::SQUEEZE, (TFitData)0.9, (TFitData)1.1, (TFitData)1e5); break;
        }

        // another requirement in the example fit scenario is that we need to link the shift parameters of all
        // references to the shift parameter of the first reference (Fraunhofer) except for the 3 reference spectrum (NO2)
        //	if(i > 0 && i != 3)//stefans
        //		ref[0].LinkParameter(CReferenceSpectrumFunction::SHIFT, ref[i], CReferenceSpectrumFunction::SHIFT);

        // at last add the reference to the summation object
        cRefSum.AddReference(ref[i]);
    }

    // create the additional polynomial with an order of 'polyTime' and add it to the summation object, too
    CPolynomialFunction cPoly(m_window.polyOrder);
    cRefSum.AddReference(cPoly);

    // the last step in the model function will be to define how the difference between the measured data and the modeled
    // data will be determined. In this case we will use the CStandardMetricFunction which actually just calculate the difference
    // between the measured data and the modeled data channel by channel. The fit will try to minimize these differences.
    // So we create the metric object and set the measured spectrum function object and the DOAS model function object as parameters
    CStandardMetricFunction cDiff(dataTarget, cRefSum);


    /////////////////////////////////////////////////////////////////
    // Now its time to create the fit object. The CStandardFit object will 
    // provide a combination of a linear Least Square Fit and a nonlinear Levenberg-Marquardt Fit, which
    // should be sufficient for most needs.
    CStandardFit cFirstFit(cDiff);

    // don't forget to the the already extracted fit range to the fit object!
    // without a valid fit range you'll get an exception.
    cFirstFit.SetFitRange(vXSec);

    // limit the number of fit iteration to 5000. This can still take a long time! More convinient values are
    // between 100 and 1000
    cFirstFit.GetNonlinearMinimizer().SetMaxFitSteps(numSteps);

    try
    {
        // prepare everything for fitting
        cFirstFit.PrepareMinimize();

        // actually do the fitting
        if (!cFirstFit.Minimize())
        {
            MessageBox(NULL, TEXT("fit fail."), TEXT("error"), MB_OK);
        }

        // finalize the fitting process. This will calculate the error measurements and other statistical stuff
        cFirstFit.FinishMinimize();

        CDOASVector vResiduum;

        // get residuum vector and expand it to a DOAS vector object. Do NOT assign the vector data to the new object!
        // display some statistical stuff about the residual data
        vResiduum.Attach(cFirstFit.GetResiduum(), false);
        m_residual.SetSize(vResiduum.GetSize());
        m_residual.Zero();
        m_residual.Add(vResiduum);

        m_result.m_delta = (double)vResiduum.Delta();
        m_result.m_stepNum = (long)cFirstFit.GetFitSteps();
        m_result.m_chiSquare = (double)cFirstFit.GetChiSquare();

        // Get the polynomial
        for (int tmpInt = 0; tmpInt < m_window.polyOrder; ++tmpInt)
        {
            m_result.m_polynomial[tmpInt] = (double)cPoly.GetCoefficient(tmpInt);
        }

        // allocate enough space to fit in all the result-values
        m_result.m_ref.resize(m_window.nRef);

        // finally display the fit results for each reference spectrum including their appropriate error

        int i;
        for (i = 0; i < iNumSpec; i++)
        {
            // Get the name of the evaluated specie
            m_result.m_ref[i].m_specieName = m_window.ref[i].m_specieName.c_str();

            // get the fit results (column, column error, shift, shift error, squeeze, squeeze error)
            m_result.m_ref[i].m_column = (double)ref[i].GetModelParameter(CReferenceSpectrumFunction::CONCENTRATION);
            m_result.m_ref[i].m_columnError = (double)ref[i].GetModelParameterError(CReferenceSpectrumFunction::CONCENTRATION);
            m_result.m_ref[i].m_shift = (double)ref[i].GetModelParameter(CReferenceSpectrumFunction::SHIFT);
            m_result.m_ref[i].m_shiftError = (double)ref[i].GetModelParameterError(CReferenceSpectrumFunction::SHIFT);
            m_result.m_ref[i].m_squeeze = (double)ref[i].GetModelParameter(CReferenceSpectrumFunction::SQUEEZE);
            m_result.m_ref[i].m_squeezeError = (double)ref[i].GetModelParameterError(CReferenceSpectrumFunction::SQUEEZE);

            // clear the fitResult vector
            m_fitResult[i].SetSize(sumChn);
            m_fitResult[i].Zero();

            // get the final fit result
            ref[i].GetValues(vXData, m_fitResult[i]);
        }

        // get the resulting polynomial
        m_fitResult[i].SetSize(sumChn);
        m_fitResult[i].Zero();
        cPoly.GetValues(vXData, m_fitResult[i]);
    }
    catch (CFitException e)
    {
        // in case that something went wrong, display the error to the user.
        // normally you will get error in two cases:
        //
        // 1. You forgot to set a valid fit range before you start fitting
        //
        // 2. A maxtrix inversion failed for some reason inside the fitting loop. Matrix inversions
        //    normally fail when there are linear dependecies in the matrix respectrively you have linear
        //    dependencies in your reference spectrum. Eg. you tried to fit the same reference spectrum twice at once.
        e.ReportError();
        //	std::cout << "Failed: " << ++iFalseCount << std::endl;
        //	std::cout << "Steps: " << cFirstFit.GetNonlinearMinimizer().GetFitSteps() << " - Chi: " << cFirstFit.GetNonlinearMinimizer().GetChiSquare() << std::endl;
        MessageBox(NULL, TEXT("fit exception"), TEXT("notice"), MB_OK);
    }

    delete[] measArray;
    delete[] skyArray;
    delete[] darkArray;

    return;
}

EvaluationResult CEvaluation::GetResult(int referenceFile) const
{
    EvaluationResult result;

    if (static_cast<size_t>(referenceFile) < m_result.m_ref.size())
    {
        result.column = m_result.m_ref[referenceFile].m_column;
        result.columnError = m_result.m_ref[referenceFile].m_columnError;
        result.shift = m_result.m_ref[referenceFile].m_shift;
        result.shiftError = m_result.m_ref[referenceFile].m_shiftError;
        result.squeeze = m_result.m_ref[referenceFile].m_squeeze;
        result.squeezeError = m_result.m_ref[referenceFile].m_squeezeError;
    }

    return result;
}

/**	read data from reference files
** @refFileList - the names of the files
** @iNumFile - the number of the files
** iNumFile<=20, because of vnYData[20]
*/
BOOL CEvaluation::ReadRefList(CString* refFileList, int iNumFile, int sumChn)
{
    m_window.nRef = iNumFile;
    CFileException exceFile;
    CStdioFile fileRef[100];
    CString szLine;
    double fValue[4096];    // this part has been speeded up by using an array as buffer
    long valuesReadNum = 0;
    double tmpDouble;
    int nColumns;

    // test reading data from reference files
    for (int i = 0; i < iNumFile; i++)
    {
        if (!fileRef[i].Open(refFileList[i], CFile::modeRead | CFile::typeText, &exceFile))
        {
            return FALSE;
        }
        valuesReadNum = 0;
        // read reference spectrum into a Vector
        while (fileRef[i].ReadString(szLine))
        {
            char* szToken = (char*)(LPCSTR)szLine;

            while (szToken = strtok(szToken, "\n"))
            {
                nColumns = sscanf(szToken, "%lf\t%lf", &tmpDouble, &fValue[valuesReadNum]);
                if (nColumns == 1)
                {
                    fValue[valuesReadNum] = tmpDouble;
                }
                if (nColumns < 1 || nColumns > 2)
                {
                    break;
                }
                // init to get next token
                szToken = NULL;
            }
            ++valuesReadNum;
            if (valuesReadNum == sumChn)
            {
                break;
            }
        }
        // it is faster to assign all values to the vector at once than having to grow the vector for every read value
        vnYData[i].SetSize(valuesReadNum);
        for (int index = 0; index < valuesReadNum; ++index)
        {
            vnYData[i].SetAt(index, (TFitData)fValue[index]);
        }

        fileRef[i].Close();
    }

    return TRUE;
}

void CEvaluation::SetParameters(int fitLow, int fitHigh, int polynomOrder, int lowPassFilter)
{
    m_window.fitLow = fitLow;
    m_window.fitHigh = fitHigh;
    m_window.polyOrder = polynomOrder;
    this->m_lowPassFiltering = lowPassFilter;
}

void CEvaluation::SetShiftAndSqueeze(int refNum, novac::SHIFT_TYPE shiftType, double shift, novac::SHIFT_TYPE squeezeType, double squeeze)
{
    if (refNum < 0 || refNum > m_window.nRef)
    {
        return;
    }
    m_window.ref[refNum].m_shiftOption = shiftType;
    m_window.ref[refNum].m_shiftValue = shift;
    m_window.ref[refNum].m_squeezeOption = squeezeType;
    m_window.ref[refNum].m_squeezeValue = squeeze;

}

/** Sets the fit window to use */
void CEvaluation::SetFitWindow(const CFitWindow& window)
{
    this->m_window.channel = window.channel;
    m_window.fitHigh = window.fitHigh;
    m_window.fitLow = window.fitLow;
    m_window.fitType = window.fitType;
    m_window.interlaced = window.interlaced;
    m_window.name.Format("%s", window.name);
    m_window.nRef = window.nRef;
    m_window.polyOrder = window.polyOrder;
    m_window.specLength = window.specLength;
    m_window.offsetFrom = window.offsetFrom;
    m_window.offsetTo = window.offsetTo;
    for (int i = 0; i < window.nRef; ++i)
    {
        m_window.ref[i] = window.ref[i];
    }
}

void CEvaluation::RemoveOffset(double* spectrum, int specLen, int offsetFrom, int offsetTo)
{
    if (offsetFrom == offsetTo)
    {
        return;
    }

    //  remove any remaining offset in the spectrum
    double avg = 0;
    for (int i = offsetFrom; i < offsetTo; i++)
    {
        avg += spectrum[i];
    }
    avg = avg / (double)(offsetTo - offsetFrom);
    Sub(spectrum, specLen, avg);
}

void CEvaluation::PrepareSpectra(double* dark, double* sky, double* meas, const CFitWindow& window)
{

    if (window.fitType == FIT_HP_DIV)
    {
        return PrepareSpectra_HP_Div(dark, sky, meas, window);
    }
    if (window.fitType == FIT_HP_SUB)
    {
        return PrepareSpectra_HP_Sub(dark, sky, meas, window);
    }
    if (window.fitType == FIT_POLY)
    {
        return PrepareSpectra_Poly(dark, sky, meas, window);
    }
}

void CEvaluation::PrepareSpectra_HP_Div(double* darkArray, double* skyArray, double* measArray, const CFitWindow& window)
{

    // 1. Subtract the dark spectrum
    Sub(measArray, darkArray, window.specLength, 0.0);
    if (m_subtractDarkFromSky)
    {
        // TODO:should never be called in re-evaluation mode or in adaptive mode if in real-time
        // TEST THIS!!!
        Sub(skyArray, darkArray, window.specLength, 0.0);
    }

    //  2. Remove any remaining offset
    RemoveOffset(measArray, window.specLength, window.offsetFrom, window.offsetTo);
    RemoveOffset(skyArray, window.specLength, window.offsetFrom, window.offsetTo);

    // 3. Divide the measured spectrum with the sky spectrum
    Div(measArray, skyArray, window.specLength, 0.0);

    // 4. high pass filter
    HighPassBinomial(measArray, window.specLength, 500);

    // 5. log(spec)
    Log(measArray, window.specLength);
}

void CEvaluation::PrepareSpectra_HP_Sub(double* darkArray, double* skyArray, double* measArray, const CFitWindow& window)
{

    // 1. spec = measured spectrum - dark spectrum
    Sub(measArray, darkArray, window.specLength, 0.0);

    // 2. remove any remaining offset in the measured spectrum
    RemoveOffset(measArray, window.specLength, window.offsetFrom, window.offsetTo);

    // 3. high pass filter
    HighPassBinomial(measArray, window.specLength, 500);

    // 4. log(spec)
    Log(measArray, window.specLength);
}

void CEvaluation::PrepareSpectra_Poly(double* darkArray, double* skyArray, double* measArray, const CFitWindow& window)
{

    // 1. remove any remaining offset in the measured spectrum
    RemoveOffset(measArray, window.specLength, window.offsetFrom, window.offsetTo);

    // 2. log(spec)
    Log(measArray, window.specLength);

    // 3. Multiply the spectrum with -1 to get the correct sign for everything
    for (int i = 0; i < window.specLength; ++i)
    {
        measArray[i] *= -1.0;
    }
}

BOOL CEvaluation::IncludeAsReference(double* array, int sumChn, int refNum)
{

    if (refNum == -1)
    {
        vnYData[m_window.nRef].SetSize(sumChn);
        for (int index = 0; index < sumChn; ++index)
        {
            vnYData[index].SetAt(index, (TFitData)array[index]);
        }

        ++m_window.nRef;
    }
    else
    {
        if (refNum > m_window.nRef - 1)
        {
            ++m_window.nRef;
        }

        vnYData[refNum].SetSize(sumChn);
        for (int index = 0; index < sumChn; ++index)
        {
            vnYData[refNum].SetAt(index, (TFitData)array[index]);
        }
    }
    return TRUE;
}
