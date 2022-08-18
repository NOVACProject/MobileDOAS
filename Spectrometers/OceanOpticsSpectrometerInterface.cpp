#include "StdAfx.h"
#include "OceanOpticsSpectrometerInterface.h"

// The OceanOptics drivers headers, located in %OMNIDRIVER_HOME%\include
#include <ArrayTypes.h>
#include <Wrapper.h>
//#include <ADC1000USB.h>
//#include <ADC1000Channel.h>

using namespace mobiledoas;

std::vector<std::string> OceanOpticsSpectrometerInterface::ScanForDevices()
{
    m_numberOfSpectrometersAttached = m_wrapper->openAllSpectrometers();

    std::vector<std::string> serialNumbers;
    serialNumbers.resize(m_numberOfSpectrometersAttached);

    for (int ii = 0; ii < m_numberOfSpectrometersAttached; ++ii)
    {
        const char* serial = m_wrapper->getSerialNumber(ii).getASCII();
        serialNumbers[ii] = std::string{ serial };
    }

    return serialNumbers;
}

void OceanOpticsSpectrometerInterface::Close()
{
    m_wrapper->closeAllSpectrometers();
}

bool OceanOpticsSpectrometerInterface::SetSpectrometer(int spectrometerIndex, int channelIndex)
{
    if (spectrometerIndex < 0 || spectrometerIndex > m_numberOfSpectrometersAttached)
    {
        return false;
    }

    const int nAvailableChannels = m_wrapper->getWrapperExtensions().getNumberOfChannels(spectrometerIndex);
    if (channelIndex < 0 || channelIndex > nAvailableChannels)
    {
        return false;
    }

    m_spectrometerIndex = spectrometerIndex;
    m_spectrometerChannel = channelIndex;

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

int OceanOpticsSpectrometerInterface::GetWavelengths(std::vector<double>& data)
{
    DoubleArray wavelengthArray = m_wrapper->getWavelengths(m_spectrometerIndex);

    const int spectrumLength = wavelengthArray.getLength();
    data.resize(spectrumLength);

    const double* spectrum = wavelengthArray.getDoubleValues();
    for (int i = 0; i < wavelengthArray.getLength(); i++) {
        data[i] = spectrum[i];
    }

    return spectrumLength;
}

int OceanOpticsSpectrometerInterface::GetSaturationIntensity()
{
    WrapperExtensions ext = m_wrapper->getWrapperExtensions();
    return ext.getSaturationIntensity(m_spectrometerIndex);
}

void OceanOpticsSpectrometerInterface::SetIntegrationTime(int usec)
{
    m_wrapper->setIntegrationTime(m_spectrometerIndex, m_spectrometerChannel, usec);
}

int OceanOpticsSpectrometerInterface::GetIntegrationTime()
{
    return m_wrapper->getIntegrationTime(m_spectrometerIndex, m_spectrometerChannel);
}

void OceanOpticsSpectrometerInterface::SetScansToAverage(int numberOfScansToAverage)
{
    m_wrapper->setScansToAverage(m_spectrometerIndex, m_spectrometerChannel, numberOfScansToAverage);
}

int OceanOpticsSpectrometerInterface::GetScansToAverage()
{
    return m_wrapper->getScansToAverage(m_spectrometerIndex, m_spectrometerChannel);
}

int OceanOpticsSpectrometerInterface::GetNextSpectrum(std::vector<double>& data)
{
    DoubleArray spectrumArray = m_wrapper->getSpectrum(m_spectrometerIndex, m_spectrometerChannel);

    // copies the spectrum-values to the output array
    const int spectrumLength = spectrumArray.getLength();

    data.resize(spectrumLength);
    const double* spectrum = spectrumArray.getDoubleValues();
    for (int i = 0; i < spectrumArray.getLength(); i++) {
        data[i] = spectrum[i];
    }

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
    auto ex = m_wrapper->getLastException();
    const char* str = ex.getASCII();
    if (str == nullptr)
    {
        return std::string{};
    }
    return std::string{ str };
}