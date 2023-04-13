#pragma once

#include <MobileDoasLib/DateTime.h>
#include <vector>
#include <memory>

namespace mobiledoas
{
    class Wind {
    public:
        // defining the point
        double  lat;
        double  lon;
        // defining the windfield
        double  ws;
        double  wd;
    };

    class CWindField
    {
    public:
        CWindField();
        CWindField(CWindField& wf);
        ~CWindField();

        /** The maximum number of layers that we can store */
        static const int  MAX_LAYERS = 64;

        /** The maximum number of hours that we can store */
        static const int  MAX_HOURS = 25;

        /** Defining which modes of interpolation that we have access to */
        static const int  INTERPOLATION_NEAREST = 0;
        //    static const int  INTERPOLATION_LINEAR = 0;
        //    static const int  INTERPOLATION_SPLINE = 0;

        /** Interpolates the wind field to the supplied positions and times.
            @param lat - the latitudes for the points to interpolate to.
            @param lon - the longitudes for the points to interpolate to.
            @param times - the timepoints to interpolate to.
            @param nPoints - the number of data points in the arrays = the number of interpolations to do.
            @param windSpeed - will be filled with interpolated windspeeds when the function returns.
            @param windDirection - will be filled with interpolated wind directions when the function returns. */
        int Interpolate(const double* lat, const double* lon, const mobiledoas::Time* times, int layer, int nPoints, double* windSpeed, double* windDirection, int mode = INTERPOLATION_NEAREST);

        /** gives the wind field at the given altitude layer for the given hour
            @param maxPoints - the maximum number of datapoints to fill in.
            @return the number of data points filled in, always less than or equal to 'maxPoints'. */
        int GetWindField(int layer, int hour, int maxPoints, double* lat, double* lon, double* ws, double* wd);

        /** Reads a wind field from file.
            @return true if the reading was successful. */
        bool ReadWindField(const char* fileName);

        /** Returns the number of layers */
        int GetLayerNum() const;

        /** Returns the altitude for the supplied layer  */
        double GetLayerAltitude(int layer);

        /** Returns an array with the hours for which we have a wind field
            at the supplied altitude layer.
            @return the number of elements filled into the array. */
        int GetHours(int layer, char hours[25]);

        /** Turns on/off the use of time-shifting */
        void UseTimeShift(bool on);

        /** Sets the timeshift to use (will only be used if m_useTimeshift is true) */
        void SetTimeShift(int timeShift);

    private:

        // ----------------- THE ACTUAL DATA ----------------------
        // Wind information is sorted into layers according to altitude.
        // Data from each altitude is stored in a separate list

        /** A set of lists, one list for every layer in altitude.
            Each list contains the datapoints for that layer. */
        std::vector<Wind> m_wind[MAX_LAYERS][MAX_HOURS];

        /** How many altitude layers that are defined */
        long m_nAltitudes;

        /** The defined altitude layers. m_altitude[i] defines the altitude
            for which the data in m_wind[i] are valid. */
        double m_altitude[MAX_LAYERS];

        /** Time shifting */
        bool m_useTimeShift;
        int m_timeShift;

        // ------------- PRIVATE METHODS ----------------

        /** Reset the data content */
        void Reset();

        /** Returns the altitude index which corresponds to a given altitude.
            @return -1 if this altitude cannot be saved. */
        int AltitudeIndex(double altitude);

        /** This function sorts the supplied list of winds. */
        static int SortList(std::vector<Wind>& m_wind);

        /** Returns the nearest neigbour to the supplied point from the list o points.
            @return - a pointer to the nearest point.
            @return - NULL if anything goes wrong.*/
        static std::unique_ptr<Wind> NearestNeighbour(const std::vector<Wind>& wind, double point_Lat, double point_Lon, bool sorted);
    };
}