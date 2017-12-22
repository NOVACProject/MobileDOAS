#include "StdAfx.h"
#include "SpectrumIO.h"
#include <iostream>

CSpectrumIO::CSpectrumIO(void)
{
}

CSpectrumIO::~CSpectrumIO(void)
{
}

int CSpectrumIO::readSTDFile(CString filename, CSpectrum *curSpec){
	char tmpStr[1024];
	int tmpInt, tmpInt2, tmpInt3, i;
	double tmpDouble;

	FILE *f = fopen(filename, "r");

	if(0 == f){
		return 1;
	}

	fscanf(f, "%s\n", tmpStr);
	if(0 != strncmp(tmpStr, "GDBGMNUP", 8)){
		fclose(f);
		return 1;
	}

	fscanf(f, "%d\n", &tmpInt);
	if(tmpInt != 1){
		fclose(f);
		return 1;
	}

	fscanf(f, "%d\n", &tmpInt);
	if(tmpInt > MAX_SPECTRUM_LENGTH || tmpInt < 1){
		fclose(f);
		return 1;
	}

	curSpec->length = tmpInt;

	/* Read the actual data */
	for(i = 0; i < curSpec->length; ++i){
		if(0 == fscanf(f, "%lf\n", &tmpDouble)){
			fclose(f);
			return 1;
		}else{
			curSpec->I[i] = tmpDouble;
		}
	}

	// the name of the spectrum, this is ignored at the moment
	if(0 == fgets(tmpStr, 1024, f)){
		fclose(f);
		return 1;
	}

	// the name of the spectrometer
	if(0 == fgets(tmpStr, 1024, f)){
		fclose(f);
		return 1;
	}else{
		curSpec->spectrometer.Format(tmpStr);
	}
	
	// the name of the detector, this is ignored at the moment
	if(0 == fgets(tmpStr, 1024, f)){
		fclose(f);
		return 1;
	}
	  
	/* Read the date */
	if(3 != fscanf(f, "%d.%d.%d\n", &tmpInt, &tmpInt2, &tmpInt3)){

	}else{
		curSpec->date[0] = tmpInt;
		curSpec->date[1] = tmpInt2;
		curSpec->date[2] = tmpInt3;
	}

	/* Read the start time */
	if(3 != fscanf(f, "%d:%d:%d\n", &tmpInt, &tmpInt2, &tmpInt3)){

	}else{
		curSpec->startTime[0] = tmpInt;
		curSpec->startTime[1] = tmpInt2;
		curSpec->startTime[2] = tmpInt3;
	}

	/* Read the stop time */
	if(3 != fscanf(f, "%d:%d:%d\n", &tmpInt, &tmpInt2, &tmpInt3)){

	}else{
		curSpec->stopTime[0] = tmpInt;
		curSpec->stopTime[1] = tmpInt2;
		curSpec->stopTime[2] = tmpInt3;
	}

	/* some information that is ignored for the moment s*/
	for(i = 0; i < 2; ++i){
		if(0 == fgets(tmpStr, 1024, f)){
		fclose(f);
		return 1;
		}
	}

	if(0 == fscanf(f, "SCANS %d\n", &tmpInt)){
		fclose(f);
		return 1;
	}else{
		curSpec->scans = tmpInt;
	}

	if(0 == fscanf(f, "INT_TIME %lf\n", &tmpDouble)){
		fclose(f);
		return 1;
	}else{
		curSpec->intTime = (int)tmpDouble;
	}

	/* The site is ignored */
	if((0 == fgets(tmpStr, 1024, f)) && (0 == strstr(tmpStr, "SITE"))){
		//  if(0 == fscanf(f, "SITE %s\n", tmpStr)){
		fclose(f);
		return 1;
	}else{
	}

	if(0 == fscanf(f, "LONGITUDE %lf\n", &tmpDouble)){
		fclose(f);
		return 1;
	}else{
		curSpec->lon = tmpDouble;
	}

	if(0 == fscanf(f, "LATITUDE %lf\n", &tmpDouble)){
		fclose(f);
		return 1;
	}else{
		curSpec->lat = tmpDouble;
	}

	fclose(f);
	return 0;
}

int CSpectrumIO::readTextFile(CString filename, CSpectrum *curSpec){
	double tmpDouble1, tmpDouble2;
	int tmpInt = 0;
	int ret;
	CFileException exceFile;
	CStdioFile fileRef;

	if(!fileRef.Open(filename, CFile::modeRead | CFile::typeText, &exceFile)){
		MessageBox(NULL,TEXT("Can not read log file"),TEXT("Error"),MB_OK);
		return FALSE;
	}

	CString szLine2;
	while(fileRef.ReadString(szLine2)){
		char* szToken = (char*)(LPCSTR)szLine2;
		
		while(szToken = strtok(szToken,"\n")){
			
			ret = sscanf(szToken, "%lf\t%lf", &tmpDouble1, &tmpDouble2);

		if(ret == 1){
			curSpec->I[tmpInt] = tmpDouble1;
		}else if(ret == 2){
			curSpec->I[tmpInt] = tmpDouble2;
		}else{

		}
		curSpec->length = ++tmpInt;

		szToken = NULL;
		}
	}

	curSpec->scans = 1;

//  fclose(f);
	return 0;

}

bool CSpectrumIO::WriteStdFile(const CString &fileName, const double *spectrum, long specLength, char* startdate, long starttime, long stoptime, double lat, double lon, long integrationTime, const CString &spectrometer, const CString &measName, long exposureNum){
	int extendedFormat = 1;
	long i;
	FILE *f;
	int hr,min,sec,hr2,min2,sec2;	
	GetHrMinSec(starttime, hr, min, sec);
	GetHrMinSec(stoptime, hr2, min2, sec2);

	f=fopen(fileName,"w");
	if(f < (FILE*)1){
		//CString tmpStr;
		//tmpStr.Format("Could not save spectrum file: %s. Not enough free space?", name);
		//MessageBox(NULL, tmpStr, "Big Error", MB_OK);
			return FAIL;
	}

	fprintf(f,"GDBGMNUP\n");
	fprintf(f,"1\n");
	fprintf(f,"%d\n",specLength);

	for(i=0;i<specLength;i++)
	{
		fprintf(f,"%.9lf\n", spectrum[i]);
	}

	// date
	std::string dmy = std::string(startdate);
	std::string d = dmy.substr(0,2);
	std::string m = dmy.substr(2, 2);
	std::string y = dmy.substr(4, 2);
	dmy = d + '.' + m + '.' + y;
	char *datetxt = new char[dmy.length()+1];
	std::strcpy(datetxt, dmy.c_str());

	// Find the name of the file itself (removing the path)
	CString name;
	name.Format(fileName);
	Common::GetFileName(name);

	fprintf(f,"%s\n", name);                /* The name of the spectrum */
	fprintf(f,"%s\n", spectrometer);  /* The name of the spectrometer */
	fprintf(f,"%s\n", spectrometer);
	
	fprintf(f,"%s\n",datetxt);
//	GetTimeText(txt);

	fprintf(f,"%02d:%02d:%02d\n",hr,min,sec);
//	fprintf(f,"%s\n",txt);
	
	fprintf(f,"%02d:%02d:%02d\n",hr2,min2,sec2);
	fprintf(f,"0.0\n");
	fprintf(f,"0.0\n");
	fprintf(f,"SCANS %ld\n",		exposureNum);
	fprintf(f,"INT_TIME %d\n",	integrationTime);
	fprintf(f,"SITE %s\n",			measName);
	fprintf(f,"LONGITUDE %f\n",	lon);
	fprintf(f,"LATITUDE %f\n",	lat);

	if(extendedFormat){
		fprintf(f, "Author = \"\"\n");
		fprintf(f, "Average = 0\n");
		fprintf(f, "AzimuthAngle = 0\n");
		fprintf(f, "Delta = 0\n");
		fprintf(f, "DeltaRel = 0\n");
		fprintf(f, "Deviation = 0\n");
		fprintf(f, "Device = \"\"\n");
		fprintf(f, "ElevationAngle = 90\n");
		fprintf(f, "ExposureTime = %d\n",				integrationTime);
		fprintf(f, "FileName = %s\n",						fileName);
		fprintf(f, "FitHigh = 0\n");
		fprintf(f, "FitLow = 0\n");
		fprintf(f, "Gain = 0\n");
		fprintf(f, "Latitude = %.6lf\n",				lat);
		fprintf(f, "LightPath = 0\n");
		fprintf(f, "LightSource = \"\"\n");
		fprintf(f, "Longitude = %.6lf\n",				lon);
		fprintf(f, "Marker = %d\n",							specLength / 2);
		fprintf(f, "MathHigh = %d\n",						specLength - 1);
		fprintf(f, "MathLow = 0\n");
		fprintf(f, "Max = 0\n");
		fprintf(f, "MaxChannel = %d\n",					specLength);
		fprintf(f, "Min = 0\n");
		fprintf(f, "MinChannel = 0\n");
		fprintf(f, "MultiChannelCounter = 0\n");
		fprintf(f, "Name = \"%s\"\n",						measName);
		fprintf(f, "NumScans = %d\n",						exposureNum);
		fprintf(f, "OpticalDensity = 0\n");
		fprintf(f, "OpticalDensityCenter = %d\n",	specLength / 2);
		fprintf(f, "OpticalDensityLeft = 0\n");
		fprintf(f, "OpticalDensityRight = %d\n",	specLength - 1);
		fprintf(f, "Pressure = 0\n");
		fprintf(f, "Remark = \"\"\n");
		fprintf(f, "ScanGeometry = 0\n"); //(DoasCore.Math.ScanGeometry)SAZ: 137.41237083135 SZA: 31.5085943481828 LAZ: 298.523110145623 LAZ: 129.285101310559 Date: 1/5/2007 10:35:07 Lat.: 0 Lon.: 0\n");
		fprintf(f, "ScanMax = 0\n");
		fprintf(f, "Temperature = 0\n");
		fprintf(f, "Variance = 0\n");
	}

	fclose(f);

	return SUCCESS;
}
