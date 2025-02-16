#include <MobileDoasLib/DualBeam/DualBeamCalculator.h>

#include <memory>

namespace mobiledoas
{

    // ------------------------------------------------
    // ------------- MEASUREMENT SERIES ---------------
    // ------------------------------------------------

    CDualBeamCalculator::CMeasurementSeries::CMeasurementSeries() {
        column = nullptr;
        time = nullptr;
        lat = nullptr;
        lon = nullptr;
        length = 0;
    }

    CDualBeamCalculator::CMeasurementSeries::CMeasurementSeries(int len) {
        column = new double[len];
        time = new double[len];
        lat = new double[len];
        lon = new double[len];
        length = len;
    }

    bool CDualBeamCalculator::CMeasurementSeries::SetLength(int len) {
        delete[] column;
        delete[] time;
        delete[] lat;
        delete[] lon;

        column = new double[len];
        if (column == nullptr)
            return FAIL;

        time = new double[len];
        if (time == nullptr)
            return FAIL;

        lat = new double[len];
        if (lat == nullptr)
            return FAIL;

        lon = new double[len];
        if (lon == nullptr)
            return FAIL;

        length = len;

        return SUCCESS;
    }

    double CDualBeamCalculator::CMeasurementSeries::AverageColumn(int from, int to) const {
        double sum = 0.0;

        // check input
        if (from > to || from < 0 || to < 0 || from >= length || to >= length)
            return 0.0;

        for (int k = from; k <= to; ++k) {
            sum += column[k];
        }
        return sum / (to - from + 1);
    }

    // calculates and returns the average time between two measurements
    double CDualBeamCalculator::CMeasurementSeries::SampleInterval() {
        if (length <= 0)
            return 0.0;

        double totalTime = time[length - 1] - time[0];
        return totalTime / (length - 1);
    }

    CDualBeamCalculator::CMeasurementSeries::~CMeasurementSeries() {
        if (length != 0) {
            delete[] column;
            delete[] time;
        }
    }

    // ------------------------------------------------
    // ----------- DUAL BEAM CALCULATOR ---------------
    // ------------------------------------------------

    CDualBeamCalculator::CDualBeamCalculator(void)
    {
    }

    CDualBeamCalculator::~CDualBeamCalculator(void)
    {
    }

    /** Performs a low pass filtering on the supplied measurement series.
        The number of iterations in the filtering is given by 'nIterations'
        if nIterations is zero, nothing will be done. */
    bool CDualBeamCalculator::LowPassFilter(const CMeasurementSeries* series, CMeasurementSeries* result, unsigned int nIterations) {

        // 1. Check for errors in input
        if (series == nullptr || series->length == 0 || result == nullptr)
            return FAIL;

        // 2. Calculate the old and the new data series lengths
        int length = series->length;							// <-- the length of the old data series
        int newLength = length - nIterations - 1;		// <-- the length of the new data series

        if (newLength <= 0)
            return FAIL;

        // 3. If no iterations should be done, the output should be a copy of the input...
        if (nIterations == 0) {
            if (SUCCESS != result->SetLength(length))
                return FAIL;

            for (int i = 0; i < length; ++i) {
                result->column[i] = series->column[i];
                result->time[i] = series->time[i];
            }
            result->length = series->length;
            return SUCCESS;
        }

        // 4. Change the length of the resulting series.
        if (SUCCESS != result->SetLength(newLength))
            return FAIL;

        // 5. Calculate the factorials
        double* factorial = new double[nIterations];
        factorial[0] = 1;
        if (nIterations > 1)
            factorial[1] = 1;
        for (unsigned int k = 2; k < nIterations; ++k)
            factorial[k] = factorial[k - 1] * (double)k;

        double coeffSum = 0; // <-- the sum of all coefficients, to make sure that we dont increase the values...

        // 6. Allocate temporary arrays for the time-stamps and the column values
        double* tmpCol = new double[newLength];
        double* tmpTime = new double[newLength];
        memset(tmpCol, 0, newLength * sizeof(double));
        memset(tmpTime, 0, newLength * sizeof(double));

        // 7. Do the filtering...
        for (unsigned int k = 1; k < nIterations + 1; ++k) {
            // 7a. The coefficient in the binomial - expansion
            double coefficient = factorial[nIterations - 1] / (factorial[nIterations - k] * factorial[k - 1]);
            coeffSum += coefficient;

            // 7b. Do the filtering for all data points in the new time series
            for (int i = 0; i < newLength; ++i) {
                tmpCol[i] += coefficient * series->column[k - 1 + i];
                tmpTime[i] += coefficient * series->time[k - 1 + i];
            }
        }

        // 8. Divide by the coeffsum to preserve the energy...
        for (int i = 0; i < newLength; ++i) {
            result->time[i] = tmpTime[i] / coeffSum;
            result->column[i] = tmpCol[i] / coeffSum;
        }
        result->length = newLength;

        delete[] factorial;
        delete[] tmpCol;
        delete[] tmpTime;

        return SUCCESS;
    }

    /** Calculates the correlation between the two vectors 'x' and 'y', both of length 'length'
            @return - the correlation between the two vectors. */
    double CDualBeamCalculator::correlation(const double* x, const double* y, long length) {
        double s_xy = 0; // <-- the dot-product X*Y
        double s_x2 = 0; // <-- the dot-product X*X
        double s_x = 0; // <-- sum of all elements in X
        double s_y = 0; // <-- sum of all elements in Y
        double s_y2 = 0; // <-- the dot-product Y*Y
        double c = 0; // <-- the final correlation
        double eps = 1e-5;

        if (length <= 0)
            return 0;

        for (int k = 0; k < length; ++k) {
            s_xy += x[k] * y[k];
            s_x2 += x[k] * x[k];
            s_x += x[k];
            s_y += y[k];
            s_y2 += y[k] * y[k];
        }

        double nom = (length * s_xy - s_x * s_y);
        double denom = sqrt(((length * s_x2 - s_x * s_x) * (length * s_y2 - s_y * s_y)));

        if ((fabs(nom - denom) < eps) && (fabs(denom) < eps))
            c = 1.0;
        else
            c = nom / denom;

        return c;
    }

    /** Shifts the vector 'shortVector' against the vector 'longVector' and returns the
                shift for which the correlation between the two is highest. */
    bool CDualBeamCalculator::FindBestCorrelation(
        const double* longVector, unsigned long longLength,
        const double* shortVector, unsigned long shortLength,
        unsigned int maximumShift,
        double& highestCorr, int& bestShift) {

        // 0. Check for errors in the input
        if (longLength == 0 || shortLength == 0)
            return FAIL;
        if (longVector == nullptr || shortVector == nullptr)
            return FAIL;
        if (longLength <= shortLength)
            return FAIL;

        // Reset
        highestCorr = 0;
        bestShift = 0;

        // To calculate the correlation, we need to pick out a subvector (with length 'shortLength)
        //	from the longVector and compare this with 'shortVector' and calculate the correlation.
        // left is the startingpoint of this subvector
        int left = 0;

        // 1. Start shifting
        while ((left + shortLength) < (int)longLength && left < (int)maximumShift) {
            double C = correlation(shortVector, longVector + left, shortLength);
            if (C > highestCorr) {
                highestCorr = C;
                bestShift = left;
            }
            ++left;
        }

        return SUCCESS;
    }

}
