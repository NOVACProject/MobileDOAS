#include "pch.h"

#ifdef MANUFACTURER_SUPPORT_OCEANOPTICS

#include "OceanOpticsSpectrometerInterface.h"

// The OceanOptics drivers headers, located in %OMNIDRIVER_HOME%\include
#define WIN32 // The WIN32 flag is also required for x64 development. See the OmniDriver programming manual.
#include <ArrayTypes.h>
#include <Wrapper.h>
#include <sstream>

using namespace mobiledoas;
using namespace oceanoptics;

OceanOpticsSpectrometerInterface::OceanOpticsSpectrometerInterface()
    : m_spectrometerChannels{ 0 }
{
    m_wrapper = new Wrapper();
}

OceanOpticsSpectrometerInterface::~OceanOpticsSpectrometerInterface()
{
    m_wrapper->closeAllSpectrometers();
    delete m_wrapper;
    m_wrapper = nullptr;
}

std::vector<std::string> OceanOpticsSpectrometerInterface::ScanForDevices()
{
    const int numberOfSpectrometersFound = m_wrapper->openAllSpectrometers();
    if (numberOfSpectrometersFound < 0)
    {
        // This happens if an error occurs. We should here log the error...
        auto errorMessage = GetLastError();
        m_spectrometersAttached.clear();
        return m_spectrometersAttached;
    }

    std::vector<std::string> serialNumbers;
    serialNumbers.resize(numberOfSpectrometersFound);

    for (int ii = 0; ii < numberOfSpectrometersFound; ++ii)
    {
        const char* serial = m_wrapper->getSerialNumber(ii).getASCII();
        serialNumbers[ii] = std::string{ serial };
    }

    m_spectrometersAttached = serialNumbers;

    return m_spectrometersAttached;
}

void OceanOpticsSpectrometerInterface::Close()
{
    m_wrapper->closeAllSpectrometers();
}

bool OceanOpticsSpectrometerInterface::Start()
{
    // nothing needs to be done here
    return true;
}

bool OceanOpticsSpectrometerInterface::Stop()
{
    m_wrapper->stopAveraging(m_spectrometerIndex);
    return true;
}

bool OceanOpticsSpectrometerInterface::SetSpectrometer(int spectrometerIndex)
{
    std::vector<int> channels{ 0 };
    return SetSpectrometer(spectrometerIndex);
}

bool OceanOpticsSpectrometerInterface::SetSpectrometer(int spectrometerIndex, const std::vector<int>& channelIndices)
{
    if (spectrometerIndex < 0 || spectrometerIndex >= static_cast<int>(m_spectrometersAttached.size()))
    {
        std::stringstream message;
        message << "Invalid spectrometer index " << spectrometerIndex << ". Number of spectrometers attached is: " << m_spectrometersAttached.size();
        m_lastErrorMessage = message.str();
        return false;
    }
    if (channelIndices.size() == 0)
    {
        m_lastErrorMessage = "Invalid channel indices provided. At least one channel must be used";
        return false;
    }

    // Verify the channel indices to use.
    // note here, getNumberOfEnabledChannels returns the number of channels which are connected to a working spectrometer,
    // whereas getNumberOfChannels returns the number of slots for spectrometers (8 for an ADC-1000USB).
    const int nAvailableChannels = m_wrapper->getWrapperExtensions().getNumberOfEnabledChannels(spectrometerIndex);

    for (int channelIndex : channelIndices)
    {
        if (channelIndex < 0 || channelIndex > nAvailableChannels)
        {
            std::stringstream message;
            message << "Invalid channel index " << channelIndex << ". Number of channels on current spectrometer is: " << nAvailableChannels;
            m_lastErrorMessage = message.str();
            return false;
        }
    }

    m_spectrometerIndex = spectrometerIndex;
    m_spectrometerChannels = channelIndices;
    m_lastErrorMessage.clear();

    return true;
}

std::string OceanOpticsSpectrometerInterface::GetSerial()
{
    const char* serial = m_wrapper->getSerialNumber(m_spectrometerIndex).getASCII();
    return std::string{ serial };
}

std::string OceanOpticsSpectrometerInterface::GetModel()
{
    const char* model = m_wrapper->getName(m_spectrometerIndex).getASCII();
    return std::string{ model };
}

int OceanOpticsSpectrometerInterface::GetNumberOfChannels()
{
    WrapperExtensions ext = m_wrapper->getWrapperExtensions();
    return ext.getNumberOfEnabledChannels(m_spectrometerIndex);
}

int OceanOpticsSpectrometerInterface::GetWavelengths(std::vector<std::vector<double>>& data)
{
    data.resize(m_spectrometerChannels.size());
    int spectrumLength = 0;

    for (int channelIdx = 0; channelIdx < static_cast<int>(m_spectrometerChannels.size()); ++channelIdx)
    {
        DoubleArray wavelengthArray = m_wrapper->getWavelengths(m_spectrometerIndex, channelIdx);

        spectrumLength = wavelengthArray.getLength();
        data[channelIdx].resize(spectrumLength);

        const double* spectrum = wavelengthArray.getDoubleValues();
        memcpy(data[channelIdx].data(), spectrum, spectrumLength * sizeof(double));
    }

    return spectrumLength;
}

int OceanOpticsSpectrometerInterface::GetSaturationIntensity()
{
    // For some spectrometers (notably the SD2000) does getSaturationIntensity return -1 
    WrapperExtensions ext = m_wrapper->getWrapperExtensions();
    int saturationIntensity = ext.getSaturationIntensity(m_spectrometerIndex);
    if (saturationIntensity < 0)
    {
        saturationIntensity = m_wrapper->getMaximumIntensity(m_spectrometerIndex);
    }

    return saturationIntensity;
}

void OceanOpticsSpectrometerInterface::SetIntegrationTime(int usec)
{
    for (int chn : m_spectrometerChannels)
    {
        m_wrapper->setIntegrationTime(m_spectrometerIndex, chn, usec);
    }
}

int OceanOpticsSpectrometerInterface::GetIntegrationTime()
{
    return m_wrapper->getIntegrationTime(m_spectrometerIndex, m_spectrometerChannels.front());
}

void OceanOpticsSpectrometerInterface::SetScansToAverage(int numberOfScansToAverage)
{
    for (int chn : m_spectrometerChannels)
    {
        m_wrapper->setScansToAverage(m_spectrometerIndex, chn, numberOfScansToAverage);
    }
}

int OceanOpticsSpectrometerInterface::GetScansToAverage()
{
    return m_wrapper->getScansToAverage(m_spectrometerIndex, m_spectrometerChannels.front());
}

int OceanOpticsSpectrometerInterface::GetNextSpectrum(std::vector<std::vector<double>>& data)
{
    data.resize(m_spectrometerChannels.size());
    int spectrumLength = 0;

    for (int channelIdx = 0; channelIdx < static_cast<int>(m_spectrometerChannels.size()); ++channelIdx)
    {
        DoubleArray spectrumArray = m_wrapper->getSpectrum(m_spectrometerIndex, m_spectrometerChannels[channelIdx]);

        // copies the spectrum-values to the output array
        spectrumLength = spectrumArray.getLength();
        data[channelIdx].resize(spectrumLength);

        const double* spectrum = spectrumArray.getDoubleValues();
        memcpy(data[channelIdx].data(), spectrum, spectrumLength * sizeof(double));
    }

    // Check the status of the last readout
    WrapperExtensions ext = m_wrapper->getWrapperExtensions();
    if (!ext.isSpectrumValid(m_spectrometerIndex))
    {
        m_lastErrorMessage = "Error reading out spectrum, last spectrum may not be valid";
        return 0;
    }

    m_lastErrorMessage.clear();

    return spectrumLength;
}

bool OceanOpticsSpectrometerInterface::SupportsDetectorTemperatureControl()
{
    return m_wrapper->isFeatureSupportedThermoElectric(m_spectrometerIndex);
}

bool OceanOpticsSpectrometerInterface::EnableDetectorTemperatureControl(bool enable, double temperatureInCelsius)
{
    ThermoElectricWrapper tew = m_wrapper->getFeatureControllerThermoElectric(m_spectrometerIndex);
    tew.setTECEnable(enable);
    tew.setDetectorSetPointCelsius(temperatureInCelsius);

    return tew.getTECEnable() != 0;
}

double OceanOpticsSpectrometerInterface::GetDetectorTemperature()
{
    if (m_wrapper->isFeatureSupportedThermoElectric(m_spectrometerIndex))
    {
        ThermoElectricWrapper tew = m_wrapper->getFeatureControllerThermoElectric(m_spectrometerIndex);
        return tew.getDetectorTemperatureCelsius();
    }

    return 0.0;
}

bool OceanOpticsSpectrometerInterface::SupportsBoardTemperature()
{
    return m_wrapper->isFeatureSupportedBoardTemperature(m_spectrometerIndex) == 1;
}

double OceanOpticsSpectrometerInterface::GetBoardTemperature()
{
    if (m_wrapper->isFeatureSupportedBoardTemperature(m_spectrometerIndex) == 1)
    {
        BoardTemperature bt = m_wrapper->getFeatureControllerBoardTemperature(m_spectrometerIndex);
        return bt.getBoardTemperatureCelsius();
    }

    return 0.0;
}

std::string OceanOpticsSpectrometerInterface::GetLastError()
{
    if (!m_lastErrorMessage.empty())
    {
        return m_lastErrorMessage;
    }

    auto ex = m_wrapper->getLastException();
    const char* str = ex.getASCII();
    if (str == nullptr)
    {
        return std::string{};
    }
    return std::string{ str };
}

#endif // MANUFACTURER_SUPPORT_OCEANOPTICS
