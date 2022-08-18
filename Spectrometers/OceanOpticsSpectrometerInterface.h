#pragma once

#include <MobileDoasLib/Measurement/SpectrometerInterface.h>

// Forward declaration of the ocean-optics Wrapper class, such that we don't need to 
// include the OceanOptics headers everywhere..
class Wrapper;

namespace mobiledoas
{
    // OceanOpticsSpectrometerInterface is an implementation of the SpectrometerInterface
    // for accessing OceanOptics spectrometers through USB.
    class OceanOpticsSpectrometerInterface : public SpectrometerInterface
    {
    public:

        /** The spectrometer to use, if there are several attached
            must be at least 0 and always smaller than 'm_numberOfSpectrometersAttached' */
        int m_spectrometerIndex = 0;

        /** The channel to use on the attached spectrometer */
        int m_spectrometerChannel = 0;

        /** The number of spectrometers that are attached to this computer */
        int m_numberOfSpectrometersAttached = 0;

#pragma region Implementing SpectrometerInterface

        virtual std::vector<std::string> ScanForDevices() override;

        virtual void Close() override;

        virtual bool SetSpectrometer(int spectrometerIndex, int channelIndex) override;

        virtual std::string GetSerial() override;

        virtual std::string GetModel() override;

        virtual int GetWavelengths(std::vector<double>& data) override;

        virtual int GetSaturationIntensity() override;

        virtual void SetIntegrationTime(int usec) override;

        virtual int GetIntegrationTime() override;

        virtual void SetScansToAverage(int numberOfScansToAverage) override;

        virtual int GetScansToAverage() override;

        virtual int GetNextSpectrum(std::vector<double>& data) override;

        virtual bool SupportsDetectorTemperatureControl() override;

        virtual bool EnableDetectorTemperatureControl(bool enable, double temperatureInCelsius) override;

        virtual double GetDetectorTemperature() override;

        virtual bool SupportsBoardTemperature() override;

        virtual double GetBoardTemperature() override;

        virtual std::string GetLastError() override;

#pragma endregion

    private:


        /** This is the object through which we will access all of Omnidriver's capabilities
            This is used to control the OceanOptics Spectrometers through USB.
            There can be only one Wrapper object in the application!!!  */
        class Wrapper* m_wrapper;

    };
}