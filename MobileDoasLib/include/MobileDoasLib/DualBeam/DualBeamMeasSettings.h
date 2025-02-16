#pragma once

namespace mobiledoas {

    /** The <b>CDualBeamMeasSettings</b> contains the parameters
            necessary for calculating the correlation between data series
            in order to calculate wind speeds or plume heights. 	*/
    class CDualBeamMeasSettings
    {
    public:
        /** The number of pixels to average in the low pass filtering.
                If lowPassFilterAverage is 0, then no filtering will be done */
        unsigned int lowPassFilterAverage = 20;

        /** The maximum number of seconds to shift */
        unsigned int shiftMax = 90;

        /** The length of the test-region, in seconds */
        unsigned int testLength = 300;

        /** The minimum column value that will be taken into account */
        double columnMin = -300.0;

        /** The minimum sigma - level (???) */
        double sigmaMin = 0.1;

        /** The plume height */
        double plumeHeight = 1000.0;

        /** The angle separation in the instrument. In degrees */
        double angleSeparation = 6.9;
    };
}
