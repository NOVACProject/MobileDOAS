#include "Evaluation/Evaluation.h"
#include <MobileDoasLib/GPS.h>
#include "Configuration/MobileConfiguration.h"
#include <MobileDoasLib/Measurement/SpectrometerInterface.h>
#include <MobileDoasLib/Measurement/SpectrumUtils.h>
#include <MobileDoasLib/ReferenceFitResult.h>
#include <MobileDoasLib/Measurement/MeasuredSpectrum.h>

#include <memory>
#include <limits>

#pragma once

#define WM_STATUSMSG  WM_USER + 8   //define the message for set statusbar message
#define WM_DRAWCOLUMN  WM_USER + 9
#define WM_DRAWMAX   WM_USER +12
#define WM_READGPS   WM_USER +14
#define WM_SHOWINTTIME  WM_USER +15
#define WM_DRAWSPECTRUM  WM_USER +16
#define WM_SHOWDIALOG  WM_USER +17
#define WM_CHANGEDSPEC  WM_USER +18
#define WM_CHANGEDSPECSCALE WM_USER +19


#define DARK_DIALOG           0
#define INVALID_GPS           1
#define CHANGED_EXPOSURETIME  2

// the maximum and minimum exposure time
#define MAX_EXPOSURETIME 5000
#define MIN_EXPOSURETIME 3

// Possible modes for the spectrometer
const enum SPECTROMETER_MODE { MODE_TRAVERSE, MODE_WIND, MODE_VIEW, MODE_DIRECTORY };

// Forward declarations
namespace novac
{
class CDateTime;
}
class CSpectrum;

/** The class <b>CSpectrometer</b> is the base class used when communicating with the
    spectrometer. This holds all basic functions for USB or serial communication,
    it keeps track of the results from the measurement etc.

    This is a virtual class and cannot be instansiated as it is. Any instances must
    be of one of the inherited classes (currently: CMeasurement_Traverse, CMeasurement_Wind
    and CMeasurement_View).
*/

// TODO: Try to clean up the storing of the results here. The evaluated columns from the master channel are stored in m_fitRegion[idx].vColumn[0],
// the corresponding errors in m_fitRegion[idx].vColumn[1] and the intensities of these spectra in m_intensityOfMeasuredSpectrum...
class CSpectrometer
{
protected:
    /** A FitRegion - struct is an easy collection of all the
            parameters and objects that are needed to perform a DOAS-evaluation
            on specific pixel-range in a measured spectrum. The FitRegion-object
            contains all information needed to perform the fit and is able to store
            the result of the fit. */
    typedef struct FitRegion
    {
        Evaluation::CFitWindow  window;
        Evaluation::CEvaluation* eval[MAX_N_CHANNELS];
        CVector vColumn[6]; /* The evaluation results from the master channel [col, colError, shift, shiftError, squeeze, squeezeError] */
        CVector vColumn2[6]; /* The evaluation results from the slave channel (if there is any) [col, colError, shift, shiftError, squeeze, squeezeError] */
    }FitRegion;

public:
    CSpectrometer(
        CView& mainForm,
        std::unique_ptr<mobiledoas::SpectrometerInterface> spectrometerInterface,
        std::unique_ptr<Configuration::CMobileConfiguration> configuration);

    virtual ~CSpectrometer();

    /* Running */

    /** m_isRunning is true as long as the measurements are running. This is set to false
        when the user wants to quit the application. All running functions will then
        return as soon as they can. */
    volatile bool m_isRunning;

    /** The actual measurement. This must be overridden in each sub-class */
    virtual void Run() = 0;

    /** Starts the measurements. Sets m_isRunning to true and calls 'Run' */
    int Start();

    /** Stops the measurements. Sets m_isRunning to false */
    int Stop();

    /** The measurement mode */
    SPECTROMETER_MODE m_spectrometerMode;

    /** Retrieves the current date and time either from the GPS or from the computer time (if no valid gps-data). */
    void GetCurrentDateAndTime(std::string& currentDate, long& currentTime);

    /** Retrieves the current date and time either from the GPS or from the computer time (if no valid gps-data). */
    void GetCurrentDateAndTime(novac::CDateTime& currentDateAndTime);

    /** This is the text to show in the status bar of the program*/
    CString m_statusMsg;

    /** The number of spectra to average before writing to file / updating flux.
        This is equal to m_sumInSpectrometer * m_sumInComputer */
    long NumberOfSpectraToAverage() const { return m_sumInComputer * m_sumInSpectrometer; }

    /** number of spectra to average in spectrometer */
    int m_sumInSpectrometer = 1;

    /** Number of spectra to average in computer */
    int m_sumInComputer = 1;

    /** The integration time that is used by the program. In milliseconds.
        Maximum value is 65 seconds (from the type). */
    short m_integrationTime;

    /** The detector temperature, as reported by the spectrometer, in degrees Celsius.
    Set to NaN if this could not be read. */
    double detectorTemperature = std::numeric_limits<double>::quiet_NaN();

    /** If detector temperature is within 2 degrees of set point temperature than set to true. */
    bool detectorTemperatureIsSetPointTemp = false;

    /** The model of the spectrometer */
    std::string m_spectrometerModel;

    /* The dynamic range of the spectrometer, i.e.the maximum(absolute) intensity of one spectrum. */
    long m_spectrometerDynRange;

    /** The number of pixels on the spectrometer's detector.
        To keep track of how long spectra we should receive */
    long m_detectorSize;

    /** The number of channels in the spectrometer to use */
    int m_NChannels;

    /** The channel to use on the attached spectrometer, this is only used if m_NChannels == 1 */
    int m_spectrometerChannel;

    /** The spectrometer to use, if there are several attached
        must be at least 0 and always smaller than 'm_numberOfSpectrometersAttached' */
    int m_spectrometerIndex;

    /** The scaled (and possibly shifted) references that were fitted
        to the measured spectrum. This is used for plotting only.
        This measured spectrum contains one vector of data for each fit region used, each region being MAX_SPECTRUM_LENGTH in size.
        Each spectrum is only updated in the fit region used (i.e. in m_fitRegion[fitRegionIdx].window.fitLow to m_fitRegion[fitRegionIdx].window.fitHigh).
        TODO: This should not be publicly available. */
    mobiledoas::MeasuredSpectrum m_fitResult;

    /** Retrieves the last collected spectrum from the given channel */
    std::vector<double> GetSpectrum(int channel) const;

    /** @return the number of spectra that have been collected so far */
    long GetColumnNumber();

    /** Retrieves the last GPS position */
    int GetGpsPos(mobiledoas::GpsData& data) const;

    /** @return true if the GPS currently has connectact with satellites */
    bool GpsGotContact() const;

    /** Retrieves the intensities for the (at most) 'maxNumberOfValues' collected spectra
        @param list (out) - will on return be filled with the intensities
        @param sum (in) the desired number of intensities.
        @return - the number of intensities actually filled into 'list' */
    long GetIntensity(std::vector<double>& list, long maxNumberOfValues) const;

    /** Retrieves the last 'maxNumberOfValues' evaluated columns
        @param list - will will on return be filled with the last 'maxNumberOfValues' retrieved columns
        @param maxNumberOfValues (in) the desired number of columns to retrieve
        @param fitRegion (in) - the fit region that we want to have the columns for
    */
    long GetColumns(std::vector<double>& list, long maxNumberOfValues, int fitRegion = 0) const;

    /** Retrieves the last 'maxNumberOfValues' estimated column errors.
        @param list - will will on return be filled with the last 'maxNumberOfValues' retrieved column errors
        @param maxNumberOfValues (in) the desired number of column errors to retrieve
        @param fitRegion (in) - the fit region that we want to have the columns for
    */
    long GetColumnErrors(std::vector<double>& list, long maxNumberOfValues, int fitRegion = 0) const;

    /** Gets the number of spectra that are averaged in the spectrometer and in the computer */
    void GetNSpecAverage(int& averageInSpectrometer, int& averageInComputer);

    /* Create Spectrum data object. */
    void CreateSpectrum(CSpectrum& spectrum, const std::vector<double>& spec, const std::string& startDate, long startTime, long elapsedSecond);

    /** This retrieves a list of all spectrometers that are connected to this computer
        Notice that this will not attempt to rebuild the list. */
    std::vector<std::string> GetConnectedSpectrometers() const;

    /** This will change the spectrometer to use, to the one with the
        given spectrometerIndex (ranging from 0 to (the number of spectrometers - 1) ).
        If no spectrometer exist with the given index then no changes will be made.
        @return the spectrometer index actually used (-1 if something goes wrong). */
    int ChangeSpectrometer(int selectedspec, const std::vector<int>& channelsToUse);

    /** Retrieves the last evaluated column, columnError, shift, shiftError, squeeze and squeezeError
        for the main evaluated specie in this traverse */
    void GetLastColumn(mobiledoas::ReferenceFitResult& evaluationResult);

    /** Retrieves the lower range for the fit region for
        fit window number 'region' */
    inline int GetFitLow(int region = 0) const
    {
        return m_fitRegion[region].window.fitLow;
    }

    /** Retrieves the upper range for the fit region for
        fit window number 'region' */
    inline int GetFitHigh(int region = 0) const
    {
        return m_fitRegion[region].window.fitHigh;
    }

    /** Copies out the last read and processed (high-pass filtered) spectrum. Useful for plotting.
        @return number of copied data points. */
    unsigned int GetProcessedSpectrum(double* dst, unsigned int maxNofElements, int chn = 0) const;

    /** Retrieves the number of fit regions that we are evaluating
        each spectrum in */
    inline int GetFitRegionNum() const
    {
        return m_fitRegionNum;
    }

    long GetNumberOfSpectraAcquired() const
    {
        return m_scanNum;
    }

    /** Retrieves the name of the fit region with the given index. */
    inline const CString& GetFitWindowName(int windowNum) const
    {
        return m_fitRegion[windowNum].window.name;
    }

    /** @return the currently used integration time, in milli seconds */
    long GetCurrentIntegrationTime() { return (long)m_integrationTime; }

    /** Returns the last calculated flux */
    double GetFlux() { return m_flux; }

    /* GetIntensityRegion returns the pixel range over which the spectrum intensity is measured. */
    std::vector<double> GetIntensityRegion() const;

    /** Retrieve the position for the (at most) 'sum' spectra.
        @param la (out) - will on return be filled with the latitudes
        @param lo (out) - will on return be filled with the longitudes
        @param al (out) - will on return be filled with the altitudes
        @param sum (in) - the desired number of positions */
    long GetLatLongAlt(double* la, double* lo, double* al, long sum);

    /** Sets the wind speed, wind direction and basename from the
        Graphical User Interface */
    void SetUserParameters(double windspeed, double winddirection, char* baseName);

    CString CurrentOutputDirectory() const { return m_subFolder; }

    void RequestIntegrationTimeChange() { this->m_adjustIntegrationTime = TRUE; }

protected:

    /* Collects a spectrum from the spectrometer.
        @param sumInComputer - the number of spectra to add together in the computer
        @param sumInSpectrometer - the number of spectra to add together in the spectrometer
        @param pResult - will on successful return be filled with the measured spectrum. Returned spectrum
            is an average of the (sumInComputer*sumInSpectrometer) collected spectra.
        @return 0 on success
        @return 1 if the collection failed or the collection should stop
         */
    int Scan(int sumInComputer, int sumInSpectrometer, mobiledoas::MeasuredSpectrum& result);

    /** the desired time resolution of the measurement
        (i.e. how often a spectrum should) be stored to file. In milliseconds */
    long m_timeResolution;

    /** Contains the name of the spectrometer, if USB-Connection
        is used this is the serial number of the spectrometer */
    std::string m_spectrometerName;

    /** The number of spectrometers that are attached to this computer */
    int m_numberOfSpectrometersAttached;

    // -------------------------------------------------------------------------------------
    // ---------------------- Managing the intensity of the spectra ------------------------
    // -------------------------------------------------------------------------------------

    /** Counts how many spectra should be averaged inside the computer and
        how many should be averaged inside the spectrometer get the desired
        time resolution with the set exposure time.
        @param timeResolution The set interval betweeen each spectrum to save, in milliseconds.
        @param serialDelay The necessary delay to read out one spectrum from the spectrometer, in milliseconds.
        @param gpsDelay The necessary delay to read out the time and position from the GPS, in milliseconds.
        @param result Will be filled with the result of the calculation.
        @return Number of spectra to co-add in the computer. */
    int CountRound(long timeResolution, mobiledoas::SpectrumSummation& result) const;

    /** Makes the initial adjustments and sets the
        parameter 'm_integrationTime' so that intensity of
        the spectra are at the desired percent of max.
        @return the set integration time (in milli seconds) */
    short AdjustIntegrationTime();

    /** Calculates the integration time,
      given the intensity of the dark and the sky spectra
      and the exposure time they were collected with*/
    long GetInttime(long pSky, long pDark, int intT = 100);

    /** The desired saturation ratio of the measured spectra, in the range 0.0 to 1.0 */
    double m_desiredSaturationRatio;

    /** if m_fixexptime > 0 the integration time will be set to
          m_fixexptime, else it will be judged automatically. */
    long m_fixexptime;

    /** True if the user wants us to update the integration time. */
    BOOL  m_adjustIntegrationTime;

    /** fills up the 'specInfo' structure with information from the supplied spectrum */
    void GetSpectrumInfo(const mobiledoas::MeasuredSpectrum& spectrum);

    /* -------  The spectra ----------- */

    /** The exposure time that we should use to collect the dark current spectrum */
    static const int DARK_CURRENT_EXPTIME = 10000;

    /** Called to calculate the flux in real-time (during the scope of the measurement)
        @return the accumulated flux so far */
    double CountFlux(double windSpeed, double windAngle);

    /** The so far accumulated flux in the measurement, set by 'CountFlux' */
    double m_flux;

    /** The wind direction used to calculate the flux.
        Set by the user at startup */
    double m_windAngle;

    /** The wind speed used to calculate the flux.
        Set by the user at startup */
    double m_windSpeed;

    /** The gas factor used to calculate the flux.
        Set to 2.66 (SO2) */
    double m_gasFactor;

    /** Evaluates the given spectrum using the given dark and sky spectra
        @param pSky - the sky spectrum(-a) to use. Should already be dark-corrected!
        @param pDark - the dark spectrum(-a) to use (for the measured spectrum,
            the sky should already be dark-corrected
        @param pSpectrum - the spectrum to evaluate.  */
    void DoEvaluation(mobiledoas::MeasuredSpectrum& sky, mobiledoas::MeasuredSpectrum& dark, mobiledoas::MeasuredSpectrum& spectrum);

    /** Copies the current sky-spectrum to 'tmpSky' */
    void GetSky();

    /** Copies the current dark-spectrum to 'tmpDark' */
    void GetDark();

    /** Reads the references files from disk */
    int ReadReferenceFiles();

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

    /** This is an array holding the intensities of the so far collected spectra. */
    std::vector<double> m_intensityOfMeasuredSpectrum;

    // ---------------------------------------------------------------
    // --------------------------- Output ----------------------------
    // ---------------------------------------------------------------

    /** Writes the header of the evaluation file for fitRegion number 'fitRegion'
        @param fitRegion - the fit-region index for which we should write the
            evaluation log file header. Must be >= 0 and < m_fitRegionNum.
    */
    void WriteBeginEvFile(int fitRegion);

    /** Writes the calculated flux to file. Called by 'CountFlux' */
    void WriteFluxLog();

    /** Writes the given string to the given file
        @param filename - the file to write to
        @param txt - the string to write */
    void WriteLogFile(CString filename, CString txt);

    /** Sets the name of the next .std file that we should write to.
        Sets the member variable 'm_stdfileName'*/
    void SetFileName();

    /** Takes care of creating the correct structure of directories
        in the output directory
        Sets the variable 'm_subFolder' */
    void CreateDirectories();

    /** Writes the last evaluation result from the given fit region
        to the specified file-name */
    void WriteEvFile(CString filename, FitRegion* fitRegion);

    /** The directory that we're currently writing to.
    *   Notice that this does NOT end with a trailing (forward/backward) slash.
        Set by calling 'CreateDirectories' */
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
    bool UpdateGpsData(mobiledoas::GpsData& gpsInfo);

    /** Retrieves the current time from the system time */
    long GetCurrentTimeFromComputerClock();

    /** Retrieves the current time from the system time */
    void GetCurrentTimeFromComputerClock(novac::CDateTime& time);

    /** Pointer to the gps reading thread */
    mobiledoas::GpsAsyncReader* m_gps = nullptr;

    /** This is true if we should use the GPS receiver (default behavior).
        Set to false if the gps is missing or nor working. */
    bool m_useGps = true;

    /** The Serial-port that we should read the GPS data from
        This is something like 'COM4' */
    char m_GPSPort[20];

    /** The baudrate that we should use to communicate with the GPS
        reveiver, typicallly 9600 */
    long m_GPSBaudRate = 9600;


    // ------------------- Handling the USB-Connection  --------------------

    /** Called to test the USB-connection.
        @return 1 if successful, else 0 */
    int TestSpectrometerConnection();

    /** Called to close the connection with the spectrometer. Should only be done
        when we're about to stop collecting spectra */
    void CloseSpectrometerConnection();

    /** @return true if the spectrometer has been disconnected */
    bool IsSpectrometerDisconnected();

    /** Attepts to reconnect with the spectrometer after the connection has been lost.
        This will not return until the connection has been regained */
    void ReconnectWithSpectrometer();

    // ------------------------ Setup ------------------------

    /** Applies the settings found in m_conf to the rest
        of the parameters */
    void ApplySettings();

    /** Applies the settings for the evaluation to the rest of the parameters.
        Called from ApplySettings but may also be called from e.g. the instrument calibration routine */
    void ApplyEvaluationSettings();

    /** Checks the settings in 'm_conf' for reasonability and
        checks that the specified files does exist */
    int CheckSettings();

    // ------------------------------ Misc ------------------------

     /* Returns the current time in UMT as a long, must have gotten
        at least one GPS-time value to work properly, otherwise the local time will be returned */
    long GetTimeValue_UMT();

    /** Plays a small sound according to the last evaluated column.
        The volume will be somewhere between 0 and 1 depending on
            if the last column is <=0 and >= m_maxColumn */
    void Sing(double factor);

    /** Updates the mobile-log... This is used to store the
        users preferences between runs */
    void UpdateMobileLog();

    /** The settings, read in from the cfg.txt - file */
    std::unique_ptr<Configuration::CMobileConfiguration> m_conf;

    /* -------  The spectra ----------- */

    /** The last dark-spectrum measured */
    mobiledoas::MeasuredSpectrum m_dark;

    /** The last dark-current measured (only used for adaptive integration times) */
    mobiledoas::MeasuredSpectrum m_darkCur;

    /** The last offset-spectrum measured (only used for adaptive integration times) */
    mobiledoas::MeasuredSpectrum m_offset;

    /* The measured sky-spectrum that we're using to evaluate the spectra.
        Notice that this is already dark-corrected.*/
    mobiledoas::MeasuredSpectrum m_sky;

    /** This is a temporary copy of the dark-spectrum */
    mobiledoas::MeasuredSpectrum m_tmpDark;

    /** This is a temporary copy of the sky-spectrum */
    mobiledoas::MeasuredSpectrum m_tmpSky;

    /* A copy of the last measured spectrum, used for plotting on the screen */
    mobiledoas::MeasuredSpectrum m_curSpectrum;

    /** The wavelengths for each pixel in the measured spectrum */
    mobiledoas::MeasuredSpectrum m_wavelength;

    /** The highpass filtered measured spectrum.
    This is used for plotting mostly */
    mobiledoas::MeasuredSpectrum m_spectrum;


    // ---------------------------------------------------------------------------------------
    // --------------------- Keeping track of the route... -------------
    // ---------------------------------------------------------------------------------------

    /** m_spectrumGpsData[i] holds the Gps information associated with spectrum number 'i' */
    struct mobiledoas::GpsData m_spectrumGpsData[65536];


    // ---------------------------------------------------------------------------------------
    // --------------------- Keeping track of the offset-level of the spectra... -------------
    // ---------------------------------------------------------------------------------------

    /** A SpectrumInfo struct is used to keep track of the properties
        of collected spectra. So far this only contains the
        (electronic)offset of the spectrum and whether a given spectrum
        is dark or not. */
    typedef struct SpectrumInfo
    {
        /** The electronic offset of the spectrum, measured in channel 2 - 24 */
        double offset = 0.0;

        /** True if the program judges that the spectrum is dark */
        bool isDark = false;

    }SpectrumInfo;

    /** Information about the last spectrum collected */
    SpectrumInfo m_specInfo[MAX_N_CHANNELS];

    /** Average intensity (at the specified pixel) of the last spectrum that we measured */
    long m_averageSpectrumIntensity[MAX_N_CHANNELS];

    /** Audio option **/
    int m_useAudio = 1;

    /** The column value that causes 'Sing' to sing at the highest available volume */
    double m_maxColumn;

    /* Spectrum number, only used to judge if this is dark, sky or measurement spectrum */
    long m_scanNum;

    /* Spectrum number, pointer into 'm_spectrumGpsData'.
        Counts how many spectra we have acquired so far.
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

    // ---------------------------------------------------------------------------------------
    // -------------------- Collecting common behavior between subclasses --------------------
    // ---------------------------------------------------------------------------------------

    /** Sets up the evaluation objects.
        @param skySpectrumIsDarkCorrected set to true if the sky-spectrum used in the evaluation
            has already been dark-corrected before doing the evaluation (normally: true) */
    void InitializeEvaluators(bool skySpectrumIsDarkCorrected);

    void WriteEvaluationLogFileHeaders();

    // ---------------------------------------------------------------------------------------
    // ----------------------- Communicating with the user and the GUI -----------------------
    // ---------------------------------------------------------------------------------------

    /** Shows a message box to the user (through the main window form) */
    void ShowMessageBox(CString message, CString label) const;

    /** Performs an instrument calibration using the provided measured spectrum (and a dark spectrum).
        Depending on the settings, this may update the references used for evaluation and may hence make it possible to re-read the references.
        @return true if the references were updated. */
    bool RunInstrumentCalibration(const double* measuredSpectrum, const double* darkSpectrum, size_t spectrumLength);

    /* UpdateSpectrumAverageIntensity calculates the average intensity of the last measured spectrum and
        updates m_averageSpectrumIntensity with this value and informs the user about the results.
        If 'checkIfDark' is set to true, the user will be informed if 'm_specInfo->isDark' is true (i.e. the spectrum is judged to be dark). */
    void UpdateSpectrumAverageIntensity(mobiledoas::MeasuredSpectrum& scanResult);

    /* UpdateUserAboutSpectrumAverageIntensity informs the user about the value of m_averageSpectrumIntensity and
        should hence only be called _after_ UpdateSpectrumAverageIntensity has been called.
        If 'checkIfDark' is set to true, the user will be informed if 'm_specInfo->isDark' is true (i.e.the spectrum is judged to be dark). */
    void UpdateUserAboutSpectrumAverageIntensity(const std::string& spectrumName, bool checkIfDark = false);

    /* Notifies the UI about an update in the integration time paramter (m_integrationTime) */
    void OnUpdatedIntegrationTime() const;

    /* Notifies the UI about a new message we want to show to the user in the status bar of the UI. */
    void UpdateStatusBarMessage(const std::string& newMessage);

    /* Notifies the UI about a new message we want to show to the user in the status bar of the UI. */
    void UpdateStatusBarMessage(const char* format, ...);

    /* Notifies the UI about an updated GPS location. */
    void UpdateGpsLocation() const;

    /* Notifies the UI that there is a new spectrum available which can be drawn. */
    void UpdateDisplayedSpectrum() const;

    /* Notifies the UI about a newly measured column value which can be displayed in the UI. */
    void OnNewColumnMeasurement() const;

    /* Notifies the UI that the currently used spectrometer has changed. */
    void OnChangedSpectrometer() const;

    /* Notifies the UI that it needs to display a certain dialog, for the measurement flow to function properly. */
    void DisplayDialog(int dialogToDisplay) const;

private:

    // -------------------- PRIVATE DATA --------------------

    /** Used by 'CountFlux' to calculate the flux.
        TODO: Is this really necessary?? */
    long m_posFlag;

    /** Used by 'CountFlux' to calculate the flux
        TODO: Is this really necessary?? */
    long m_zeroPosNum;

    /** The offset of the last dark-spectrum collected. This is used
        to keep track of wheter we should warn the user about the fact
        that the offset level might have dropped (or increased) since
        the collection of the last dark */
    double m_lastDarkOffset[MAX_N_CHANNELS];

    /** The time difference, in seconds, between UTC and local time */
    long m_timeDiffToUtc = 0U;

    /** The date and time of when the measurement started */
    CString m_measurementStartTimeStr;

    /** This is the object through which we are accessing the spectrometer hardware.
        Notice that there should only be one such instance in the application. */
    std::unique_ptr<mobiledoas::SpectrometerInterface> m_spectrometer;

    /* The main form of the application, used to send messages to. */
    CWnd& m_mainForm;

    /** The base-name of the measurement. As set by the user */
    CString m_measurementBaseName;

    /** The board temperature, as reported by the spectrometer, in degrees Celsius.
        Set to NaN if this could not be read. */
    double m_boardTemperature = std::numeric_limits<double>::quiet_NaN();

    // -------------------- PRIVATE METHODS --------------------

    /** Makes a more clever adjustment of the parameter
        'integrationTime' so that intensity of
        the spectra are at the desired percent of max.
        NB: Called from the function 'AdjustIntegrationTime' !!! */
    short AdjustIntegrationTime_Calculate(long minExpTime, long maxExpTime);
};
