#include "pch.h"

#ifdef MANUFACTURER_SUPPORT_AVANTES 

#include "AvantesSpectrometerInterface.h"
#include "avantes/avaspec.h"
#include <sstream>
#include <thread>
#include <assert.h>

#include <iostream>

using namespace mobiledoas;
using namespace avantes;
using namespace std::chrono_literals;

#pragma region Implementing the AvantesSpectrometerInterfaceState to manage the internal state

struct AvantesSpectrometerInterfaceState
{
    // The list of devices which returned from the last call to AVS_GetList 
    std::vector<AvsIdentityType> devices;

    // The current device, index into 'devices'.
    int currentDeviceIdx = 0;

    // The current measurement setup.
    MeasConfigType measurementConfig;

    // Handle to the currently active device
    AvsHandle currentSpectrometerHandle = INVALID_AVS_HANDLE_VALUE;

    // The dynamic range of the currently active device
    double currentSpectrometerDynamicRange = 0;

    // The properties of the current device, contains information such as the number of pixels on the detector and the type of the detector.
    DeviceConfigType currentDeviceConfiguration;

    // True if the measurements are currently running on the currentSpectrometerHandle
    bool measurementIsRunning = false;
};

void DeactivateCurrentDevice(AvantesSpectrometerInterfaceState* state)
{
    if (state->currentSpectrometerHandle != INVALID_AVS_HANDLE_VALUE)
    {
        AVS_Deactivate(state->currentSpectrometerHandle);
        state->currentSpectrometerHandle = INVALID_AVS_HANDLE_VALUE;
    }
    state->currentDeviceIdx = 0;
    state->measurementIsRunning = false;
}

#pragma endregion

#pragma region Utils

const char* FormatAvsErrorCode(int avantesErrorCode)
{
    switch (avantesErrorCode)
    {
    case ERR_SUCCESS:
        return "Success";
    case ERR_INVALID_PARAMETER:
        return "Invalid parameter";
    case ERR_OPERATION_NOT_SUPPORTED:
        return "Operation not supported";
    case ERR_DEVICE_NOT_FOUND:
        return "Device not found";
    case ERR_INVALID_DEVICE_ID:
        return "Invalid device-id";
    case ERR_OPERATION_PENDING:
        return "Operation pending, the previous operation has not yet completed";
    case ERR_TIMEOUT:
        return "Timeout";
    case ERR_INVALID_PASSWORD:
        return "Invalid password";
    case ERR_INVALID_MEAS_DATA:
        return "Invalid measurement data";
    case ERR_INVALID_SIZE:
        return "Invalid size";
    case ERR_INVALID_PIXEL_RANGE:
        return "Invalid pixel range";
    case ERR_INVALID_INT_TIME:
        return "Invalid integration time";
    case ERR_INVALID_COMBINATION:
        return "Invalid combination";
    case ERR_INVALID_CONFIGURATION:
        return "Invalid configuration";
    case ERR_NO_MEAS_BUFFER_AVAIL:
        return "No measurement buffer available";
    case ERR_UNKNOWN:
        return "Unknown error";
    case ERR_COMMUNICATION:
        return "Communication error";
    case ERR_NO_SPECTRA_IN_RAM:
        return "No more spectra available in RAM";
    case ERR_INVALID_DLL_VERSION:
        return "Library version information could not be retrieved";
    case ERR_NO_MEMORY:
        return "Memory allocation error in the library";
    case ERR_DLL_INITIALISATION:
        return "Function called before AVS_Init() is called";
    case ERR_INVALID_STATE:
        return "Function failed because AvaSpec is in wrong state";
    case ERR_INVALID_REPLY:
        return "Reply is not a recognized protocol message";
    case ERR_ACCESS:
        return "Error occurred while opening a bus device on the host";
    case ERR_INTERNAL_READ:
        return "A read error has occurred";
    case ERR_INTERNAL_WRITE:
        return "A write error has occurred";
    case ERR_ETHCONN_REUSE:
        return "Library could not be initialized due to an Ethernet connection reuse";
    case ERR_INVALID_DEVICE_TYPE:
        return "The device-type information stored in the isn’t recognized as one of the known device types";
    case ERR_SECURE_CFG_NOT_READ:
        return "The secure configuration has not yet been read, most likely due to device not correctly initialized";
    case ERR_UNEXPECTED_MEAS_RESPONSE:
        return "Unexpected response from spectrometer while getting measurement data";
    case ERR_INVALID_PARAMETER_NR_PIXELS:
        return "NrOfPixel in Device data incorrec";
    case ERR_INVALID_PARAMETER_ADC_GAIN:
        return "Gain Setting out of range";
    case ERR_INVALID_PARAMETER_ADC_OFFSET:
        return "Offset Setting out of range";
    case ERR_INVALID_MEASPARAM_AVG_SAT2:
        return "Use of Saturation Detection Level 2 is not compatible with the Averaging function";
    case ERR_INVALID_MEASPARAM_AVG_RAM:
        return "Use of Averaging is not compatible with the StoreToRam function";
    case ERR_INVALID_MEASPARAM_SYNC_RAM:
        return "Use of the Synchronize setting is not compatible with the StoreToRam function";
    case ERR_INVALID_MEASPARAM_LEVEL_RAM:
        return "Use of Level Triggering is not compatible with the StoreToRam function";
    case ERR_INVALID_MEASPARAM_SAT2_RAM:
        return "Use of Level Saturation Detection Level 2 Parameter is not compatible with the StoreToRam function";
    case ERR_INVALID_MEASPARAM_FWVER_RAM:
        return "The StoreToRam function is not supported in this library version";
    case ERR_INVALID_MEASPARAM_DYNDARK:
        return "Dynamic Dark Correction not supported";
    case ERR_NOT_SUPPORTED_BY_SENSOR_TYPE:
        return "Use of AVS_SetSensitivityMode() not supported by detector type";
    case ERR_NOT_SUPPORTED_BY_FW_VER:
        return "Use of AVS_SetSensitivityMode() not supported by firmware version";
    case ERR_NOT_SUPPORTED_BY_FPGA_VER:
        return "Use of AVS_SetSensitivityMode() not supported by FPGA version";
    case ERR_SL_CALIBRATION_NOT_AVAILABLE:
        return "Spectrometer stray light calibration not available";
    case ERR_SL_STARTPIXEL_NOT_IN_RANGE:
        return "Incorrect start pixel found in EEPROM";
    case ERR_SL_ENDPIXEL_NOT_IN_RANGE:
        return "Incorrect end pixel found in EEPROM";
    case ERR_SL_STARTPIX_GT_ENDPIX:
        return "Incorrect start or end pixel found in EEPROM";
    case ERR_SL_MFACTOR_OUT_OF_RANGE:
        return "Multiplication factor out of allowed range";
    case ETH_CONN_STATUS_CONNECTED:
        return "Eth connection established, with connection recovery enabled";
    case ETH_CONN_STATUS_CONNECTED_NOMON:
        return "Eth connection ready, without connection recovery ";
    case ETH_CONN_STATUS_NOCONNECTION:
        return "Unrecoverable connection failure or disconnect from user, AvaSpec library will stop trying to connect with the spectrometer";
    default:
        return "Unkown error code: ";
    }
}

#pragma endregion

AvantesSpectrometerInterface::AvantesSpectrometerInterface()
    :m_lastErrorMessage("")
{
    // initialize the library to work with USB devices.
    // Notice that it is possible to support Ethernet devices here as well by changing the port number here.
    AVS_Init(0);

    auto state = new AvantesSpectrometerInterfaceState();

    // Setup some default values
    state->measurementConfig.m_IntegrationTime = 100; // 100ms
    state->measurementConfig.m_NrAverages = 1;
    state->measurementConfig.m_CorDynDark.m_Enable = 0;
    state->measurementConfig.m_Smoothing.m_SmoothModel = 0; // disable smooth
    state->measurementConfig.m_Trigger.m_Mode = 0; // 0 = Software, 1 = Hardware, 2 = Single Scan
    state->measurementConfig.m_Control.m_StrobeControl = 0;

    m_state = state;
}

AvantesSpectrometerInterface::~AvantesSpectrometerInterface()
{
    ReleaseDeviceLibraryResources();

    delete m_state;
    m_state = nullptr;
}

std::vector<std::string> AvantesSpectrometerInterface::ScanForDevices()
{
    std::vector<std::string> serialnumbers;

    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;

    const int numberOfDevicesFound = AVS_UpdateUSBDevices();
    unsigned int requiredSize = numberOfDevicesFound * sizeof(AvsIdentityType);
    int ret = AVS_GetList(0, &requiredSize, nullptr);
    if (ret == ERR_INVALID_SIZE)
    {
        m_lastErrorMessage = "First call to AVS_GetList returned INVALID_SIZE. This indicates a programming error.";
        return serialnumbers;
    }

    state->devices.resize(numberOfDevicesFound);
    ret = AVS_GetList(requiredSize, &requiredSize, state->devices.data());
    if (ret == ERR_INVALID_SIZE)
    {
        m_lastErrorMessage = "Second call to AVS_GetList returned INVALID_SIZE. This indicates a programming error.";
        return serialnumbers;
    }

    // Reset the current state
    state->currentDeviceIdx = 0;
    DeactivateCurrentDevice(state);

    // Extract the serial numbers
    for (int deviceIdx = 0; deviceIdx < numberOfDevicesFound; ++deviceIdx)
    {
        serialnumbers.push_back(state->devices[deviceIdx].SerialNumber);
    }

    m_lastErrorMessage = "";
    m_spectrometersAttached = serialnumbers;

    return serialnumbers;
}

void AvantesSpectrometerInterface::ReleaseDeviceLibraryResources()
{
    // Stop any running measurements, if any.
    Stop();

    // Release the resources
    AVS_Done();

    // Cleanup the state
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    AVS_Deactivate(state->currentSpectrometerHandle);
    state->currentSpectrometerHandle = INVALID_AVS_HANDLE_VALUE;
}

void AvantesSpectrometerInterface::Close()
{
    ReleaseDeviceLibraryResources();
}

void onMeasuredSpectrum(AvsHandle* /*handle*/, int* status)
{
    if (status == nullptr)
    {
        std::cout << "null status" << std::endl;
        return;
    }

    std::cout << " onMeasuredSpectrum called with status: " << *status << std::endl;
}

bool AvantesSpectrometerInterface::Start()
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    if (state->currentSpectrometerHandle == INVALID_AVS_HANDLE_VALUE)
    {
        m_lastErrorMessage = "Start failed, no spectrometer selected.";
        return false;
    }

    // Calling AVS_MeasureCallback with a value of -1 starts the acquisitions indefinetely - however, this has been found to lock the application
    // when a previous instance has not been terminated gracefully and is hence avoided.
    int returnCode = AVS_MeasureCallback(state->currentSpectrometerHandle, onMeasuredSpectrum, 1);
    if (returnCode != ERR_SUCCESS)
    {
        std::stringstream message;
        message << "Start failed, AVS_MeasureCallback returned error code " << returnCode << " ('" << FormatAvsErrorCode(returnCode) << "')";
        m_lastErrorMessage = message.str();
        return false;
    }

    state->measurementIsRunning = true;
    return true;
}

bool AvantesSpectrometerInterface::Stop()
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    if (state->currentSpectrometerHandle == INVALID_AVS_HANDLE_VALUE)
    {
        // Not an error, this can happen in the normal flow
        return false;
    }

    int returnCode = AVS_StopMeasure(state->currentSpectrometerHandle);
    if (returnCode != ERR_SUCCESS)
    {
        std::stringstream message;
        message << "Stop failed, AVS_StopMeasure returned error code " << returnCode << " ('" << FormatAvsErrorCode(returnCode) << "')";
        m_lastErrorMessage = message.str();
        return false;
    }

    state->measurementIsRunning = false;
    return true;
}

bool AvantesSpectrometerInterface::SetSpectrometer(int spectrometerIndex)
{
    std::vector<int> channels;
    return SetSpectrometer(spectrometerIndex, channels);
}

bool AvantesSpectrometerInterface::SetSpectrometer(int spectrometerIndex, const std::vector<int>& /*channelIndices*/)
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    if (spectrometerIndex < 0 || spectrometerIndex >= state->devices.size())
    {
        std::stringstream message;
        message << "Invalid spectrometer index " << spectrometerIndex << ". Number of spectrometers attached is: " << state->devices.size();
        m_lastErrorMessage = message.str();
        return false;
    }

    DeactivateCurrentDevice(state);

    // Select the current device
    AvsIdentityType* device = &(state->devices[spectrometerIndex]);
    state->currentSpectrometerHandle = AVS_Activate(device);
    state->currentDeviceIdx = spectrometerIndex;
    if (state->currentSpectrometerHandle == INVALID_AVS_HANDLE_VALUE)
    {
        m_lastErrorMessage = "Failed to set spectrometer, AVS_Activate returned Invalid Handle";
        return false;
    }

    // Get some properties of the device. Notice that this does in fact return quite a bit more data than what is used here...
    unsigned int configSize = sizeof(DeviceConfigType);
    int returnCode = AVS_GetParameter(state->currentSpectrometerHandle, configSize, &configSize, &(state->currentDeviceConfiguration));
    if (returnCode != ERR_SUCCESS)
    {
        std::stringstream msg;
        msg << "SetSpectrometer failed, AVS_GetParameter returned error code " << returnCode << " ('" << FormatAvsErrorCode(returnCode) << "')";
        m_lastErrorMessage = msg.str();
        return false;
    }

    // Set the dynamic range of the device to use the 'normal' range (14 bits ADC) and not the expanded 16 bits.
    state->currentSpectrometerDynamicRange = 16383.75;
    returnCode = AVS_UseHighResAdc(state->currentSpectrometerHandle, false);
    if (returnCode != ERR_SUCCESS)
    {
        std::stringstream msg;
        msg << "Error happened in SetSpectrometer, AVS_UseHighResAdc returned error code " << returnCode << " ('" << FormatAvsErrorCode(returnCode) << "')";
        m_lastErrorMessage = msg.str();

        state->currentSpectrometerDynamicRange = 16383.75;
    }

    // Prepare the measurement setup
    state->measurementConfig.m_StartPixel = 0;
    state->measurementConfig.m_StopPixel = state->currentDeviceConfiguration.m_Detector.m_NrPixels - 1;
    state->measurementConfig.m_IntegrationDelay = 0;
    state->measurementConfig.m_IntegrationTime = 1; // 1ms
    int ret = AVS_PrepareMeasure(state->currentSpectrometerHandle, &(state->measurementConfig));
    if (ret != ERR_SUCCESS)
    {
        std::stringstream msg;
        msg << "Error happened in SetSpectrometer, AVS_PrepareMeasure returned error code " << returnCode << " ('" << FormatAvsErrorCode(returnCode) << "')";
        m_lastErrorMessage = msg.str();
        return false;
    }

    m_lastErrorMessage = "";

    return true;
}

std::string AvantesSpectrometerInterface::GetSerial()
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    if (state->currentSpectrometerHandle == INVALID_AVS_HANDLE_VALUE)
    {
        m_lastErrorMessage = "GetSerial failed, no spectrometer selected.";
        return "";
    }
    assert(state->currentDeviceIdx >= 0 && state->currentDeviceIdx < state->devices.size());
    if (state->currentDeviceIdx < 0 || state->currentDeviceIdx >= state->devices.size())
    {
        std::stringstream msg;
        msg << "GetSerial failed, currentDeviceIndex is invalid (" << state->currentDeviceIdx << ") out of " << state->devices.size() << " devices.";
        return "";
    }

    m_lastErrorMessage = "";

    return std::string(state->devices[state->currentDeviceIdx].SerialNumber);
}

std::string AvantesSpectrometerInterface::GetModel()
{
    // I have not found a way to read this out from the device itself, use a hard-coded value.
    return "AVASPEC";
}

int AvantesSpectrometerInterface::GetNumberOfChannels()
{
    // Avantes spectrometers only support one channel per spectrometer.
    return 1;
}

int AvantesSpectrometerInterface::GetWavelengths(std::vector<std::vector<double>>& data)
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    if (state->currentSpectrometerHandle == INVALID_AVS_HANDLE_VALUE)
    {
        m_lastErrorMessage = "GetWavelengths failed, no spectrometer selected.";
        return 0;
    }

    unsigned short numberOfPixels = 0;
    int returnCode = AVS_GetNumPixels(state->currentSpectrometerHandle, &numberOfPixels);
    if (returnCode != ERR_SUCCESS)
    {
        std::stringstream msg;
        msg << "GetWavelengths failed, AVS_GetNumPixels returned error code " << returnCode << " ('" << FormatAvsErrorCode(returnCode) << "')";
        m_lastErrorMessage = msg.str();
        return 0;
    }

    // The Avantes spectrometers only have one channel, so resize the data for that
    data.resize(1);
    data[0].resize(numberOfPixels);

    returnCode = AVS_GetLambda(state->currentSpectrometerHandle, data[0].data());
    if (returnCode != ERR_SUCCESS)
    {
        std::stringstream msg;
        msg << "GetWavelengths failed, AVS_GetLambda returned error code " << returnCode << " ('" << FormatAvsErrorCode(returnCode) << "')";
        m_lastErrorMessage = msg.str();
        return 0;
    }

    m_lastErrorMessage = "";

    return numberOfPixels;
}

int AvantesSpectrometerInterface::GetSaturationIntensity()
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    if (state->currentSpectrometerHandle == INVALID_AVS_HANDLE_VALUE)
    {
        m_lastErrorMessage = "GetSaturationIntensity failed, no spectrometer selected.";
        return 0;
    }

    return int(state->currentSpectrometerDynamicRange);
}

void AvantesSpectrometerInterface::SetIntegrationTime(int usec)
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    if (state->currentSpectrometerHandle == INVALID_AVS_HANDLE_VALUE)
    {
        m_lastErrorMessage = "SetIntegrationTime failed, no spectrometer selected.";
        return;
    }

    const float newIntegrationTimeInMs = static_cast<float>(usec / 1000.0);
    if (std::abs(newIntegrationTimeInMs - state->measurementConfig.m_IntegrationTime) < 0.001)
    {
        // Nothing to change
        return;
    }

    Stop(); // Stop the currently running measurement, if any.

    state->measurementConfig.m_IntegrationTime = newIntegrationTimeInMs;

    const int ret = AVS_PrepareMeasure(state->currentSpectrometerHandle, &(state->measurementConfig));
    if (ret != ERR_SUCCESS)
    {
        std::stringstream msg;
        msg << "SetIntegrationTime failed, AVS_PrepareMeasure returned error code " << ret << " ('" << FormatAvsErrorCode(ret) << "')";
        m_lastErrorMessage = msg.str();
        return;
    }

    m_lastErrorMessage = "";
}

int AvantesSpectrometerInterface::GetIntegrationTime()
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    float integrationTimeInMs = state->measurementConfig.m_IntegrationTime;
    return int(integrationTimeInMs * 1000);
}

void AvantesSpectrometerInterface::SetScansToAverage(int numberOfScansToAverage)
{
    if (numberOfScansToAverage <= 0)
    {
        std::stringstream msg;
        msg << "SetScansToAverage failed from bad input value: " << numberOfScansToAverage;
        m_lastErrorMessage = msg.str();
        return;
    }

    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    if (state->currentSpectrometerHandle == INVALID_AVS_HANDLE_VALUE)
    {
        m_lastErrorMessage = "SetScansToAverage failed, no spectrometer selected.";
        return;
    }

    if (uint32(numberOfScansToAverage) == state->measurementConfig.m_NrAverages)
    {
        // Nothing to change
        return;
    }

    Stop(); // Stop the currently running measurement, if any.

    state->measurementConfig.m_NrAverages = numberOfScansToAverage;

    const int ret = AVS_PrepareMeasure(state->currentSpectrometerHandle, &(state->measurementConfig));
    if (ret != ERR_SUCCESS)
    {
        std::stringstream msg;
        msg << "SetScansToAverage failed, AVS_PrepareMeasure returned error code " << ret << " ('" << FormatAvsErrorCode(ret) << "')";
        m_lastErrorMessage = msg.str();
        return;
    }

    m_lastErrorMessage = "";
}

int AvantesSpectrometerInterface::GetScansToAverage()
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    return state->measurementConfig.m_NrAverages;
}

int AvantesSpectrometerInterface::GetNextSpectrum(std::vector<std::vector<double>>& data)
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    if (state->currentSpectrometerHandle == INVALID_AVS_HANDLE_VALUE)
    {
        m_lastErrorMessage = "GetNextSpectrum failed, no spectrometer selected.";
        return 0;
    }

    if (!state->measurementIsRunning)
    {
        if (!Start())
        {
            m_lastErrorMessage = "GetNextSpectrum failed: " + m_lastErrorMessage;
            return 0;
        }
    }

    auto startTime = std::chrono::steady_clock::now();

    // Wait for the measurement to complete..
    while (0 == AVS_PollScan(state->currentSpectrometerHandle))
    {
        std::this_thread::sleep_for(1ms);
    }

    state->measurementIsRunning = false;

    // Read out the data.
    unsigned int timeLabel = 0; // Timestamp of the aquired spectrum. In tens of microseconds since the spectrometer was started. Can be used to identify spectra.
    // There is only one channel on the Avantes devices, so just set it
    data.resize(1);
    data[0].resize(state->measurementConfig.m_StopPixel - state->measurementConfig.m_StartPixel + 1);
    int returnCode = AVS_GetScopeData(state->currentSpectrometerHandle, &timeLabel, data[0].data());
    if (returnCode != ERR_SUCCESS)
    {
        std::stringstream message;
        message << "GetNextSpectrum failed, AVS_GetScopeData returned error code " << returnCode << " ('" << FormatAvsErrorCode(returnCode) << "')";
        m_lastErrorMessage = message.str();
        return 0;
    }

    auto stopTime = std::chrono::steady_clock::now();
    std::stringstream msg;
    msg << "Spectrum acquisition took " << std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime).count() << " ms" << std::endl;
    m_lastErrorMessage = msg.str(); // for debugging...

    return static_cast<int>(data[0].size());
}

bool AvantesSpectrometerInterface::SupportsDetectorTemperatureControl()
{
    return false;
}

bool AvantesSpectrometerInterface::EnableDetectorTemperatureControl(bool /*enable*/, double /*temperatureInCelsius*/)
{
    return false;
}

double AvantesSpectrometerInterface::GetDetectorTemperature()
{
    return 0.0;
}

bool AvantesSpectrometerInterface::SupportsBoardTemperature()
{
    return false;
}

double AvantesSpectrometerInterface::GetBoardTemperature()
{
    return 0.0;
}

std::string AvantesSpectrometerInterface::GetLastError()
{
    return m_lastErrorMessage;
}

#endif // MANUFACTURER_SUPPORT_AVANTES 
