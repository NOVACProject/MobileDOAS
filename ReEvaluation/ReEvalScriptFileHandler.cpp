#include "stdafx.h"
#include "reevalscriptfilehandler.h"
#include "../Common.h"

using namespace FileHandler;

CReEvalScriptFileHandler::CReEvalScriptFileHandler(void)
{
}

CReEvalScriptFileHandler::~CReEvalScriptFileHandler(void)
{
}

/** Writes this script to the given file 
	@return 0 on success */
int CReEvalScriptFileHandler::WriteToFile(const CString &fileName, const ReEvaluation::CReEvaluation_Script &script){
	// Open the file for writing
	FILE *f = fopen(fileName, "w");
	if(f == nullptr){
		return 1;
	}
	
	// Write the header
	fprintf(f, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
	fprintf(f, "<!-- This file can be used to batch re-evaluate MobileDOAS traverses -->\n\n");
	fprintf(f, "<MobileDOAS_BatchReEvaluation>\n");
	
	// Write the general things...
	fprintf(f, "\t<MaxThreads>%d</MaxThreads>\n", script.m_maxThreadNum);
	
	// Write each of the jobs that is to be processed
	POSITION pos = script.m_jobs.GetHeadPosition();
	while(pos != nullptr){
		const ReEvaluation::CReEvaluation_Script::Job &j = script.m_jobs.GetNext(pos);
		
		fprintf(f, "\t<Job>\n");
		fprintf(f, "\t\t<EvalLog>%s</EvalLog>\n", (LPCTSTR)j.evaluationLog);
		fprintf(f, "\t\t<SettingsFile>%s</SettingsFile>\n", (LPCTSTR)j.settingsFile);
		fprintf(f, "\t</Job>\n");
	}
	
	// remember to close the file
	fprintf(f, "</MobileDOAS_BatchReEvaluation>\n");
	fclose(f);
	
	return 0;
}

/** Reads in a script from a file 
	@return 0 on success */
int CReEvalScriptFileHandler::ReadFromFile(const CString &fileName, ReEvaluation::CReEvaluation_Script &script){
	CFileException exceFile;
	CStdioFile file;

	// 1. Open the file
	if(!file.Open(fileName, CFile::modeRead | CFile::typeText, &exceFile)){
		return 1;
	}
	this->m_File = &file;

	// 2. Parse the file
	if(Parse(script)){
		file.Close();    // error in parsing
		return 1;
	}else{
		file.Close();    // parsing was ok
		return 0;
	}
}

int CReEvalScriptFileHandler::Parse(ReEvaluation::CReEvaluation_Script &script){
	// Reset the script before we start parsing anything
	script.Clear();


	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// The Settings for the number of threads
		if(Equals(szToken, "MaxThreads")){
			Parse_LongItem(TEXT("/MaxThreads"), script.m_maxThreadNum);
			continue;
		}

		// The Settings for an individual job
		if(Equals(szToken, "Job")){
			ParseJob(script);
			continue;
		}
	}

	return 0;
}

int CReEvalScriptFileHandler::ParseJob(ReEvaluation::CReEvaluation_Script &script){
	CString evalLog, settingsFile;
	int nFoundItems = 0;

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of this section
		if(Equals(szToken, "/Job")){
			// If we've found both an evaluation-log file and 
			//  an settings file then insert the job to the list
			if(nFoundItems == 2){
				ReEvaluation::CReEvaluation_Script::Job j;
				j.evaluationLog.Format(evalLog);
				j.settingsFile.Format(settingsFile);
				script.m_jobs.AddTail(j);
			}
			return 0;
		}

		// The evaluation-log file
		if(Equals(szToken, "EvalLog")){
			Parse_StringItem(TEXT("/EvalLog"), evalLog);
			++nFoundItems;
			continue;
		}

		// The file with the settings
		if(Equals(szToken, "SettingsFile")){
			Parse_StringItem(TEXT("/SettingsFile"), settingsFile);
			++nFoundItems;
			continue;
		}
	}
	return 0;
}
