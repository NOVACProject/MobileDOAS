#include <MobileDoasLib/Flux/WindField.h>
#include <algorithm>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4706) // Assignment within conditional expresssion
#endif // _MSC_VER

namespace mobiledoas {

    CWindField::CWindField(void)
    {
        Reset();
    }

    CWindField::CWindField(CWindField& wf)
    {
        Reset();

        // copy the other wind field
        for (int i = 0; i < MAX_LAYERS; ++i) {
            m_altitude[i] = wf.m_altitude[i];

            // copy the lists
            for (int j = 0; j < MAX_HOURS; ++j) {
                this->m_wind[i][j] = std::vector<Wind>(wf.m_wind[i][j]);
            }
        }
        this->m_nAltitudes = wf.m_nAltitudes;
    }


    CWindField::~CWindField(void)
    {
    }

    /** Reads a wind field from file.
      It is here assumed that the file is saved in chunks of tab separated data,
      each using the following format:

      Lat=XX\tLong=YY\n
      Altitude  Hour  wd  ws  whatever  Hour  wd  ws  whatever  ...

      with at most 'MAX_HOURS' repititions on the 'Hour wd ws' sequence
    */
    bool CWindField::ReadWindField(const char* fileName) {
        const int BUFSIZE = 4096;
        double alt = 0;
        char hour = 0;
        Wind wind;

        // set all data to 0
        wind.lat = wind.lon = wind.wd = wind.ws = 0;

        bool readingHeaderLine = false; // true if the current line is a header line, explaining what each column represents
        int   hourColumn[MAX_HOURS];   // in which column(s) the hour information is found, absolute column number
        int   wdColumn[MAX_HOURS];     // in which column the wind direction is saved, absolute column number
        int   wsColumn[MAX_HOURS];     // in which column the wind speed is saved, absolute column number
        int   altColumn = 0;    // in which column the altitude is saved, absolute column number
        int   nHourColumns = 0; // the number of repititons on each line.

        double fValue;
        char buf[BUFSIZE];

        FILE* f = fopen(fileName, "r");
        if (f == nullptr) {
            return false;
        }

        // Reset the data content
        Reset();

        // Read the file, one line at a time
        while (fgets(buf, BUFSIZE - 1, f)) {
            // assume that we are not reading the headerline, until otherwise proven
            readingHeaderLine = false;

            // Check if this is a 'Lat=XX\tLong=YY' section
            char* pt = 0;
            if (pt = strstr(buf, "Lat=")) {
                if (2 != sscanf(pt, "Lat=%lf\tLong=%lf", &wind.lat, &wind.lon)) {
                    break;  // format error
                }
                continue; // read the next line
            }

            // Check if this is a 'Altitude=XX' section
            if (pt = strstr(buf, "Altitude=")) {
                if (1 != sscanf(pt, "Altitude=%lf ", &alt)) {
                    break;  // format error
                }
                continue; // read the next line
            }

            // Tokenize the line and see what's in it
            char* szToken = buf;
            int column = -1;
            while (szToken = strtok(szToken, "\t")) {
                ++column;

                // if this is a header line
                if (strstr(szToken, "Altitude")) {
                    nHourColumns = 0;
                    readingHeaderLine = true;
                    altColumn = column;
                    szToken = NULL;
                    continue; // read the next token
                }

                // if we're reading the header
                if (readingHeaderLine) {
                    if (strstr(szToken, "Hour")) {
                        ++nHourColumns;
                        hourColumn[nHourColumns - 1] = column;
                    }
                    if (strstr(szToken, "WD")) {
                        wdColumn[nHourColumns - 1] = column;
                    }
                    if (strstr(szToken, "WS")) {
                        wsColumn[nHourColumns - 1] = column;
                    }

                    // unknown string in the header line, ignore it
                    szToken = NULL;
                    continue;
                }

                // we're reading the data
                if (sscanf(szToken, "%lf", &fValue) != 1) {
                    szToken = NULL;
                    continue; // the data is not parsable as a number
                }

                // find out what kind of data we've just read
                if (column == altColumn) {
                    alt = fValue;
                    szToken = NULL;
                    continue;
                }

                for (int i = 0; i < nHourColumns; ++i) {
                    if (column == hourColumn[i]) {
                        hour = (char)fValue;
                        szToken = NULL;
                        break; // quit the for... loop
                    }
                    if (column == wdColumn[i]) {
                        wind.wd = fValue;
                        szToken = NULL;
                        break; // quit the for... loop
                    }
                    if (column == wsColumn[i]) {
                        // We've collected the data we need, insert it
                        wind.ws = fValue;

                        int index = AltitudeIndex(alt);
                        if (index > -1) {
                            m_wind[index][hour].push_back(wind);
                        }

                        szToken = NULL;
                        break; // quit the for... loop
                    }
                }

                szToken = NULL;
            }// end while(szToken =..

        }

        fclose(f);

        return true;
    }

    void CWindField::Reset() {
        m_nAltitudes = 0;
        memset(m_altitude, 0, MAX_LAYERS * sizeof(double));

        // don't use timeshift
        m_useTimeShift = false;
        m_timeShift = 0;

        // clear the lists
        for (int i = 0; i < MAX_LAYERS; ++i) {
            for (int j = 0; j < MAX_HOURS; ++j) {
                m_wind[i][j].clear();
            }
        }
    }

    /** Returns the altitude index which corresponds to a given altitude.
            @return -1 if this altitude cannot be saved. */
    int CWindField::AltitudeIndex(double altitude) {

        // look if the altitude is already inserted into the data collection
        for (int i = 0; i < m_nAltitudes; ++i) {
            if (fabs(altitude - m_altitude[i]) < 0.01 * m_altitude[i]) {
                return i;
            }
        }

        // altitude not found, insert it?
        if (m_nAltitudes < MAX_LAYERS) {
            m_altitude[m_nAltitudes] = altitude;
            return m_nAltitudes++;
        }

        // layers are full, cannot insert new data point
        return -1;
    }

    int CWindField::GetLayerNum() const {
        return m_nAltitudes;
    }

    double CWindField::GetLayerAltitude(int layer) {
        if (layer < 0 || layer > m_nAltitudes)
            return -1;

        return m_altitude[layer];
    }

    int CWindField::GetWindField(int layer, int hour, int maxPoints, double* lat, double* lon, double* ws, double* wd) {
        if (layer < 0 || layer > m_nAltitudes)
            return 0;
        if (hour < 0 || hour > MAX_HOURS)
            return 0;
        if (maxPoints <= 0)
            return 0;


        // the number of points
        int nPoints = 0;

        // iterate over the entire list
        for (size_t idx = 0; idx < std::min(m_wind[layer][hour].size(), static_cast<size_t>(maxPoints)); ++idx) {
            Wind& wind = m_wind[layer][hour][idx];
            lat[nPoints] = wind.lat;
            lon[nPoints] = wind.lon;
            ws[nPoints] = wind.ws;
            wd[nPoints] = wind.wd;
            ++nPoints;
        }

        return nPoints;
    }

    /** Turns on/off the use of time-shifting */
    void CWindField::UseTimeShift(bool on) {
        m_useTimeShift = on;
    }

    /** Sets the timeshift to use (will only be used if m_useTimeshift is true) */
    void CWindField::SetTimeShift(int timeShift) {
        m_timeShift = std::max(timeShift, 0);
    }

    /** Returns an array with the hours for which we have a wind field
        at the supplied altitude layer.
        @return the number of elements filled into the array. */
    int CWindField::GetHours(int layer, char hours[25]) {
        if (layer < 0 || layer > m_nAltitudes)
            return 0;

        int nHours = 0;

        for (int i = 0; i < MAX_HOURS; ++i) {
            if (m_wind[layer][i].size() > 0) {
                hours[nHours++] = (char)i;
            }
        }

        return nHours;
    }

    /** Interpolates the wind field to the supplied positions and times.
        @param lat - the latitudes for the points to interpolate to.
        @param lon - the longitudes for the points to interpolate to.
        @param times - the timepoints to interpolate to.
        @param nPoints - the number of data points in the arrays = the number of interpolations to do.
        @param windSpeed - will be filled with interpolated windspeeds when the function returns.
        @param windDirection - will be filled with interpolated wind directions when the function returns. */
    int CWindField::Interpolate(const double* lat, const double* lon, const mobiledoas::Time* times, int layer, int nPoints, double* windSpeed, double* windDirection, int mode) {
        int k, n;

        if (nPoints <= 0)
            return 0;

        // 0. Check the time shifting
        if (m_useTimeShift)
            m_timeShift = std::max(m_timeShift, 0);
        else
            m_timeShift = 0;

        // 1. Pick out the hours to interpolate between
        int hours[25], nHours = 0;
        if (times == nullptr) {
            nHours = 1;
            hours[0] = 0;
        }
        else {
            for (k = 0; k < nPoints; ++k) {
                for (n = 0; n < nHours; ++n) {
                    if (hours[n] == times[k].hour + 1 - m_timeShift) {
                        break;
                    }
                }
                if (n == nHours) {
                    if (times[k].hour + 1 - m_timeShift < 0) {
                        // str.Format("Cannot time-shift %d hours. That would give negative hours. Quitting", m_timeShift);
                        // MessageBox(NULL, str, "Error", MB_OK);
                        return 0;
                    }
                    hours[nHours++] = times[k].hour + 1 - m_timeShift;
                    if (nHours > 25) {
                        // MessageBox(NULL, "Too large timespan of traverse, cannot interpolate windfield", "Error", MB_OK);
                        return 0;
                    }
                }
            }
        }

        // 2. Sort the lists, to make things faster
        for (n = 0; n < nHours; ++n)
            SortList(m_wind[layer][hours[n]]);

        // 3. Interpolate
        int nPointsInterpolated = 0;
        for (k = 0; k < nPoints; ++k) {
            std::unique_ptr<Wind> w1 = nullptr;
            std::unique_ptr<Wind> w2 = nullptr;
            if (mode == INTERPOLATION_NEAREST) {
                // w1 is the wind field just before the measurement
                w1 = NearestNeighbour(m_wind[layer][times[k].hour - m_timeShift], lat[k], lon[k], true);

                // w2 is the wind field just after the measurement
                if (times[k].minute >= 30)
                    w2 = NearestNeighbour(m_wind[layer][times[k].hour + 1 - m_timeShift], lat[k], lon[k], true);
                else
                    w2 = NearestNeighbour(m_wind[layer][times[k].hour - 1 - m_timeShift], lat[k], lon[k], true);
            }
            if (w1 != nullptr && w2 != nullptr) {
                windSpeed[k] = (w1->ws * times[k].minute + w2->ws * (60 - times[k].minute)) / 60;
                windDirection[k] = (w1->wd * times[k].minute + w2->wd * (60 - times[k].minute)) / 60;
                ++nPointsInterpolated;
            }
            else if (w1 != nullptr) {
                windSpeed[k] = w1->ws;
                windDirection[k] = w1->wd;
                ++nPointsInterpolated;
            }
            else if (w1 != nullptr) {
                windSpeed[k] = w2->ws;
                windDirection[k] = w2->wd;
                ++nPointsInterpolated;
            }
        }

        return nPointsInterpolated;
    }

    // an operator for comparing two winds, used in the function 'CWindField::SortList'

    bool isLessThan(Wind& w1, Wind& w2) {
        if (w1.lat > w2.lat)
            return false;
        if (w1.lat < w2.lat)
            return true;
        if (w1.lon <= w2.lon)
            return true;
        return false;
    }

    /** This function sorts the supplied list of winds.
        The sorting is first done on the latitude and then on longitude. */
    int CWindField::SortList(std::vector<Wind>& wind) {

        std::sort(wind.begin(), wind.end(), isLessThan);

        return 0;
    }

    std::unique_ptr<Wind> CWindField::NearestNeighbour(const std::vector<Wind>& wind, double lat, double lon, bool /*sorted*/) {

        if (wind.size() == 0) {
            return nullptr;
        }

        std::unique_ptr<Wind> best;
        double distance2, dlat, dlon;
        double nearestDistance = 1e9;

        // TODO: This function performes a simple search to find the point - not efficient!
        //      if the list is sorted, this fact should be used
        for (size_t idx = 0; idx < wind.size(); ++idx) {
            Wind w = wind[idx];

            dlat = w.lat - lat;
            dlon = w.lon - lon;
            distance2 = dlat * dlat + dlon * dlon;

            if (distance2 < nearestDistance) {
                best = std::make_unique<Wind>(w);
                nearestDistance = distance2;
            }
        }

        // return the closes neighbour
        return best;
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

