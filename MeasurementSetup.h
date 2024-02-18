#pragma once

#include <memory>
#include "Spectrometer.h"
#include "Configuration/MobileConfiguration.h"

// CreateSpectrometer creates and returns one spectrometer for the current measurement mode.
// Returns nullptr if no spectrometer could be created.
CSpectrometer* CreateSpectrometer(SPECTROMETER_MODE measurementMode, CView& mainForm, std::unique_ptr<Configuration::CMobileConfiguration> conf);