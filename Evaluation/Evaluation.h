// Evaluation.h: interface for the CEvaluation class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVALUATION_H__DB88EE51_7ED0_4131_AE07_79F0F0C3106C__INCLUDED_)
#define AFX_EVALUATION_H__DB88EE51_7ED0_4131_AE07_79F0F0C3106C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../BasicMath.h"
#include "../FIT\Vector.h"	// Added by ClassView

#include "FitWindow.h"
#include "EvaluationResult.h"
#include <MobileDoasLib/Definitions.h>

namespace Evaluation
{

struct EvaluationResult
{
    double column = 0.0;
    double columnError = 0.0;
    double shift = 0.0;
    double shiftError = 0.0;
    double squeeze = 0.0;
    double squeezeError = 0.0;
};

class CEvaluation : public CBasicMath
{
public:

    CEvaluation();
    virtual ~CEvaluation();

    // -------------------------------------------------------------
    // --------------------- PUBLIC METHODS ------------------------
    // -------------------------------------------------------------

    /** Reads the supplied list of references.
            @param refFileList	- an array of CStrings containing the file-names to read
            @param iNumFile			- the number of reference-files to read
            @param sumChn				- the number of data points that we want to have in each file. */
    BOOL ReadRefList(CString* refFileList, int iNumFile, int sumChn);

    // initialize the evaluation. Must be called before 'Evaluate' is called
    void SetParameters(int fitLow, int fitHigh, int polynomOrder, int lowPassFilter = 0);

    /** Sets the fit window to use */
    void  SetFitWindow(const CFitWindow& window);

    // initialize the shift parameters for the evaluation, if these are more 
    //  complicated than what can be handled by using the 'Evaluate'-function itself
    void SetShiftAndSqueeze(int refNum, novac::SHIFT_TYPE shiftType, double shift, novac::SHIFT_TYPE squeezeType, double squeeze);

    /** Evaluate the following spectra, the parameters for the fit are defined in 'm_fitWindow' */
    void Evaluate(const double* darkArray, const double* skyArray, const double* specMem, long numSteps = 400);

    /** Returns the result from the last evaluation.
        If there are more than one referencefile, only the results from evaluating
        referencefile number 'referenceFile' will be returned. */
    EvaluationResult GetResult(int referenceFile = 0) const;

    double GetDelta() const { return m_result.m_delta; }
    double GetChiSquare() const { return m_result.m_chiSquare; }

    /** Includes an array as a reference file */
    BOOL IncludeAsReference(double* array, int sumChn, int refNum);

    /** Removes the offset from the supplied spectrum */
    void RemoveOffset(double* spectrum, int specLen, int from, int to);

    // -------------------------------------------------------------
    // ----------------------- PUBLIC DATA -------------------------
    // -------------------------------------------------------------

    CVector vYData[1];
    CVector vnYData[100];

    /** The fit window object, this defines the parameters for the fit */
    CFitWindow m_window;

    // The results
    CVector m_fitResult[MAX_N_REFERENCES + 1];
    CVector m_residual;

    // parameters
    bool m_subtractDarkFromSky; // Whether we should subtract the dark spectrum from the sky or not

    // The high-pass filtered spectrum
    std::vector<double> m_filteredSpectrum;

private:
    // -------------------------------------------------------------
    // -------------------- PRIVATE METHODS ------------------------
    // -------------------------------------------------------------

    // Prepares the spectra for evaluation
    void PrepareSpectra(double* dark, double* sky, double* meas, const CFitWindow& window);

    // Prepares the spectra for evaluation
    void PrepareSpectra_HP_Div(double* dark, double* sky, double* meas, const CFitWindow& window);

    // Prepares the spectra for evaluation
    void PrepareSpectra_HP_Sub(double* dark, double* sky, double* meas, const CFitWindow& window);

    // Prepares the spectra for evaluation
    void PrepareSpectra_Poly(double* dark, double* sky, double* meas, const CFitWindow& window);

    // -------------------------------------------------------------
    // ---------------------- PRIVATE DATA -------------------------
    // -------------------------------------------------------------

    /** The result of the last evaluation that was performed using this evaluator */
    CEvaluationResult m_result;

    /** The fit window object, this holds the parameters for the fitting */
    CFitWindow m_fitWindow;

    /** resultSet is used as buffer when returning the result of the evaluation */
    double resultSet[6];

    /** 'lowPassFiltering' is true only if the spectra should be low-pass filtered
            before evaluation. This is by default false. */
    int m_lowPassFiltering;

};
}
#endif // !defined(AFX_EVALUATION_H__DB88EE51_7ED0_4131_AE07_79F0F0C3106C__INCLUDED_)
