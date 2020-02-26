#pragma once

#include "../Spectrometer.h"

class CMeasurement_Directory :
	public CSpectrometer
{
public:
	CMeasurement_Directory();
	~CMeasurement_Directory();


	/** This is used to monitor directory for spectra collected and show the latest in the interface
		without performing any data-analysis
	*/
	void Run();
};

