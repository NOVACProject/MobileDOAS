#include "StdAfx.h"
#include "evaluationresult.h"
#include "../Common.h"

using namespace Evaluation;

CEvaluationResult::CEvaluationResult(void)
{
    this->m_chiSquare = 0.0f;
    this->m_delta = 0.0f;
    this->m_speciesNum = 0;
    this->m_stepNum = 0;
    memset(m_polynomial, 0, 5 * sizeof(double));
    m_evaluationStatus = 0;
}

CEvaluationResult::CEvaluationResult(const CEvaluationResult& b)
{
    int nRef = (int)b.m_ref.size();
    this->m_ref.resize(nRef);
    for (long i = 0; i < nRef; ++i) {
        mobiledoas::ReferenceFitResult ref;
        ref.m_column = b.m_ref[i].m_column;
        ref.m_columnError = b.m_ref[i].m_columnError;
        ref.m_shift = b.m_ref[i].m_shift;
        ref.m_shiftError = b.m_ref[i].m_shiftError;
        ref.m_squeeze = b.m_ref[i].m_squeeze;
        ref.m_squeezeError = b.m_ref[i].m_squeezeError;
        ref.m_specieName = b.m_ref[i].m_specieName;
        this->m_ref[i] = ref;
    }
    memcpy(this->m_polynomial, b.m_polynomial, 5 * sizeof(double));

    this->m_chiSquare = b.m_chiSquare;
    this->m_delta = b.m_delta;
    this->m_stepNum = b.m_stepNum;
    this->m_evaluationStatus = b.m_evaluationStatus;
    this->m_speciesNum = b.m_speciesNum;
}

CEvaluationResult::~CEvaluationResult(void)
{
    static int called = 0;
    ++called;
}

// makes this a copy of 'b'
CEvaluationResult& CEvaluationResult::operator =(const CEvaluationResult& b) {

    int nRef = (int)b.m_ref.size();
    this->m_ref.resize(nRef);
    for (long i = 0; i < nRef; ++i) {
        mobiledoas::ReferenceFitResult ref;
        ref.m_column = b.m_ref[i].m_column;
        ref.m_columnError = b.m_ref[i].m_columnError;
        ref.m_shift = b.m_ref[i].m_shift;
        ref.m_shiftError = b.m_ref[i].m_shiftError;
        ref.m_squeeze = b.m_ref[i].m_squeeze;
        ref.m_squeezeError = b.m_ref[i].m_squeezeError;
        ref.m_specieName = b.m_ref[i].m_specieName;
        this->m_ref[i] = ref;
    }
    memcpy(this->m_polynomial, b.m_polynomial, 5 * sizeof(double));

    this->m_chiSquare = b.m_chiSquare;
    this->m_delta = b.m_delta;
    this->m_stepNum = b.m_stepNum;

    m_evaluationStatus = b.m_evaluationStatus;
    m_speciesNum = b.m_speciesNum;

    return *this;
}

bool CEvaluationResult::InsertSpecie(const CString& name) {
    mobiledoas::ReferenceFitResult ref;
    ref.m_specieName = std::string((LPCSTR)name);
    m_ref.push_back(ref);
    ++m_speciesNum;
    return SUCCESS;
}

bool CEvaluationResult::CheckGoodnessOfFit(const double fitIntensity, const double offsetLevel, double chi2Limit, double upperLimit, double lowerLimit) {
    // assume that this is an ok evaluation
    m_evaluationStatus &= ~MARK_BAD_EVALUATION;

    // TODO: UPDATE THIS!!!
      // first check the chi2 of the fit
    if (chi2Limit > -1) {
        if (m_chiSquare > chi2Limit) {
            m_evaluationStatus |= MARK_BAD_EVALUATION;
        }
    }
    else {
        if (m_chiSquare > 1.2) {
            m_evaluationStatus |= MARK_BAD_EVALUATION;
        }
    }

    // TODO: UPDATE THIS!!!
    // then check the intensity of the spectrum in the fit region
    if (upperLimit > -1) {
        if (fitIntensity > upperLimit)
            m_evaluationStatus |= MARK_BAD_EVALUATION;
    }
    else {
        //if(fitIntensity > 4000.0)
        //  m_evaluationStatus |= MARK_BAD_EVALUATION;
    }

    // TODO: UPDATE THIS!!!
    if (lowerLimit > -1) {
        if (fitIntensity - offsetLevel < lowerLimit)
            m_evaluationStatus |= MARK_BAD_EVALUATION;
    }
    else {
        //if(fitIntensity - offsetLevel < 300.0)
        //  m_evaluationStatus |= BAD_EVALUATION;
    }

    return (m_evaluationStatus & MARK_BAD_EVALUATION);
}

/** Marks the current spectrum with the supplied mark_flag.
    Mark flag must be MARK_BAD_EVALUATION, or MARK_DELETED
    @return SUCCESS on success. */
bool  CEvaluationResult::MarkAs(int MARK_FLAG) {
    // check the flag
    switch (MARK_FLAG) {
    case MARK_BAD_EVALUATION: break;
    case MARK_DELETED: break;
    default: return FAIL;
    }

    // set the corresponding bit
    m_evaluationStatus |= MARK_FLAG;

    return SUCCESS;
}

/** Removes the current mark from the desired spectrum
    Mark flag must be MARK_BAD_EVALUATION, or MARK_DELETED
    @return SUCCESS on success. */
bool  CEvaluationResult::RemoveMark(int MARK_FLAG) {
    // check the flag
    switch (MARK_FLAG) {
    case MARK_BAD_EVALUATION: break;
    case MARK_DELETED: break;
    default: return FAIL;
    }

    // remove the corresponding bit
    m_evaluationStatus &= ~MARK_FLAG;

    return SUCCESS;
}
