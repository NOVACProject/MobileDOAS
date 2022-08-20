#pragma once

#include <string>
#include <vector>

namespace mobiledoas
{
    /** SpectrometerInterface is an abstract base class for accessing the spectrometer hardware.
        Different implementations enable different models and makes of spectrometers */
    class SpectrometerInterface
    {
    public:

        // ScanDevices searches for spectrometers connected to this computer.
        // This must be called before any other of the methods in this class.
        // @return the serial numbers of the devices found.
        virtual std::vector<std::string> ScanForDevices() = 0;

        // Close closes the connection to the spectrometer library and will return all resources to the system.
        // After this, all other methods below will return an error.
        virtual void Close() = 0;

        // Stop will stop all running acquisitions.
        // This does not close the connections to the spectrometers.
        virtual void Stop() = 0;

        // SetSpectrometer decides on which of the connected spectrometers (of this particular make) to use.
        // Provided spectrometerIndex must be >= 0 and less than the number of devices returned from ScanForDevices.
        // Provided channelIndices must each be >= 0 and less than the number of channels available on the device.
        // @return true if the spectrometer connection was made, otherwise false.
        // This requires that ScanForDevices() has been called successfully.
        virtual bool SetSpectrometer(int spectrometerIndex, const std::vector<int>& channelIndices) = 0;

        // GetSerial returns the serial number of the current spectrometer.
        // Reqiures that SetSpectrometer has been called successfully.
        virtual std::string GetSerial() = 0;

        // GetModel returns the spectrometer model name of the current spectrometer.
        // Reqiures that SetSpectrometer has been called successfully.
        virtual std::string GetModel() = 0;

        // GetNumberOfChannels returns the number of channels inside the current spectrometer.
        // Reqiures that SetSpectrometer has been called successfully.
        virtual int GetNumberOfChannels() = 0;

        // GetWavelengths returns the set wavelength for each pixel on the detector.
        // This requires that the device has a pixel-to-wavelength calibration.
        // This will fill in the values into the provided vector, there will be one vector of data for each channel.
        // @return the number of values filled in.
        // Reqiures that SetSpectrometer has been called successfully.
        virtual int GetWavelengths(std::vector<std::vector<double>>& data) = 0;

        // GetSaturationIntensity returns the maximum intensity in the current spectrometer.
        // Reqiures that SetSpectrometer has been called successfully.
        virtual int GetSaturationIntensity() = 0;

        // Sets the integration time (exposure time) in microseconds.
        // Reqiures that SetSpectrometer has been called successfully.
        virtual void SetIntegrationTime(int usec) = 0;

        // GetIntegrationTime returns the currently set integration time, in microseconds
        // Reqiures that SetSpectrometer has been called successfully.
        virtual int GetIntegrationTime() = 0;

        // SetScansToAverage sets the number of spectra to average together in the spectrometer hardware
        // Reqiures that SetSpectrometer has been called successfully.
        virtual void SetScansToAverage(int numberOfScansToAverage) = 0;

        // GetScansToAverage returns the currently set number of spectra to average together in the spectrometer hardware.
        // Reqiures that SetSpectrometer has been called successfully.
        virtual int GetScansToAverage() = 0;

        // GetNextSpectrum returns the next spectrum readout. This blocks until there is a spectrum available.
        // This will fill in the values into the provided vector, there will be one vector of data for each channel.
        // @return the number of values read out (the length of the spectrum).
        // Reqiures that SetSpectrometer has been called successfully.
        virtual int GetNextSpectrum(std::vector<std::vector<double>>& data) = 0;

        // SupportsDetectorTemperatureControl returns true if this device is able to set a specific temperature on the detector.
        // Reqiures that SetSpectrometer has been called successfully.
        virtual bool SupportsDetectorTemperatureControl() = 0;

        // EnableDetectorTemperatureControl enables the detector temperature control and sets the desired temperature in Celsius.
        // Will only make a difference if SupportsDetectorTemperatureControl() returns true.
        // @return true if the detector temperature control was enabled.
        // Reqiures that SetSpectrometer has been called successfully.
        virtual bool EnableDetectorTemperatureControl(bool enable, double temperatureInCelsius) = 0;

        // GetDetectorTemperature returns the current temperature of the detector.
        // This will return zero if the device does not support reading the detector temperature.
        // Reqiures that SetSpectrometer has been called successfully.
        virtual double GetDetectorTemperature() = 0;

        // SupportsBoardTemperature returns true if this device is able to read the temperature of the board.
        // Reqiures that SetSpectrometer has been called successfully.
        virtual bool SupportsBoardTemperature() = 0;

        // GetBoardTemperature returns the current temperature of the board.
        // This will return zero if SupportsBoardTemperature() is false.
        // Reqiures that SetSpectrometer has been called successfully.
        virtual double GetBoardTemperature() = 0;

        // Returns the last set error message, if any. Returns empty if no error.
        virtual std::string GetLastError() = 0;

    };

}