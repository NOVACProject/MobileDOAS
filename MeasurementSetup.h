#pragma once

#include <memory>
#include "Spectrometer.h"

// CreateSpectrometer creates and returns one spectrometer for the current measurement mode.
// Returns nullptr if no spectrometer could be created.
CSpectrometer* CreateSpectrometer(SPECTROMETER_MODE measurementMode);