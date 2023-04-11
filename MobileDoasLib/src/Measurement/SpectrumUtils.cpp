#include <MobileDoasLib/Measurement/SpectrumUtils.h>
#include <vector>
#include <numeric>


bool mobiledoas::CheckIfDark(double spectrum[MAX_SPECTRUM_LENGTH], int detectorSize) {
    // consider pixels 50 to 70. Remove the highest 3 in case they are 'hot'. Then calculate the average:
    std::vector<double> vec;
    double m_darkIntensity;
    double m_spectrumIntensity;

    for (int i = 50; i < 70; i++) {
        vec.push_back(spectrum[i]);
    }
    for (int i = 0; i < 3; i++) {
        vec.erase(max_element(vec.begin(), vec.end()));
    }
    m_darkIntensity = std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();

    // now calculate the average of the middle 20 pixels, again excluding the 3 highest intensities. 
    vec.clear();
    int start = int(floor(detectorSize / 2) - 10);
    int end = int(floor(detectorSize / 2) + 10);
    for (int i = start; i < end; i++) {
        vec.push_back(spectrum[i]);
    }
    for (int i = 0; i < 3; i++) {
        vec.erase(max_element(vec.begin(), vec.end()));
    }
    m_spectrumIntensity = std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();

    // the spectrum is considered dark if the center intensity is less than twice as high as the dark intensity.
    // this should be applicable to any spectrometer, as long as pixels 50 to 70 are dark (which is true for NOVAC spectrometers).
    double ratio = m_spectrumIntensity / m_darkIntensity;
    if (ratio > 2.0) {
        return false;
    }
    else {
        return true;
    }
}

// TODO: This needs tests and validation of the input parameters
long mobiledoas::AverageIntensity(double* pSpectrum, long specCenter, long specCenterHalfWidth) {

    double sum = 0.0;
    long num;
    // take the average of the 10 pixel surrounding the spec center
    if (specCenter<= specCenterHalfWidth)
        specCenter= specCenterHalfWidth;
    if (specCenter>= MAX_SPECTRUM_LENGTH - specCenterHalfWidth)
        specCenter= MAX_SPECTRUM_LENGTH - 2 * specCenterHalfWidth;

    for (int j = specCenter- specCenterHalfWidth; j < specCenter+ specCenterHalfWidth; j++) {
        sum += pSpectrum[j];
    }

    num = 2 * specCenterHalfWidth;
    sum = fabs(sum / (double)num);

    return (long)sum;
}
