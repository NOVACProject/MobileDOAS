#pragma once
#include "../Common.h"
#include "../Evaluation/FitWindow.h"
#include "../Common/XMLFileReader.h"
#include <SpectralEvaluation/Spectra/WavelengthRange.h>


/** The <b>CMobileConfiguration</b> class stores the settings
        that are used in the configuration of the mobile program */

namespace Configuration {
    class CMobileConfiguration : protected FileHandler::CXMLFileReader
    {
    public:
        // The options for how to connect to the spectrometer
        static const int CONNECTION_USB = 0;
        static const int CONNECTION_DIRECTORY = 1;
        static const int CONNECTION_RS232 = 2;

        // The options for how to determine the exposure-time
        static const int EXPOSURETIME_AUTOMATIC = 0;
        static const int EXPOSURETIME_FIXED = 1;
        static const int EXPOSURETIME_ADAPTIVE = 2;

        /** Default constructor */
        CMobileConfiguration(void);

        /** Constructs a new CMobileConfiguration object by reading
                in the settings found in the given fileName. If the given
                file does not exist or it does not contain settings for the
                MobileDOAS program, then the settings will be set to default. */
        CMobileConfiguration(const CString& fileName);

        /** Default destructor */
        ~CMobileConfiguration(void);

        /** Resets all values to the default */
        void Clear();

        // -------------------------------------------------------
        // ------------------- THE SETTINGS ----------------------
        // -------------------------------------------------------

        // -------- spectrometer -------------
        /** How to connect to the spectrometer */
        int m_spectrometerConnection;

        /** The port-to use, if using serial-connection */
        std::string m_serialPort;

        /** The baudrate to use, if using serial-connection */
        int m_baudrate;

        /** The number of channels on the spectrometer to use */
        int m_nChannels;

        /** The set point for the CCD temperature in Celsius */
        double m_setPointTemperature;

        // ------------ Exposure time -------------------

        /** The channel number around which the intensity of the spectrum will be calculated. */
        long m_specCenter;

        /** The number of pixels to the left and to the right of m_specCenter which
            will be used to calculate the intensity of the spectrum.
            In total 2 * m_specCenterHalfWidth pixels are used. */
        const long m_specCenterHalfWidth = 10;

        /** The desired saturation level at that channel (in percent 0-100%) */
        long m_percent;

        /** Whether to use automatic determination of the exposure-time or to use a fixed exp.time */
        int m_expTimeMode;

        /** The exposure-time to use if we're to use a fixed exposure-time */
        int m_fixExpTime;

        /** The time-resolution, i.e. the desired interval between two consecutive spectra. in ms */
        long m_timeResolution;

        /** The saturation range */
        int m_saturationLow;
        int m_saturationHigh;

        // ---------------- GPS-reciever -----------------
        /** Equal to 1 if we want to use a gps-reciever */
        int m_useGPS;

        /** The serial-port of the GPS-reciever */
        std::string m_gpsPort;

        /** The baudrate of the GPS-reciever */
        long m_gpsBaudrate;

        // ---------------- Evaluation -------------
        /** The Fit-windows */
        Evaluation::CFitWindow m_fitWindow[MAX_FIT_WINDOWS];

        /** How many fit-windows have been defined */
        int m_nFitWindows;

        // ------------------ Audio -------------------

        /** use audio or not (1=true, 0=false)*/
        int m_useAudio = 1;

        /** The expected maximum column value, used for the sound */
        double m_maxColumn;

        /** Auxilliary information - the name of the configuration file that this
                information comes from. */
        std::string m_cfgFile;

        // ------------------ MISC -------------------
        /** Offset from... */
        int m_offsetFrom;

        /** Offset to... */
        int m_offsetTo;

        /** whether to skip dark measurement (1=true, 0=false)*/
        int m_noDark = 0;

        // --------------- Autmatic Calibration ------------
        struct AutomaticCalibration
        {
        public:
            /** If enabled then the automatic calibration will run at startup. */
            bool m_enable = false;

            /** If enabled then new references will be generated and replace the user-configured references. */
            BOOL m_generateReferences = FALSE;

            /** Set to true to high-pass filter the created references (and convert them into ppmm) */
            BOOL m_filterReferences = TRUE;

            /** The full path to the high resolved solar spectrum */
            CString m_solarSpectrumFile;

            /** Path to the intial calibration file (either .std, .clb or .xs).
                If this is a file in the extended std format then it may also contain the instrument line shape
                (and hence make the instrumentLineshapeFile unnecessary). */
            CString m_initialCalibrationFile;

            /** Path to the initial instrument line shape file (.slf) if any is provided.
                Ususally not set if m_initialCalibrationFile is .std. */
            CString m_instrumentLineshapeFile;

            /** The type of file for the initialCalibrationFile and instrumentLineshapeFile
            *   (only used for displaying the correct options in the user interface).
                0 corresponds to both initialCalibrationFile and instrumentLineshapeFile provided
                1 corresponds to only initialCalibrationFile */
            int m_initialCalibrationType = 0;

            /** The option for if an instrument line shape should be fitted as well during
            *   the retrieval of the pixel-to-wavelength calibration.
            *   0 corresponds to no fitting of an instrument line shape,
            *   1 corresponds to fitting a super-gaussian instrument line shape.  */
            int m_instrumentLineShapeFitOption = 1;

            /** The wavelength region in which the instrument line shape should be fitted (in nm).  */
            novac::WavelengthRange m_instrumentLineShapeFitRegion = novac::WavelengthRange(330.0, 350.0);

            void Clear()
            {
                m_enable = false;
                m_solarSpectrumFile = "";
                m_initialCalibrationFile = "";
                m_instrumentLineshapeFile = "";
                m_initialCalibrationType = 0;
                m_instrumentLineShapeFitOption = 0;
                m_instrumentLineShapeFitRegion = novac::WavelengthRange(330.0, 350.0);
            }
        };

        /** The settings for if we should perform instrument calibrations during the measurement */
        AutomaticCalibration m_calibration;

        // ------------------ Directory acquisition -------------------
        /** the directory to watch for acquired data (STD files) */
        CString m_directory;

        /** dynamic range of the spectrometer */
        long m_spectrometerDynamicRange;

        /** time in ms to sleep between directory read */
        int m_sleep;

        /** default files for sky/dark/darkcur/offset */
        CString m_defaultSkyFile;
        CString m_defaultDarkFile;
        CString m_defaultDarkcurFile;
        CString m_defaultOffsetFile;

    private:

        // ------------------ PRIVATE METHODS -------------------

        /** Reading in a configuration file in .xml file format */
        void ReadCfgXml(const CString& fileName);

        /** Starts the parsing */
        int Parse();

        /** Parses the settings for the GPS */
        int ParseGPS();

        /** Parses the settings for the intensity */
        int ParseIntensity();

        /** Parses a section on how to subtract the offset */
        int ParseOffset();

        /** Parses a fit-window settings section */
        int ParseFitWindow();

        /** Parses the settings for how (and if) the spectra should be calibrated */
        int ParseCalibrationSettings();

        /** Parses a reference-file section */
        int ParseReference(novac::CReferenceFile& reference);

        /** Parses a shift or squeeze section */
        int Parse_ShiftOrSqueeze(const CString& label, novac::SHIFT_TYPE& option, double& lowValue /**, double &highValue*/);

        /** Parses directory mode section */
        int ParseDirectoryMode();

    };
}