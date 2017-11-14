#pragma once
#include "../Evaluation/FitWindow.h"

#define IGNORE_DARK   0
#define IGNORE_LIMIT  1
#define IGNORE_LIST   2

#define MAX_IGNORE_LIST_LENGTH 256

/** The options for the sky-spectrum */
const enum SKY_OPTION {USE_SKY_FIRST, USE_SKY_ALL, USE_SKY_CUSTOM, USE_SKY_USER};

namespace ReEvaluation{
	class CReEvaluationSettings
	{
	public:
		CReEvaluationSettings(void);
		~CReEvaluationSettings(void);
		
		typedef struct IgnoreOptions{
			int		selection;
			
			// the options for ignoring spectra
			//	are based on intensity, this must
			//	be measured at a specific channel.
			double	intensity;
			int		channel;
		}IgnoreOptions;

		
		/** the number of spectra to average together */
		long    m_nAverageSpectra;
		
		/** Options for which spectra to ignore */
		IgnoreOptions m_ignoreDark;
		IgnoreOptions m_ignoreSaturated;

		long    m_ignoreList_Lower[MAX_IGNORE_LIST_LENGTH];
	
		/** The option for the dark spectrum. 
			Possible values are;
				0 - there's only one dark spectrum, 
					in the beginning of the traverse
				1 - there are several dark spectra,
					for each spectrum - calculate an
					interpolation of the dark spectra
					to remove the dark current. 
		*/
		int	m_fInterpolateDark;
		
		/** The option for the sky-spectrum */
		SKY_OPTION  m_skySelection;
		double      m_skyIntensityLow, m_skyIntensityHigh;
		long        m_skyIntensityChannel;
		double      m_skyColumnLow, m_skyColumnHigh;
		CString     m_skySpectrumFile;
		CString     m_skySpectrumDark;
		
		/** The actual fit window to use */
		Evaluation::CFitWindow  m_window;

	};
}