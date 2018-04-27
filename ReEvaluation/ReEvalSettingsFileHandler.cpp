#include "stdafx.h"
#include "reevalsettingsfilehandler.h"

using namespace FileHandler;

CReEvalSettingsFileHandler::CReEvalSettingsFileHandler(void)
{
}

CReEvalSettingsFileHandler::~CReEvalSettingsFileHandler(void)
{
}


/** Parses a file with settings for the re-evaluation.
	@return 0 on success 
	*/
int CReEvalSettingsFileHandler::ParseFile(ReEvaluation::CReEvaluationSettings &settings, const CString &fileName){
	CFileException exceFile;
	CStdioFile file;

	// 1. Open the file
	if(!file.Open(fileName, CFile::modeRead | CFile::typeText, &exceFile)){
		return 1;
	}
	this->m_File = &file;

	// 2. Parse the file
	if(Parse(settings)){
		file.Close();    // error in parsing
		return 1;
	}else{
		file.Close();    // parsing was ok
		return 0;
	}
}

/** Writes a file with settings for the re-evaluation 
	@return 0 on success 
*/
int CReEvalSettingsFileHandler::WriteFile(const ReEvaluation::CReEvaluationSettings &settings, const CString &fileName){
	// Try to open the file
	FILE *f = fopen(fileName, "w");
	if(f == nullptr){
		return 1;
	}

	// ---------- Writing the file -----------
	fprintf(f, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
	fprintf(f, "<!-- This file contains settings for re-evaluating MobileDOAS traverses -->\n\n");

	fprintf(f, "<MobileDOAS_ReEvalSettings>\n");

	fprintf(f, "\t<Average>%d</Average>\n",			settings.m_nAverageSpectra);

	// settings for ignoring dark spectra
	fprintf(f, "\t<IgnoreDark>\n");
	fprintf(f, "\t\t<option>%d</option>\n",			settings.m_ignoreDark.selection);
	fprintf(f, "\t\t<channel>%d</channel>\n",		settings.m_ignoreDark.channel);
	fprintf(f, "\t\t<intensity>%lf</intensity>\n",	settings.m_ignoreDark.intensity);
	fprintf(f, "\t</IgnoreDark>\n");

	// settings for ignoring saturated spectra
	fprintf(f, "\t<IgnoreSaturated>\n");
	fprintf(f, "\t\t<option>%d</option>\n",			settings.m_ignoreSaturated.selection);
	fprintf(f, "\t\t<channel>%d</channel>\n",		settings.m_ignoreSaturated.channel);
	fprintf(f, "\t\t<intensity>%lf</intensity>\n",	settings.m_ignoreSaturated.intensity);
	fprintf(f, "\t</IgnoreSaturated>\n");

	// calculating the offset
	fprintf(f, "\t<Offset>\n");
	fprintf(f, "\t\t<from>%d</from>\n", settings.m_window.offsetFrom);
	fprintf(f, "\t\t<to>%d</to>\n",    settings.m_window.offsetTo);
	fprintf(f, "\t</Offset>\n");

	// Options for using the dark
	fprintf(f, "\t<Dark>\n");
	if(settings.m_fInterpolateDark){
		fprintf(f, "\t\t<option>1</option>\n");
	}else{
		fprintf(f, "\t\t<option>0</option>\n");
	}
	fprintf(f, "\t</Dark>\n");
		
	// Options for using the sky-spectrum
	fprintf(f, "\t<Sky>\n");
	fprintf(f, "\t\t<option>%d</option>\n", settings.m_skySelection);
	if(settings.m_skySelection == USE_SKY_CUSTOM){
		fprintf(f, "\t\t<column>\n");
		fprintf(f, "\t\t\t<high>%lf</high>\n",	settings.m_skyColumnHigh);
		fprintf(f, "\t\t\t<low>%lf</low>\n",	settings.m_skyColumnLow);
		fprintf(f, "\t\t</column>\n");
		fprintf(f, "\t\t<intensity>\n");
		fprintf(f, "\t\t\t<channel>%d</channel>\n",		settings.m_skyIntensityChannel);
		fprintf(f, "\t\t\t<low>%lf</low>\n",			settings.m_skyIntensityLow);
		fprintf(f, "\t\t\t<high>%lf</high>\n",			settings.m_skyIntensityHigh);
		fprintf(f, "\t\t</intensity>\n");
	}else if(settings.m_skySelection == USE_SKY_USER){
		fprintf(f, "\t\t<path_sky>%s</path_sky>\n", (LPCTSTR)settings.m_skySpectrumFile);
		fprintf(f, "\t\t<path_dark>%s</path_dark>\n", (LPCTSTR)settings.m_skySpectrumDark);
	}
	fprintf(f, "\t</Sky>\n");

	// --- And now, for the moment we've all been waiting for....
	// ---    the fit windows!!
	
	fprintf(f, "\t<FitWindow>\n");

	fprintf(f, "\t\t<name>%s</name>\n", (LPCTSTR)settings.m_window.name);
	fprintf(f, "\t\t<fitLow>%d</fitLow>\n",				settings.m_window.fitLow);
	fprintf(f, "\t\t<fitHigh>%d</fitHigh>\n",			settings.m_window.fitHigh);
	fprintf(f, "\t\t<spec_channel>%d</spec_channel>\n",	settings.m_window.channel);
	fprintf(f, "\t\t<polynomial>%d</polynomial>\n",		settings.m_window.polyOrder);
	fprintf(f, "\t\t<fittype>%d</fittype>\n",			settings.m_window.fitType);
	fprintf(f, "\t\t<channel>%d</channel>\n",			settings.m_window.channel);

	for(int j = 0; j < settings.m_window.nRef; ++j){
		fprintf(f, "\t\t<Reference>\n");
		fprintf(f, "\t\t\t<name>%s</name>\n", (LPCTSTR)settings.m_window.ref[j].m_specieName);
		fprintf(f, "\t\t\t<path>%s</path>\n", (LPCTSTR)settings.m_window.ref[j].m_path);
		fprintf(f, "\t\t\t<gasFactor>%.2lf</gasFactor>\n",	settings.m_window.ref[j].m_gasFactor);

		// Shift
		if(settings.m_window.ref[j].m_shiftOption == Evaluation::SHIFT_FIX){
			fprintf(f, "\t\t\t<shift>fix to %.2lf</shift>\n",		settings.m_window.ref[j].m_shiftValue);
		}else if(settings.m_window.ref[j].m_shiftOption == Evaluation::SHIFT_FREE){
			fprintf(f, "\t\t\t<shift>free</shift>\n");
		}else if(settings.m_window.ref[j].m_shiftOption == Evaluation::SHIFT_LINK){
			fprintf(f, "\t\t\t<shift>link to %.0lf</shift>\n",		settings.m_window.ref[j].m_shiftValue);
		}else if(settings.m_window.ref[j].m_shiftOption == Evaluation::SHIFT_OPTIMAL){
			fprintf(f, "\t\t\t<shift>find optimal</shift>\n");
		}

		// Squeeze
		if(settings.m_window.ref[j].m_squeezeOption == Evaluation::SHIFT_FIX){
			fprintf(f, "\t\t\t<squeeze>fix to %.2lf</squeeze>\n",		settings.m_window.ref[j].m_squeezeValue);
		}else if(settings.m_window.ref[j].m_squeezeOption == Evaluation::SHIFT_FREE){
			fprintf(f, "\t\t\t<squeeze>free</squeeze>\n");
		}else if(settings.m_window.ref[j].m_squeezeOption == Evaluation::SHIFT_LINK){
			fprintf(f, "\t\t\t<squeeze>link to %.0lf</squeeze>\n",		settings.m_window.ref[j].m_squeezeValue);
		}else if(settings.m_window.ref[j].m_squeezeOption == Evaluation::SHIFT_OPTIMAL){
			fprintf(f, "\t\t\t<squeeze>find optimal</squeeze>\n");
		}
		
		fprintf(f, "\t\t</Reference>\n");
	}
	fprintf(f, "\t</FitWindow>\n");

	fprintf(f, "</MobileDOAS_ReEvalSettings>\n");


	// remember to close the file
	fclose(f);

	return 0;
}

int CReEvalSettingsFileHandler::Parse(ReEvaluation::CReEvaluationSettings &settings){

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// The number of spectra to average together
		if(Equals(szToken, "Average")){
			Parse_LongItem("/Average", settings.m_nAverageSpectra);
			continue;
		}

		// The settings for ignoring dark spectra
		if(Equals(szToken, "IgnoreDark")){
			Parse_IgnoreDark(settings);
			continue;
		}

		// The settings for ignoring saturated spectra
		if(Equals(szToken, "IgnoreSaturated")){
			Parse_IgnoreSaturated(settings);
			continue;
		}

		// The Offset Settings
		if(Equals(szToken, "Offset")){
			ParseOffset(settings);
			continue;
		}

		// The Sky Settings
		if(Equals(szToken, "Sky")){
			ParseSky(settings);
			continue;
		}

		// The Dark Settings
		if(Equals(szToken, "Dark")){
			ParseDark(settings);
			continue;
		}

		// The Fit-window Settings
		if(Equals(szToken, "FitWindow")){
			ParseFitWindow(settings);
			continue;
		}
	}

	return 0;
}

int CReEvalSettingsFileHandler::Parse_IgnoreDark(ReEvaluation::CReEvaluationSettings &settings){
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
		if(Equals(szToken, "/IgnoreDark")){
			return 0;
		}

		// The option
		if(Equals(szToken, "option")){
			Parse_IntItem(TEXT("/option"), settings.m_ignoreDark.selection);
			continue;
		}

		// The channel
		if(Equals(szToken, "channel")){
			Parse_IntItem(TEXT("/channel"), settings.m_ignoreDark.channel);
			continue;
		}

		// The intensity
		if(Equals(szToken, "intensity")){
			Parse_FloatItem(TEXT("/intensity"), settings.m_ignoreDark.intensity);
			continue;
		}
	}
	return 0;
}

int CReEvalSettingsFileHandler::Parse_IgnoreSaturated(ReEvaluation::CReEvaluationSettings &settings){
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
		if(Equals(szToken, "/IgnoreSaturated")){
			return 0;
		}

		// The option
		if(Equals(szToken, "option")){
			Parse_IntItem(TEXT("/option"), settings.m_ignoreSaturated.selection);
			continue;
		}

		// The channel
		if(Equals(szToken, "channel")){
			Parse_IntItem(TEXT("/channel"), settings.m_ignoreSaturated.channel);
			continue;
		}

		// The intensity
		if(Equals(szToken, "intensity")){
			Parse_FloatItem(TEXT("/intensity"), settings.m_ignoreSaturated.intensity);
			continue;
		}
	}
	return 0;
}

int CReEvalSettingsFileHandler::ParseOffset(ReEvaluation::CReEvaluationSettings &settings){

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 2)
		continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
		continue;
		}

		// the end of the Offset section
		if(Equals(szToken, "/Offset")){
		return 0;
		}

		// Measure offset from...
		if(Equals(szToken, "from")){
			Parse_IntItem(TEXT("/from"), settings.m_window.offsetFrom);
			continue;
		}
		
		// Measure offset to...
		if(Equals(szToken, "to")){
			Parse_IntItem(TEXT("/to"), settings.m_window.offsetTo);
			continue;
		}
	}
	return 0;
}

int CReEvalSettingsFileHandler::ParseSky(ReEvaluation::CReEvaluationSettings &settings){
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
		if(Equals(szToken, "/Sky")){
			return 0;
		}

		// The option
		if(Equals(szToken, "option")){
			Parse_IntItem(TEXT("/option"), (int &)settings.m_skySelection);
			continue;
		}

		// The limits on the column
		if(Equals(szToken, "column")){
			ParseSkyColumn(settings);
			continue;
		}

		// The intensity
		if(Equals(szToken, "intensity")){
			ParseSkyIntensity(settings);
			continue;
		}
	}
	return 0;
}

int CReEvalSettingsFileHandler::ParseSkyColumn(ReEvaluation::CReEvaluationSettings &settings){
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
		if(Equals(szToken, "/column")){
			return 0;
		}

		// The upper limit
		if(Equals(szToken, "high")){
			Parse_FloatItem(TEXT("/high"), settings.m_skyColumnHigh);
			continue;
		}

		// The lower limit
		if(Equals(szToken, "low")){
			Parse_FloatItem(TEXT("/low"), settings.m_skyColumnLow);
			continue;
		}
	}
	return 0;
}

int CReEvalSettingsFileHandler::ParseSkyIntensity(ReEvaluation::CReEvaluationSettings &settings){
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
		if(Equals(szToken, "/intensity")){
			return 0;
		}

		// The channel
		if(Equals(szToken, "channel")){
			Parse_LongItem(TEXT("/channel"), settings.m_skyIntensityChannel);
			continue;
		}

		// The upper limit
		if(Equals(szToken, "high")){
			Parse_FloatItem(TEXT("/high"), settings.m_skyIntensityHigh);
			continue;
		}

		// The lower limit
		if(Equals(szToken, "low")){
			Parse_FloatItem(TEXT("/low"), settings.m_skyIntensityLow);
			continue;
		}
	}
	return 0;
}

int CReEvalSettingsFileHandler::ParseDark(ReEvaluation::CReEvaluationSettings &settings){
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
		if(Equals(szToken, "/Dark")){
			return 0;
		}

		// The option
		if(Equals(szToken, "option")){
			Parse_IntItem(TEXT("/option"), settings.m_fInterpolateDark);
			continue;
		}
	}
	return 0;
}


/** Parses a fit-window settings section */
int CReEvalSettingsFileHandler::ParseFitWindow(ReEvaluation::CReEvaluationSettings &settings){
	Evaluation::CFitWindow &curWindow = settings.m_window;
	
	// Reset the window first before we start inserting any references into it
	curWindow.nRef = 0;

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of the Fit-window section
		if(Equals(szToken, "/FitWindow")){
			return 0;
		}

		// The name of the fit-window
		if(Equals(szToken, "name")){
			Parse_StringItem(TEXT("/name"), curWindow.name);
			continue;
		}

		// The fit-range
		if(Equals(szToken, "fitLow")){
			Parse_IntItem(TEXT("/fitLow"), curWindow.fitLow);
			continue;
		}

		if(Equals(szToken, "fitHigh")){
			Parse_IntItem(TEXT("/fitHigh"), curWindow.fitHigh);
			continue;
		}

		// The channel that this window will be used for
		if(Equals(szToken, "spec_channel")){
			Parse_IntItem(TEXT("/spec_channel"), curWindow.channel);
			continue;
		}
		
		// The polynomial to use
		if(Equals(szToken, "polynomial")){
			Parse_IntItem(TEXT("/polynomial"), curWindow.polyOrder);
			continue;
		}

		// The channel that this fit window is valid for
		if(Equals(szToken, "channel")){
			Parse_IntItem(TEXT("/channel"), curWindow.channel);
			continue;
		}
		
		// The type of fit to use
		if(Equals(szToken, "fittype")){
			Parse_IntItem(TEXT("/fittype"), (int &)curWindow.fitType);
			continue;
		}
		
		// The References
		if(Equals(szToken, "Reference")){
			ParseReference(curWindow.ref[curWindow.nRef]);
			++curWindow.nRef;
			continue;
		}
	}

	return 0;
}

/** Parses a reference-file section */
int CReEvalSettingsFileHandler::ParseReference(Evaluation::CReferenceFile &reference){
	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
		continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
		continue;
		}

		// the end of the Reference section
		if(Equals(szToken, "/Reference")){
		return 0;
		}

		// The name of the specie
		if(Equals(szToken, "name")){
			Parse_StringItem(TEXT("/name"), reference.m_specieName);
			continue;
		}

		// The path of the specie
		if(Equals(szToken, "path")){
			Parse_StringItem(TEXT("/path"), reference.m_path);
			continue;
		}

		// The gas-factor of the specie
		if(Equals(szToken, "gasFactor")){
			this->Parse_FloatItem(TEXT("/gasFactor"), reference.m_gasFactor);
			continue;
		}

		// The shift to use
		if(Equals(szToken, "shift")){
			this->Parse_ShiftOrSqueeze(TEXT("/shift"), reference.m_shiftOption, reference.m_shiftValue);
			continue;
		}

		// The squeeze to use
		if(Equals(szToken, "squeeze")){
			this->Parse_ShiftOrSqueeze(TEXT("/squeeze"), reference.m_squeezeOption, reference.m_squeezeValue);
			continue;
		}
	}

	return 0;
}

/** Parses a shift or squeeze section */
int CReEvalSettingsFileHandler::Parse_ShiftOrSqueeze(const CString &label, Evaluation::SHIFT_TYPE &option, double &lowValue/*, double &highValue*/){
	char *pt = nullptr;

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
		if(Equals(szToken, label)){
		return 0;
		}
		// convert the string to lowercase
		_strlwr(szToken);

		if(pt = strstr(szToken, "fix to")){
			sscanf(szToken, "fix to %lf", &lowValue);
			option = Evaluation::SHIFT_FIX;
		}else if(pt = strstr(szToken, "free")){
			option = Evaluation::SHIFT_FREE;
		//}else if(pt = strstr(szToken, "limit")){
		//	sscanf(szToken, "limit from %lf to %lf", &lowValue, &highValue);
		//	option = Evaluation::SHIFT_LIMIT;
		}else if(pt = strstr(szToken, "link")){
			sscanf(szToken, "link to %lf", &lowValue);
			option = Evaluation::SHIFT_LINK;
		}else if(pt = strstr(szToken, "optimal")){
			option = Evaluation::SHIFT_OPTIMAL;
		}
	}
	return 0;
}
