#include "pch.h"
#include "AvantesSpectrometerInterface.h"
#include "include/avantes/avaspec.h"
#include <sstream>
#include <thread>

using namespace mobiledoas;
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

AvantesSpectrometerInterface::AvantesSpectrometerInterface()
    :m_lastErrorMessage("")
{
    // initialize the library to work with USB devices.
    AVS_Init(0);

    auto state = new AvantesSpectrometerInterfaceState();

    // Setup some default values
    state->measurementConfig.m_IntegrationTime = 100;
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

    return serialnumbers;
}

void AvantesSpectrometerInterface::ReleaseDeviceLibraryResources()
{
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

bool AvantesSpectrometerInterface::Start()
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    if (state->currentSpectrometerHandle == INVALID_AVS_HANDLE_VALUE)
    {
        m_lastErrorMessage = "Start failed, no spectrometer selected.";
        return false;
    }

    // Calling AVS_MeasureCallback with a value of -1 starts the acquisitions indefinetely
    int returnCode = AVS_MeasureCallback(state->currentSpectrometerHandle, nullptr, -1);
    if (returnCode != ERR_SUCCESS)
    {
        std::stringstream message;
        message << "Start failed, AVS_MeasureCallback returned " << returnCode << " which indicates an error.";
        m_lastErrorMessage = message.str();
        return false;
    }

    state->measurementIsRunning = true;
    return true;
}

bool AvantesSpectrometerInterface::Stop()
{
    // TODO:
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

    // Get some properties of the device
    unsigned short detectorSize;
    int returnCode = AVS_GetNumPixels(state->currentSpectrometerHandle, &detectorSize);
    if (returnCode != ERR_SUCCESS)
    {
        std::stringstream msg;
        msg << "SetSpectrometer failed, AVS_GetNumPixels returned: " << returnCode;
        m_lastErrorMessage = msg.str();
        return 0;
    }

    // Prepare the measurement setup
    state->measurementConfig.m_StartPixel = 0;
    state->measurementConfig.m_StopPixel = detectorSize - 1;
    state->measurementConfig.m_IntegrationDelay = 0;
    state->measurementConfig.m_IntegrationTime = 1000;
    int ret = AVS_PrepareMeasure(state->currentSpectrometerHandle, &(state->measurementConfig));
    if (ret != ERR_SUCCESS)
    {
        std::stringstream msg;
        msg << "PrepareMeasure returned error: " << ret << ", measurement parameters not set.";
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
    // TODO:
    return "";
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
        msg << "GetWavelengths failed, AVS_GetNumPixels returned: " << returnCode;
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
        msg << "GetWavelengths failed, AVS_GetLambda returned: " << returnCode;
        m_lastErrorMessage = msg.str();
        return 0;
    }

    m_lastErrorMessage = "";

    return numberOfPixels;
}

int AvantesSpectrometerInterface::GetSaturationIntensity()
{
    // TODO:
    return 0;
}

void AvantesSpectrometerInterface::SetIntegrationTime(int usec)
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    if (state->currentSpectrometerHandle == INVALID_AVS_HANDLE_VALUE)
    {
        m_lastErrorMessage = "SetIntegrationTime failed, no spectrometer selected.";
        return;
    }

    state->measurementConfig.m_IntegrationTime = static_cast<float>(usec / 1000.0);

    const int ret = AVS_PrepareMeasure(state->currentSpectrometerHandle, &(state->measurementConfig));
    if (ret != ERR_SUCCESS)
    {
        std::stringstream msg;
        msg << "SetIntegrationTime failed, AVS_PrepareMeasure returned error: " << ret << ", measurement parameters not set.";
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
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    if (state->currentSpectrometerHandle == INVALID_AVS_HANDLE_VALUE)
    {
        m_lastErrorMessage = "SetScansToAverage failed, no spectrometer selected.";
        return;
    }

    state->measurementConfig.m_NrAverages = numberOfScansToAverage;

    const int ret = AVS_PrepareMeasure(state->currentSpectrometerHandle, &(state->measurementConfig));
    if (ret != ERR_SUCCESS)
    {
        std::stringstream msg;
        msg << "SetScansToAverage failed, AVS_PrepareMeasure returned error: " << ret << ", measurement parameters not set.";
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

    // Read out the data.
    unsigned int timeLabel = 0; // Timestamp of the aquired spectrum. In tens of microseconds since the spectrometer was started. Can bes used to identify spectra.
    // There is only one channel on the Avantes devices, so just set it
    data.resize(1);
    data[0].resize(state->measurementConfig.m_StopPixel - state->measurementConfig.m_StartPixel + 1);
    int returnCode = AVS_GetScopeData(state->currentSpectrometerHandle, &timeLabel, data[0].data());
    if (returnCode != ERR_SUCCESS)
    {
        std::stringstream message;
        message << "GetNextSpectrum failed, AVS_GetScopeData returned " << returnCode << " which indicates an error.";
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

bool AvantesSpectrometerInterface::EnableDetectorTemperatureControl(bool enable, double temperatureInCelsius)
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
