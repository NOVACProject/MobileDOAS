#include "StdAfx.h"
#include "referencefile.h"
#include "../Common.h"

//#include "../Evaluation/Evaluation.h"

using namespace Evaluation;

CReferenceFile::CReferenceFile(void)
{
	this->m_path.Format("");
	this->m_specieName.Format("");
	this->m_gasFactor = GASFACTOR_SO2;
	m_columnOption = SHIFT_FREE;
	m_columnValue = 0.0;
	this->m_shiftOption = SHIFT_FIX;
	this->m_shiftValue = 0.0;
	this->m_squeezeOption = SHIFT_FIX;
	this->m_squeezeValue = 1.0;
}

CReferenceFile::~CReferenceFile(void)
{
}

/** assignment operator */
CReferenceFile &CReferenceFile::operator=(const CReferenceFile &ref2){
	this->m_path.Format("%s", (LPCTSTR)ref2.m_path);
	this->m_specieName.Format("%s", (LPCTSTR)ref2.m_specieName);
	this->m_gasFactor	= ref2.m_gasFactor;

	this->m_columnOption = ref2.m_columnOption;
	this->m_columnValue = ref2.m_columnValue;
	this->m_shiftOption = ref2.m_shiftOption;
	this->m_shiftValue = ref2.m_shiftValue;
	this->m_squeezeOption = ref2.m_squeezeOption;
	this->m_squeezeValue = ref2.m_squeezeValue;
	return *this;
}
