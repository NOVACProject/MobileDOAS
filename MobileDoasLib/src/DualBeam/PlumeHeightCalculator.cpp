#include <MobileDoasLib/DualBeam/PlumeHeightCalculator.h>
#include <MobileDoasLib/GpsData.h>
#include <vector>

namespace mobiledoas {

    CPlumeHeightCalculator::CPlumeHeightCalculator(void)
    {
    }

    CPlumeHeightCalculator::~CPlumeHeightCalculator(void)
    {
    }

    /** Calculates the plume-height from the difference in centre-of mass
            position. */
    double CPlumeHeightCalculator::GetPlumeHeight_CentreOfMass(const CMeasurementSeries* forwardLookingSerie, const CMeasurementSeries* backwardLookingSerie, double sourceLat, double sourceLon, double angleSeparation, double* windDir) {
        double windDirection = 1e19;
        double oldWindDirection = 0;
        double plumeDirection = 0.0;
        int nIterations = 0;
        int k; // iterator
        int massIndexB = 0;
        int massIndexF = 0;

        // 1. Make local copies of the data
        long length = forwardLookingSerie->length;
        std::vector<double> lat(length);
        std::vector<double> lon(length);
        std::vector<double> col1(length);
        std::vector<double> col2(length);

        memcpy(lat.data(), forwardLookingSerie->lat, length * sizeof(double));
        memcpy(lon.data(), forwardLookingSerie->lon, length * sizeof(double));
        memcpy(col1.data(), forwardLookingSerie->column, length * sizeof(double));
        memcpy(col2.data(), backwardLookingSerie->column, length * sizeof(double));

        // 2. Remove the offset from the traverses
        RemoveOffset(col1.data(), length);
        RemoveOffset(col2.data(), length);

        // 3. Calculate the wind-direction from the measured data 

        // calculate the distances between each pair of measurement points
        std::vector<double> distances(length);
        for (k = 0; k < length - 1; ++k) {
            distances[k] = mobiledoas::GPSDistance(lat[k], lon[k], lat[k + 1], lon[k + 1]);
        }

        // Iterate until we find a suitable wind-direction
        while (fabs(windDirection - oldWindDirection) > 5) {

            // get the mass center of the plume
            massIndexF = GetCentreOfMass(col1.data(), distances.data(), length);

            oldWindDirection = windDirection;

            // the wind direction
            windDirection = mobiledoas::GPSBearing(lat[massIndexF], lon[massIndexF], sourceLat, sourceLon);
            plumeDirection = 180 + windDirection;

            // re-calculate the distances
            for (k = 0; k < length - 1; ++k) {
                double bearing = mobiledoas::GPSBearing(lat[k], lon[k], lat[k + 1], lon[k + 1]);
                distances[k] = mobiledoas::GPSDistance(lat[k], lon[k], lat[k + 1], lon[k + 1]);
                distances[k] *= cos(HALF_PI + DEGREETORAD * (bearing - plumeDirection));
            }

            // Keep track of how may iterations we've done
            ++nIterations;
            if (nIterations > 10)
                break;
        }

        // 4. Calculate the distance between the two centre of mass positions
        //		Geometrically corrected for the wind-direction
        massIndexB = GetCentreOfMass(col2.data(), distances.data(), backwardLookingSerie->length);

        // 4a. The distances
        double centreDistance = mobiledoas::GPSDistance(lat[massIndexF], lon[massIndexF], lat[massIndexB], lon[massIndexB]);

        // 4b. The direction
        double centreDirection = mobiledoas::GPSBearing(lat[massIndexF], lon[massIndexF], lat[massIndexB], lon[massIndexB]);

        // 4c. The Geometrically corrected distance
        double centreDistance_corrected = fabs(centreDistance * cos(HALF_PI + DEGREETORAD * (centreDirection - plumeDirection)));

        // 4d. The Plume Height
        double plumeHeight = centreDistance_corrected / tan(DEGREETORAD * angleSeparation);

        if (windDir != nullptr)
            *windDir = windDirection;

        return plumeHeight;
    }

    /** Calculates the centre of mass distance.
            @param columns - the measured columns in the traverse
            @param distances - the distance between each two measurement points
            @return the index of the centre of mass point in the given arrays*/
    int CPlumeHeightCalculator::GetCentreOfMass(const double* columns, const double* distances, long length) {
        if (length <= 0 || columns == nullptr || distances == nullptr)
            return -1;

        // 1. Calculate the total mass of the plume
        double totalMass = 0.0;
        for (int k = 0; k < length - 1; ++k) {
            totalMass += columns[k] * distances[k];
            if (fabs(columns[k]) > 300 || fabs(distances[k]) > 100) {
                puts("StrangeData");
            }
        }

        // 2. Half of the total mass
        double midMass = std::abs(totalMass * 0.5);

        // 3. Find the index of the half centre of mass
        double totalMassSoFar = 0.0;
        for (int k = 0; k < length - 1; ++k) {
            totalMassSoFar += columns[k] * distances[k];
            if (std::abs(totalMassSoFar) > midMass) {
                return (k - 1);
            }
        }

        // 4. Error: could not find centre of mass
        return -1;
    }

    /** Removes the offset from the given set of columns */
    void CPlumeHeightCalculator::RemoveOffset(double* columns, long length) {
        static const int BUFFER = 30;
        double x[2 * BUFFER], y[2 * BUFFER];
        int index = 0;
        // Copy the first 30 and the last 30 datapoints to a buffer
        for (int i = 0; i < BUFFER; ++i) {
            x[index] = i;
            y[index] = columns[i];
            ++index;
        }
        for (int i = length - BUFFER; i < length; ++i) {
            x[index] = i;
            y[index] = columns[i];
            ++index;
        }

        // Adapt a 1:st order polynomial to this dataset
        double k, m;
        AdaptStraightLine(x, y, 2 * BUFFER, &k, &m);

        // Remove this line from the dataset
        for (int i = 0; i < length; ++i) {
            columns[i] -= (k * i + m);
        }
    }


    /*  adapts parameters k and m so that y = k*x + m, in a
        least square sense.
        Algorithm found at: http://mathworld.wolfram.com/LeastSquaresFittingPolynomial.html */
    int CPlumeHeightCalculator::AdaptStraightLine(double* x, double* y, unsigned int l, double* k, double* m) {
        double sx = 0, sy = 0, sx2 = 0, sxy = 0, det_inv;
        double M_inv[2][2], XTy[2]; /*M=X^T * X, M_inv = M^-1, XTy = X^T * y */
        unsigned int i;

        if ((x == 0) || (y == 0) || (l == 0) || (k == 0) || (m == 0)) {
            return 1;
        }

        for (i = 0; i < l; ++i) {
            sx += x[i];
            sy += y[i];
            sx2 += x[i] * x[i];
            sxy += x[i] * y[i];
        }

        det_inv = 1 / (sx2 * l - sx * sx);
        M_inv[0][0] = sx2 * det_inv;
        M_inv[0][1] = -sx * det_inv;
        M_inv[1][0] = -sx * det_inv;
        M_inv[1][1] = l * det_inv;

        XTy[0] = sy;
        XTy[1] = sxy;

        *(m) = M_inv[0][0] * XTy[0] + M_inv[0][1] * XTy[1];
        *(k) = M_inv[1][0] * XTy[0] + M_inv[1][1] * XTy[1];

        return 0;
    }

}
