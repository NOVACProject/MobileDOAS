#pragma once

namespace Evaluation
{
	// the options for the shift and squeeze
	enum SHIFT_TYPE{SHIFT_FREE, SHIFT_FIX, SHIFT_LINK, SHIFT_OPTIMAL};


	/** The class <b>CReferenceFile</b> is used to store
		the options for a single reference file that is
		to be included in the DOAS fit.
		Options stored are e.g. the options for the shift
		and squeeze and the path to the file containing the
		reference-spectrum. 
	*/
	class CReferenceFile
	{
	public:
		CReferenceFile(void);
		~CReferenceFile(void);

		/** The name of the specie */
		CString m_specieName;

		/** The path to the reference file */
		CString m_path;

		/** The gas-factor of the specie */
		double				m_gasFactor;

		/** assignment operator */
		CReferenceFile &operator=(const CReferenceFile &ref2);

		/** The option for the column value (normaly SHIFT_FREE) */
		SHIFT_TYPE    m_columnOption;

		/** The value for the column (if set) */
		double        m_columnValue;

		/** The option for the shift */
		SHIFT_TYPE    m_shiftOption;

		/** The value for the shift */
		double        m_shiftValue;

		/** The option for the squeeze */
		SHIFT_TYPE    m_squeezeOption;

		/** The value for the squeeze */
		double        m_squeezeValue;
	};
}