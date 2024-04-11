#include "pch.h"

#ifdef MANUFACTURER_SUPPORT_OCEANOPTICS

#include "OceanOpticsSpectrometerSerialInterface.h"

#include <sstream>

#define SERIALDELAY 2000

using namespace mobiledoas;
using namespace oceanoptics;

OceanOpticsSpectrometerSerialInterface::OceanOpticsSpectrometerSerialInterface()
{
}

OceanOpticsSpectrometerSerialInterface::~OceanOpticsSpectrometerSerialInterface()
{
}

void OceanOpticsSpectrometerSerialInterface::SetBaudrate(long speed)
{
    serial.SetBaudrate(speed);
}

void OceanOpticsSpectrometerSerialInterface::SetPort(const std::string& port)
{
    serial.SetPort(port);
}

std::vector<std::string> OceanOpticsSpectrometerSerialInterface::ScanForDevices()
{
    if (serial.InitCommunication() != 0)
    {
        // Failed to initialize the communication, no spectrometers can be used.
        m_spectrometersAttached.clear();
        return m_spectrometersAttached;
    }

    m_spectrometersAttached = std::vector<std::string>{ "Unknown" };
    return m_spectrometersAttached;
}

std::vector<std::string> OceanOpticsSpectrometerSerialInterface::ListDevices() const
{
    return this->m_spectrometersAttached;
}

void OceanOpticsSpectrometerSerialInterface::Close()
{
    serial.CloseAll();
}

bool OceanOpticsSpectrometerSerialInterface::Start()
{
    // nothing needs to be done here
    return true;
}

bool OceanOpticsSpectrometerSerialInterface::Stop()
{
    return true;
}

bool OceanOpticsSpectrometerSerialInterface::SetSpectrometer(int /*spectrometerIndex*/)
{
    return false;
}

bool OceanOpticsSpectrometerSerialInterface::SetSpectrometer(int /*spectrometerIndex*/, const std::vector<int>& /*channelIndices*/)
{
    return true;
}

std::string OceanOpticsSpectrometerSerialInterface::GetSerial()
{
    // const char* serial = m_wrapper->getSerialNumber(m_spectrometerIndex).getASCII();
    // return std::string{ serial };
    return std::string{};
}

std::string OceanOpticsSpectrometerSerialInterface::GetModel()
{
    return std::string();
}

int OceanOpticsSpectrometerSerialInterface::GetNumberOfChannels()
{
    return 1;
}

int OceanOpticsSpectrometerSerialInterface::GetWavelengths(std::vector<std::vector<double>>& /*data*/)
{
    // TODO: Implement
    return 0;
}

int OceanOpticsSpectrometerSerialInterface::GetSaturationIntensity()
{
    // TODO: Implement
    return 0;
}

void OceanOpticsSpectrometerSerialInterface::SetIntegrationTime(int usec)
{
    // TODO: This could very well overflow, need to notify the user when this happens.
    m_integrationTime = static_cast<short>(usec / 1000);

    InitSpectrometer(0, m_integrationTime, static_cast<short>(m_sumInSpectrometer));
}

int OceanOpticsSpectrometerSerialInterface::GetIntegrationTime()
{
    return m_integrationTime;
}

void OceanOpticsSpectrometerSerialInterface::SetScansToAverage(int numberOfScansToAverage)
{
    m_sumInSpectrometer = numberOfScansToAverage;

    InitSpectrometer(0, m_integrationTime, static_cast<short>(numberOfScansToAverage));
}

int OceanOpticsSpectrometerSerialInterface::GetScansToAverage()
{
    return m_sumInSpectrometer;
}

/** This function swaps the place of the MostSignificantByte and
*   the LeastSignificantByte of the given number */
unsigned short Swp(unsigned short in)
{
    unsigned char* p1, * p2;
    unsigned short ut;

    p1 = (unsigned char*)&ut;
    p2 = (unsigned char*)&in;
    p1[0] = p2[1];
    p1[1] = p2[0];
    return(ut);
}

int OceanOpticsSpectrometerSerialInterface::GetNextSpectrum(std::vector<std::vector<double>>& data)
{

    unsigned short sbuf[8192];
    char txt[256];
    int i, j;

    const long maxlen = 65536;
    double* smem1 = (double*)malloc(sizeof(double) * (2 + maxlen)); // initialize one buffer
    if (smem1 == 0)
    {
        m_lastErrorMessage = "Not enough memory";
        return 0;
    }
    memset((void*)smem1, 0, sizeof(double) * (2 + maxlen));

    // scan the serial port
    {
        //Empty the serial buffer
        serial.FlushSerialPort(100);
        //Send command to spectrometer for sending data
        txt[0] = 'S';
        char* bptr = (char*)&sbuf[0];
        serial.Write(txt, 1);

        txt[0] = 0;

        if (serial.Check(1000))
        {
            serial.Read(txt, 1);
        }

        // wait first byte to come
        long waitTime = m_sumInSpectrometer * m_integrationTime + SERIALDELAY;

        serial.Check(waitTime);


        //checkSerial 100 ms is for reading all data from buffer
        i = 0;
        while (serial.Check(300) && i < 16384)
        {
            j = serial.Read(&bptr[i], 16384 - i);
            i += j;
        }
        if (i == 0)
        {
            free(smem1);
            m_lastErrorMessage = "Communication timeout";
            return 0;
        }

        if (sbuf[0] != 0xffff)
        {
            free(smem1);
            m_lastErrorMessage = "First byte of transmission is incorrect";
            return 0;
        }

        //delete header byte number
        i = i / 2 - 8;


        for (j = 0; j < i; j++)
        {
            sbuf[j] = Swp(sbuf[j + 8]);
        }

        smem1[0] = i;
        for (j = 0; j < smem1[0]; j++)
        {
            smem1[3 + j * 2] += sbuf[j];
        }
    }


    data.resize(1); // Only one channel here. TODO: Splitting dual spectrometer data??
    const int spectrumLength = int(std::floor(smem1[0]));
    data[0].resize(spectrumLength);
    for (int n = 0; n < spectrumLength; n++)
    {
        data[0][n] = smem1[3 + n * 2] / (m_sumInSpectrometer);
    }

    if (smem1)
    {
        free(smem1);
    }

    return spectrumLength;
}

bool OceanOpticsSpectrometerSerialInterface::SupportsDetectorTemperatureControl()
{
    return false;
}

bool OceanOpticsSpectrometerSerialInterface::EnableDetectorTemperatureControl(bool /*enable*/, double /*temperatureInCelsius*/)
{
    return false;
}

double OceanOpticsSpectrometerSerialInterface::GetDetectorTemperature()
{
    return 0.0;
}

bool OceanOpticsSpectrometerSerialInterface::SupportsBoardTemperature()
{
    return false;
}

double OceanOpticsSpectrometerSerialInterface::GetBoardTemperature()
{
    return 0.0;
}

std::string OceanOpticsSpectrometerSerialInterface::GetLastError()
{
    return std::string{};
}

int OceanOpticsSpectrometerSerialInterface::InitSpectrometer(short channel, short inttime, short sumSpec)
{
    unsigned short sbuf;
    char txt[256];
    unsigned char* p;
    sbuf = 257;
    p = (unsigned char*)&channel;
    txt[0] = 'H';
    txt[1] = p[1];
    txt[2] = p[0];
    serial.Write(txt, 3);

    while (serial.Check(30))
        serial.Read(&txt, 1);

    p = (unsigned char*)&inttime;
    txt[0] = 'I';
    txt[1] = p[1];
    txt[2] = p[0];
    serial.Write(txt, 3);

    while (serial.Check(30))
        serial.Read(&txt, 1);

    p = (unsigned char*)&sumSpec;
    txt[0] = 'A';
    txt[1] = p[1];
    txt[2] = p[0];
    serial.Write(txt, 3);

    while (serial.Check(30))
        serial.Read(&txt, 1);

    return 0;
}

#endif // MANUFACTURER_SUPPORT_OCEANOPTICS
