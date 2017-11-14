#pragma once

#include "../Common/XMLFileReader.h"
#include "ReEvaluation_Script.h"

namespace FileHandler{
	class CReEvalScriptFileHandler : protected FileHandler::CXMLFileReader
	{
	public:
		CReEvalScriptFileHandler(void);
		~CReEvalScriptFileHandler(void);
		
		/** Writes this script to the given file 
			@return 0 on success */
		int WriteToFile(const CString &fileName, const ReEvaluation::CReEvaluation_Script &script);
		
		/** Reads in a script from a file 
			@return 0 on success */
		int ReadFromFile(const CString &fileName, ReEvaluation::CReEvaluation_Script &script);
		
	protected:
		/** Starts the parsing */
		int Parse(ReEvaluation::CReEvaluation_Script &script);
		int ParseJob(ReEvaluation::CReEvaluation_Script &script);
	};
}