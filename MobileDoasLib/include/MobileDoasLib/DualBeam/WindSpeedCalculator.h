#pragma once

#include <MobileDoasLib/DualBeam/DualBeamCalculator.h>
#include <MobileDoasLib/DualBeam/DualBeamMeasSettings.h>

namespace mobiledoas {

    /** The <b>CWindSpeedCalculator</b> class contains the basic
            algorithms for calculating wind speeds from measured data series
            of column variations using the correlation between the two
            data series. The idea being that the two time series have been
            measured in such a way that one of them measured on a more up-wind
            position along the plume than the other one. Assuming a certain
            altitude of the plume, the speed of the plume can be calculated
            by calculating the temporal delay between the two time series. */
    class CWindSpeedCalculator : public CDualBeamCalculator
    {
    public:
        // ---------- Error Codes ------------------
        static const int ERROR_NO_ERROR = 0;
        static const int ERROR_EMPTY_SERIES = 1;	// <-- at least one of the given time series is empty
        static const int ERROR_LOWPASSFILTER = 2;  // <-- an error occured in the low-pass filtering
        static const int ERROR_DIFFERENT_SAMPLEINTERVALS = 3;	// <-- the sample interval of the two time-series are different
        static const int ERROR_TOO_SHORT_DATASERIES = 4;	// <-- the time-series is too short for the current settings

        CWindSpeedCalculator(void);
        ~CWindSpeedCalculator(void);

        /** Calculate the time delay between the two provided time series
            @param delay - The return value of the function. Will be set to the time
                delay between the two data series, in seconds. Will be positive if the
                'upWindColumns' comes temporally before the 'downWindColumns', otherwise negative.
            @param upWindSerie - the measurement for the more upwind time series
            @param downWindSerie - the measurement for the more downwind time series
            @param settings - The settings for how the calculation should be done
        */
        bool CalculateDelay(double& delay,
            const CMeasurementSeries* upWindSerie,
            const CMeasurementSeries* downWindSerie,
            const CDualBeamMeasSettings& settings);

        /** The calculated values. These will be filled in after a call to 'CalculateDelay'
                Before that they are null and cannot be used. The length of these arrays are 'm_length' */
        double* shift, * corr, * used, * delays;
        int m_length;

        /** The error code for the last error that has occured here */
        int		m_lastError;

        /** Intializes the arrays 'shift', 'corr', 'used' and 'delays' before they are used*/
        void InitializeArrays();

    };
}