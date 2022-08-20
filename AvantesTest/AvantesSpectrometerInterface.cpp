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
    int currentDevice = 0;

    // The current measurement setup.
    MeasConfigType measurementConfig;

    // Handle to the currently active device
    AvsHandle currentSpectrometerHandle = INVALID_AVS_HANDLE_VALUE;
};

void DeactivateCurrentDevice(AvantesSpectrometerInterfaceState* state)
{
    if (state->currentSpectrometerHandle != INVALID_AVS_HANDLE_VALUE)
    {
        AVS_Deactivate(state->currentSpectrometerHandle);
        state->currentSpectrometerHandle = INVALID_AVS_HANDLE_VALUE;
    }
    state->currentDevice = 0;
}

#pragma endregion

AvantesSpectrometerInterface::AvantesSpectrometerInterface()
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
    // Release the resources
    AVS_Done();

    // Cleanup the state
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    AVS_Deactivate(state->currentSpectrometerHandle);
    state->currentSpectrometerHandle = INVALID_AVS_HANDLE_VALUE;

    delete m_state;
    m_state = nullptr;
}

std::vector<std::string> AvantesSpectrometerInterface::ScanForDevices()
{
    std::vector<std::string> serialnumbers;

    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;

    const int numberOfDevicesFound = AVS_UpdateUSBDevices();
    unsigned int requiredSize = numberOfDevicesFound * sizeof(AvsIdentityType);
    AVS_GetList(0, &requiredSize, nullptr);

    state->devices.resize(numberOfDevicesFound);
    AVS_GetList(requiredSize, &requiredSize, state->devices.data());

    // Reset the current state
    state->currentDevice = 0;
    DeactivateCurrentDevice(state);

    // Extract the serial numbers
    for (int deviceIdx = 0; deviceIdx < numberOfDevicesFound; ++deviceIdx)
    {
        serialnumbers.push_back(state->devices[deviceIdx].SerialNumber);
    }

    return serialnumbers;
}

void AvantesSpectrometerInterface::Close()
{

}

void AvantesSpectrometerInterface::Stop()
{

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
        message << "Invalid spectrometer index " << spectrometerIndex << ". Number of spectrometers attached is: " << m_numberOfSpectrometersAttached;
        m_lastErrorMessage = message.str();
        return false;
    }

    DeactivateCurrentDevice(state);

    // Select the current device
    AvsIdentityType* device = &(state->devices[spectrometerIndex]);
    state->currentSpectrometerHandle = AVS_Activate(device);
    state->currentDevice = spectrometerIndex;

    // Get some properties of the device
    unsigned short detectorSize;
    AVS_GetNumPixels(state->currentSpectrometerHandle, &detectorSize);

    // Prepare the measurement setup
    state->measurementConfig.m_StartPixel = 0;
    state->measurementConfig.m_StopPixel = detectorSize - 1;
    state->measurementConfig.m_IntegrationDelay = 0;
    state->measurementConfig.m_IntegrationTime = 1000;
    AVS_PrepareMeasure(state->currentSpectrometerHandle, &(state->measurementConfig));

    return state->currentSpectrometerHandle != INVALID_AVS_HANDLE_VALUE;
}

std::string AvantesSpectrometerInterface::GetSerial()
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    if (state->currentSpectrometerHandle == INVALID_AVS_HANDLE_VALUE)
    {
        // No spectrometer selected.
        return "";
    }
    assert(state->currentDevice >= 0 && state->currentDevice < state->devices.size());
    if (state->currentDevice < 0 || state->currentDevice >= state->devices.size())
    {
        // TODO: this is actually an error..
        return "";
    }

    return std::string(state->devices[state->currentDevice].SerialNumber);
}

std::string AvantesSpectrometerInterface::GetModel()
{
    return "";
}

int AvantesSpectrometerInterface::GetNumberOfChannels()
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    if (state->currentSpectrometerHandle == INVALID_AVS_HANDLE_VALUE)
    {
        // No spectrometer selected.
        return 0;
    }

    unsigned short numberOfPixels = 0;
    int returnCode = AVS_GetNumPixels(state->currentSpectrometerHandle, &numberOfPixels);

    if (returnCode != ERR_SUCCESS)
    {
        // TODO: Error handling
        return 0;
    }

    return static_cast<int>(numberOfPixels);
}

int AvantesSpectrometerInterface::GetWavelengths(std::vector<std::vector<double>>& data)
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    if (state->currentSpectrometerHandle == INVALID_AVS_HANDLE_VALUE)
    {
        // No spectrometer selected.
        return 0;
    }

    const int numberOfPixels = GetNumberOfChannels();
    if (numberOfPixels == 0)
    {
        return 0;
    }

    // The Avantes spectrometers only have one channel, so resize the data for that
    data.resize(1);
    data[0].resize(numberOfPixels);

    int returnCode = AVS_GetLambda(state->currentSpectrometerHandle, data[0].data());
    if (returnCode != ERR_SUCCESS)
    {
        // TODO: Error handling
        return 0;
    }

    return numberOfPixels;
}

int AvantesSpectrometerInterface::GetSaturationIntensity()
{
    return 0;
}

void AvantesSpectrometerInterface::SetIntegrationTime(int usec)
{
    AvantesSpectrometerInterfaceState* state = (AvantesSpectrometerInterfaceState*)m_state;
    state->measurementConfig.m_IntegrationTime = usec / 1000.0;

    AVS_PrepareMeasure(state->currentSpectrometerHandle, &(state->measurementConfig));
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
    state->measurementConfig.m_NrAverages = numberOfScansToAverage;

    AVS_PrepareMeasure(state->currentSpectrometerHandle, &(state->measurementConfig));
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
        // The user should have selected a spectrometer first..
        return 0;
    }

    int returnCode = AVS_MeasureCallback(state->currentSpectrometerHandle, nullptr, 1);
    if (returnCode != ERR_SUCCESS)
    {
        // TODO: Error handling
        return 0;
    }

    // Wait for the measurement to complete..
    while (0 == AVS_PollScan(state->currentSpectrometerHandle))
    {
        std::this_thread::sleep_for(1ms);
    }

    // Read out the data.
    unsigned int timeLabel = 0; // TODO: what is this for?
    // There is only one channel on the Avantes devices, so just set it
    data.resize(1);
    data[0].resize(state->measurementConfig.m_StopPixel - state->measurementConfig.m_StartPixel + 1);
    returnCode = AVS_GetScopeData(state->currentSpectrometerHandle, &timeLabel, data[0].data());
    if (returnCode != ERR_SUCCESS)
    {
        // TODO: Error handling
        return 0;
    }

    return data[0].size();
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
    return "";
}
