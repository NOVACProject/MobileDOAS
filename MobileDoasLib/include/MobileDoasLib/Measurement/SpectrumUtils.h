#pragma once

#include <MobileDoasLib/Definitions.h>
#include<vector>

// SpectrumUtils collects together some commonly used spectrum related functions

namespace mobiledoas
{
/// <summary>
/// CheckIfDark returns true if the provided measured spectrum should be considered dark.
/// Inputs are the measured spectrum and the length of the detector(in pixels).
/// TODO: This could need some adaptation to different spectrometer models
/// </summary>
/// <param name="spectrum">The last measured spectrum.</param>
/// <returns>True if the measured spectrum is considered 'dark'</returns>
bool CheckIfDark(const std::vector<double>& spectrum);

/// <summary>
/// Returns the average intensity of the supplied spectrum in the given spectrum region.
/// The region is centered at specCenter with a width of 2*specCenterHalfWidth pixels.
/// I.e. from index (specCenter - specCenterHalfWidth) to index (specCenter + specCenterHalfWidth).
/// <param name="spectrum">The measured spectrum.</param>
/// <param name="specCenter">The index around which the intensity should be measured.</param>
/// <param name="specCenterHalfWidth">The half index with of the intensity measurement region.</param>
/// </summary>
long AverageIntensity(const std::vector<double>& spectrum, long specCenter, long specCenterHalfWidth);

/// <summary>
/// Calculates the pixel range over which an intensity measurement should be made with the given settings for spectrum center and half width.
/// The first point is inclusive and the last exclusive, meaning the true region is [result.first, result.second[
/// </summary>
/// <param name="specCenter">The index around which the intensity should be measured.</param>
/// <param name="specCenterHalfWidth">The half index with of the intensity measurement region.</param>
/// <param name="spectrumSize">The full length of the measred spectra.</param>
std::pair<long, long> GetIntensityMeasurementRegion(long specCenter, long specCenterHalfWidth, long spectrumSizes);

/// <summary>
/// Retrieves the (electronic-)offset of the supplied spectrum */
/// </summary>
double GetOffset(const std::vector<double>& spectrum);

/// <summary>
/// Basic representation of where spectra should be added, in the spectrometer directly or
/// in the computer after readout */
/// </summary>
struct SpectrumSummation
{
    int SumInComputer = 1;
    int SumInSpectrometer = 1;
};

/// <summary>
/// Counts how many spectra should be averaged inside the computer and
/// how many should be averaged inside the spectrometer get the desired
/// time resolution with the set exposure time.
/// @param timeResolution The set interval betweeen each spectrum to save, in milliseconds.
/// @param serialDelay The necessary delay to read out one spectrum from the spectrometer, in milliseconds.
/// @param gpsDelay The necessary delay to read out the time and position from the GPS, in milliseconds.
/// @param result Will be filled with the result of the calculation.
/// @return Number of spectra to co-add in the computer. */
/// </summary>
int CountRound(long timeResolution, long integrationTimeMs, long readoutDelay, long maximumAveragesInSpectrometer, SpectrumSummation& result);

struct SpectrumIntensityMeasurement
{
    SpectrumIntensityMeasurement(double saturationRatio, short integrationTime)
        : saturationRatio(saturationRatio), integrationTime(integrationTime)
    {
    }

    SpectrumIntensityMeasurement(long absoluteIntensity, long dynamicRange, short integrationTime)
        : saturationRatio(absoluteIntensity / (double)dynamicRange), integrationTime(integrationTime)
    {
    }

    // The current saturation ratio, in the range 0.0 to 1.0
    double saturationRatio;

    // The current integration (aka exposure) time.
    short integrationTime;
};

/// <summary>
/// AdjustIntegrationTimeToLastIntensity calculates a new integration time based on the given intensity and integration time 
/// in the last measurement performed.
/// </summary>
/// <param name="spectrumIntensityMeasurement">The current saturation ratio and integration time in the last measured spectrum.</param>
/// <param name="lastUsedIntegrationTime">The integration time (in ms) used in the last measured spectrum.</param>
/// <param name="minTolerableRatio">Minimum allowed saturation ratio (range 0.0 to 1.0)</param>
/// <param name="maxTolerableRatio">Maximum allowed saturation ratio (range 0.0 to 1.0)</param>
/// <param name="minAllowedIntegrationTime">The smallest allowed integration time (in ms).</param>
/// <param name="maxAllowedIntegrationTime">The largest allowed integration time (in ms).</param>
/// <returns>The new recommended integration time to use.</returns>
short AdjustIntegrationTimeToLastIntensity(
    SpectrumIntensityMeasurement spectrumIntensityMeasurement,
    double minTolerableRatio,
    double maxTolerableRatio,
    long minAllowedIntegrationTime,
    long maxAllowedIntegrationTime);

/// <summary>
/// EstimateNewIntegrationTime attempts to calculate a valid integration time given two meaurements performed,
/// one at a shorter integration time and one at a longer integration time.
/// </summary>
/// <param name="shortMeasurement">The saturation ratio and integration time of the shorter measurement.</param>
/// <param name="longMeasurement">The saturation ratio and integration time of the longer measurement.</param>
/// <param name="desiredSaturationRatio">The desired saturation ratio, in range 0.0 to 1.0.</param>
/// <param name="minAllowedIntegrationTime">The smallest allowed integration time (in ms).</param>
/// <param name="maxAllowedIntegrationTime">The largest allowed integration time (in ms).</param>
/// <returns></returns>
short EstimateNewIntegrationTime(
    SpectrumIntensityMeasurement shortMeasurement,
    SpectrumIntensityMeasurement longMeasurement,
    double desiredSaturationRatio,
    long minAllowedIntegrationTime,
    long maxAllowedIntegrationTime);
}