#pragma once

#include <MobileDoasLib/Definitions.h>

// SpectrumUtils collects together some commonly used spectrum related functions

namespace mobiledoas
{
    /** CheckIfDark returns true if the provided measured spectrum should be considered dark.
    *   Inputs are the measured spectrum and the length of the detector (in pixels).
    *   TODO: This could need some adaptation to different spectrometer models **/
    bool CheckIfDark(double spectrum[MAX_SPECTRUM_LENGTH], int detectorSize);

    /** Returns the average intensity of the supplied spectrum in the given spectrum region.
    *   The region is centered at specCenter with a width of 2*specCenterHalfWidth pixels.
    *   @param pSpectrum pointer to the first pixel in the measured spectrum.
    *        This is assumed to be MAX_SPECTRUM_LENGTH number of pixels long. */
    long AverageIntensity(double* pSpectrum, long specCenter, long specCenterHalfWidth);

    /** Retrieves the (electronic-)offset of the supplied spectrum */
    double GetOffset(double spectrum[MAX_SPECTRUM_LENGTH]);

}