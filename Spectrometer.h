// Spectrometer.h: interface for the CSpectrometer class.
//
//////////////////////////////////////////////////////////////////////
#include "Evaluation/Evaluation.h"
#include "DMSpecDoc.h"
#include "GPS.h"
#include "Configuration/MobileConfiguration.h"
#include "SerialConnection.h"
#include "Version.h"
#include "Common/SpectrumIO.h"

#include <ArrayTypes.h> // located in %OMNIDRIVER_HOME%\include
#include <Wrapper.h>
//#include <ADC1000USB.h>
//#include <ADC1000Channel.h>


#if !defined(AFX_COMMUNICATION_H__7C04DDEA_2314_405E_A09D_02B403AC7762__INCLUDED_)
#define AFX_COMMUNICATION_H__7C04DDEA_2314_405E_A09D_02B403AC7762__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define WM_STATUSMSG		WM_USER + 8   //define the message for set statusbar message
#define WM_DRAWCOLUMN		WM_USER + 9
#define WM_DRAWFLUX			WM_USER +10
#define WM_ERASECOLUMN		WM_USER +11
#define WM_DRAWMAX			WM_USER +12
#define WM_READGPS			WM_USER +14
#define WM_SHOWINTTIME		WM_USER +15
#define WM_DRAWSPECTRUM		WM_USER +16
#define WM_SHOWDIALOG		WM_USER +17
#define WM_CHANGEDSPEC		WM_USER +18
#define WM_CHANGEDSPECSCALE	WM_USER +19


#define DARK_DIALOG           0
#define INVALID_GPS           1
#define CHANGED_EXPOSURETIME  2

// the maximum and minimum exposure time
#define MAX_EXPOSURETIME 5000
#define MIN_EXPOSURETIME 3

// Possible modes for the spectrometer
const enum SPECTROMETER_MODE {MODE_TRAVERSE, MODE_WIND, MODE_VIEW, MODE_CALIBRATE};

extern CFormView* pView;


/** The class <b>CSpectrometer</b> is the base class used when communicating with the
	spectrometer. This holds all basic functions for USB or serial communication,
	it keeps track of the results from the measurement etc.
	
	This is a virtual class and cannot be instansiated as it is. Any instances must
	be of one of the inherited classes (currently: CMeasurement_Traverse, CMeasurement_Wind
	CMeasurement_Calibrate and CMeasurement_View).
*/

class CSpectrometer  
{
protected:
	/** A FitRegion - struct is an easy collection of all the
			parameters and objects that are needed to perform a DOAS-evaluation
			on specific pixel-range in a measured spectrum. The FitRegion-object
			contains all information needed to perform the fit and is able to store
			the result of the fit. */
	typedef struct FitRegion{
		Evaluation::CFitWindow		window;
		Evaluation::CEvaluation*	eval[MAX_N_CHANNELS];
		CVector						vColumn[6]; /* The evaluation results from the master channel [col, colError, shift, shiftError, squeeze, squeezeError] */
		CVector						vColumn2[6]; /* The evaluation results from the slave channel (if there is any) [col, colError, shift, shiftError, squeeze, squeezeError] */
	}FitRegion;
public:
	CSpectrometer();
	virtual ~CSpectrometer();

	/* Running */
	
	/** m_isRunning is true as long as the measurements are running. This is set to false
		when the user wants to quit the application. All running functions will then
		return as soon as they can. */
	volatile bool				m_isRunning;

	/** The actual measurement. This must be overridden in each sub-class */
	virtual void				Run() = 0;
	
	/** Starts the measurements. Sets m_isRunning to true and calls 'Run' */
	int							Start();
	
	/** Stops the measurements. Sets m_isRunning to false */
	int							Stop();

	/** The measurement mode */
	SPECTROMETER_MODE			m_spectrometerMode;

	/* Collects a spectrum from the spectrometer.
		@param sumInComputer - the number of spectra to add together in the computer
		@param sumInSpectrometer - the number of spectra to add together in the spectrometer
		@param pResult - will on successful return be filled with the measured spectrum. Returned spectrum 
			is an average of the (sumInComputer*sumInSpectrometer) collected spectra.
		@return 0 on success 
		 */
	int Scan(long sumInComputer, long sumInSpectrometer, double pResult[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH]);

	/* Collects a spectrum from the spectrometer using the USB-connection
		@param sumInComputer - the number of spectra to add together in the computer
		@param sumInSpectrometer - the number of spectra to add together in the spectrometer
		@param pResult - will on successful return be filled with the measured spectrum. Returned spectrum 
			is an average of the (sumInComputer*sumInSpectrometer) collected spectra.
		@return 0 on success 
		 */
	int ScanUSB(long sumInComputer, long sumInSpectrometer, double pResult[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH]);

	/** The number of channels in the spectrometer to use */
	int     m_NChannels; 
	
	/** The number of spectra to average before writing to file / updating flux.
		This is equal to m_sumInSpectrometer * m_sumInComputer */ 
	long    m_totalSpecNum;
	
	/** number of spectra to average in spectrometer */
	short   m_sumInSpectrometer; 
	
	/** Number of spectra to average in computer */
	int     m_sumInComputer;
	
	/** the desired time resolution of the measurement (i.e. how often a spectrum 
			should) be stored to file. In milli seconds */
	long    m_timeResolution;
	
	/** Contains the name of the spectrometer, if USB-Connection 
		is used this is the serial number of the spectrometer */
	CString m_spectrometerName;
	
	/** The number of pixels on the spectrometer's detector.
		To keep track of how long spectra we should receive */
	long    m_detectorSize;
	
	/** The dynamic range of the spectrometer */
	long    m_spectrometerDynRange;
	
	/** The model of the spectrometer */
	CString m_spectrometerModel;
	
	/** The spectrometer to use, if there are several attached 
		must be at least 0 and always smaller than 'm_numberOfSpectrometersAttached' */
	int		m_spectrometerIndex;
	
	/** The channel to use on the attached spectrometer, this is only used if m_NChannels == 1 */
	int		m_spectrometerChannel; 

	/** The number of spectrometers that are attached to this computer */
	int		m_numberOfSpectrometersAttached;

	/** This will change the spectrometer to use, to the one with the 
		given spectrometerIndex (ranging from 0 to (the number of spectrometers - 1) ). 
		If no spectrometer exist with the given index then no changes will be made.
		@return the spectrometer index actually used
		*/
	int ChangeSpectrometer(int selectedspec, int channel = 0);
	
	/** This retrieves a list of all spectrometers that are connected to this computer */
	void GetConnectedSpecs(CList <CString, CString&> &connectedSpectrometers);

	// -------------------------------------------------------------------------------------
	// ---------------------- Managing the intensity of the spectra ------------------------
	// -------------------------------------------------------------------------------------

	/** Counts how many spectra should be averaged inside the computer and
	    how many should be averaged inside the spectrometer get the desired
	    timeresolution with the set exposure time. */
	int     CountRound(long timeResolution, long serialDelay,long gpsDelay,int* pResults);

	/** Returns the average intensity of the supplied spectrum. */
	long    AverageIntens(double* pSpectrum,long totalNum);

	/** Makes the initial adjustments and sets the 
	  parameter 'integrationTime' so that intensity of 
	  the spectra are at the desired percent of max. */
	short    AdjustIntegrationTime();

	/** Makes a more clever adjustment of the parameter 
			'integrationTime' so that intensity of 
		  the spectra are at the desired percent of max. 
			
			NB: Called from the function 'AdjustIntegrationTime' !!! */
	short		 AdjustIntegrationTime_Calculate(long minExpTime, long maxExpTime);

	/** Calculates the integration time, 
	  given the intensity of the dark and the sky spectra 
	  and the exposure time they were collected with*/
	long GetInttime(long pSky,long pDark, int intT = 100);   

	/** The integration time that is used by the program. In milli seconds*/
	short    m_integrationTime;

	/** The desired intensity of the measured spectra, 
	    in fractions of the maximum value */
	double  m_percent;

	/** if m_fixexptime > 0 the integration time will be set to
	      m_fixexptime, else it will be judged automatically. */
	long    m_fixexptime;

	/** True if the user wants us to update the integration time. */
	BOOL  m_adjustIntegrationTime;

	/** fills up the 'specInfo' structure with information from the supplied spectrum */
	void GetSpectrumInfo(double spectrum[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH]);

	/** Retrieves the (electronic-)offset of the supplied spectrum */
	double GetOffset(double spectrum[MAX_SPECTRUM_LENGTH]);

	/* -------  The spectra ----------- */
	
	/** The exposure time that we should use to collect the dark current spectrum */
	static const int DARK_CURRENT_EXPTIME = 10000;

	/** The scaled (and possibly shifted) references that were fitted
		to the measured spectrum. This is used for plotting mostly */
	double  m_fitResult[MAX_FIT_WINDOWS][MAX_SPECTRUM_LENGTH];

	/** Called to calculate the flux in real-time (during the scope of the measurement) 
		@return the accumulated flux so far */
	double  CountFlux(double windSpeed,double windAngle);
	
	/** The so far accumulated flux in the measurement, set by 'CountFlux' */
	double  m_flux;
	
	/** The wind direction used to calculate the flux.
		Set by the user at startup */
	double  m_windAngle;

	/** The wind speed used to calculate the flux.
		Set by the user at startup */
	double  m_windSpeed;

	/** The gas factor used to calculate the flux.
		Set to 2.66 (SO2) */
	double  m_gasFactor;

	/** Evaluates the given spectrum using the given dark and sky spectra 
		@param pSky - the sky spectrum(-a) to use. Should already be dark-corrected!
		@param pDark - the dark spectrum(-a) to use (for the measured spectrum, 
			the sky should already be dark-corrected
		@param pSpectrum - the spectrum to evaluate.		*/
	void  DoEvaluation(double pSky[][MAX_SPECTRUM_LENGTH], double pDark[][MAX_SPECTRUM_LENGTH], double pSpectrum[][MAX_SPECTRUM_LENGTH]);
	
	/** Copies the current sky-spectrum to 'tmpSky' */
	void  GetSky();

	/** Copies the current dark-spectrum to 'tmpDark' */
	void  GetDark();

	/** Reads the references files from disk */
	int   ReadReferenceFiles();
	
	/** The evaluated results for the last spectrum 
		This is a 3D-matrix, where 
		<b>index 1</b> is the fit window number (each spectrum can be evaluated
			in up to MAX_FIT_WINDOWS regions)
		<b>index 2</b> is the channel number (at each aqcuisition we can get
			up to MAX_N_CHANNELS spectra from the spectrometer)
		<b>index 3</b> is the type of result

		evaluateResult[i][j][0] is the fitted column for spectrum j in fit window i.
		evaluateResult[i][j][1] is the estimated column error for spectrum j in fit window i.
		evaluateResult[i][j][2] is the fitted shift for spectrum j in fit window i.
		evaluateResult[i][j][3] is the estimated shift error for spectrum j in fit window i.
		evaluateResult[i][j][4] is the fitted squeeze for spectrum j in fit window i.
		evaluateResult[i][j][5] is the estimated squeeze error for spectrum j in fit window i.
	*/
	double evaluateResult[MAX_FIT_WINDOWS][MAX_N_CHANNELS][6];
	
	/** This is the evaluation result in the first fit-window for the first 
		spectrometer channel. Same as evaluateResult[0][0] */
	double  m_result[6]; /* [column, columnError, shift, shiftError, squeeze, squeezeError] */

	/** This is an array holding the intensities of the so far collected spectra */
	CVector vIntensity;

	// ---------------------------------------------------------------
	// --------------------------- Output ----------------------------
	// ---------------------------------------------------------------
	
	/** Writes the header of the evaluation file for fitRegion number 'fitRegion' 
		@param fitRegion - the fit-region index for which we should write the 
			evaluation log file header. Must be >= 0 and < m_fitRegionNum.
	*/
	void    WriteBeginEvFile(int fitRegion);
	
	/** Writes the calculated flux to file. Called by 'CountFlux' */
	void    WriteFluxLog();
	
	/** Writes the given string to the given file 
		@param filename - the file to write to
		@param txt - the string to write */
	void    WriteLogFile(CString filename, CString txt);
	
	/** Sets the name of the next .std file that we should write to. 
		Sets the member variable 'm_stdfileName'*/
	void    SetFileName();
	
	/** Takes care of creating the correct structure of directories
		in the output directory
		Sets the variable 'm_subFolder' */
	void    CreateDirectories();
	
	/** Writes the last evaluation result from the given fit region 
		to the specified file-name */
	void    WriteEvFile(CString filename, FitRegion *fitRegion);
	
	/** The directory that we're currently writing to.
		Set by 'CreateDirectories' */
	CString m_subFolder;
	
	/** The file-name (including full path) of the next .std - file
		that we should write (one name for each channel). 
		This is set by 'SetFileName()' */
	CString m_stdfileName[MAX_N_CHANNELS];

	// ---------------------------------------------------------------
	// ----------------------- The GPS -------------------------------
	// ---------------------------------------------------------------
	
	/** Updates the member variables 'specTime' and 'pos' with the last read 
		data from the GPS-thread 
		This will NOT call the Gps itself, nor cause any block.
		@return true if the updated data is valid (i.e. if the GPS can retrieve lat/long).
		@return false if the data is not valid or the GPS isn't used. */
	bool UpdateGpsData();
	
	/** Retrieves the last GPS position */
	int GetGpsPos(gpsData& data) const;
	
	/** Retrieves the current time, either from the GPS or the system time */
	long GetCurrentTime();

	/** Retrieves the current date, either from the GPS or the system time.
		The date is a string formatted as: mmddyy (6 characters) */
	std::string GetCurrentDate();
	
	/** Pointer to the gps reading thread */
	CGPS*   m_gps = nullptr;
	
	/** This is true if we should use the GPS receiver (default behavior).
		Set to false if the gps is missing or nor working. */
	bool	m_useGps = true;
	
	/** The Serial-port that we should read the GPS data from 
		This is something like 'COM4' */
	char    m_GPSPort[20];
	
	/** The baudrate that we should use to communicate with the GPS 
		reveiver, typicallly 9600 */
	long    m_GPSBaudRate = 9600;


	// ----------------------------- Handling the serial communication -----------------
	
	/** The serial-communication object. This is used if we are to communicate with
		the spectrometer through the serial port */
	CSerialConnection serial;
	
	/** Initializes the spectrometer. Only necessary if we're using the serial port 
		@param channel - the channel to use (0 <-> master, 1 <-> slave, 257 <-> master & slave)
		@param inttime - the integration time to use (in milli seconds)
		@param sumSpec - the number of spectra to co-add in the spectrometer
	*/
	int InitSpectrometer(short channel,short inttime,short sumSpec);

	// ------------------- Handling the USB-Connection  --------------------
	
	/** Called to test the USB-connection. 
		@return 1 if successful, else 0 */
	int     TestUSBConnection();
	
	/** Called to close the USB-connection. Should only be done
		when we're about to stop collecting spectra */
	int		CloseUSBConnection();
	
	// ------------------------ Setup ------------------------
	
	/** Applies the settings found in m_conf to the rest 
		of the parameters */
	void    ApplySettings();
	
	/** Checks the settings in 'm_conf' for reasonability and 
		checks that the specified files does exist */
	int     CheckSettings();

	// ------------------------------ Misc ------------------------
	
	 /* Returns the current time in UMT as a long, must have gotten 
		at least one GPS-time value to work properly, otherwise the local time will be returned */
	long    GetTimeValue_UMT();
	
	/** Plays a small sound according to the last evaluated column.
		The volume will be somewhere between 0 and 1 depending on
			if the last column is <=0 and >= m_maxColumn */
	void    Sing(double factor);
	
	/** Updates the mobile-log... This is used to store the
		users preferences between runs */
	void    UpdateMobileLog();

	//  ----------------- Communicating with other parts of the program -----------------
	
	/** gets the number of spectra that are averaged into one */
	void    GetNSpecAverage(int N[MAX_N_CHANNELS]);

	/** Retrieves the last evaluated column.
		@return a pointer to 'm_result' */
	double* GetLastColumn();
	
	/** Returns the last calculated flux */
	double  GetFlux(){ return m_flux; }
	
	/** Retrieves the last 'sum' evaluated columns 
		@param (out) a pointer to an array, will on return be filled with at most 'sum' retrieved columns
		@param sum (in) the desired number of columns to retrieve
		@param fitRegion (in) - the fit region that we want to have the columns for
	*/
	long    GetColumns(double *columnList, long sum, int fitRegion = 0);

	/** Retrieves the last 'sum' estimated columns errors
		@param (out) a pointer to an array, will on return be filled with at most 'sum' estimated columns errors
		@param sum (in) the desired number of columns errors to retrieve
		@param fitRegion (in) - the fit region that we want to have the columns errors for
	*/
	long    GetColumnErrors(double *columnList,long sum, int fitRegion = 0);
	
	/** Retrieve the position for the (at most) 'sum' spectra. 
		@param la (out) - will on return be filled with the latitudes
		@param lo (out) - will on return be filled with the longitudes
		@param al (out) - will on return be filled with the altitudes
		@param sum (in) - the desired number of positions */
	long    GetLatLongAlt(double *la, double *lo, double *al, long sum);

	/** Retrieves the position for the last collected spectrum
		@param la (out) - will on return be filled with the latitude
		@param lo (out) - will on return be filled with the longitude
		@param al (out) - will on return be filled with the altitude */
	void    GetCurrentPos(double *la, double *lo, double *al);
	
	/** Retrieves the GPS-time for the last collected spectrum as a 
		long. */
	long    GetCurrentGPSTime();
	
	/** Retrieves the intensities for the (at most) 'sum' collected spectra
		@param list (out) - will on return be filled with the intensities
		@param sum (in) the desired number of intensities. 
		@return - the number of intensities actually filled into 'list' */
	long    GetIntensity(double *list,long sum);
	
	/** Sets the wind speed, wind direction and basename from the 
		Graphical User Interface */
	void    SetUserParameters(double windspeed, double winddirection, char *baseName);
	
	/** Retrieves the last collected spectrum from the given channel */
	double* GetSpectrum(int channel);
	
	/** Retrieves the wavelength-calibration for the given channel */
	double* GetWavelengths(int channel);
	
	/** @return the number of spectra that have been collected so far */
	long    GetColumnNumber();
	
	/** @return the currently used integration time, in milli seconds */
	long    RequestIntTime(){ return (long)m_integrationTime; }
	
	/** This is the text to show in the status bar of the program*/
	CString m_statusMsg;

	/** Retrieves the lower range for the fit region for 
		fit window number 'region' */
	inline int GetFitLow(int region = 0) const {
		return m_fitRegion[region].window.fitLow;
	}

	/** Retrieves the upper range for the fit region for 
		fit window number 'region' */
	inline int GetFitHigh(int region = 0) const {
		return m_fitRegion[region].window.fitHigh;
	}

	/** Retrieves the number of fit regions that we are evaluating
		each spectrum in */
	inline int GetFitRegionNum() const{
		return m_fitRegionNum;
	}

	/** Retrieves the name of the fit region with the given index. */
	inline const CString &GetFitWindowName(int windowNum) const{
		return m_fitRegion[windowNum].window.name;
	}

	long GetNumberOfSpectraAcquired() const {
		return m_scanNum;
	}

	/** Copies out the last read and processed (high-pass filtered) spectrum. Useful for plotting.
		@return number of copied data points. */
	unsigned int GetProcessedSpectrum(double* dst, unsigned int maxNofElements, int chn = 0) const;

protected:

	/** This is 'true' if we should use the USB-port, if 'false'
		then we should use the serial port */
	bool m_connectViaUsb;

	/** The settings, read in from the cfg.txt - file */
	Configuration::CMobileConfiguration *m_conf;

	/* -------  The spectra ----------- */

	/** The last dark-spectrum measured */
	double m_dark[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];

	/** The last dark-current measured (only used for adaptive integration times) */
	double m_darkCur[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];

	/** The last offset-spectrum measured (only used for adaptive integration times) */
	double m_offset[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];

	/** The measured sky-spectrum that we're using to evaluate the spectra */
	double m_sky[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];

	/** This is a temporary copy of the dark-spectrum */
	double m_tmpDark[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];

	/** This is a temporary copy of the sky-spectrum */
	double m_tmpSky[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];

	/** a copy of the last measured spectrum, used for plotting on the screen */
	double m_curSpectrum[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];

	/** The wavelengths for each pixel in the measured spectrum */
	double m_wavelength[MAX_N_CHANNELS][MAX_SPECTRUM_LENGTH];

	/** The highpass filtered measured spectrum.
	This is used for plotting mostly */
	double  m_spectrum[MAX_FIT_WINDOWS][MAX_SPECTRUM_LENGTH];


	// ---------------------------------------------------------------------------------------
	// --------------------- Keeping track of the route... -------------
	// ---------------------------------------------------------------------------------------

	/** m_spectrumGpsData[i] holds the Gps information associated with spectrum number 'i' */
	struct gpsData m_spectrumGpsData[65536];


	// ---------------------------------------------------------------------------------------
	// --------------------- Keeping track of the offset-level of the spectra... -------------
	// ---------------------------------------------------------------------------------------

	/** A SpectrumInfo struct is used to keep track of the properties
		of collected spectra. So far this only contains the
		(electronic)offset of the spectrum and whether a given spectrum
		is dark or not. */
	typedef struct SpectrumInfo {
		/** The electronic offset of the spectrum, measured in channel 2 - 24 */
		double offset;
		/** True if the program judges that the spectrum is dark */
		bool   isDark;
	}SpectrumInfo;

	/** Information about the last spectrum collected */
	SpectrumInfo m_specInfo[MAX_N_CHANNELS];

	/** Average intensity (at the specified pixel) of the last spectrum that we measured */
	long    m_averageSpectrumIntensity[MAX_N_CHANNELS];
	
	/** Audio option **/
	int m_useAudio = 1;

	/** The column value that causes 'Sing' to sing at the highest available volume */
	double  m_maxColumn;

	/* Spectrum number, only used to judge if this is dark, sky or measurement spectrum */
	long m_scanNum;

	/* Spectrum number, pointer into 'pos' and 'specTime'. Counts how many spectra we have acquired so far. 
		(this differs from m_scanNum but it's not exactly clear how...) */
	long m_spectrumCounter;

	// ----------- Evaluation ------------------

	/** This is the information on the fit windows that we should evaluate
		each spectrum in. We can store at most 'MAX_FIT_WINDOWS' windows */
	FitRegion m_fitRegion[MAX_FIT_WINDOWS];
	
	/** This is how many fit windows we should evaluate each fit window in
		(i.e. the number of useful elements in 'm_fitRegion').
		Must be >= 0 and <= MAX_FIT_WINDOWS */
	long m_fitRegionNum;

	/** The base-name of the measurement. As set by the user */
	CString m_measurementBaseName;

private:


	/** Used by 'CountFlux' to calculate the flux.
		TODO: Is this really necessary?? */
	long    m_posFlag;

	/** Used by 'CountFlux' to calculate the flux
		TODO: Is this really necessary?? */
	long    m_zeroPosNum;

	/** The offset of the last dark-spectrum collected. This is used
		to keep track of wheter we should warn the user about the fact
		that the offset level might have dropped (or increased) since
		the collection of the last dark */
	double m_lastDarkOffset[MAX_N_CHANNELS];

	/** The time difference, in seconds, between UTC and local time */
	long m_timeDiffToUtc = 0U;

	/** The date and time of when the measurement started */
	CString m_measurementStartTimeStr;

	/** This is the object through which we will access all of Omnidriver's capabilities
		This is used to control the OceanOptics Spectrometers through USB.
		There can be only one Wrapper object in the application!!!		*/
	Wrapper m_wrapper;

	/** The wrapper extensions is used to get additional functionality when
		handling the OceanOptics spectrometers using the USB-port */
	WrapperExtensions	m_wrapperExt;
};

#endif // !defined(AFX_COMMUNICATION_H__7C04DDEA_2314_405E_A09D_02B403AC7762__INCLUDED_)
