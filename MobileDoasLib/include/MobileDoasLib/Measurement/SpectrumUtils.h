#pragma once

#include <MobileDoasLib/Definitions.h>

// SpectrumUtils collects together some commonly used spectrum related functions

namespace mobiledoas
{
    /** CheckIfDark returns true if the provided measured spectrum should be considered dark.
    *   Inputs are the measured spectrum and the length of the detector (in pixels).
    *   TODO: This could need some adaptation to different spectrometer models **/
    bool CheckIfDark(double spectrum[MAX_SPECTRUM_LENGTH], int detectorSize);


}