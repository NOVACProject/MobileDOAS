#include <MobileDoasLib/DualBeam/WindSpeedCalculator.h>
#include <memory>

namespace mobiledoas
{

    CWindSpeedCalculator::CWindSpeedCalculator(void)
    {
        shift = nullptr;
        corr = nullptr;
        used = nullptr;
        delays = nullptr;
        m_length = 0;
        m_lastError = ERROR_NO_ERROR;
    }

    CWindSpeedCalculator::~CWindSpeedCalculator(void)
    {
        delete[] shift;
        delete[] corr;
        delete[] used;
    }

    bool CWindSpeedCalculator::CalculateDelay(
        double& /*delay*/,
        const CMeasurementSeries* upWindSerie,
        const CMeasurementSeries* downWindSerie,
        const CDualBeamMeasSettings& settings) {

        // Reset the error code form the last call
        m_lastError = ERROR_NO_ERROR;

        CMeasurementSeries modifiedUpWind;
        CMeasurementSeries modifiedDownWind;

        // 0. Error checking of the input
        if (upWindSerie == nullptr || upWindSerie->length == 0) {
            m_lastError = ERROR_EMPTY_SERIES;
            return FAIL;
        }
        if (downWindSerie == nullptr || downWindSerie->length == 0) {
            m_lastError = ERROR_EMPTY_SERIES;
            return FAIL;
        }

        // 1. Start with the low pass filtering
        if (SUCCESS != LowPassFilter(upWindSerie, &modifiedUpWind, settings.lowPassFilterAverage)) {
            m_lastError = ERROR_LOWPASSFILTER;
            return FAIL;
        }
        if (SUCCESS != LowPassFilter(downWindSerie, &modifiedDownWind, settings.lowPassFilterAverage)) {
            m_lastError = ERROR_LOWPASSFILTER;
            return FAIL;
        }

        // 1b. Get the sample time
        double sampleInterval = modifiedDownWind.SampleInterval();
        if (fabs(modifiedUpWind.SampleInterval() - sampleInterval) > 0.5) {
            m_lastError = ERROR_DIFFERENT_SAMPLEINTERVALS;
            return FAIL; // <-- we cannot have different sample intervals of the two time series
        }

        // 1c. Calculate the length of the comparison-interval
        //		in data-points instead of in seconds
        int		comparisonLength = (int)round(settings.testLength / sampleInterval);

        // 1d. Calculate the how many pixels that we should shift maximum
        int		maximumShift = (int)round(settings.shiftMax / sampleInterval);

        // 1e. check that the resulting series is long enough
        if (modifiedDownWind.length - maximumShift - comparisonLength < maximumShift + 1) {
            m_lastError = ERROR_TOO_SHORT_DATASERIES;
            return FAIL; // <-- data series to short to use current settings of test length and shiftmax
        }

        // 2. Allocate some assistance arrays 
        //		(Note that it is the down wind data series that is shifted)
        m_length = modifiedDownWind.length;
        InitializeArrays();

        // The number of datapoints skipped because we cannot see the plume.
        int skipped = 0;

        // 3. Iterate over the set of sub-arrays in the down-wind data series
        //		Offset is the starting-point in this sub-array whos length is 'comparisonLength'
        for (int offset = 0; offset < m_length - (int)maximumShift - comparisonLength; ++offset) {
            double highestCorr;
            int bestShift;

            // 3a. Pick out the sub-vectors
            double* series1 = modifiedUpWind.column + offset;
            double* series2 = modifiedDownWind.column + offset;
            unsigned int series1Length = modifiedUpWind.length - offset;
            unsigned int series2Length = comparisonLength;

            // 3b. The midpoint in the subvector
            int midPoint = (int)round(offset + comparisonLength / 2);

            //	3c. Check if we see the plume...
            if (upWindSerie->AverageColumn(offset, offset + comparisonLength) < settings.columnMin) {
                // we don't see the plume. 
                skipped = skipped + 1;
                continue;
            }

            // 3d. Do a shifting...
            FindBestCorrelation(series1, series1Length, series2, series2Length, maximumShift, highestCorr, bestShift);

            // 3e. Calculate the time-shift
            delays[midPoint] = bestShift * sampleInterval;
            corr[midPoint] = highestCorr;
            shift[midPoint] = bestShift - 1;
            used[midPoint] = 1;
        }

        return SUCCESS;
    }

    void CWindSpeedCalculator::InitializeArrays() {
        delete[]	shift, corr, used, delays;
        shift = new double[m_length];
        corr = new double[m_length];
        used = new double[m_length];
        delays = new double[m_length]; // <-- the delays
        memset(corr, 0, m_length * sizeof(double));
        memset(shift, 0, m_length * sizeof(double));
        memset(used, 0, m_length * sizeof(double));
        memset(delays, 0, m_length * sizeof(double));
    }

}