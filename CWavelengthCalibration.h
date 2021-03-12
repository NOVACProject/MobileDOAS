#pragma once

#ifndef WAVELENGTHCALIBRATION_H
#define WAVELENGTHCALIBRATION_H

/** The class <b>CWavelengthCalibration</b> is used to assist in the wavelength
	calibration of the spectrometers */
class CWavelengthCalibration
{
public:
	CWavelengthCalibration(void);
	~CWavelengthCalibration(void);
	
	/** This is an assistant data type to store
		the position of each line */
	typedef struct{
		double pixelNumber;
		double wavelength;
		double maxIntensity;
		bool   use;
	}LinePosition;
	
	/** The lines that we have identified */
	LinePosition m_lines[256];
	
	unsigned int m_lineNum;
	
	static const double HgLines[];
	static const int	nHgLines;
	
	/** This returns the position of the nearest Hg-line 
		to the given wavelength.
		-1.0 is returned if the given wavelength is way out of bounds for this function...
		@isDouble - will on successful return be filled with information regarding if this is a double
			peak or not
		 */
	static double GetNearestHgLine(double wavelength, bool &isDouble);
	
};


#endif