#pragma once

#include <MobileDoasLib/DateTime.h>
#include "WindField.h"

namespace mobiledoas
{

    typedef struct gpsPosition {
        double latitude = 0.0;
        double longitude = 0.0;
        double altitude = 0.0;
    }gpsPosition;

#define MAX_TRAVERSELENGTH 16384

    class CTraverse
    {
    public:
        CTraverse(void);
        ~CTraverse(void);

        // -------------------------------------------------------------
        // ----------------------- PUBLIC DATA -------------------------
        // -------------------------------------------------------------

        /** The number of points in the log file */
        long      m_recordNum;

        /** The number of channels that was used in this traverse */
        long      m_channelNum;

        /** The dynamic range of the spectrometer used in this traverse */
        long		m_dynRange;

        /** The gasfactor for this traverse */
        double    m_gasFactor;

        /** The offset for the traverse*/
        double    m_Offset;

        /** The max column value for the traverse */
        double		m_maxColumn;

        /** The flux calculation will not always be done from the first spectrum or to
            the last spectrum (e.g. if there's no GPS data in the beginning or end of the traverse).
            This is the spectrum-index where the fluxcalculation will start. */
        long      m_lowIndex;

        /** The flux calculation will not always be done from the first spectrum or to
            the last spectrum (e.g. if there's no GPS data in the beginning or end of the traverse).
            This is the spectrum-index where the fluxcalculation will stop. */
        long      m_highIndex;

        /** The filename of the logfile */
        std::string m_fileName;

        /** The path of the logfile */
        std::string m_filePath;

        // ??the specie names in the most recently read log file - temporary variable??
        std::string m_specieName;

        // -------------- TRAVERSE STATISTICS --------------------

        /* information about the traverse - calculated at the same time as the flux.
            The plume width is the geometrically corrected distance for all measurements
                where the column value is higher than half of the plume's maximum value. For
                a simple plume this will be the FWHM of the plume. */
        double    m_plumeWidth;
        double    m_traverseLength;
        double    m_correctedTraverseLength;
        long      m_spectraInPlume;

        // ---------------- THE MEASURED DATA -----------------------

        /** latitudes */
        double    latitude[MAX_TRAVERSELENGTH];

        /** longitudes */
        double    longitude[MAX_TRAVERSELENGTH];

        /** altitudes */
        double    altitude[MAX_TRAVERSELENGTH];

        /** measured columns */
        std::vector<double> columnArray;

        /** Estimated column errors */
        std::vector<double> columnError;

        /** measured intensities */
        double    intensArray[MAX_TRAVERSELENGTH];

        /** the time the spectrum was collected */
        mobiledoas::Time      time[MAX_TRAVERSELENGTH];

        /** A wind field, interpolated to the measurement positions of this traverse */
        double    m_windDirection[MAX_TRAVERSELENGTH];
        double    m_windSpeed[MAX_TRAVERSELENGTH];

        /** m_useWindfield is true if the wind direction and wind speed int 'm_windDirection'
            and 'm_windSpeed' have been defined. */
        bool      m_useWindField;

        /** A temporary buffer */
        double    tmpColumn[MAX_TRAVERSELENGTH];

        /** True if there was a gps connection during the traverse */
        bool      m_hasGPS;

        // --------------- SPECTRUM PROPERTIES ---------------------
        /** The number of averaged spectra */
        long      m_nSpectra;

        /** The exposure time */
        long      m_expTime;

        /** The (pixel) region used for the fitting of the spectra */
        long      m_fitRegion[2];

        /** The reference file used to evaluate the spectra */
        std::string m_refFile;


        // -----------------------------  Output ----------------------------
        bool* m_fCreateAdditionalLog;
        std::string m_additionalLogName;

        // -------------------------------------------------------------
        // --------------------- PUBLIC METHODS ------------------------
        // -------------------------------------------------------------

        // ------------------- MANAGING THE DATA ---------------------
        long  DeleteLowIntensityPoints(double intensityLimit);
        long  DeleteHighIntensityPoints(double intensityLimit);
        long  DeletePoints(long lowIndex, long highIndex);

        // ------------------------ CALCULATING FLUX --------------------
        /** Calculates the flux using the wind field defined by the vectors 'm_windspeed'
            and 'm_windDirection'. */
        double  GetTotalFlux();

        /** Calculates the flux using the supplied wind speed and direction
            for all measured points */
        double  GetTotalFlux(double windSpeed, double windDirection);

        /**Get all information about the plume center
            maxBuffer[0] - column in the center - where max column locates
            maxBuffer[1] - latitude  - where max column locates
            maxBuffer[2] - longitude  - where max column locates
            maxBuffer[3] - wind angle  - where max column locates
            avBuffer[0] - column in the center - where average column locates
            avBuffer[1] - latitude  - where average column locates
            avBuffer[2] - longitude  - where average column locates
            avBuffer[3] - wind angle  - where average column locates
        */
        int		GetPlumeCenter(double srclat, double srclon, double* maxBuffer, double* avBuffer);

        /** Calculates the offset of the traverse. The output will be saved in the
                parameter 'm_Offset' and returned.	*/
        double	CalculateOffset();

        /** @return false if the given measurement point is a good point.
                @return true if the given point is bad or the index is out of bounds */
        bool	IsBadPoint(int spectrumIndex);

        // --------------- OPERATORS ---------------------
        CTraverse& operator=(const CTraverse& t);


    private:
        // -------------------------------------------------------------
        // ---------------------- PRIVATE DATA -------------------------
        // -------------------------------------------------------------

        // -------------------------------------------------------------
        // -------------------- PRIVATE METHODS ------------------------
        // -------------------------------------------------------------

        /** Calulates the partial flux between two measurement positions.
            @param column - the accumulated column between the two points.
            @param pos1 - the position for the first point
            @param pos2 - the position for the second point.
            @param windSpeed - the wind speed to use
            @param windDirection - the wind direction to use in the calculation. */
        double  CalculateFlux(double column, const gpsPosition& pos1, const gpsPosition& pos2, double windSpeed, double windDirection);

        /** Calculates the offset of the given set of column data.
            This requires that all the data points are 'good'
            @return the offset value. This will be 0.0 if nothing
                could be calculated */
        double CalculateOffset(std::vector<double>& columnData, long nDataPoints) const;

    };
}