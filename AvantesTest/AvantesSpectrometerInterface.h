#pragma once

#include <vector>
#include <MobileDoasLib/Measurement/SpectrometerInterface.h>

namespace mobiledoas
{
    // AvantesSpectrometerInterface is an implementation of the SpectrometerInterface
    // for accessing OceanOptics spectrometers through USB.
    class AvantesSpectrometerInterface : public SpectrometerInterface
    {
    public:
        AvantesSpectrometerInterface();
        virtual ~AvantesSpectrometerInterface();

        // This object is unfortunately not copyable since it contains a pointer which in itself cannot be copied.
        AvantesSpectrometerInterface(const AvantesSpectrometerInterface& other) = delete;
        AvantesSpectrometerInterface& operator=(const AvantesSpectrometerInterface& other) = delete;

        /** The spectrometer to use, if there are several attached.
            must be at least 0 and always smaller than 'm_numberOfSpectrometersAttached' */
        int m_spectrometerIndex = 0;

        /** The channels to use on the attached spectrometer */
        std::vector<int> m_spectrometerChannels;

        /** The number of spectrometers that are attached to this computer */
        int m_numberOfSpectrometersAttached = 0;

#pragma region Implementing SpectrometerInterface

        virtual std::vector<std::string> ScanForDevices() override;

        virtual void Close() override;

        virtual void Stop() override;

        virtual bool SetSpectrometer(int spectrometerIndex) override;

        virtual bool SetSpectrometer(int spectrometerIndex, const std::vector<int>& channelIndices) override;

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

        // Handling the internal state.
        void *m_state = nullptr;

        // The last error message set by this class (note that the m_wrapper may also have an error message set).
        std::string m_lastErrorMessage;

    };
}