#pragma once

#include <MobileDoasLib/Measurement/SpectrometerInterface.h>
#include "../SerialConnection.h"

namespace mobiledoas
{
    // OceanOpticsSpectrometerSerialInterface is an implementation of the SpectrometerInterface
    // for accessing OceanOptics spectrometers through Serial.
    class OceanOpticsSpectrometerSerialInterface : public SpectrometerInterface
    {
    public:
        OceanOpticsSpectrometerSerialInterface();
        virtual ~OceanOpticsSpectrometerSerialInterface();

        // This object is unfortunately not copyable since it contains a pointer which in itself cannot be copied.
        OceanOpticsSpectrometerSerialInterface(const OceanOpticsSpectrometerSerialInterface& other) = delete;
        OceanOpticsSpectrometerSerialInterface& operator=(const OceanOpticsSpectrometerSerialInterface& other) = delete;

        void SetBaudrate(long speed);

        void SetPort(const CString& port);


#pragma region Implementing SpectrometerInterface

        virtual std::vector<std::string> ScanForDevices() override;

        virtual void Close() override;

        virtual bool Start() override;

        virtual bool Stop() override;

        virtual bool SetSpectrometer(int spectrometerIndex) override;

        virtual bool SetSpectrometer(int spectrometerIndex, const std::vector<int>& channelIndices) override;

        virtual int GetReadoutDelay() override { return 500; }

        virtual std::string GetSerial() override;

        virtual std::string GetModel() override;

        virtual int GetNumberOfChannels() override;

        virtual int GetWavelengths(std::vector<std::vector<double>>& data) override;

        virtual int GetSaturationIntensity() override;

        virtual void SetIntegrationTime(int usec) override;

        virtual int GetIntegrationTime() override;

        virtual void SetScansToAverage(int numberOfScansToAverage) override;

        virtual int GetScansToAverage() override;

        virtual int GetNextSpectrum(std::vector<std::vector<double>>& data) override;

        virtual bool SupportsDetectorTemperatureControl() override;

        virtual bool EnableDetectorTemperatureControl(bool enable, double temperatureInCelsius) override;

        virtual double GetDetectorTemperature() override;

        virtual bool SupportsBoardTemperature() override;

        virtual double GetBoardTemperature() override;

        virtual std::string GetLastError() override;

#pragma endregion

    private:

        /** The serial-communication object.*/
        CSerialConnection serial;

        // The last error message set by this class.
        std::string m_lastErrorMessage;

        // The number of spectra to co-add in the spectrometer
        int m_sumInSpectrometer = 15;

        // The integration time, in milliseconds
        short m_integrationTime = 100;

        /** Initializes the spectrometer.
            @param channel - the channel to use (0 <-> master, 1 <-> slave, 257 <-> master & slave)
            @param inttime - the integration time to use (in milli seconds)
            @param sumSpec - the number of spectra to co-add in the spectrometer */
        int InitSpectrometer(short channel, short inttime, short sumSpec);
    };
}