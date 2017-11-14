#include "stdafx.h"
#include "wavelengthcalibration.h"
#include <math.h>

const double CWavelengthCalibration::HgLines[] = {226.22, 237.83, 248.20, 253.652, 265.20, 275.28, 280.35, 289.36, 
  296.73, 302.150, 312.57, 313.155, 334.148, 365.015, 365.44, 366.33, 390.64, 404.656, 407.783, 433.92, 435.833,
  434.75, 491.60,  546.07, 576.96, 579.07};
const int CWavelengthCalibration::nHgLines = sizeof(HgLines) / sizeof(double);

inline double cDistance(double a, double b) {return (a-b)*(a-b);}

CWavelengthCalibration::CWavelengthCalibration(void)
{
	m_lineNum = 0;
	memset(m_lines, 0, sizeof(LinePosition) * 256);
}

CWavelengthCalibration::~CWavelengthCalibration(void)
{
}

/** This returns the position of the nearest Hg-line 
	to the given wavelength */
double CWavelengthCalibration::GetNearestHgLine(double wavelength, bool &isDouble){
	if(wavelength < HgLines[0] - 20)
		return -1.0;
	if(wavelength > HgLines[nHgLines-1] + 20)
		return -1.0;

	int lowIndex = 0;
	int highIndex = nHgLines - 1;
	
	while(highIndex - lowIndex > 3){
		int midPoint = (highIndex + lowIndex) / 2;
		
		if(wavelength > HgLines[midPoint])
			lowIndex = midPoint;
		else
			highIndex = midPoint;
	}

	double closestDistance = 1e99;
	double nearestLine = -1.0;
	int nearestIndex = -1;
	for(int k = lowIndex; k <= highIndex; ++k){
		double d = cDistance(wavelength, HgLines[k]);
		if(d < closestDistance){
			nearestLine		= HgLines[k];
			nearestIndex	= k;
			closestDistance = d;
		}
	}

	// find out if this is a double-line or not
	isDouble = false;
	if(nearestIndex > 0 && nearestIndex < nHgLines-1){
		if(fabs(HgLines[nearestIndex] - HgLines[nearestIndex+1]) < 1.0)
			isDouble = true;
		else if(fabs(HgLines[nearestIndex] - HgLines[nearestIndex-1]) < 1.0)
			isDouble = true;
	}

	return nearestLine;
}