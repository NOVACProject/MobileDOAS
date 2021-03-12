#pragma once

#include "../Spectrometer.h"
#include "../CWavelengthCalibration.h"

/** The class <b>CMeasurement_View</b> is the implementation of the viewing of 
	spectra from the spectrometer. It extends the functions found in CSpectrometer
*/

class CMeasurement_Calibrate : public CSpectrometer
{
public:
	CMeasurement_Calibrate(void);
	~CMeasurement_Calibrate(void);
	
	/** This is the calibration object, used to store the wavelength calibration */
	CWavelengthCalibration *m_calibration;
	
	/** This is a copy of the last collected spectrum */
	double m_lastSpectrum[MAX_SPECTRUM_LENGTH];
		
	/** This is used to collect spectra from the spectrometer and show them in the interface
		without performing any data-analysis	
	*/
	void Run();	

private:
	/** Looks at the current spectrum and assigns a 
		mercury line to each of the peaks in the spectrum */
	void AssignLines();
	

	typedef struct{
		double	maxIntensity;
		int		lowPixel;
		int		highPixel;
		bool	saturated;
	}Peak;
};
