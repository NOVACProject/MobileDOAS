#pragma once

#include "../Common.h"
#include "../Common/CDateTime.h"
#include <afxtempl.h>

namespace Flux
{
  class Wind{
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
    CWindField(void);
    CWindField(CWindField &wf);
    ~CWindField(void);

    /** The maximum number of layers that we can store */
    static const int  MAX_LAYERS = 64;

    /** The maximum number of hours that we can store */
    static const int  MAX_HOURS = 25;

    /** Defining which modes of interpolation that we have access to */
    static const int  INTERPOLATION_NEAREST = 0;
//    static const int  INTERPOLATION_LINEAR = 0;
//    static const int  INTERPOLATION_SPLINE = 0;

    /** gives the wind field at the given point and time of day. @return FAIL or SUCCESS. */
    bool  GetWind(const double lat, const double lon, const Time &time, double &windSpeed, double &windDirection) const;

    /** Gives a copy of the wind at a given layer and given hour */
    bool GetWindData(const int layer, const int hour, CList<Wind *, Wind*> &w);

    /** Interpolates the wind field to the supplied positions and times.
        @param lat - the latitudes for the points to interpolate to.
        @param lon - the longitudes for the points to interpolate to. 
        @param times - the timepoints to interpolate to.
        @param nPoints - the number of data points in the arrays = the number of interpolations to do.
        @param windSpeed - will be filled with interpolated windspeeds when the function returns.
        @param windDirection - will be filled with interpolated wind directions when the function returns. */
    int   Interpolate(const double *lat, const double *lon, const Time *times, int layer, int nPoints, double *windSpeed, double *windDirection, int mode = INTERPOLATION_NEAREST);

    /** gives the wind field at the given altitude layer for the given hour
        @param maxPoints - the maximum number of datapoints to fill in.
        @return the number of data points filled in, always less than or equal to 'maxPoints'. */
    int   GetWindField(int layer, int hour, int maxPoints, double *lat, double *lon, double *ws, double *wd);

    /** Inserts a new data point into the windfield. @return FAIL or SUCCESS */
    bool  Insert(const double lat, const double lon, const int hour, const double windSpeed, const double windDirection);

    /** Reads a wind field from file */
    bool  ReadWindField(const CString &fileName);

    /** Returns the number of layers */
    int   GetLayerNum();

    /** Returns the altitude for the supplied layer  */
    double   GetLayerAltitude(int layer);

    /** Returns an array with the hours for which we have a wind field
        at the supplied altitude layer. 
        @return the number of elements filled into the array. */
    int      GetHours(int layer, char hours[25]);

		/** Turns on/off the use of time-shifting */
		void		UseTimeShift(bool on);

		/** Sets the timeshift to use (will only be used if m_useTimeshift is true) */
		void		SetTimeShift(int timeShift);

  private:

    // ----------------- THE ACTUAL DATA ----------------------
    // Wind information is sorted into layers according to altitude.
    // Data from each altitude is stored in a separate list

    /** A set of lists, one list for every layer in altitude.
        Each list contains the datapoints for that layer. */
    CList<Wind*, Wind*> m_wind[MAX_LAYERS][MAX_HOURS];

    /** How many altitude layers that are defined */
    long    m_nAltitudes;

    /** The defined altitude layers. m_altitude[i] defines the altitude
        for which the data in m_wind[i] are valid. */
    double   m_altitude[MAX_LAYERS];

		/** Time shifting */
		bool	m_useTimeShift;
		int		m_timeShift;

    // ------------- PRIVATE METHODS ----------------

    /** Reset the data content */
    void Reset();

    /** Returns the altitude index which corresponds to a given altitude.
        @return -1 if this altitude cannot be saved. */
    int AltitudeIndex(double altitude);

    /** This function sorts the supplied list of winds. */
    static int SortList(CList<Wind*, Wind*> &m_wind);

    /** Returns the nearest neigbour to the supplied point from the list o points.
        @return - a pointer to the nearest point.
        @return - NULL if anything goes wrong.*/
    static Wind *NearestNeighbour(const CList<Wind*, Wind*> &wind, double point_Lat, double point_Lon, bool sorted);
  };
}