#include "StdAfx.h"
#include "MeasurementSetup.h"
#include "MeasurementModes/Measurement_Directory.h"
#include "MeasurementModes/Measurement_Traverse.h"
#include "MeasurementModes/Measurement_View.h"
#include "MeasurementModes/Measurement_Wind.h"
#include <SpectrometersLib/Spectrometers.h>


// GetSingleSpectrometerInterface selects _one_ spectrometer interface for which it is possible to find connected devices.
// This will currently return the first interface for which there are devices connected.
// Multiple devices from different manufaturers is currently not supported.
std::unique_ptr<mobiledoas::SpectrometerInterface> GetSingleSpectrometerInterface(std::vector < std::unique_ptr<mobiledoas::SpectrometerInterface>>& allInterfaces) {

    for (size_t idx = 0; idx < allInterfaces.size(); ++idx) {
        auto allDevices = allInterfaces[idx]->ScanForDevices();
        if (allDevices.size() != 0) {
            return std::move(allInterfaces[idx]);
        }
    }

    return nullptr;
}

CSpectrometer* CreateSpectrometer(SPECTROMETER_MODE measurementMode, CView& mainForm, std::unique_ptr<Configuration::CMobileConfiguration> conf)
{

    if (measurementMode == MODE_DIRECTORY)
    {
        // Directory mode doesn't require a SpectrometerInterface
        return new CMeasurement_Directory(mainForm, nullptr, std::move(conf));
    }

    auto connectionType = mobiledoas::SpectrometerConnectionType::USB;
    if (conf->m_spectrometerConnection == Configuration::CMobileConfiguration::CONNECTION_RS232)
    {
        connectionType = mobiledoas::SpectrometerConnectionType::RS232;
    }

    // TODO: use the conf->m_spectrometerConnection such that we can know if the connection is via USB or Serial.
    // That determines which SpectrometerInterface to instantiate..
    //if (!m_connectViaUsb) {
    //    // serial.isRunning = &m_isRunning;
    //    auto spec = std::make_unique<mobiledoas::OceanOpticsSpectrometerSerialInterface>();
    //    spec->SetBaudrate(m_conf->m_baudrate);
    //    spec->SetPort(m_conf->m_serialPort);
    //    m_spectrometer = std::move(spec);
    //}

    auto allInterfaces = mobiledoas::ListSpectrometerInterfaces(connectionType);

    if (allInterfaces.size() == 0)
    {
        MessageBox(nullptr, "Failed to create any spectrometer interface instance.", "Error", MB_OK);
        return nullptr;
    }

    auto spectrometerInterface = GetSingleSpectrometerInterface(allInterfaces);
    if (spectrometerInterface == nullptr)
    {
        MessageBox(nullptr, "Failed to find any connected spectrometer of a supported model.", "Error", MB_OK);
        return nullptr;
    }

    switch (measurementMode)
    {
    case MODE_TRAVERSE:
        return new CMeasurement_Traverse(mainForm, std::move(spectrometerInterface), std::move(conf));
    case MODE_VIEW:
        return new CMeasurement_View(mainForm, std::move(spectrometerInterface), std::move(conf));
    case MODE_WIND:
        return new CMeasurement_Wind(mainForm, std::move(spectrometerInterface), std::move(conf));
    }

    MessageBox(nullptr, "Error in program-logic: CreateSpectrometer was called with an unsupported measurement mode", "Error", MB_OK);

    return nullptr;
}

