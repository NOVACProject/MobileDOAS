// GPS.cpp: implementation of the CGPS class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math.h>
#include <time.h>

#include "GPS.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#define PI 3.14159265 // TODO: is this needed here?

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGPS::CGPS(){
	gpsInfo.nSatellites = 0;
	gpsInfo.gpsPos.altitude = 0;
	gpsInfo.gpsPos.latitude = 0;
	gpsInfo.gpsPos.longitude = 0;
	gpsInfo.gpsTime = 0;

	gotContact = false;

	m_gpsThread = NULL;
	fRun = false;
}

CGPS::CGPS(char* pCOMPort, long pBaudrate) {
	CGPS::CGPS();
	serial.baudrate = pBaudrate;
	strcpy(serial.serialPort, pCOMPort);
	if (!serial.Init(pBaudrate)) {
		MessageBox(NULL, "Could not communicate with GPS. No GPS-data can be retrieved!", "Error", MB_OK | MB_SYSTEMMODAL);
	}
	gotContact = true;
}

CGPS::~CGPS(){

	m_gpsThread = NULL;
	serial.Close();
}


/**Get position information - latitude and longitude
*
*/

/** Get the UCT time */
long CGPS::GetTime(){
	return this->gpsInfo.gpsTime;
}

double CGPS::GetAltitude(){
	return this->gpsInfo.gpsPos.altitude;
}

double CGPS::GetLatitude(){
	return this->gpsInfo.gpsPos.latitude;
}

double CGPS::GetLongitude(){
	return this->gpsInfo.gpsPos.longitude;
}

char* CGPS::GetDate() {
	return this->gpsInfo.gpsDate;
}

/** Parse the read GPS-Information */
/** See http://www.gpsinformation.org/dale/nmea.htm/ */
int CGPS::Parse(char *string){

	char sep[]    = ",";   /* the separator */
	char *token   = 0;
	char *stopStr = "\0";
	
	token = strtok(string, sep);  /* get first sentence identifier */

	if (token == NULL)
		return 0;

	while (token != NULL) {
		
		if (0 == strncmp(token, "$GPRMC", 6)) {	// fisrt sentence should be GPRMC

			/* 1: the time */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				this->gpsInfo.gpsTime = strtol(token, &stopStr, 10);
			}

			/* 2: the fix status */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				if (0 == strncmp(token, "A", 1)) {
					this->gpsInfo.nSatellites = 3; /* we can see at least three satellites */
				}
				else {
					this->gpsInfo.nSatellites = -1; /* void */
				}
			}

			/* 3: the latitude */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				this->gpsInfo.gpsPos.latitude = DoubleToAngle(strtod(token, &stopStr));
			}

			/* 4: north/south hemisphere */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				if (0 == strncmp(token, "S", 1))
					this->gpsInfo.gpsPos.latitude = -this->gpsInfo.gpsPos.latitude;
			}

			/* 5: the longitude  */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				this->gpsInfo.gpsPos.longitude = DoubleToAngle(strtod(token, &stopStr));
			}

			/* 6: east/west hemisphere */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				if (0 == strncmp(token, "W", 1))
					this->gpsInfo.gpsPos.longitude = -this->gpsInfo.gpsPos.longitude;
			}

			/* 7: the speed [knots] (ignore) */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				double speed = strtod(token, &stopStr); // not used
			}

			/* 8: bearing [degrees] (ignore) */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				double bearing = strtod(token, &stopStr); // not used
			}

			/* 9: date (mmddyy) */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				sprintf(this->gpsInfo.gpsDate, "%s", token);
			}

			/* 10: magnetic variation(ignore) */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				double mv = strtod(token, &stopStr); // not used
			}
			if (NULL == (token = strtok(NULL, "*"))) {
				return 0;
			}
			else {
				char* mvd = token; // not used
			}

			/* 11:checksum          (ignore) */
		}

		if (0 == strncmp(token, "$GPGGA", 6)) {	// second sentence should be GPGGA
			/* 1: the time */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				this->gpsInfo.gpsTime = strtol(token, &stopStr, 10);
			}

			/* 2: the latitude */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				this->gpsInfo.gpsPos.latitude = DoubleToAngle(strtod(token, &stopStr));
			}

			/* 3: north/south hemisphere */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				if (0 == strncmp(token, "S", 1))
					this->gpsInfo.gpsPos.latitude = -this->gpsInfo.gpsPos.latitude;
			}

			/* 4: longitude */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				this->gpsInfo.gpsPos.longitude = DoubleToAngle(strtod(token, &stopStr));
			}

			/* 5: east/west hemisphere */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				if (0 == strncmp(token, "W", 1))
					this->gpsInfo.gpsPos.longitude = -this->gpsInfo.gpsPos.longitude;
			}

			/* 6: quality of fix (ignore) */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				int quality = strtol(token, &stopStr, 10);
			}

			/* 7: number of satellites being used */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				this->gpsInfo.nSatellites = strtol(token, &stopStr, 10);
			}

			/* 8: "horizontal dillution of precision" (ignore) */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				double hd = strtod(token, &stopStr);
			}

			/* 9: Altitude */
			if (NULL == (token = strtok(NULL, sep))) {
				return 0;
			}
			else {
				this->gpsInfo.gpsPos.altitude = strtod(token, &stopStr);
			}

			// the remainder of stuff
			/*10: geoidal separation in meters (ignore) */
			/*11: age of the deferrential correction data (ignore) */
			/*12: deferential station's ID (ignore) */
			/*13: checksum for the sentence (ignore) */
		}

		token = strtok(NULL, "\n"); // go to end of line
		if (token != NULL) {
			token = strtok(NULL, sep); // get next sentence identifier
		}
	}

	return 1;
}

/* Write log file  */
void CGPS::WriteGPSLog(char *pFile,double *pPos,double pTime){

	FILE *f;
	f = fopen(pFile,"a+");
	if(f < (FILE*)1)
		return;
	
	fprintf(f,"%6.2f\t\t%6.2f\t\t%f\n",pPos[0],pPos[1],pTime);
	fclose(f);
}

void CGPS::WriteLog(char *pFile,char* txt){
	FILE *f;
	f = fopen(pFile,"a+");
	if(f < (FILE*)1)
		return;
	
	fprintf(f,"%s\n",txt);

	fclose(f);
}


/* Get N flag and S flag */
void CGPS::GetDirection(int *flags){
	flags[0] = (gpsInfo.gpsPos.latitude < 0) ? -1 : 1;
	flags[1] = (gpsInfo.gpsPos.longitude < 0) ? -1 : 1;
}

/* The GPS reports latitude and longitude in the format ddmm.mmmm
  , this function converts this to the format dd.dddd */
double CGPS::DoubleToAngle(double rawData){
	int degree;
	double remainder, fDegree;

	remainder	= fmod(rawData,100.0); // The minutes
	degree		= (int)(rawData/100);	 // The degrees
	fDegree		= degree + remainder/60.0;

	return fDegree;
}

void CGPS::Run(){
	m_gpsThread = AfxBeginThread(CollectGPSData, (LPVOID)this, THREAD_PRIORITY_NORMAL,0,0,NULL);
}

void CGPS::Stop(){
	this->fRun = false;
	if(m_gpsThread != NULL){
	}
}

// IsRunning returns true if the gps-collecting thread is running.
bool CGPS::IsRunning(){
	if(m_gpsThread == NULL)
		return false;

	// the thread is probably running
	return true;
}


int CGPS::ReadGPS(){
	long cnt;
	char gpstxt[256];

	gpstxt[0] = 0;

	do{
		cnt = 0;
		serial.FlushSerialPort(10);
		if(serial.Check(550)){
			while(serial.Check(100) && cnt<256){ // Read GPRMC and GPGGA
				serial.Read(gpstxt+cnt,1);
				cnt++;
			}
		}else{
			printf("timeout in getting gps\n");
			serial.FlushSerialPort(1);
			this->gotContact = false;
			return(0);
		}
	}while(!this->Parse(gpstxt));

	#ifdef _DEBUG
	m_logFile.Format("gps.log"); // for testing only
	if(strlen(m_logFile) > 0){
		FILE *f = fopen(g_exePath + m_logFile, "a+");
		fprintf(f, "%s\t%d\t", gpsInfo.gpsDate, gpsInfo.gpsTime);
		fprintf(f, "%lf\t%lf\t%lf\t", gpsInfo.gpsPos.latitude, gpsInfo.gpsPos.longitude, gpsInfo.gpsPos.altitude);
		fprintf(f, "%d\n", gpsInfo.nSatellites);
		fclose(f);
	}
	#endif

	// we've got contact with the gps again
	this->gotContact = true;

	return 1;
}

void CGPS::CloseSerial(){
	this->serial.Close();
}

UINT CollectGPSData(LPVOID pParam){
	CGPS *gps = (CGPS *)pParam;
	gps->fRun = true;

	while(1){
		if(!gps->fRun){
			gps->CloseSerial();
			return 0;
		}

		gps->ReadGPS();

		/* make a small pause */
		Sleep(100);
	}

	return 0;
}
