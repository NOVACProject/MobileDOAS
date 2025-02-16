/*
 * avaspec - Public interface for Avantes spectrometer library
 *
 * For windows messages instead of callback define 
 * USE_POSTMESSAGE in the project properties!
 *
 */

#ifndef _AVASPEC_H_
#define _AVASPEC_H_

#define _CRTDBG_MAP_ALLOC

#pragma once
#include "type.h"

#if defined(__linux) || defined(__APPLE__)
	#define WM_APP 0
	#define DLL_INT int
	#define DLL_bool bool 
	#define DLL_AvsHandle AvsHandle
#else
#include "extcode.h"  // for labview event

#pragma warning( disable : 4996 )

	#ifndef WINDOWS
	#if defined(WIN32) || defined(_WIN64)
	#define WINDOWS
    #endif
	#endif

	#ifndef STATIC
		#ifdef AS5216_EXPORTS
		#define DLL_API extern "C" __declspec (dllexport)
		#else
		#define DLL_API extern "C" __declspec (dllimport)
		#endif

		#define DLL_INT DLL_API int __stdcall 
		#define DLL_AvsHandle DLL_API AvsHandle __stdcall 
		#define DLL_bool DLL_API bool __stdcall
	#endif

	#ifdef STATIC
		#define DLL_INT int 
		#define DLL_AvsHandle AvsHandle 
		#define DLL_bool bool
	#endif

#ifdef USE_POSTMESSAGE
#define AVS_POSTMESSAGE
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push,1)

#define USER_ID_LEN             64
#define AVS_SERIAL_LEN          10
#define MAX_TEMP_SENSORS        3
#define ROOT_NAME_LEN           6

#define VERSION_LEN             16
#define AVASPEC_ERROR_MSG_LEN   8
#define AVASPEC_MIN_MSG_LEN     6  // Minimum size of an AvaSpec message

#define OEM_DATA_LEN            4096 // Reserved for OEM data

#define NR_WAVELEN_POL_COEF     5
#define NR_NONLIN_POL_COEF      8
#define MAX_VIDEO_CHANNELS      2
#define NR_DEFECTIVE_PIXELS     30
#define MAX_NR_PIXELS           4096
#define NR_TEMP_POL_COEF        5
#define NR_DAC_POL_COEF         2

#define SAT_PEAK_INVERSION      2
#define SW_TRIGGER_MODE         0
#define HW_TRIGGER_MODE         1
#define SS_TRIGGER_MODE         2
#define EXTERNAL_TRIGGER        0
#define SYNC_TRIGGER            1
#define EDGE_TRIGGER_SOURCE     0
#define LEVEL_TRIGGER_SOURCE    1

#define ILX_FIRST_USED_DARK_PIXEL        2
#define ILX_USED_DARK_PIXELS            14
#define ILX_TOTAL_DARK_PIXELS           18

#define TCD_FIRST_USED_DARK_PIXEL        0
#define TCD_USED_DARK_PIXELS            12
#define TCD_TOTAL_DARK_PIXELS           13

#define HAMS9840_FIRST_USED_DARK_PIXEL  0
#define HAMS9840_USED_DARK_PIXELS       8
#define HAMS9840_TOTAL_DARK_PIXELS      8

#define HAMS10420_11850_FIRST_USED_DARK_PIXEL 0
#define HAMS10420_11850_USED_DARK_PIXELS      4
#define HAMS10420_11850_TOTAL_DARK_PIXELS     4

#define HAMS11071_FIRST_USED_DARK_PIXEL 0
#define HAMS11071_USED_DARK_PIXELS      4
#define HAMS11071_TOTAL_DARK_PIXELS     4

#define HAMS7031_11501_FIRST_USED_DARK_PIXEL  0
#define HAMS7031_11501_USED_DARK_PIXELS       4
#define HAMS7031_11501_TOTAL_DARK_PIXELS      4

#define	HAMS11155_TOTAL_DARK_PIXELS		20

#define MIN_ILX_INTTIME                 1.1   //[ms]

#define MILLI_TO_MICRO                  1000

#define NR_DIGITAL_OUTPUTS              13
#define NR_DIGITAL_INPUTS               13
#define NTC1_ID                         0
#define NTC2_ID                         1
#define TEC_ID                          2

#define NR_ANALOG_OUTPUTS               2

#define CLIENT_ID_SIZE                  32
#define ETHSET_RES_SIZE                 79

long const INVALID_AVS_HANDLE_VALUE = 1000L;
extern bool useLogging;

typedef long AvsHandle;

typedef enum
{
    UNKNOWN,
    USB_AVAILABLE,
    USB_IN_USE_BY_APPLICATION,
    USB_IN_USE_BY_OTHER,
	ETH_AVAILABLE,
	ETH_IN_USE_BY_APPLICATION,
	ETH_IN_USE_BY_OTHER,
	ETH_ALREADY_IN_USE_USB
} DEVICE_STATUS;

typedef enum {
    RS232,
    USB5216,
    USBMINI,
    USB7010,
    ETH7010
} InterfaceType;

typedef enum {
	TYPE_UNKNOWN,
	TYPE_AS5216,
	TYPE_ASMINI,
	TYPE_AS7010
} AvsDeviceType;

typedef struct 
{
    char            SerialNumber[AVS_SERIAL_LEN];
    char            UserFriendlyName[USER_ID_LEN];
    unsigned char   Status;
} AvsIdentityType;

typedef uint8 SensorType;

typedef struct
{
    SensorType      m_SensorType;
    uint16          m_NrPixels;
    float           m_aFit[NR_WAVELEN_POL_COEF];
    bool            m_NLEnable;
    double          m_aNLCorrect[NR_NONLIN_POL_COEF];
    double          m_aLowNLCounts;
    double          m_aHighNLCounts;
    float           m_Gain[MAX_VIDEO_CHANNELS];
    float           m_Reserved;
    float           m_Offset[MAX_VIDEO_CHANNELS];
    float           m_ExtOffset;
    uint16          m_DefectivePixels[NR_DEFECTIVE_PIXELS];
} DetectorType;     
                    
typedef struct      
{                   
    uint16          m_SmoothPix;
    uint8           m_SmoothModel;
} SmoothingType;    
                    
typedef struct      
{                   
    SmoothingType   m_Smoothing;
    float           m_CalInttime;
    float           m_aCalibConvers[MAX_NR_PIXELS];
} SpectrumCalibrationType;

typedef struct
{
    SpectrumCalibrationType m_IntensityCalib;
    uint8                   m_CalibrationType;
    uint32                  m_FiberDiameter;
} IrradianceType;

typedef struct
{
    float           m_aSpectrumCorrect[MAX_NR_PIXELS];
} SpectrumCorrectionType; 

typedef struct
{
    uint8           m_Enable;
    uint8           m_ForgetPercentage;
} DarkCorrectionType;

typedef struct
{
    uint8           m_Mode;
    uint8           m_Source;
    uint8           m_SourceType;
} TriggerType;

typedef struct
{
    uint16          m_StrobeControl;
    uint32          m_LaserDelay;
    uint32          m_LaserWidth;
    float           m_LaserWaveLength;
    uint16          m_StoreToRam;
} ControlSettingsType; 

typedef struct
{
	unsigned char   InterfaceType;	        // 1
	unsigned char   serial[AVS_SERIAL_LEN]; // 10
	unsigned short  port;                   // 2
	unsigned char   status;                 // 1
	unsigned int    RemoteHostIp;           // 4 (IP address of computer connected to spectrometer)
	unsigned int    LocalIp;				// 4 (IP address of spectrometer)
	unsigned char   reserved[4];            // 4
} BroadcastAnswerType;

typedef struct
{
    uint16              m_StartPixel;		   // 2
    uint16              m_StopPixel;           // 2
    float               m_IntegrationTime;     // 4
    uint32              m_IntegrationDelay;    // 4
    uint32              m_NrAverages;		   // 4
    DarkCorrectionType  m_CorDynDark;          // 2
    SmoothingType       m_Smoothing;		   // 3
    uint8               m_SaturationDetection; // 1 
    TriggerType         m_Trigger;			   // 3
    ControlSettingsType m_Control;			   // 16
} MeasConfigType;

typedef struct
{
    uint16          m_Date;
    uint16          m_Time;
} TimeStampType;

typedef struct
{
    bool            m_Enable;
    MeasConfigType  m_Meas;
    int16           m_Nmsr;
} StandAloneType;

typedef struct
{
	int32			m_Nmsr;
	uint8			m_Reserved[8]; // for future use and backwards compatibility
} DynamicStorageType;

typedef struct
{
    float           m_aFit[NR_TEMP_POL_COEF];
} TempSensorType;

typedef struct
{
    bool            m_Enable;
    float           m_Setpoint;     // [degree Celsius]
    float           m_aFit[NR_DAC_POL_COEF];
} TecControlType;

typedef struct
{
    float           AnalogLow[2];
    float           AnalogHigh[2];
    float           DigitalLow[10];
    float           DigitalHigh[10];
} ProcessControlType;

typedef struct 
{
	uint32			m_IpAddr;
	uint32			m_NetMask;
	uint32			m_Gateway;
	uint8			m_DhcpEnabled;
	uint16			m_TcpPort;
	uint8			m_LinkStatus;
    uint8           m_ClientIdType;
    char            m_ClientIdCustom[CLIENT_ID_SIZE];
    uint8           m_Reserved[ETHSET_RES_SIZE];
} EthernetSettingsType;

typedef struct
{
    uint8           m_data[OEM_DATA_LEN];
} OemDataType;

// HEARTBEAT_RESP message data (response to HEARTBEAT message)
typedef struct
{
    unsigned int    m_BitMatrix; // Built-In Test matrix
    unsigned int    m_Reserved;
} HeartbeatRespType;

typedef unsigned int HeartbeatReqType;

#define  SETTINGS_RESERVED_LEN ((62 * 1024) - sizeof(uint32) - /* CRC calculated by firmware */ \
                                (sizeof(uint16) + /* m_Len */ \
                                sizeof(uint16) +  /* m_ConfigVersion */ \
                                USER_ID_LEN + \
                                sizeof(DetectorType) + \
                                sizeof(IrradianceType) + \
                                sizeof(SpectrumCalibrationType) + \
                                sizeof(SpectrumCorrectionType) + \
                                sizeof(StandAloneType) + \
                                sizeof(DynamicStorageType) + \
                                (sizeof(TempSensorType) * MAX_TEMP_SENSORS) + \
                                sizeof(TecControlType) + \
                                sizeof(ProcessControlType) + \
                                sizeof(EthernetSettingsType) + \
                                sizeof(OemDataType)))

typedef struct
{
    uint16                  m_Len;
    uint16                  m_ConfigVersion;
    char                    m_aUserFriendlyId[USER_ID_LEN];
    DetectorType            m_Detector;
    IrradianceType          m_Irradiance;
    SpectrumCalibrationType m_Reflectance;
    SpectrumCorrectionType  m_SpectrumCorrect;
    StandAloneType          m_StandAlone;
    DynamicStorageType		m_DynamicStorage;
    TempSensorType          m_aTemperature[MAX_TEMP_SENSORS];
    TecControlType          m_TecControl;
    ProcessControlType      m_ProcessControl;
	EthernetSettingsType	m_EthernetSettings;
    uint8                   m_aReserved[SETTINGS_RESERVED_LEN];
    OemDataType             m_OemData; // OEM part is at the end of the configuration!
} DeviceConfigType;

typedef struct
{
	uint32	m_TotalScans;  // Internal Storage Size; the size of the scan data buffer
	uint32	m_UsedScans;   // Internal Storage Scan Count; the number of used elements
	uint32	m_Flags;       // DSTR measurement mode flags as described below;
	unsigned char m_IsStopEvent; // m_Flags:bit<0> 1 = Measurement stopped due to STOP received or measurement ready, 0 otherwise
    unsigned char m_IsOverflowEvent; // m_Flags:bit<1> 1 = FIFO overflow error occurred, 0 otherwise
    unsigned char m_IsInternalErrorEvent; // m_Flags:bit<2> 1 = DSTR measurement has stopped due to an internal error, 0 otherwise
    unsigned char m_Reserved; // Padding byte (reserved for future use)
} DstrStatusType;

typedef enum
{
    SENS_HAMS8378_256 = 1,
    SENS_HAMS8378_1024,
    SENS_ILX554,
    SENS_HAMS9201,
    SENS_TCD1304,
    SENS_TSL1301,
    SENS_TSL1401,
    SENS_HAMS8378_512,
    SENS_HAMS9840,
    SENS_ILX511,
    SENS_HAMS10420_11850,
    SENS_HAMS11071_2048X64,
    SENS_HAMS7031_11501,
    SENS_HAMS7031_1024X58,
    SENS_HAMS11071_2048X16,
    SENS_HAMS11155_2048,
    SENS_SU256LSB,
    SENS_SU512LDB,
    SENS_HAMS11638 = 21,
    SENS_HAMS11639,
    SENS_HAMS12443,
    SENS_HAMG9208_512,
	SENS_HAMG13913,
	SENS_HAMS13496
} SENS_TYPE;

#pragma pack(pop)

//----------------------------------------------------------------------------
//
// Name         : AVS_Init
//
// Description  : Tries to open com-port and ask spectrometer configuration
//
// Parameters   : a_COMPort   : -1,  use both Ethernet and USB ports
//                              0,   use USB port
//                              1-4, use COM port (not supported by the AS7010)
//                              256, use Ethernet port (only AS7010)
//
// Returns      : integer     :  >0, number of attached devices
//                               <0, error occured
//
// Remark(s)    : Blocks application
//
//----------------------------------------------------------------------------
DLL_INT AVS_Init( short a_Port );

//----------------------------------------------------------------------------
//
// Name         : AVS_Done
//
// Description  : Closes port and releases memory
//
// Parameters   : -
//
// Returns      : integer :  0, successfully closed
//                          -1, error occured
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_Done( void );

//----------------------------------------------------------------------------
//
// Name         : AVS_GetNrOfDevices
//
// Description  : Scans for attached devices and returns the number of devices
//                detected
//				  For backwards compatibility, replace with USB/ETH version
//
// Parameters   : -
//
// Returns      : int (>=0)          : number of devices in list
//
// Remark(s)    : The DLL updates its internal list if this function is called
//                So this function has to be called each time a WM_DEVICE_CHANGE
//                notification is received
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetNrOfDevices(void);

//----------------------------------------------------------------------------
//
// Name         : AVS_UpdateUSBDevices
//
// Description  : Scans for attached USB devices and returns the number of devices
//                in the devicelist (both USB and ETH devices)
//
// Parameters   : -
//
// Returns      : int (>=0)          : number of devices in list
//
// Remark(s)    : 
//                
//----------------------------------------------------------------------------
DLL_INT AVS_UpdateUSBDevices(void);

//----------------------------------------------------------------------------
//
// Name         : AVS_UpdateETHDevices
//
// Description  : Scans for attached ETH devices and returns the number of devices
//                in the devicelist (both USB and ETH devices)
//
// Parameters   : a_ListSize        : number of bytes allocated by the caller to
//                                    store the a_pList data
//                a_pRequiredSize   : Number of bytes needed to store information
//                a_pList           : pointer to allocated buffer to store information
//
// Returns      : int (>=0)          : number of devices in list
//                ERROR_INVALID_SIZE : (if a_pRequiredSize > a_ListSize)
//
// Remark(s)    : 
//
//----------------------------------------------------------------------------
DLL_INT AVS_UpdateETHDevices( unsigned int         a_ListSize,
	                          unsigned int*        a_pRequiredSize,
		                      BroadcastAnswerType* a_pList );

//----------------------------------------------------------------------------
//
// Name         : AVS_GetList
//
// Description  : Returns device information for each spectrometer connected
//                to the ports indicated at AVS_Init
//
// Parameters   : a_ListSize        : number of bytes allocated by the caller to
//                                    store the a_pList data
//                a_pRequiredSize   : Number of bytes needed to store information
//                a_pList           : pointer to allocated buffer to store information
//
// Returns      : int (>=0)          : number of devices in list
//                ERROR_INVALID_SIZE : (if a_pRequiredSize > a_ListSize)
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetList
(
    unsigned int      a_ListSize,
    unsigned int*     a_pRequiredSize,
    AvsIdentityType*  a_pList
);

//----------------------------------------------------------------------------
//
// Name       : AVS_Activate
//
// Description: Activates selected spectrometer for communication
//
//              Do not call AVS_ActivateConnCb() when AVS_Activate() is called 
//              before.
//
// Parameters : a_pDeviceId: AvsIdentity of desired spectrometer
//
// Returns    : AvsHandle: handle to be used in subsequent calls
//              INVALID_HANDLE_VALUE: in case of error
//
// Remark(s)  : -
//
//----------------------------------------------------------------------------
DLL_AvsHandle AVS_Activate( AvsIdentityType* a_pDeviceId );

//----------------------------------------------------------------------------
//
// Name       : AVS_ActivateConn / AVS_ActivateConnCb
//
// Description: AVS_ActivateConn: depending on the define, either the Windows Message 
//              version or the callback version is used.
//              Windows Message version: the WM_CONN_STATUS message is sent to the window 
//              with the a_hWnd handle.
//              Callback version: Activates selected spectrometer for communication and 
//              registers a Connection Status callback routine. This callback routine 
//              will be called by the DLL when a connection status change has
//              occured. This callback routine must be used by the application 
//              layer to ensure connection reliability.
//
//              Do not call AVS_Activate() when AVS_ActivateConn() has been called 
//              before. For now the function works only for Ethernet interface.
//
// Parameters : a_pDeviceId: AvsIdentity of desired spectrometer
//              a_hWnd     : handle of window to which the WM_COMM_STATUS message should
//                           be sent
//              __Conn     : function pointer which is called by the AvaSpec library
//                           on Ethernet connection status change
//
// Returns    : AvsHandle: handle to be used in subsequent calls
//              INVALID_HANDLE_VALUE: in case of error
//
// Remark(s)  : -
//
//----------------------------------------------------------------------------
#ifdef AVS_POSTMESSAGE
DLL_AvsHandle AVS_ActivateConn(AvsIdentityType* a_pDeviceId, void *a_hWnd);
#else
DLL_AvsHandle AVS_ActivateConn(AvsIdentityType* a_pDeviceId, void(*__Conn)(AvsHandle*, int));
#endif
DLL_AvsHandle AVS_ActivateConnCb(AvsIdentityType* a_pDeviceId, void(*__Conn)(AvsHandle*, int));

//----------------------------------------------------------------------------
//
// Name         : AVS_Deactivate
//
// Description  : De-activates selected spectrometer for communication
//
// Parameters   : a_hDevice    : Device handle from AVS_Activate
//
// Returns      : -
//

// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_bool AVS_Deactivate
(
    AvsHandle    a_hDevice
);

//----------------------------------------------------------------------------
//
// Name         : AVS_GetHandleFromSerial
//
// Description  : Searches serial number for handle id.
//
// Parameters   : a_pSerial    : serial number
//
// Returns      : AvsHandle    : INVALID_AVS_HANDLE_VALUE if not found
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_AvsHandle  AVS_GetHandleFromSerial( char *a_pSerial );


//----------------------------------------------------------------------------
//
// Name         : AVS_GetStatusBySerial
//----------------------------------------------------------------------------
DLL_INT AVS_GetStatusBySerial( char *a_pSerial, int *a_status );


//----------------------------------------------------------------------------
//
// Name         : AVS_Register
//
// Description  : Installs an application windows handle to which device
//                attachment/removal messages have to be sent
//
// Parameters   : a_hWnd    : application window handle
//
// Returns      : -
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_bool AVS_Register( void *a_Hwnd );

//----------------------------------------------------------------------------
//
// Name         : AVS_Measure
//
// Description  : Start measurement
//
// Parameters   : a_hDevice         : device handle
//                a_hWnd            : handle of window to which ready message
//                                    should be sent
//                a_Nmsr            : number of measurements requested
//                                    (-1 is continous)
//
// Returns      : integer : 0, successfully started
//                          error code on error
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
#ifdef AVS_POSTMESSAGE
DLL_INT AVS_Measure( AvsHandle a_hDevice, void *a_hWnd, short a_Nmsr );
#else
DLL_INT AVS_Measure( AvsHandle a_hDevice, void (*__Done)(AvsHandle*, int*), short a_Nmsr );
#endif

DLL_INT AVS_MeasureCallback( AvsHandle a_hDevice, void (*__Done)(AvsHandle*, int*), short a_Nmsr );

#ifdef WINDOWS
#ifndef STATIC
DLL_INT AVS_MeasureLV( AvsHandle a_hDevice, LVUserEventRef *msg, int param, short a_Nmsr );
#endif
#endif

//----------------------------------------------------------------------------
//
// Name         : AVS_PrepareMeasure
//
// Description  : Prepares measurement on the spectrometer using the specified
//                measurement configuration.
//
// Parameters   : a_hDevice     : device handle
//                a_pMeasConfig : pointer to buffer containing a measurement
//                                configuration
//
// Returns      : SUCCESS       : parameters are set
//                ERROR_DEVICE_UNINITIALISED : no communication
//                ERROR_INVALID_DEVICE_ID    : handle unknown
//                ERROR_INVALID_PARAMETER    : measurement configuration invalid
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_PrepareMeasure( AvsHandle a_hDevice, MeasConfigType* a_pMeasConfig );

//----------------------------------------------------------------------------
//
// Name         : AVS_StopMeasure
//
// Description  : Stops the measurements (needed if Nmsr = infinite), can also
//                be used to stop a pending measurement with long integrationtime
//
// Parameters   : a_hDevice : device handle
//
// Returns      : integer         : 0, ok
//                                  error code, communication error
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_StopMeasure( AvsHandle a_hDevice );

//----------------------------------------------------------------------------
//
// Name         : AVS_PollScan
//
// Description  : Poll advent of new data (e.g. for VB, LabVIEW)
//
// Parameters   : a_hDevice : device handle
//
// Returns      : Integer, 0 when data are not ready
//                         1 when data are available
//                        <0: error code
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_PollScan( AvsHandle a_hDevice );

//----------------------------------------------------------------------------
//
// Name         : AVS_GetScopeData
//
// Description  : Returns the values for each pixel
//
// Parameters   : a_hDevice     : device handle
//                a_pTimeLabel  : ticks count last pixel of spectrum is received
//                                by microcontroller, ticks in 10 mS units since
//                                spectrometer started
//                a_pSpectrum   : pointer to array of doubles containing dark
//                                values, array size equal to number of pixels
//
// Returns      : integer       : 0, successfully started
//                                error code on error
//
// Remark(s)    : array size not checked
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetScopeData( AvsHandle a_hDevice, unsigned int* a_pTimeLabel, double* a_pSpectrum );

//----------------------------------------------------------------------------
//
// Name         : AVS_GetSaturatedPixels
//
// Description  : Returns for each pixel whether the pixel was saturated or not
//
// Parameters   : a_hDevice : device handle
//                a_pSaturated  : pointer to array of unsigned chars containing
//                                0 or 1 depending on whether the pixel is saturated or not,
//                                array size equal to number of pixels
//
// Returns      : integer       : 0, successfully started
//                                error code on error
//
// Remark(s)    : array size not checked
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetSaturatedPixels
(
    AvsHandle       a_hDevice,
    unsigned char*  a_pSaturated
);

//----------------------------------------------------------------------------
//
// Name         : AVS_GetLambda
//
// Description  : Returns the wavelength values corresponding to the pixels
//
// Parameters   : a_hDevice     : device handle
//                a_pWaveLength : pointer to array of doubles,
//                                array size equal to number of pixels
//
// Returns      : integer       : 0, successfully started
//                                error code on error
//
// Remark(s)    : array size not checked
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetLambda( AvsHandle a_hDevice, double* a_pWaveLength);

//----------------------------------------------------------------------------
//
// Name         : AVS_GetNumPixels
//
// Description  : Returns the number of pixels
//
// Parameters   : a_hDevice     : device handle
//                a_pNumPixels  : buffer to store number of pixels
//
// Returns      : integer       : 0, number of pixels available
//                                error code on error
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetNumPixels( AvsHandle a_hDevice, unsigned short* a_pNumPixels );

//----------------------------------------------------------------------------
//
// Name         : AVS_GetParameter
//
// Description  : Returns the device parameter structure
//
// Parameters   : a_hDevice     : device handle
//                a_Size        : size of a_pDeviceParm buffer
//                a_pRequiredSize: needed buffer size
//                a_pDeviceParm : pointer to allocated buffer
//
// Returns      : integer         : 0, info available
//                                  error code on error
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetParameter
(
    AvsHandle           a_hDevice,
    unsigned int        a_Size,
    unsigned int*   	a_pRequiredSize,
    DeviceConfigType*   a_pDeviceParm
);

//----------------------------------------------------------------------------
//
// Name         : AVS_SetParameter
//
// Description  : Sets device parameters
//
// Parameters   : a_hDevice    : device handle
//                a_pDeviceParm: structure containing device parameters
//
// Returns      : integer      : 0, ok
//                               error code, communication error
//
// Remark(s)    : contents of structure not checked
//
//----------------------------------------------------------------------------
DLL_INT AVS_SetParameter
(
    AvsHandle           a_hDevice,
    DeviceConfigType*   a_pDeviceParm
);

//----------------------------------------------------------------------------
//
// Name        : AVS_ResetParameter
//
// Description : Resets onboard device parameter section to its factory 
//               settings. This command will result in the loss of all 
//               user-specific device configuration settings. The user-
//               specific device configuration is set by the AvaSpec function
//               AVS_SetParameter(), as defined in this document. 
//
// Parameters  : a_hDevice: device handle
//
// Returns     : integer : 0, ok
//                         error code, communication error
//
//----------------------------------------------------------------------------
DLL_INT AVS_ResetParameter
(
    AvsHandle  a_hDevice
);

//----------------------------------------------------------------------------
//
// Name         : AVS_GetVersionInfo
//
// Description  : Returns the status of the software version of the different parts.
//
// Parameters   : a_hDevice : device handle
//                a_pFPGAVersion, pointer to buffer to store version (16 chars)
//                a_pFirmwareVersion, pointer to buffer to store version (16 chars)
//            	  a_pDLLVersion pointer to buffer to store version (16 chars)
//
// Returns      : integer         : 0, ok
//                                  <0 on error
//
// Remark(s)    : Does not check the size of the buffers allocated by the caller.
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetVersionInfo
(
    AvsHandle  a_hDevice,
    char*      a_pFPGAVersion,
    char*      a_pFirmwareVersion,
    char*      a_pDLLVersion
);

//----------------------------------------------------------------------------
//
// Name         : AVS_GetDLLVersion
//
// Description  : Retrieves the file version information for the avaspec DLL.
//                The version info will look like 9.8.3.0 (major.minor.revision.build)
//
// Parameters   : a_pVersionString : the version information string
//
// Returns      : integer : 0, ok
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetDLLVersion
(
    char*      a_pVersionString
);

//----------------------------------------------------------------------------
//
// Name         : AVS_SetSyncMode
//
// Description  : Disables/enables support for synchronous measurement.
//                DLL takes care of dividing Nmsr request into Nmsr number
//                of single measurement requests.
//
// Parameters   : a_hDevice : device handle
//                a_Enable  : enables/disables support
//
// Returns      : integer         : 0, ok
//                                  error code, communication error
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_SetSyncMode
(
	AvsHandle       a_hDevice,
	unsigned char   a_Enable
);

//----------------------------------------------------------------------------
//
// Name         : AVS_SetPrescanMode
//
// Description  : Sets prescan mode (skip first measurement result),
//                at the moment only useful for the AvaSpec-3648 to switch
//                between Prescan (default) and Clear mode
//
// Parameters   : a_Prescan
//
// Returns      : integer         : 0, ok
//                                  error code, communication error
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_SetPrescanMode
(
    AvsHandle  a_hDevice,
    bool       a_Prescan
);

//----------------------------------------------------------------------------
//
// Name         : AVS_UseHighResAdc
//
// Description  : Sets DLL in 16-bit data mode, when hardware ID >= 3.
//                Data is not longer treated as 14-bit data.
//                Will also influence gain and offset checks.
//
// Parameters   : a_hDevice : device handle
//              : a_Enable  : true = 16bit resolution, false = 14bit resolution
//
// Returns      : integer         : 0, ok
//                                  error code, communication error
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_UseHighResAdc
(
    AvsHandle  a_hDevice,
    bool       a_Enable
);

//----------------------------------------------------------------------------
//
// Name         : AVS_GetAnalogIn
//
// Description  : Returns the status of the specified digital input
//
// Parameters   : a_hDevice : device handle
//                a_AnalogInId  : input identifier
//                a_pAnalogIn   : pointer to buffer to store result
//
// Returns      : integer       : 0, successfully started
//                                error code on error
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetAnalogIn
(
    AvsHandle       a_hDevice,
    unsigned char   a_AnalogInId,
    float*          a_pAnalogIn
);

//----------------------------------------------------------------------------
//
// Name         : AVS_GetDigIn
//
// Description  : Returns the state of the digital input
//
// Parameters   : a_hDevice : device handle
//                a_DigInId : digital input
//                a_pDigIn  : value of digital input
//
// Returns      : integer         : 0, ok
//                                  <0 on error
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetDigIn
(
    AvsHandle       a_hDevice,
    unsigned char   a_DigInId,
    unsigned char*  a_pDigIn
);

//----------------------------------------------------------------------------
//
// Name         : AVS_SetAnalogOut
//
// Description  : Sets analog output
//
// Parameters   : a_hDevice       : device handle
//                a_PortId        : output identifier
//                a_Value         : output value in Volts (0 - 3.3)
//
// Returns      : integer         : 0, ok
//                                  error code, communication error
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_SetAnalogOut
(
	AvsHandle      a_hDevice,
	unsigned char  a_PortId,
	float          a_Value
);

//----------------------------------------------------------------------------
//
// Name         : AVS_SetDigOut
//
// Description  : Sets state of digital output
//
// Parameters   : a_hDevice : device handle
//                a_PortId  : digital output id.
//                a_Status  : new state digital output
//
// Returns      : integer         : 0, ok
//                                  error code, communication error
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_SetDigOut
(
    AvsHandle       a_hDevice,
    unsigned char   a_PortId,
    unsigned char   a_Status
);

//----------------------------------------------------------------------------
//
// Name         : AVS_SetPwmOut
//
// Description  : Sets state of pwm output
//
// Parameters   : a_hDevice : device handle
//                a_PortId  : digital output id.
//                a_Freq    : desired PWM frequency (500 - 300000 Hz0
//                a_Duty    : percentage high time in single PWM period
//
// Returns      : integer         : 0, ok
//                                  error code, communication error
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_SetPwmOut
(
    AvsHandle       a_hDevice,
    unsigned char   a_PortId,
    unsigned long   a_Freq,
    unsigned char   a_Duty
);

//----------------------------------------------------------------------------
//
// Name       : AVS_GetDarkPixelData
//
// Description: Returns the optical black pixel values of the last performed
//              measurement. 
//
// Parameters : a_hDevice   : device handle
//              a_pDarkData : array of double, 	size=18 for the AvaSpec-2048-USB2 and AvaSpec-2048L-USB2
//						size=13 for the AvaSpec-3648-USB2
//						size= 8 for the AvaSpec-2048x14-USB2
//
// Returns    : integer,  1, on success
//                        0, error, no data available 
//
// Remark(s)  : Should be called by the application after calling AVS_GetScopeData
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetDarkPixelData 
(
    AvsHandle   a_hDevice,
    double*     a_pDarkData
);

//----------------------------------------------------------------------------
//
// Name       : AVS_GetIPAddress
//
// Description: Returns the ip address of the device
//
// Parameters : a_pDeviceId: AvsIdentity of desired spectrometer
//              a_pIp      : Output; will be filled with A NULL terminated character string 
//                           representing a "." (dotted) notation number. Size of this buffer
//                           must be at least 16 bytes long (including NULL termination).
//              a_size     : number of allocated char for a_pIp;
//
// Returns    : integer,  1, on success
//                        0, error, no data available 
//
// Remark(s)  : Should be called by the application after calling AVS_GetScopeData
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetIPAddress( AvsIdentityType*  a_pDeviceId, char *a_pIp, int *a_size );

//----------------------------------------------------------------------------
//
// Name       : AVS_GetComType
//
// Description: Returns the communication protocol
//
// Parameters : a_pDeviceId: AvsIdentity of desired spectrometer
//              a_type     : Communication type as defined below;
//                             RS232 = 0,
//                             USB5216 = 1,
//                             USBMINI = 2,
//                             USB7010 = 3,
//                             ETH7010 = 4
//                             -1 when identity given with a_pDeviceId is unknown
//
// Returns    : integer,  0, on success
//                        Any other AvaSpec return value on error
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetComType( AvsIdentityType*  a_pDeviceId, int *a_type );


//----------------------------------------------------------------------------
//
// Name         : AVS_SetSensitivityMode
//
// Description  : Sets sensitivity mode (lownoise or high sensitivity),
//              at the moment only useful for the NIR to switch
//              between lownoise and high sensitivity mode
//
// Parameters   : a_HiSensitivity: 0 = lownoise; >0 = high sensitivity
//
// Returns      : integer         : 0, ok
//                                  error code, communication error
//
// Remark(s)    : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_SetSensitivityMode
(
    AvsHandle   a_hDevice,
    uint32      a_SensitivityMode
);

//----------------------------------------------------------------------------
//
// Name       : AVS_GetIpConfig
//
// Description: Returns the ip config of the device
//
// Parameters : a_hDevice	  : device handle
//              a_Data        : structure which contains the Ip config of the device
//
// Returns    : integer,  1, on success
//                        0, error, no data available 
//
// Remark(s)  : -
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetIpConfig( AvsHandle a_hDevice, EthernetSettingsType *a_Data );

//----------------------------------------------------------------------------
//
// Name       : AVS_SuppressStrayLight
//
// Description  : Returns the values for each pixel
//
// Parameters   : a_hDevice       : device handle
//                a_Multifactor   : multiplication factor in stray light algorithm
//                a_pSrcSpectrum  : pointer to source array of doubles containing scope
//                                  minus dark values, array size equal to number of pixels
//                a_pDestSpectrum : pointer to array of corrected doubles, array size equal
//                                  to number of pixels                
//
// Returns      : integer         : 0, successfully started
//                                  error code on error
//
// Remark(s)    : array size not checked
//
//----------------------------------------------------------------------------
DLL_INT AVS_SuppressStrayLight
(
    AvsHandle   a_hDevice, 
    float       a_Multifactor, 
    double*     a_pSrcSpectrum, 
    double*     a_pDestSpectrum 
);

//----------------------------------------------------------------------------
//
// Name       : AVS_Heartbeat
//
// Description: Indicates that the device is alive (send from host to its client) where the
//              response contains additional spectrometer information.
//
// Parameters : a_hDevice: device handle
//              a_pHbReq: bitmapped Heartbeat request values (input), see Interface Requirement  
//                        Specification (doc ID 020388) for the full description of the Heartbeat request.
//              a_pHbResp: Heartbeat response structure (output), as received from the spectrometer
//
// Returns    : integer: 0, successfully received
//                          error code on error
//
// Remark(s)  : array size not checked
//
//----------------------------------------------------------------------------
DLL_INT AVS_Heartbeat( AvsHandle a_hDevice, HeartbeatReqType *a_pHbReq, HeartbeatRespType *a_pHbResp );

//----------------------------------------------------------------------------
//
// Name       : AVS_ResetDevice
//
// Description: After this command is replied the device will perform a hard reset.
//                 
// Parameters : a_hDevice: device handle
//
// Returns    : integer: 0, ok
//                       <0 on error
//
//----------------------------------------------------------------------------
DLL_INT AVS_ResetDevice( AvsHandle a_hDevice );


//----------------------------------------------------------------------------
//
// Name         : AVS_GetOemParameter
//
// Description  : Returns the OEM part of the device parameter structure
//
// Parameters   : a_hDevice   : device handle
//                a_pOemData  : pointer to allocated buffer in which the 
//                              OEM data will be copied.
//
// Returns      : integer     : 0, info available
//                              AvaSpec error code on error
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetOemParameter
(
    AvsHandle        a_hDevice,
    OemDataType*     a_pOemData
);

//----------------------------------------------------------------------------
//
// Name         : AVS_SetOemParameter
//
// Description  : Sets the OEM part of the device parameter
//
// Parameters   : a_hDevice   : Device handle
//                a_pOemData  : Structure containing OEM data which will be 
//                              copied in the Device Configuration
//
// Returns      : integer     : 0, Data available
//                              AvaSpec error code on error
//
// Remark(s)    : contents of structure not checked
//
//----------------------------------------------------------------------------
DLL_INT AVS_SetOemParameter
(
    AvsHandle       a_hDevice,
    OemDataType*    a_pOemData
);

//----------------------------------------------------------------------------
//
// Name         : AVS_EnableLogging
//
// Description  : Enables or disables logging to [userdirectory]\Avantes\AvaSoft8\avaspec.dll.log
//
// Parameters   : a_EnableLogging  : enables or disables logging
//
// Returns      : bool :  >  operatin succesfull or not
//
//----------------------------------------------------------------------------
DLL_bool AVS_EnableLogging(bool a_EnableLogging);

//----------------------------------------------------------------------------
//
// Name         : AVS_GetDeviceType
//
// Description  : Returns the device type
//
// Parameters   : a_hDevice   : Device handle
//				  a_pDeviceType : pointer to variable for copying the device type value to
//
// Returns      : integer     : ERR_SUCCESS on succesfully retrieving the device type
//                              AvaSpec error code on error
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetDeviceType(AvsHandle a_hDevice, AvsDeviceType* a_pDeviceType);

//----------------------------------------------------------------------------
//
// Name       : AVS_SetDstrStatus / AVS_SetDstrStatusCallback
//
// Description: Used to set the address of the window the DSTR (Dynamic StoreToRam) status 
//              message is sent to c.q. the address of the callback function that is 
//              called when the DSTR status has changed.
//              AVS_SetDstrStatus: depending on the define, either the Windows Message 
//              version or the callback version is used.
//              Windows Message version: the WM_DSTR_STATUS message is sent to the window 
//              with the a_hWnd handle.
//              Callback version: Registers a DSTR Status callback routine.
//              This callback routine will be called by the DLL when a DSTR status change 
//              has occured. 
//
// Parameters : a_hDevice  : Device handle
//              a_hWnd     : handle of window to which the WM_DSTR_STATUS message should
//                           be sent
//              __Dstr     : function pointer which is called by the AvaSpec library
//                           on DSTR status change
//
// Returns    : integer,  0, on success
//                        Any other AvaSpec return value on error
//
// Remark(s)  : -
//
//----------------------------------------------------------------------------
#ifdef AVS_POSTMESSAGE
DLL_INT AVS_SetDstrStatus(AvsHandle a_hDevice, void *a_hWnd);
#else
DLL_INT AVS_SetDstrStatus(AvsHandle a_hDevice, void(*__Dstr)(AvsHandle*, unsigned int));
#endif

DLL_INT AVS_SetDstrStatusCallback(AvsHandle a_hDevice, void(*__Dstr)(AvsHandle*, unsigned int));

//----------------------------------------------------------------------------
//
// Name       : AVS_GetDstrStatus
//
// Description: Reads the DSTR (Dynamic StoreToRam) status which is received 
//              from  the spectrometer
//
// Parameters : a_hDevice: device handle
//              a_pDstrStatus: pointer to the DSTR status context. See the 
//                             DstrStatusType struct type for the full 
//                             description of the DSTR status.
//
// Returns    : integer: 0, successfully received
//                          error code on error
//
// Remark(s)  : array size not checked
//
//----------------------------------------------------------------------------
DLL_INT AVS_GetDstrStatus(AvsHandle a_hDevice, DstrStatusType* a_pDstrStatus);

//----------------------------------------------------------------------------

// AvaSpec return error codes
#define ERR_SUCCESS                     0
#define ERR_INVALID_PARAMETER          -1
#define ERR_OPERATION_NOT_SUPPORTED    -2
#define ERR_DEVICE_NOT_FOUND           -3
#define ERR_INVALID_DEVICE_ID		   -4
#define ERR_OPERATION_PENDING          -5
#define ERR_TIMEOUT                    -6
#define ERR_INVALID_PASSWORD           -7
#define ERR_INVALID_MEAS_DATA          -8
#define ERR_INVALID_SIZE               -9
#define ERR_INVALID_PIXEL_RANGE        -10
#define ERR_INVALID_INT_TIME           -11
#define ERR_INVALID_COMBINATION        -12
#define ERR_INVALID_CONFIGURATION      -13
#define ERR_NO_MEAS_BUFFER_AVAIL       -14
#define ERR_UNKNOWN                    -15
#define ERR_COMMUNICATION              -16
#define ERR_NO_SPECTRA_IN_RAM          -17
#define ERR_INVALID_DLL_VERSION        -18
#define ERR_NO_MEMORY                  -19
#define ERR_DLL_INITIALISATION         -20
#define ERR_INVALID_STATE              -21
#define ERR_INVALID_REPLY              -22
#define ERR_CONNECTION_FAILURE         ERR_COMMUNICATION
#define ERR_ACCESS                     -24
#define ERR_INTERNAL_READ              -25
#define ERR_INTERNAL_WRITE             -26
#define ERR_ETHCONN_REUSE              -27
#define ERR_INVALID_DEVICE_TYPE		   -28
#define ERR_SECURE_CFG_NOT_READ        -29
#define ERR_UNEXPECTED_MEAS_RESPONSE   -30

// Return error codes; DeviceData check
#define ERR_INVALID_PARAMETER_NR_PIXELS    -100
#define ERR_INVALID_PARAMETER_ADC_GAIN     -101
#define ERR_INVALID_PARAMETER_ADC_OFFSET   -102

// Return error codes; PrepareMeasurement check
#define ERR_INVALID_MEASPARAM_AVG_SAT2     -110
#define ERR_INVALID_MEASPARAM_AVG_RAM      -111
#define ERR_INVALID_MEASPARAM_SYNC_RAM     -112
#define ERR_INVALID_MEASPARAM_LEVEL_RAM    -113
#define ERR_INVALID_MEASPARAM_SAT2_RAM     -114
#define ERR_INVALID_MEASPARAM_FWVER_RAM    -115 // StoreToRAM in 0.20.0.0 and later
#define ERR_INVALID_MEASPARAM_DYNDARK      -116

// Return error codes; SetSensitivityMode check
#define ERR_NOT_SUPPORTED_BY_SENSOR_TYPE   -120
#define ERR_NOT_SUPPORTED_BY_FW_VER        -121
#define ERR_NOT_SUPPORTED_BY_FPGA_VER      -122

// Return error codes; SuppressStrayLight check
#define ERR_SL_CALIBRATION_NOT_AVAILABLE   -140
#define ERR_SL_STARTPIXEL_NOT_IN_RANGE     -141
#define ERR_SL_ENDPIXEL_NOT_IN_RANGE       -142
#define ERR_SL_STARTPIX_GT_ENDPIX          -143
#define ERR_SL_MFACTOR_OUT_OF_RANGE        -144

// Connection status codes (distributed by the callback handler registered with AVS_ActivateConnCb())
#define ETH_CONN_STATUS_CONNECTING         0 // Waiting to establish ethernet connection (may be send more than once on regular time base)
                                             // This state could also be given after ETH_CONN_STATUS_CONNECTED, in case of connection loss.
#define ETH_CONN_STATUS_CONNECTED          1 // Eth connection established, with connection recovery enabled
#define ETH_CONN_STATUS_CONNECTED_NOMON    2 // Eth connection ready, without connection recovery 
#define ETH_CONN_STATUS_NOCONNECTION       3 // Unrecoverable connection failure or disconnect from user, AvaSpec library will stop trying to connect with the spectrometer!

#if defined(_M_X64)
#define WM_MEAS_READY       (WM_APP + 1)
#else
#define WM_MEAS_READY       (WM_USER + 1)
#endif
#define WM_CONN_STATUS      (WM_APP + 15) // Please check message numbers already used in AvaSoft 8!
#define WM_DSTR_STATUS	    (WM_APP + 16)

#define WM_DBG_INFO         (WM_USER + 2)
#define WM_DEVICE_RESET     (WM_USER + 3)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
