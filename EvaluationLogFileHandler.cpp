#include "stdafx.h"
#include "evaluationlogfilehandler.h"

CEvaluationLogFileHandler::CEvaluationLogFileHandler(void)
{
	m_numTraverses = 0;
}

CEvaluationLogFileHandler::~CEvaluationLogFileHandler(void)
{
}

/** Reads an evaluation log-file the result is stored in the 
		array, m_traverses. */
bool CEvaluationLogFileHandler::ReadEvaluationLogFile(const CString &logFileName){
	double fileVersion = 0.0; // the version number of the program that generated this file. Fileversions < 4.0 does not have a header-line
	char buf[4096];
	int spectrumNum = 0; // how many spectra there are in this log-file

	// 1. Try to open the file, if it's not possible to open it then return
	FILE *f = fopen(logFileName, "r");
	if(NULL == f){
  		MessageBox(NULL,TEXT("Can not read log file"),TEXT("Error"),MB_OK);
		return FAIL;
	}

	// 2. Read the file, one line at a time.
	bool readingHeader = true;
	while(fgets(buf, 4095, f)){ // fgets reads until the next newline character

		// Ignore empty lines
		if(strlen(buf) < 2){
			readingHeader = false; // for an old-time evaluation log the only empty line is between the header and the data
			continue;
		}

		// 2a. If we're still reading the header, check if we can find anything useful
		if(readingHeader){

			if(char *pt = strstr(buf, "VERSION=")){
				// we've found a version number
				sscanf(pt + 8, "%lf", &fileVersion);
				if(fileVersion < 4.0){
					SetColumns_32(); // Assing the columns for a evaluation-log file of version 3.2 (the last version before 4.0)
				}
			}

			if(strstr(buf, "#Time")){
				// We've found a header-line telling us which column contains what
				CString line;
				line.Format("%s", buf);
				ParseHeaderLine(line);
				readingHeader = false; // <-- we're not reading the header anymore
			}

			continue; // read the next line
		}

		// -------- IF WE GET TO HERE THEN WE'RE CURRENTLY READING THE DATA --------

		//  Split this line into tokens and try to parse them.
		char* szToken = buf;
		int curColumn = 0; // the current column
		while(szToken = strtok(szToken,"\t")){

			// first check if this is a time.
			if(curColumn == m_columns.gpsTime){
				//if(3 == sscanf(szToken, "%d:%d:%d", &fValue[0], &fValue[1], &fValue[2])){
				//	Time tid;
				//	tid.hour = fValue[0]; tid.minute = fValue[1]; tid.second = fValue[2];
				//	m_traverses[0].SetStartTime(spectrumNum, tid);
				//	continue;
				// }
			}

			// parse the value

			if(curColumn == m_columns.altitude){
//				m_traverses[0].SetAltitude(spectrumNum, fValue);
			}else if(curColumn == m_columns.latitude){

			}else if(curColumn == m_columns.longitude){

			}else if(curColumn == m_columns.expTime){

			}else if(curColumn == m_columns.numSpec){

			}else{
				for(int chn = 0; chn < m_columns.nChannels; ++chn){
					for(int k = 0; k < m_columns.nSpecies; ++k){
						if(curColumn == m_columns.intensity[chn]){

						}else if(curColumn == m_columns.column[chn][k]){

						}else if(curColumn == m_columns.columnError[chn][k]){

						}
					}
				}
			}

			++curColumn;
		}
	}

	// Close the file.
	fclose(f);

	return SUCCESS;
}

/** Reads and parses the header line of the log-file.
		This line tells us which column corresponds to which value in
		the log-file. This function parses the line and stores the 
		result in the 'm_columns' object. */
bool	CEvaluationLogFileHandler::ParseHeaderLine(const CString &headerLine){
	

	return SUCCESS;
}

/** Sets the correct columns for log-file of version 3.2 */
bool	CEvaluationLogFileHandler::SetColumns_32(){
	

	return SUCCESS;
}
