#include <MobileDoasLib/Measurement/SpectrumUtils.h>
#include <vector>
#include <numeric>

namespace mobiledoas
{

bool CheckIfDark(const std::vector<double>& spectrum)
{
    int detectorSize = static_cast<int>(spectrum.size());

    // consider pixels 50 to 70. Remove the highest 3 in case they are 'hot'. Then calculate the average:
    std::vector<double> vec;
    double m_darkIntensity;
    double m_spectrumIntensity;

    for (int i = 50; i < 70; i++)
    {
        vec.push_back(spectrum[i]);
    }
    for (int i = 0; i < 3; i++)
    {
        vec.erase(max_element(vec.begin(), vec.end()));
    }
    m_darkIntensity = std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();

    // now calculate the average of the middle 20 pixels, again excluding the 3 highest intensities. 
    vec.clear();
    int start = int(floor(detectorSize / 2) - 10);
    int end = int(floor(detectorSize / 2) + 10);
    for (int i = start; i < end; i++)
    {
        vec.push_back(spectrum[i]);
    }
    for (int i = 0; i < 3; i++)
    {
        vec.erase(max_element(vec.begin(), vec.end()));
    }
    m_spectrumIntensity = std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();

    // the spectrum is considered dark if the center intensity is less than twice as high as the dark intensity.
    // this should be applicable to any spectrometer, as long as pixels 50 to 70 are dark (which is true for NOVAC spectrometers).
    double ratio = m_spectrumIntensity / m_darkIntensity;
    if (ratio > 2.0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

// TODO: This needs tests and validation of the input parameters
long AverageIntensity(const std::vector<double>& spectrum, long specCenter, long specCenterHalfWidth)
{

    double sum = 0.0;
    long num;
    // take the average of the 10 pixel surrounding the spec center
    if (specCenter <= specCenterHalfWidth)
        specCenter = specCenterHalfWidth;
    if (specCenter >= (long)spectrum.size() - specCenterHalfWidth)
        specCenter = (long)spectrum.size() - 2 * specCenterHalfWidth;

    for (int j = specCenter - specCenterHalfWidth; j < specCenter + specCenterHalfWidth; j++)
    {
        sum += spectrum[j];
    }

    num = 2 * specCenterHalfWidth;
    sum = fabs(sum / (double)num);

    return (long)sum;
}

// TODO: Input data validationn and test
std::pair<long, long> GetIntensityMeasurementRegion(long specCenter, long specCenterHalfWidth, long spectrumSizes)
{
    if (specCenter <= specCenterHalfWidth)
    {
        specCenter = specCenterHalfWidth;
    }

    if (specCenter >= spectrumSizes - specCenterHalfWidth)
    {
        specCenter = spectrumSizes - 2 * specCenterHalfWidth;
    }

    return std::make_pair(specCenter - specCenterHalfWidth, specCenter + specCenterHalfWidth);
}

// TODO: This needs tests and validation of the input parameters
double GetOffset(double spectrum[MAX_SPECTRUM_LENGTH])
{

    double offset = 0.0;
    for (int i = 6; i < 18; ++i)
    {
        offset += spectrum[i];
    }
    offset /= 12;

    return offset;
}

double GetOffset(const std::vector<double>& spectrum)
{
    double offset = 0.0;
    for (int i = 6; i < 18; ++i)
    {
        offset += spectrum[i];
    }
    offset /= 12;

    return offset;
}

// TODO: This needs tests and validation of the input parameters
int CountRound(long timeResolution, long integrationTimeMs, long readoutDelay, long maximumAveragesInSpectrometer, SpectrumSummation& result)
{

    const long gpsDelay = 10;

    int sumOne = 0;
    int nRound = 0;

    if (maximumAveragesInSpectrometer > 100)
    {
        // Some devices, such as the USB2000+ or the AVANTES devices can sum as many spectra as we want in the
        // spectrometer, we therefore don't need to sum anything in the computer.
        nRound = 1;
        sumOne = std::max(1, (int)(timeResolution / (1.1 * integrationTimeMs)));
    }
    else
    {
        long results[15];
        long rounds[15];
        double nSpec[15];

        // Test the different possibilities for splitting the co-adds beteween the computer and the spectrometer
        //  and return the variant which gives the maximum number of readouts.
        for (sumOne = 1; sumOne <= 15; sumOne++)
        {
            nRound = (timeResolution - gpsDelay) / (sumOne * integrationTimeMs + readoutDelay);
            rounds[sumOne - 1] = nRound;
            long totalTime = (sumOne * integrationTimeMs + readoutDelay) * nRound + gpsDelay;
            results[sumOne - 1] = totalTime;
            nSpec[sumOne - 1] = (double)(sumOne * nRound);
        }
        double maxSpecPerTime = nSpec[0] / (double)results[0];
        int index = 0;

        for (int i = 1; i < 15; ++i)
        {
            if (rounds[i - 1] > 0)
            {
                if (results[i] <= 1.1 * timeResolution && nSpec[i] / (double)results[i] >= maxSpecPerTime)
                {
                    maxSpecPerTime = nSpec[i] / (double)results[i];
                    index = i;
                }
            }
        }

        sumOne = index + 1;
        nRound = (timeResolution - gpsDelay) / (sumOne * integrationTimeMs + readoutDelay);
        if (nRound <= 0)
        {
            sumOne = 1;
            nRound = 1;
        }
    }

    result.SumInComputer = nRound;
    result.SumInSpectrometer = sumOne;

    return nRound;
}

short AdjustIntegrationTimeToLastIntensity(
    SpectrumIntensityMeasurement spectrumIntensityMeasurement,
    double minTolerableRatio,
    double maxTolerableRatio,
    long minAllowedExposureTime,
    long maxAllowedExposureTime)
{
    const double ratioTolerance = 0.3;

    // Guard against invalid input parameters
    minTolerableRatio = std::max(0.0, std::min(1.0, minTolerableRatio));
    maxTolerableRatio = std::max(0.0, std::min(1.0, maxTolerableRatio));
    maxAllowedExposureTime = std::min(maxAllowedExposureTime, 65535L); // the result is clamped to 65535 (from the type short)

    if (spectrumIntensityMeasurement.saturationRatio >= minTolerableRatio && spectrumIntensityMeasurement.saturationRatio <= maxTolerableRatio)
    {
        // nothing needs to be done...
        return spectrumIntensityMeasurement.integrationTime;
    }

    // Adjust the integration time
    long desiredIntegrationTime = (spectrumIntensityMeasurement.saturationRatio < minTolerableRatio) ?
        (long)round(spectrumIntensityMeasurement.integrationTime * (1.0 + ratioTolerance)) :
        (long)round(spectrumIntensityMeasurement.integrationTime / (1.0 + ratioTolerance));

    long newRecommendation = std::max(minAllowedExposureTime, std::min(maxAllowedExposureTime, desiredIntegrationTime));
    return static_cast<short>(newRecommendation);
}

short EstimateNewIntegrationTime(
    SpectrumIntensityMeasurement shortMeasurement,
    SpectrumIntensityMeasurement longMeasurement,
    double desiredSaturationRatio,
    long minAllowedExposureTime,
    long maxAllowedExposureTime)
{
    // Notice that the following forumula was used in earlier versions of MobileDoas, this does not seem to be correct...
    // m_integrationTime = (short)((m_spectrometerDynRange - int_short) * (maxExpTime - minExpTime) * m_percent / (int_long - int_short));

    const double increaseInSaturationRatioPerMsIntegrationTime = (longMeasurement.saturationRatio - shortMeasurement.saturationRatio) / (double)(longMeasurement.integrationTime - shortMeasurement.integrationTime);
    const double saturationRatioRemainingAboveLongExposure = desiredSaturationRatio - longMeasurement.saturationRatio;

    double recommendedChangeToLongExpTime = saturationRatioRemainingAboveLongExposure / increaseInSaturationRatioPerMsIntegrationTime;

    long newRecommendation = (long)(recommendedChangeToLongExpTime + longMeasurement.integrationTime);

    // Limit to the allowed range
    newRecommendation = std::max(minAllowedExposureTime, std::min(newRecommendation, maxAllowedExposureTime));

    return static_cast<short>(newRecommendation);
}

}