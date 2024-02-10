#pragma once

#include <vector>
#include <MobileDoasLib/Definitions.h>

namespace mobiledoas
{

/// <summary>
/// MeasuredSpectrum represents a spectrum measured directly from the spectrometer.
/// This contains multiple channels, for support of the multi channel spectrometer (SD2000),
/// as well as convenience methods for the most commonly used operations.
/// </summary>
struct MeasuredSpectrum
{
    MeasuredSpectrum()
    {
        Resize(MAX_N_CHANNELS, MAX_SPECTRUM_LENGTH);
    }

    MeasuredSpectrum(int numberOfChannels, int spectrumLength)
    {
        Resize(numberOfChannels, spectrumLength);
    }

    void SetToZero();

    void Resize(int numberOfChannels, int spectrumLength);

    /// <summary>
    /// Brackets operator retrieves the measured data for the given channel index.
    /// </summary>
    /// <param name="channelIdx">The channel to retrieve.</param>
    /// <returns>The measured data for the given channel.</returns>
    std::vector<double>& operator[](size_t channelIdx)
    {
        return data[channelIdx];
    }

    /// <summary>
    /// Copies the data from this measured spectrum to the destination.
    /// </summary>
    /// <param name="destination">Destination of the data. Will be resized to this size if necessary.</param>
    void CopyTo(MeasuredSpectrum& destination) const;

    /// <summary>
    /// Copies data from the provided array of data into the given channel number.
    /// </summary>
    /// <param name="channelNumber">The destination channel number.</param>
    /// <param name="source">The data to copy.</param>
    /// <param name="numberOfElements>The number of elements to copy.</param>
    void CopyFrom(int channelNumber, double source[], int numberOfElements);

    int NumberOfChannels() const { return static_cast<int>(data.size()); }

    int SpectrumLength() const
    {
        return (data.size() == 0) ? 0 : static_cast<int>(data[0].size());
    }

    std::vector<std::vector<double>> data;
};

}
