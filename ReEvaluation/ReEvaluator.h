#pragma once

#define MAX_TRAVERSE_SHOWN 16384

#define MAX_TRAVERSE_LENGTH 16384

#define MAX_SAVED_SPECTRA 128

#define INTENSITY_DARK 1
#define INTENSITY_SATURATED 2

#define MINIMUM_CREDIBLE_INTENSITY 600

#include <math.h>

#include "../Common/SpectrumIO.h"
#include "../Evaluation/Evaluation.h"
#include "../Common.h"
#include "../Evaluation/FitWindow.h"
#include "ReEvaluationSettings.h"

namespace ReEvaluation
{

class CReEvaluator
{
public:
    CReEvaluator(void);
    ~CReEvaluator(void);

    // -------------------- DATA --------------------------

    // running or not
    bool  fRun;

    // Pausing between each spectrum
    int   m_pause;
    bool  m_sleeping;

    CString m_evalLogFileName;
    CString m_specFileDir;
    CString m_outputDir;
    CString m_outputLog;

    // the data from the evaluation log
    int     m_nChannels;
    int     m_nSpecies; // the number of species evaluated for in the original evaluation-log file
    int     m_recordNum[2]; // number of spectra for each channel
    int     m_time[MAX_TRAVERSE_LENGTH];		// <-- the gps-time for collection
    int     m_nspec[MAX_TRAVERSE_LENGTH];		// <-- The number of spectra averaged
    int     m_exptime[MAX_TRAVERSE_LENGTH];		// <-- the exposure time for each spectrum
    double  m_lat[MAX_TRAVERSE_LENGTH];			// <-- the latitude for each spectrum
    double  m_lon[MAX_TRAVERSE_LENGTH];			// <-- the longitude for each spectrum
    int     m_alt[MAX_TRAVERSE_LENGTH];			// <-- the altitude (from the gps) for each spectrum
    double  m_int[2][MAX_TRAVERSE_LENGTH];		// <-- the intensity for each spectrum (and each channel)
    double  m_oldCol[2][MAX_TRAVERSE_LENGTH];	// <-- the real-time evaluated column
    double  m_offset[2][MAX_TRAVERSE_LENGTH];	// <-- the electric offset of each spectrum

    /** The spectrum number that we're currently on */
    int     m_curSpec;

    /** The settings for how to make the re-evaluation */
    CReEvaluationSettings m_settings;

    // the information about which spectra are dark.
    long    m_darkSpecList[2][256];
    long    m_darkSpecListLength[2];

    // reading information about the traverse
    int     ReadEvaluationLog();
    int     ReadSettings();

    // The result of the fit
    double     m_evResult[MAX_N_REFERENCES][6];

    // handling the shift and squeeze
    double   optimumShift, optimumShiftError;
    double   optimumSqueeze, optimumSqueezeError;

    // the wavelength calibration
    bool    m_includePreCalSky;
    CString m_calibSky;

    // The average of the residuals, shows if there is something that has been forgotten
    double m_avgResidual[MAX_SPECTRUM_LENGTH];
    double m_residual[MAX_SPECTRUM_LENGTH];
    long   m_nAveragedInResidual;

    // when doing lengthy calculations the value of this double varies from 0 (in the beginning) to 1 (in the end)
    //  every now and then a WM_PROGRESS message is sent to the window m_mainView if m_mainView != NULL
    //  when the process is finished the window is sent the message WM_DONE
    double  m_progress;
    CWnd* m_mainView = nullptr;

    const enum RUNNING_MODE { MODE_NOTHING, MODE_SLEEPING, MODE_REEVALUATION, MODE_READING_OFFSETS };

    RUNNING_MODE m_mode; // telling the world what we're doing

    // a string that is updated with information about progres in the calculations.
    //  every time the string is changed a message is sent to 'm_mainView'
    CString m_statusMsg;

    // the residuum of the last fit
    double m_spectrum[MAX_SPECTRUM_LENGTH];
    double m_fitResult[MAX_SPECTRUM_LENGTH];

    // The delta of the last fit
    double  m_delta;

    // The chi square of the last fit
    double  m_chiSquare;

    // Information about the spectrometer that collected the spectra
    long		m_spectrometerDynRange;	// <-- the dynamic range of the spectrometer
    long		m_detectorSize;					// <-- the number of pixels on the spectrometer's detector
    CString m_spectrometerName;			// <-- the serial-number of the spectrometer, if available
    CString	m_spectrometerModel;		// <-- The model of the specctrometer

    // ------------------------ METHODS --------------------------------

    bool Stop(); // halt the current operation

    // The Evaluation
    bool DoEvaluation();

    // Getting information about the spectra 

    /** ReadAllOffsets loops through all the spectra defined in the evaluationlog
                and records their electronic offset and notes which spectra are dark.
                The indices of the dark spectra are saved in 'm_darkSpecList'.
                The electronic offsets are saved in 'm_offset' */
    int     ReadAllOffsets();

    /** Returns the electronic offset of the supplied spectrum */
    double  GetOffset(CSpectrum& spec);

    /** Returns true if the supplied spectrum can be regarded as a dark spectrum */
    bool    IsDark(CSpectrum& spec);

private:
    // --------------------------- DATA --------------------
    double  m_fileVersion; // the version of the evaluation log

    // ------------------------ METHODS --------------------------------

    /** handling the shift and squeeze */
    bool FindOptimalShiftAndSqueeze(Evaluation::CEvaluation& evaluator, int channel, CSpectrum& skySpectrum, CSpectrum& darkSpectrum);

    /** Rads the references needed */
    bool ReadReferences(Evaluation::CEvaluation* evaluator);

    /** Reads spectrum from file */
    bool ReadSpectrumFromFile(CSpectrum& spec, CString filename, int channel);

    /** Reads the sky spectrum */
    bool ReadSkySpectrum(CSpectrum& spec, int channel);

    bool IncludeSkySpecInFit(Evaluation::CEvaluation& eval, const CSpectrum& skySpectrum, Evaluation::CFitWindow& window);

    /** returns the next spectrum */
    bool GetSpectrum(CSpectrum& spec, int number, int channel);

    /** reads the given spectrum number from file */
    bool ReadSpectrum(CSpectrum& spec, int number, int channel);

    /** Returns the dark spectrum that is connected with a specific spectrum */
    bool GetDarkSpectrum(CSpectrum& dark, int number, int channel);

    /** checks the spectrum to the settings
            and returns 'true' if the spectrum should not be evaluated */
    int  Ignore(CSpectrum& spec, int spectrumNumber);

    /** check the settings */
    bool MakeInitialSanityCheck();

    /**  misc output */
    bool CreateOutputDirectory();

    /** Writes the beginning of the evaluation log file */
    bool WriteEvaluationLogHeader(int channel);

    /** Writes evaluated data to the evaluation log-file */
    bool AppendResultToEvaluationLog(int specIndex, int channel, Evaluation::CEvaluation& evaluator);

    /** Writes a file with the average of all residuals */
    bool WriteAverageResidualToFile();

    /** Saves the spectra to file */
    bool SaveSpectra(CSpectrum& sky, CString filename, int channel);

    /** Extracts the result from the given evaluator */
    bool GetResult(const Evaluation::CEvaluation& evaluator);

    /** Checks if all spectra have the same exposure time,
            returns true if all spectra have same exp-time, otherwise return false.
            Note that only the exp-times in the master channel are checked, it is assumed
            that the exp-time in the master and the slave channel are the same. */
    bool	AllSpectraHaveSameExpTime();
};
}