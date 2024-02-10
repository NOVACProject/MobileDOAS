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
    if (specCenter >= MAX_SPECTRUM_LENGTH - specCenterHalfWidth)
        specCenter = MAX_SPECTRUM_LENGTH - 2 * specCenterHalfWidth;

    for (int j = specCenter - specCenterHalfWidth; j < specCenter + specCenterHalfWidth; j++)
    {
        sum += spectrum[j];
    }

    num = 2 * specCenterHalfWidth;
    sum = fabs(sum / (double)num);

    return (long)sum;
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

}