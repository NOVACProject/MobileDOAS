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


    /** Basic representation of where spectra should be added, in the spectrometer directly or
        in the computer after readout */
    struct SpectrumSummation
    {
        int SumInComputer = 1;
        int SumInSpectrometer = 1;
    };

    /** Counts how many spectra should be averaged inside the computer and
        how many should be averaged inside the spectrometer get the desired
        time resolution with the set exposure time.
        @param timeResolution The set interval betweeen each spectrum to save, in milliseconds.
        @param serialDelay The necessary delay to read out one spectrum from the spectrometer, in milliseconds.
        @param gpsDelay The necessary delay to read out the time and position from the GPS, in milliseconds.
        @param result Will be filled with the result of the calculation.
        @return Number of spectra to co-add in the computer. */
    int CountRound(long timeResolution, long integrationTimeMs, long readoutDelay, long maximumAveragesInSpectrometer, SpectrumSummation& result);

}