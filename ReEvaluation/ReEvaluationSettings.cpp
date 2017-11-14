#include "stdafx.h"
#include "reevaluationsettings.h"

using namespace ReEvaluation;

CReEvaluationSettings::CReEvaluationSettings(void)
{
	m_nAverageSpectra = 1;
	m_fInterpolateDark	= 0;

	// Options for ignoring dark spectra
	m_ignoreDark.selection = IGNORE_LIMIT;
	m_ignoreDark.channel = 1144;
	m_ignoreDark.intensity = 1000;
	memset(m_ignoreList_Lower, -1, MAX_IGNORE_LIST_LENGTH*sizeof(long));
	
	// Options for ignoring saturated spectra
	m_ignoreSaturated.selection = 0;
	m_ignoreSaturated.channel = 1144;
	m_ignoreSaturated.intensity = 3900;
	
	// the options for choosing the sky spectrum
	m_skySelection = USE_SKY_FIRST;
	m_skyIntensityLow = 1000;
	m_skyIntensityHigh = 4000;
	m_skyIntensityChannel = 1144;
	m_skyColumnLow = -1e10;
	m_skyColumnHigh = 1e10;
	m_skySpectrumFile.Format("");
	m_skySpectrumDark.Format("");
}

CReEvaluationSettings::~CReEvaluationSettings(void)
{
}
