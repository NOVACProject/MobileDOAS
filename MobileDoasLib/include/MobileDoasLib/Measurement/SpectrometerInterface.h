#pragma once

#include <string>
#include <vector>

namespace mobiledoas
{
    enum class SpectrometerConnectionType {
        USB,
        RS232
    };

    /** SpectrometerInterface is an abstract base class for accessing the spectrometer hardware.
        Different implementations enable different models and makes of spectrometers */
    class SpectrometerInterface
    {
    public:

        virtual ~SpectrometerInterface() {}

        // ConnectionType returns the connection type which this SpectrometerInterface supports.
        virtual SpectrometerConnectionType ConnectionType() = 0;

        // ScanDevices searches for spectrometers connected to this computer.
        // This must be called before any other of the methods in this class.
        // @return the serial numbers of the devices found.
        virtual std::vector<std::string> ScanForDevices() = 0;

        // ListDevices returns the list of spectrometers connected to this computer.
        // This will return the same list as the last call to ScanForDevices found and returned.
        // @return the serial numbers of the devices connected.
        virtual std::vector<std::string> ListDevices() const = 0;

        // Close closes the connection to the spectrometer library and will return all resources to the system.
        // After this, all other methods below will return an error.
        virtual void Close() = 0;

        // Start will start the spectrum acquisition.
        // This may be necessary to call before calling 'GetNextSpectrum' in order to get a spectrum at all.
        // This requires that ScanForDevices() and SetSpectrometer() have been called successfully.
        // @return true if the measurement was started successfully.
        virtual bool Start() = 0;

        // Stop will stop all running acquisitions.
        // This does not close the connections to the spectrometers.
        // @return true if the measurement was stopped successfully.
        virtual bool Stop() = 0;

        // SetSpectrometer decides on which of the connected spectrometers (of this particular make) to use.
        // Provided spectrometerIndex must be >= 0 and less than the number of devices returned from ScanForDevices.
        // If the spectrometer has multiple channels, then the first channel will be selected and used.
        // @return true if the spectrometer connection was made, otherwise false.
        // This requires that ScanForDevices() has been called successfully.
        virtual bool SetSpectrometer(int spectrometerIndex) = 0;

        // SetSpectrometer decides on which of the connected spectrometers (of this particular make) to use.
        // Provided spectrometerIndex must be >= 0 and less than the number of devices returned from ScanForDevices.
        // Provided channelIndices must each be >= 0 and less than the number of channels available on the device.
        // @return true if the spectrometer connection was made, otherwise false.
        // This requires that ScanForDevices() has been called successfully.
        virtual bool SetSpectrometer(int spectrometerIndex, const std::vector<int>& channelIndices) = 0;

        // GetReadoutDelay returns the expected delay for a single readout from this spectrometer.
        // The readout typically depends on the connection type between the computer and the device.
        virtual int GetReadoutDelay() = 0;

        // GetSerial returns the serial number of the current spectrometer.
        // Requires that SetSpectrometer has been called successfully.
        virtual std::string GetSerial() = 0;

        // GetModel returns the spectrometer model name of the current spectrometer.
        // Requires that SetSpectrometer has been called successfully.
        virtual std::string GetModel() = 0;

        // GetNumberOfChannels returns the number of channels inside the current spectrometer.
        // The number of channels is a special thing with the OceanOptics SD2000 spectrometers which are
        // able to house multiple spectrometers inside one box. Each such individal spectrometer is known as a 'channel'.
        // Requires that SetSpectrometer has been called successfully.
        virtual int GetNumberOfChannels() = 0;

        // GetWavelengths returns the set wavelength for each pixel on the detector.
        // This requires that the device has a pixel-to-wavelength calibration.
        // This will fill in the values into the provided vector, there will be one vector of data for each channel.
        // @return the number of values filled in.
        // Requires that SetSpectrometer has been called successfully.
        virtual int GetWavelengths(std::vector<std::vector<double>>& data) = 0;

        // GetSaturationIntensity returns the maximum intensity in the current spectrometer.
        // Requires that SetSpectrometer has been called successfully.
        virtual int GetSaturationIntensity() = 0;

        // Sets the integration time (exposure time) in microseconds.
        // Requires that SetSpectrometer has been called successfully.
        virtual void SetIntegrationTime(int usec) = 0;

        // GetIntegrationTime returns the currently set integration time, in microseconds
        // Requires that SetSpectrometer has been called successfully.
        virtual int GetIntegrationTime() = 0;

        // SetScansToAverage sets the number of spectra to average together in the spectrometer hardware
        // Requires that SetSpectrometer has been called successfully.
        virtual void SetScansToAverage(int numberOfScansToAverage) = 0;

        // GetScansToAverage returns the currently set number of spectra to average together in the spectrometer hardware.
        // Requires that SetSpectrometer has been called successfully.
        virtual int GetScansToAverage() = 0;

        // GetNextSpectrum returns the next spectrum readout. This blocks until there is a spectrum available.
        // This will fill in the values into the provided vector, there will be one vector of data for each channel.
        // @return the number of values read out (the length of the spectrum).
        // @return zero if something goes wrong while doing the readout (reason can be retrieved using GetLastError()).
        // Requires that SetSpectrometer has been called successfully.
        virtual int GetNextSpectrum(std::vector<std::vector<double>>& data) = 0;

        // SupportsDetectorTemperatureControl returns true if this device is able to set a specific temperature on the detector.
        // Requires that SetSpectrometer has been called successfully.
        virtual bool SupportsDetectorTemperatureControl() = 0;

        // EnableDetectorTemperatureControl enables the detector temperature control and sets the desired temperature in Celsius.
        // Will only make a difference if SupportsDetectorTemperatureControl() returns true.
        // @return true if the detector temperature control was enabled.
        // Requires that SetSpectrometer has been called successfully.
        virtual bool EnableDetectorTemperatureControl(bool enable, double temperatureInCelsius) = 0;

        // GetDetectorTemperature returns the current temperature of the detector.
        // This will return zero if the device does not support reading the detector temperature.
        // Requires that SetSpectrometer has been called successfully.
        virtual double GetDetectorTemperature() = 0;

        // SupportsBoardTemperature returns true if this device is able to read the temperature of the board.
        // Requires that SetSpectrometer has been called successfully.
        virtual bool SupportsBoardTemperature() = 0;

        // GetBoardTemperature returns the current temperature of the board.
        // This will return zero if SupportsBoardTemperature() is false.
        // Requires that SetSpectrometer has been called successfully.
        virtual double GetBoardTemperature() = 0;

        // Returns the last set error message, if any. Returns empty if no error.
        virtual std::string GetLastError() = 0;

    };

}