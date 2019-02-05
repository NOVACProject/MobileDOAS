#include "StdAfx.h"
#include "Spectrum.h"
#include "../Common/DateTime.h"
#include <algorithm>
#include <string>

#undef min
#undef max

CSpectrum::CSpectrum(void)
{
	this->Clear();
}

CSpectrum::~CSpectrum(void)
{
}

CSpectrum::CSpectrum(const CSpectrum& other) {
	this->isDark = other.isDark;
	this->length = other.length;
	this->exposureTime = other.exposureTime;
	this->lat = other.lat;
	this->lon = other.lon;
	this->altitude = other.altitude;
	this->scans = other.scans;
	this->spectrometerSerial.Format(other.spectrometerSerial);
	this->spectrometerModel.Format(other.spectrometerModel);
	this->name.Format(other.name);
	this->boardTemperature = other.boardTemperature;
	this->detectorTemperature = other.detectorTemperature;
	this->fitHigh = other.fitHigh;
	this->fitLow = other.fitLow;

	memcpy(this->date, other.date, 3 * sizeof(int));
	memcpy(this->startTime, other.startTime, 3 * sizeof(int));
	memcpy(this->stopTime, other.stopTime, 3 * sizeof(int));

	memcpy(this->I, other.I, MAX_SPECTRUM_LENGTH * sizeof(double));
}

CSpectrum& CSpectrum::operator=(CSpectrum other)
{
	swap(*this, other);
	return *this;
}

void swap(CSpectrum & first, CSpectrum & second)
{
	using std::swap;

	swap(first.isDark, second.isDark);
	swap(first.length, second.length);
	swap(first.exposureTime, second.exposureTime);
	swap(first.lat, second.lat);
	swap(first.lon, second.lon);
	swap(first.altitude, second.altitude);
	swap(first.scans, second.scans);
	swap(first.spectrometerSerial, second.spectrometerSerial);
	swap(first.spectrometerModel, second.spectrometerModel);
	swap(first.name, second.name);
	swap(first.boardTemperature, second.boardTemperature);
	swap(first.boardTemperature, second.detectorTemperature);
	swap(first.fitHigh, second.fitHigh);
	swap(first.fitLow, second.fitLow);

	for(int ii = 0; ii < 3; ++ii) {
		swap(first.date[ii], second.date[ii]);
		swap(first.startTime[ii], second.startTime[ii]);
		swap(first.stopTime[ii], second.stopTime[ii]);
	}

	for (int ii = 0; ii < first.length; ++ii) {
		swap(first.I[ii], second.I[ii]);
	}
}

void CSpectrum::GetMinMax(double& minValue, double&maxValue) const
{
	if (0 == this->length)
	{
		minValue = 0.0;
		maxValue = 0.0;
		return;
	}

	minValue = I[0];
	maxValue = I[0];

	for (int ii = 0; ii < this->length; ++ii)
	{
		minValue = std::min(minValue, I[ii]);
		maxValue = std::max(maxValue, I[ii]);
	}
}

double CSpectrum::GetMax() const {
	double maxValue = 0;

	for (int i = 0; i < this->length; ++i)
	{
		maxValue = std::max(maxValue, I[i]);
	}

	return maxValue;
}

double CSpectrum::GetAverage() const {

	if (this->length == 0)
		return 0;
	else {
		double sum = this->GetSum();
		return (sum / this->length);
	}
}

double CSpectrum::GetAverage(int low, int high) const {
	low = std::max(low, 0);
	high = std::min(high, this->length);

	if (this->length == 0) {
		return 0;
	}

	if (high == low) {
		return 0;
	}

	double sum = this->GetSum(low, high);
	return (sum / (high - low));
}

double CSpectrum::GetSum() const {
	double sum = 0;
	
	for (int i = 0; i < this->length; ++i) {
		sum += I[i];
	}

	return sum;
}

double CSpectrum::GetSum(int low, int high) const {
	double sum = 0;

	if (low > high) {
		return 0;
	}

	low  = std::max(low, 0);
	high = std::min(high, this->length);

	for (int i = low; i < high; ++i) {
		sum += I[i];
	}

	return sum;
}

bool CSpectrum::Add(CSpectrum &spec2) {
	if (this->length != spec2.length) {
		return false;
	}

	for (int i = 0; i < this->length; ++i) {
		this->I[i] += spec2.I[i];
	}

	scans += spec2.scans;

	return true;
}

bool CSpectrum::Div(CSpectrum &spec2) {
	if (this->length != spec2.length) {
		return false;
	}

	for (int i = 0; i < this->length; ++i) {
		if (spec2.I[i] == 0) {
			this->I[i] = 0;
		}
		else {
			this->I[i] /= spec2.I[i];
		}
	}
	return true;
}

bool CSpectrum::Sub(CSpectrum &spec2) {
	if (this->length != spec2.length) {
		return false;
	}

	for (int i = 0; i < this->length; ++i) {
		this->I[i] -= spec2.I[i];
	}
	return true;
}

bool CSpectrum::Add(double value) {
	for (int i = 0; i < this->length; ++i) {
		this->I[i] += value;
	}
	return true;
}

bool CSpectrum::Div(double value) {
	if (value == 0) {
		return false;
	}

	for (int i = 0; i < this->length; ++i) {
		this->I[i] /= value;
	}
	return true;
}

bool CSpectrum::Mult(double value) {
	if (value == 0) {
		return false;
	}

	for (int i = 0; i < this->length; ++i) {
		this->I[i] *= value;
	}
	return true;
}

bool CSpectrum::Sub(double value) {
	for (int i = 0; i < this->length; ++i) {
		this->I[i] -= value;
	}
	return true;
}

// clearing out the information in the spectrum
void CSpectrum::Clear() {
	length = 0;
	scans = 0;
	exposureTime = 0;
	lat = 0.0;
	lon = 0.0;
	altitude = 0.0;
	spectrometerSerial.Format("");
	spectrometerModel.Format("");
	name.Format("");
	fitHigh = std::numeric_limits<double>::quiet_NaN();
	fitLow = std::numeric_limits<double>::quiet_NaN();
	boardTemperature = std::numeric_limits<double>::quiet_NaN();
	detectorTemperature = std::numeric_limits<double>::quiet_NaN();

	date[0] = date[1] = date[2] = 0;
	startTime[0] = startTime[1] = startTime[2] = 0;
	stopTime[0] = stopTime[1] = stopTime[2] = 0;

	isDark = false;

	memset(I, 0, MAX_SPECTRUM_LENGTH * sizeof(double));
}

void CSpectrum::SetDate(std::string startDate) {
	std::string dmy = std::string(startDate);
	date[0] = std::stoi(dmy.substr(4, 2)); // 2-digit year
	date[1] = std::stoi(dmy.substr(2, 2)); // month
	date[2] = std::stoi(dmy.substr(0, 2)); // day
}

void CSpectrum::SetStartTime(long time) {
	GetHrMinSec(time, startTime[0], startTime[1], startTime[2]);
}

void CSpectrum::SetStopTime(long time) {
	GetHrMinSec(time, stopTime[0], stopTime[1], stopTime[2]);
}

int CSpectrum::readSTDFile(CString filename) {
	char tmpStr[1024];
	int tmpInt, tmpInt2, tmpInt3, i;
	double tmpDouble;

	FILE *f = fopen(filename, "r");

	if (0 == f) {
		return 1;
	}

	fscanf(f, "%1022s\n", tmpStr);
	if (0 != strncmp(tmpStr, "GDBGMNUP", 8)) {
		fclose(f);
		return 1;
	}

	fscanf(f, "%d\n", &tmpInt);
	if (tmpInt != 1) {
		fclose(f);
		return 1;
	}

	fscanf(f, "%d\n", &tmpInt);
	if (tmpInt > MAX_SPECTRUM_LENGTH || tmpInt < 1) {
		fclose(f);
		return 1;
	}

	this->length = tmpInt;

	/* Read the actual data */
	for (i = 0; i < this->length; ++i) {
		if (0 == fscanf(f, "%lf\n", &tmpDouble)) {
			fclose(f);
			return 1;
		}
		else {
			this->I[i] = tmpDouble;
		}
	}

	// the name of the spectrum, this is ignored at the moment
	if (0 == fgets(tmpStr, 1024, f)) {
		fclose(f);
		return 1;
	}

	// the name of the spectrometer
	if (0 == fgets(tmpStr, 1024, f)) {
		fclose(f);
		return 1;
	}
	else {
		this->spectrometerSerial.Format(tmpStr);
	}

	// the name of the detector, this is ignored at the moment
	if (0 == fgets(tmpStr, 1024, f)) {
		fclose(f);
		return 1;
	}

	/* Read the date */
	if (3 != fscanf(f, "%d.%d.%d\n", &tmpInt, &tmpInt2, &tmpInt3)) {

	}
	else {
		this->date[0] = tmpInt;
		this->date[1] = tmpInt2;
		this->date[2] = tmpInt3;
	}

	/* Read the start time */
	if (3 != fscanf(f, "%d:%d:%d\n", &tmpInt, &tmpInt2, &tmpInt3)) {

	}
	else {
		this->startTime[0] = tmpInt;
		this->startTime[1] = tmpInt2;
		this->startTime[2] = tmpInt3;
	}

	/* Read the stop time */
	if (3 != fscanf(f, "%d:%d:%d\n", &tmpInt, &tmpInt2, &tmpInt3)) {

	}
	else {
		this->stopTime[0] = tmpInt;
		this->stopTime[1] = tmpInt2;
		this->stopTime[2] = tmpInt3;
	}

	/* some information that is ignored for the moment s*/
	for (i = 0; i < 2; ++i) {
		if (0 == fgets(tmpStr, 1024, f)) {
			fclose(f);
			return 1;
		}
	}

	if (0 == fscanf(f, "SCANS %d\n", &tmpInt)) {
		fclose(f);
		return 1;
	}
	else {
		this->scans = tmpInt;
	}

	if (0 == fscanf(f, "INT_TIME %lf\n", &tmpDouble)) {
		fclose(f);
		return 1;
	}
	else {
		this->exposureTime = (int)tmpDouble;
	}

	/* The site is ignored */
	if ((0 == fgets(tmpStr, 1024, f)) && (0 == strstr(tmpStr, "SITE"))) {
		//  if(0 == fscanf(f, "SITE %s\n", tmpStr)){
		fclose(f);
		return 1;
	}
	else {
	}

	if (0 == fscanf(f, "LONGITUDE %lf\n", &tmpDouble)) {
		fclose(f);
		return 1;
	}
	else {
		this->lon = tmpDouble;
	}

	if (0 == fscanf(f, "LATITUDE %lf\n", &tmpDouble)) {
		fclose(f);
		return 1;
	}
	else {
		this->lat = tmpDouble;
	}

	fclose(f);
	return 0;
}


bool CSpectrum::WriteStdFile(const CString &fileName) {
	FILE *f;

	f = fopen(fileName, "w");
	if (f < (FILE*)1) {
		return FAIL;
	}

	fprintf(f, "GDBGMNUP\n");
	fprintf(f, "1\n");
	fprintf(f, "%ld\n", length);

	for (long i = 0; i<length; i++)
	{
		fprintf(f, "%.9lf\n", I[i]);
	}

	// Find the name of the file itself (removing the path)
	CString fname;
	fname.Format(fileName);
	Common::GetFileName(fname);
	fprintf(f, "%s\n", fname);                /* The name of the spectrum */
	fprintf(f, "%s\n", spectrometerModel);  /* The name of the spectrometer */
	fprintf(f, "%s\n", spectrometerSerial);  /* The name of the spectrometer */

	fprintf(f, "%02d.%02d.%02d\n", date[2], date[1], date[0]); // start date
	fprintf(f, "%02d:%02d:%02d\n", startTime[0], startTime[1], startTime[2]); // start time
	fprintf(f, "%02d:%02d:%02d\n", stopTime[0], stopTime[1], stopTime[2]); // stop time
	fprintf(f, "0.0\n");	// wavelength of highest channel
	fprintf(f, "0.0\n");	// always zero
	fprintf(f, "SCANS %ld\n", scans);	// number of scans
	fprintf(f, "INT_TIME %ld\n", exposureTime / 1000);	// integration time in seconds
	fprintf(f, "SITE %s\n", name);	// site name
	fprintf(f, "LONGITUDE %f\n", lon); // site longitude
	fprintf(f, "LATITUDE %f\n", lat);	// site latitude

	// extended format
	double min, max;
	GetMinMax(min, max);
	fprintf(f, "Altitude = %.1lf\n", altitude);
	//fprintf(f, "Author = \"\"\n");
	fprintf(f, "Average = %ld\n", GetAverage());
	//fprintf(f, "AzimuthAngle = 0\n");
	//fprintf(f, "Delta = 0\n");
	//fprintf(f, "DeltaRel = 0\n");
	//fprintf(f, "Deviation = 0\n");
	//fprintf(f, "Device = \"\"\n");
	//fprintf(f, "ElevationAngle = 90\n");
	fprintf(f, "ExposureTime = %ld\n", exposureTime/1000);
	fprintf(f, "FileName = %s\n", fileName);
	fprintf(f, "FitHigh = %d\n", fitHigh);
	fprintf(f, "FitLow = %d\n", fitLow);
	//fprintf(f, "Gain = 0\n");
	fprintf(f, "Latitude = %.6lf\n", lat);
	//fprintf(f, "LightPath = 0\n");
	//fprintf(f, "LightSource = \"\"\n");
	fprintf(f, "Longitude = %.6lf\n", lon);
	//fprintf(f, "Marker = %ld\n", length / 2);
	//fprintf(f, "MathHigh = %ld\n", length - 1);
	//fprintf(f, "MathLow = 0\n");
	fprintf(f, "Max = %ld\n", max);
	//fprintf(f, "MaxChannel = %ld\n", length);
	fprintf(f, "Min = %ld\n", min);
	//fprintf(f, "MinChannel = 0\n");
	//fprintf(f, "MultiChannelCounter = 0\n");
	fprintf(f, "Name = \"%s\"\n", name);
	fprintf(f, "NumScans = %ld\n", scans);
	//fprintf(f, "OpticalDensity = 0\n");
	//fprintf(f, "OpticalDensityCenter = %ld\n", length / 2);
	//fprintf(f, "OpticalDensityLeft = 0\n");
	//fprintf(f, "OpticalDensityRight = %ld\n", length - 1);
	//fprintf(f, "Pressure = 0\n");
	//fprintf(f, "Remark = \"\"\n");
	//fprintf(f, "ScanGeometry = 0\n"); //(DoasCore.Math.ScanGeometry)SAZ: 137.41237083135 SZA: 31.5085943481828 LAZ: 298.523110145623 LAZ: 129.285101310559 Date: 1/5/2007 10:35:07 Lat.: 0 Lon.: 0\n");
	//fprintf(f, "ScanMax = 0\n");
	if (!std::isnan(boardTemperature)) {
		fprintf(f, "BoardTemperature = %ld\n", boardTemperature);
	}
	if (!std::isnan(boardTemperature)) {
		fprintf(f, "DetectorTemperature = %ld\n", detectorTemperature);
	}
	//fprintf(f, "Variance = 0\n");
	

	fclose(f);

	return SUCCESS;
}
