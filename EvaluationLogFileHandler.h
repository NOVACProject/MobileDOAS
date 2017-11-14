#pragma once

#include "Evaluation/TraverseResult.h"

#include "Common.h"
#include "Evaluation/FitWindow.h"

class CEvaluationLogFileHandler
{
public:

	/** Default constructor */
	CEvaluationLogFileHandler(void);

	/** Default destructor */
	~CEvaluationLogFileHandler(void);

	/** Reads an evaluation log-file the result is stored in the 
			array, m_traverses. 
			Returns SUCCESS or FAIL. */
	bool ReadEvaluationLogFile(const CString &logFileName);

	/** The traverses found in the last read evaluation-log file.
			There is one traverse defined for each channel that was read in
			the last read evaluation log file. */
	CTraverseResult m_traverses[MAX_N_CHANNELS];

	/** The number of traverses read, i.e. the number of channels in 
			the last evaluation log file. */
	long						m_numTraverses;

protected:

	/** Reads and parses the header line of the log-file.
			This line tells us which column corresponds to which value in
			the log-file. This function parses the line and stores the 
			result in the 'm_columns' object. This only works for log-files
			with version >= 4.0 since the earlier versions does not have
			a header-line */
	bool	ParseHeaderLine(const CString &headerLine);

	/** Sets the correct columns for log-file of version 3.2 */
	bool	SetColumns_32();

	/** The LogColumns enables us to find out what kind of data
			there is to be found in each of the columns in the log-file. 
			E.g the item 'expTime' tells us in which column the 
			exposure time of the spectra can be found. 
			Columns are indexed starting on zero. */
	typedef struct LogColumns{
		int	nSpecies; // <-- the number of species this traverse has been evaluated for
		int	nChannels; // <-- the number of spectrometer channels that was used during this traverse
		int gpsTime;
		int	latitude;
		int longitude;
		int altitude;
		int	expTime;
		int numSpec;
		int intensity[MAX_N_CHANNELS];	// there's one intensity value for each spectrometer-channel
		int column[MAX_N_CHANNELS][MAX_N_REFERENCES]; // for each channel there can be at most MAX_N_REFERENCES referece-files
		int columnError[MAX_N_CHANNELS][MAX_N_REFERENCES]; // for each channel there can be at most MAX_N_REFERENCES referece-files
		int	specFile[MAX_N_CHANNELS];		// the spectrum file
	}LogColumns;

	/** Our own LogColumns-object */
	LogColumns	m_columns;
};
