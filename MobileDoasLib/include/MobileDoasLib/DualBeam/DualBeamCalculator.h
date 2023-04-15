#pragma once

#include <MobileDoasLib/DualBeam/DualBeamMeasSettings.h>
#include <MobileDoasLib/Definitions.h>

namespace mobiledoas {

    /** The <b>CDualBeamCalculator</b> class contains the basic
            algorithms for calculating wind speeds from measured data series
            of column variations, or calculating plume heights from data series measured
            while performing a traverse using the correlation between the two
            data series */
    class CDualBeamCalculator
    {
    public:
        class CMeasurementSeries {
        public:
            CMeasurementSeries();   // <-- Creates an empty measurement series
            CMeasurementSeries(int len);    // <-- Creates a measurement series of length 'len'
            ~CMeasurementSeries();
            bool SetLength(int len);    // <-- changes the length of the measurment series to 'len'
            double	AverageColumn(int from, int to) const; // <-- calculated the average column value between 'from' and 'to'
            double	SampleInterval();   // <-- calculates and returns the average time between two measurements
            double* column;
            double* time;
            double* lat;
            double* lon;
            long    length;
        };

        CDualBeamCalculator(void);
        ~CDualBeamCalculator(void);

        /** Performs a low pass filtering on the supplied measurement series.
                The number of iterations in the filtering is given by 'nIterations'
                if nIterations is zero, nothing will be done. */
        static bool LowPassFilter(const CMeasurementSeries* series, CMeasurementSeries* result, unsigned int nIterations);


        /** Calculates the correlation between the two vectors 'x' and 'y', both of length 'length'
                @return - the correlation between the two vectors. */
        static double	correlation(const double* x, const double* y, long length);

        /** Shifts the vector 'shortVector' against the vector 'longVector' and returns the
                    shift for which the correlation between the two is highest.
                    The length of the longVector must be larger than the length of the short vector! */
        static bool FindBestCorrelation(
            const double* longVector, unsigned long longLength,
            const double* shortVector, unsigned long shortLength,
            unsigned int maximumShift,
            double& highestCorr, int& bestShift);

    };
}