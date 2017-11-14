#pragma once
#include <afxtempl.h>
#include "../Common.h"
#include "FitWindow.h"

/** The class <b>CTraverseResult</b> is used to store the evaluated
		data from one spectrometer channel during one traverse. 
		If several spectrometer channels are used, then one CTraverseResult
		should be defined for each channel.
		*/
class CTraverseResult
{
public:
	CTraverseResult(void);
	~CTraverseResult(void);

	/** Returns the number of spectra in this traverse */
	unsigned long	GetSpectrumNum();

	/** Returns the number of species that these spectra were
			evaluated for */
	unsigned long	GetSpecieNum();

	/** Sets the evaluated column for the given spectrum and specie */
	void	SetColumn(int spectrumIndex, int specieIndex, double column);

	/** Sets the evaluated column error for the given spectrum and specie */
	void	SetColumnError(int spectrumIndex, int specieIndex, double columnError);

	/** Sets the gps-altitude for the given spectrum */
	void	SetAltitude(int spectrumIndex, double altitude);

	/** Sets the gps-latitude for the given spectrum */
	void	SetLatitude(int spectrumIndex, double latitude);

	/** Sets the gps-longitude for the given spectrum */
	void	SetLongitude(int spectrumIndex, double longitude);

	/** Sets the exposure-time for the given spectrum */
	void	SetExptime(int spectrumIndex, long exptime);

	/** Sets the number of exposures for the given spectrum */
	void	SetNumExposures(int spectrumIndex, int numExp);

	/** Sets the intensity for the given spectrum */
	void	SetIntensity(int spectrumIndex, double intensity);

	/** Sets the start-time for the given spectrum */
	void	SetStartTime(int spectrumIndex, const Time &startTime);

protected:

	/** A datastructure to hold the result from the evaluation of one
			spectrum. 
		For each spectrum in the traverse, we can fit several trace gas
		references, this datastructure is intended to contain the result
		of the evaluation of one spectrum. */
	typedef struct EvaluationResult{
		double column[MAX_N_REFERENCES];
		double columnError[MAX_N_REFERENCES];
		double shift[MAX_N_REFERENCES];
		double shiftError[MAX_N_REFERENCES];
		double squeeze[MAX_N_REFERENCES];
		double squeezeError[MAX_N_REFERENCES];
	}EvaluationResult;

	/** A datastructure to hold additional information about one spectrum */
	typedef struct SpectrumInfo{
		long	expTime;		// <-- the exposure-time [ms]
		long	numExposures;	// <-- the number of exposures
		double	intensity;		// <-- the intensity of the spectrum
	}SpectrumInfo;

	/** The datastructure that knows everything about a spectrum. */
	typedef struct Measurement{
		EvaluationResult	result;		// <-- the evaluated result
		SpectrumInfo		info;		// <-- additional info
		gpsPosition			gps;		// <-- the latitude, longitude where the spectrum was collected
		Time				gmtTime;	// <-- the UTC-time when the spectrum was collected
	}Measurement;

	/** The evaluated results. */
	CArray <Measurement, Measurement&> m_measurement;

	/** The number of spectra in this traverse */
	unsigned long		m_spectrumNum;

	/** The species that the spectra were evaluated for. */
	CString	m_species[MAX_N_REFERENCES];

	/** The number of species that the spectra were evaluated for.
		This is the length of the array 'm_species' */
	unsigned long	m_speciesNum;

	/** Returns a pointer to a suitably allocated SpectrumInfo-object.
			if spectrumIndex is out of bounds, then NULL is returned. */
	SpectrumInfo *GetSpectrumInfo(int spectrumIndex);

	/** Returns a pointer to a suitably allocated gpsPosition-object.
			if spectrumIndex is out of bounds, then NULL is returned. */
	gpsPosition *GetGPS(int spectrumIndex);

	/** Returns a pointer to a suitably allocated EvaluationResult-object.
			if spectrumIndex is out of bounds, then NULL is returned. */
	EvaluationResult *GetEvaluationResult(int spectrumIndex);

	/** Returns a pointer to a suitably allocated Time-object.
			if spectrumIndex is out of bounds, then NULL is returned. */
	Time *GetTime(int spectrumIndex);
};
