#pragma once

namespace DualBeamMeasurement{

	/** The <b>CDualBeamMeasSettings</b> contains the parameters
			necessary for calculating the correlation between data series
			in order to calculate wind speeds or plume heights. 	*/
	class CDualBeamMeasSettings
	{
	public:
		CDualBeamMeasSettings(void);
		~CDualBeamMeasSettings(void);

		/** The number of pixels to average in the low pass filtering.
				If lowPassFilterAverage is 0, then no filtering will be done */
		unsigned int		lowPassFilterAverage;

		/** The maximum number of seconds to shift */
		unsigned int		shiftMax;

		/** The length of the test-region, in seconds */
		unsigned int		testLength;

		/** The minimum column value that will be taken into account */
		double					columnMin;

		/** The minimum sigma - level (???) */
		double					sigmaMin;

		/** The plume height */
		double					plumeHeight;

		/** The angle separation in the instrument. In degrees */
		double					angleSeparation;
	};
}
