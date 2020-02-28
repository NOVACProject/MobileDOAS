#pragma once
#include "../Common.h"
#include "../Evaluation/FitWindow.h"
#include "../Common/XMLFileReader.h"

/** The <b>CMobileConfiguration</b> class stores the settings
		that are used in the configuration of the mobile program */

namespace Configuration{
	class CMobileConfiguration : protected FileHandler::CXMLFileReader
	{
	public:
		// The options for how to connect to the spectrometer
		static const int CONNECTION_USB		= 0;
		static const int CONNECTION_RS232	= 1;

		// The options for how to determine the exposure-time
		static const int EXPOSURETIME_AUTOMATIC	= 0;
		static const int EXPOSURETIME_FIXED		= 1;
		static const int EXPOSURETIME_ADAPTIVE	= 2;

		/** Default constructor */
		CMobileConfiguration(void);

		/** Constructs a new CMobileConfiguration object by reading 
				in the settings found in the given fileName. If the given
				file does not exist or it does not contain settings for the
				MobileDOAS program, then the settings will be set to default. */
		CMobileConfiguration(const CString &fileName);

		/** Default destructor */
		~CMobileConfiguration(void);

		/** Resets all values to the default */
		void Clear();

		// -------------------------------------------------------
		// ------------------- THE SETTINGS ----------------------
		// -------------------------------------------------------

		// -------- spectrometer -------------
		/** How to connect to the spectrometer */
		int			m_spectrometerConnection;

		/** The port-to use, if using serial-connection */
		CString		m_serialPort;

		/** The baudrate to use, if using serial-connection */
		int			m_baudrate;

		/** The number of channels on the spectrometer to use */
		int			m_nChannels;

		/** The set point for the CCD temperature in Celsius */
		double		m_setPointTemperature;

		// ------------ Exposure time -------------------
		
		/** The channel number around which the intensity of the spectrum will be calculated. */
		long		m_specCenter;

		/** The number of pixels to the left and to the right of m_specCenter which 
			will be used to calculate the intensity of the spectrum.
			In total 2 * m_specCenterHalfWidth pixels are used. */
		const long	m_specCenterHalfWidth = 10;

		/** The desired saturation level at that channel (in percent 0-100%) */
		long		m_percent;

		/** Wheather to use automatic determination of the exposure-time or
				to use a fixed exp.time */
		int			m_expTimeMode;

		/** The exposure-time to use if we're to use a fixed exposure-time */
		int			m_fixExpTime;

		/** The time-resolution, in ms */
		long		m_timeResolution;

		/** The saturation range */
		int		m_saturationLow;
		int		m_saturationHigh;

		// ---------------- GPS-reciever -----------------
		/** Equal to 1 if we want to use a gps-reciever */
		int			m_useGPS;

		/** The serial-port of the GPS-reciever */
		CString		m_gpsPort;

		/** The baudrate of the GPS-reciever */
		long		m_gpsBaudrate;

		// ---------------- Evaluation -------------
		/** The Fit-windows */
		Evaluation::CFitWindow	m_fitWindow[MAX_FIT_WINDOWS];

		/** How many fit-windows have been defined */
		int						m_nFitWindows;

		// ------------------ Audio -------------------
		
		/** use audio or not (1=true, 0=false)*/
		int m_useAudio = 1;

		/** The expected maximum column value, used for the sound */
		double		m_maxColumn;

		/** Auxilliary information - the name of the configuration file that this
				information comes from. */
		CString		m_cfgFile;

		// ------------------ MISC -------------------
		/** Offset from... */
		int				m_offsetFrom;

		/** Offset to... */
		int				m_offsetTo;


		/** whether to skip dark measurement (1=true, 0=false)*/
		int m_noDark = 0;

		// ------------------ Directory acquisition -------------------
		/** the directory to watch for acquired data (STD files) */
		CString		m_directory;

	private:

		// ------------------ PRIVATE METHODS -------------------

		/** Reading in a configuration file in .xml file format */
		void ReadCfgXml(const CString &fileName);

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

		/** Parses a reference-file section */
		int	ParseReference(Evaluation::CReferenceFile &reference);

		/** Parses a shift or squeeze section */
		int Parse_ShiftOrSqueeze(const CString &label, Evaluation::SHIFT_TYPE &option, double &lowValue /**, double &highValue*/);

		// ------------------ PRIVATE DATA -------------------

	};
}