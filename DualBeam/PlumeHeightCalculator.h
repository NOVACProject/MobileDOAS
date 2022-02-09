#pragma once

#include "../Common.h"
#include "DualBeamCalculator.h"


namespace DualBeamMeasurement {

    /** The <b>CPlumeHeightCalculator</b> class contains the basic
            algorithms for calculating plume heights from data series
            measured using the dual-beam technique while passing under a plume.
            data series. The idea being that the two time series have been
            measured in such a way that one of them saw the plume earlier than
            the other one. */
    class CPlumeHeightCalculator : public CDualBeamCalculator
    {
    public:
        CPlumeHeightCalculator(void);
        ~CPlumeHeightCalculator(void);

        /** Calculates the plume-height from the difference in centre-of mass
                position. */
        double GetPlumeHeight_CentreOfMass(
            const CMeasurementSeries* forwardLookingSerie,
            const CMeasurementSeries* backwardLookingSerie,
            double	sourceLat, double sourceLon, double angleSeparation, double* windDir = nullptr);

    protected:
        /** Calculates the centre of mass distance.
                @param columns - the measured columns in the traverse
                @param distances - the distance between each two measurement points
                @return the index of the centre of mass point in the given arrays*/
        int GetCentreOfMass(const double* columns, const double* distances, long length);

        /** Removes the offset from the given set of columns */
        void RemoveOffset(double* columns, long length);
    };
}