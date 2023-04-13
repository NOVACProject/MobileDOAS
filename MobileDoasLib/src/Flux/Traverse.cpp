#include <MobileDoasLib/Flux/Traverse.h>
#include <MobileDoasLib/GpsData.h>
#include <MobileDoasLib/Definitions.h>
#include <MobileDoasLib/Flux/Flux1.h>
#include <SpectralEvaluation/VectorUtils.h>

namespace mobiledoas {

    CTraverse::CTraverse(void)
    {
        columnArray.resize(MAX_TRAVERSELENGTH);
        columnError.resize(MAX_TRAVERSELENGTH);

        this->m_channelNum = 0;
        this->m_dynRange = 4095;
        this->m_expTime = 0;
        this->m_fileName = "";
        this->m_filePath = "";
        this->m_fitRegion[0] = 0;
        this->m_fitRegion[1] = 1;
        this->m_gasFactor = GASFACTOR_SO2;
        this->m_hasGPS = true;
        this->m_highIndex = 0;
        this->m_lowIndex = 0;
        this->m_nSpectra = 0;
        this->m_Offset = 0;
        this->m_plumeWidth = 0;
        this->m_recordNum = 0;
        this->m_refFile = "";
        this->m_specieName = "";
        this->m_spectraInPlume = 0;
        this->m_traverseLength = 0;
        this->m_correctedTraverseLength = 0;
        m_useWindField = false;

        m_fCreateAdditionalLog = nullptr;
        m_additionalLogName = "";
    }

    CTraverse::~CTraverse(void)
    {
    }

    long CTraverse::DeletePoints(long lowIndex, long highIndex) {
        long i;
        int gap;
        gap = highIndex - lowIndex + 1;

        for (i = lowIndex; i < m_recordNum; ++i) {
            columnArray[i] = columnArray[i + gap];
            columnError[i] = columnError[i + gap];
            longitude[i] = longitude[i + gap];
            latitude[i] = latitude[i + gap];
            altitude[i] = altitude[i + gap];
            intensArray[i] = intensArray[i + gap];
            columnArray[i + gap] = 0;
            columnError[i + gap] = 0;
            longitude[i + gap] = 0;
            latitude[i + gap] = 0;
            altitude[i + gap] = 0;
            intensArray[i + gap] = 0;
        }
        m_recordNum = m_recordNum - gap;	//after delete points,record number decreases
        return gap;
    }

    /* Delete all data points with an intensity lower than @intensityLimit */
    long CTraverse::DeleteLowIntensityPoints(double intensityLimit) {
        long nDeleted = 0;

        for (long i = 0; i < m_recordNum; ++i) {
            if (intensArray[i] < intensityLimit) {
                DeletePoints(i, i + 1);
                --i;
                ++nDeleted;
            }
        }

        return nDeleted;
    }

    /* Delete all data points with an intensity higher than @intensityLimit */
    long CTraverse::DeleteHighIntensityPoints(double intensityLimit) {
        long nDeleted = 0;

        for (long i = 0; i < m_recordNum; ++i) {
            if (intensArray[i] > intensityLimit) {
                DeletePoints(i, i + 1);
                --i;
                ++nDeleted;
            }
        }

        return nDeleted;
    }

    /** Calculates the flux using the wind field defined by the vectors 'm_windspeed'
        and 'm_windDirection'. */
    double  CTraverse::GetTotalFlux() {
        return GetTotalFlux(0, 1e9);
    }

    double CTraverse::GetTotalFlux(double windSpeed, double windDirection) {
        double totalFlux = 0.0;
        bool    useWindField = (fabs(windDirection) > 1e6) ? true : false;

        long i;
        double column, accColumn;

        // the last point which we know had connection with the GPS-satellites
        gpsPosition lastPointWithGPS;
        int         lastPointWithGPSIndex = 0;

        // the current point
        gpsPosition thisPoint;

        // extra traverse information
        m_traverseLength = 0;
        m_correctedTraverseLength = 0;
        m_plumeWidth = 0;
        m_spectraInPlume = 0;

        accColumn = 0;

        if (m_fCreateAdditionalLog != nullptr && m_fCreateAdditionalLog && m_additionalLogName.size() != 0) {
            FILE* f = fopen(m_additionalLogName.c_str(), "w");
            if (0 == f) {
                (*m_fCreateAdditionalLog) = false;
            }
            else {
                fprintf(f, "Lat [dd.ddd]\tLong [dd.ddd]\tAvgColumn [ppmm]\tDistance [m]\tGeom. corrected distance [m]\tWind Direction [deg]\tWind Speed[m/s]\tPartial Flux [kg/s]\n");
                fclose(f);
            }
        }

        /* Correction for the fact that if the GPS-connection
          is gone in the beginning of the traverse, no flux can be calculated */
        while (latitude[m_lowIndex] == 0 && longitude[m_lowIndex] == 0)
            ++m_lowIndex;

        /* Check that we have any points with valid GPS-data, and that the selected
            part of the traverse contains at least one data point */
        if (m_lowIndex >= m_highIndex) {
            // MessageBox(NULL, "No data points with valid GPS data found. No Flux could be calculated", "Error", MB_OK);
            return 0.0;
        }

        /** Get the maximum column in the selected part of the traverse */
        m_maxColumn = Max(begin(columnArray) + m_lowIndex, begin(columnArray) + m_highIndex) - m_Offset;

        /* We know that this point has GPS-connection,
            save the gps data for this point in case the next point does not have gps-data. */
        lastPointWithGPS.latitude = latitude[m_lowIndex];
        lastPointWithGPS.longitude = longitude[m_lowIndex];
        lastPointWithGPSIndex = 0;

        /* loop through all the (selected) data points and calculate flux */
        for (i = m_lowIndex + 1; i < m_highIndex; i++) {

            // Check if this point has gps connection
            if ((latitude[i] == 0) && (longitude[i] == 0)) {
                accColumn += columnArray[i] - m_Offset;
                continue;
            }

            thisPoint.latitude = latitude[i];
            thisPoint.longitude = longitude[i];

            // the column
            column = columnArray[i] - m_Offset;

            // if necessary, get the windspeed and direction
            if (useWindField) {
                windSpeed = (m_windSpeed[i] + m_windSpeed[lastPointWithGPSIndex]) / 2;
                windDirection = (m_windDirection[i] + m_windDirection[lastPointWithGPSIndex]) / 2;
            }

            // get the average column value along the path. ADDED 2006-05-15
            if (accColumn > 0) {
                column = (column + accColumn) / (i - lastPointWithGPSIndex);
            }

            // calculate the flux
            totalFlux += CalculateFlux(column, lastPointWithGPS, thisPoint, windSpeed, windDirection);

            /* We know that this point has GPS-connection,
              save the gps data for this point in case the next point does not have gps-data. */
            lastPointWithGPS.latitude = thisPoint.latitude;
            lastPointWithGPS.longitude = thisPoint.longitude;
            lastPointWithGPSIndex = i;

            // reset the accumulated column
            accColumn = 0.0;
        }

        return totalFlux;
    }

    double CTraverse::CalculateFlux(double column, const gpsPosition& pos1, const gpsPosition& pos2, double windSpeed, double windDirection)
    {
        double lat1 = pos1.latitude;
        double lat2 = pos2.latitude;
        double lon1 = pos1.longitude;
        double lon2 = pos2.longitude;

        // the distance travelled. Unit: [m]
        double  distance = mobiledoas::GPSDistance(lat1, lon1, lat2, lon2);

        // the total mass per cross section of the plume. Unit [kg/m^2]
        double  massColumn = 1E-6 * column * m_gasFactor;

        // the correction factor for the wind. Unit [-]
        double  windFactor = mobiledoas::GetWindFactor(lat1, lon1, lat2, lon2, windDirection);

        // the flux. Unit [kg/s] = [kg/m^2] * [m] * [m/s] * [-]
        double  flux = massColumn * distance * windSpeed * windFactor;

        // Gathering Extra traverse information
        m_traverseLength += distance;
        m_correctedTraverseLength += distance * windFactor;
        if (column > m_maxColumn * 0.5) {
            m_plumeWidth += distance * windFactor;
            ++m_spectraInPlume;
        }
        if (m_fCreateAdditionalLog != nullptr && m_fCreateAdditionalLog && m_additionalLogName.size() != 0) {
            FILE* f = fopen(m_additionalLogName.c_str(), "a+");
            fprintf(f, "%lf\t%lf\t%lf\t", lat2, lon2, column);
            fprintf(f, "%lf\t%lf\t%lf\t%lf\t%lf\n",
                distance, distance * windFactor, windDirection, windSpeed, flux);
            fclose(f);
        }

        return flux;
    }

    /**Get all information about the plume center
    *@maxBuffer 0 - column in the center - where max column locates
    *@maxBuffer 1 - latitude  - where max column locates
    *@maxBuffer 2 - longitude  - where max column locates
    *@maxBuffer 3 - wind angle  - where max column locates
    *@avBuffer 0 - column in the center - where average column locates
    *@avBuffer 1 - latitude  - where average column locates
    *@avBuffer 2 - longitude  - where average column locates
    *@avBuffer 3 - wind angle  - where average column locates
    */
    int CTraverse::GetPlumeCenter(double srclat, double srclon, double* maxBuffer, double* avBuffer)
    {
        long n, maxIndex, avIndex, times;
        double sumColumn, maxColumn, avColumn, maxLat, maxLon, avLat, avLon;
        memcpy((void*)tmpColumn, (void*)columnArray.data(), sizeof(double) * m_recordNum);
        sumColumn = 0.0;
        maxColumn = 0.0;
        avIndex = 0;
        maxIndex = 0;
        //offset = -27;
        for (n = 0; n < m_recordNum; n++) {
            sumColumn += tmpColumn[n];
            if (tmpColumn[n] >= maxColumn) {
                maxColumn = tmpColumn[n];
                maxIndex = n;
            }
        }
        avColumn = sumColumn / 2;

        sumColumn = 0;

        for (n = 0; n < m_recordNum; ++n) {
            sumColumn += tmpColumn[n];
            times = (long)(sumColumn / avColumn);
            if (times == 1) {
                avIndex = n;
                break;
            }
        }

        maxLat = latitude[maxIndex];
        maxLon = longitude[maxIndex];
        maxBuffer[3] = mobiledoas::GPSBearing(maxLat, maxLon, srclat, srclon);
        maxBuffer[0] = maxColumn;
        maxBuffer[1] = maxLat;
        maxBuffer[2] = maxLon;
        maxBuffer[4] = maxIndex;
        avLat = latitude[avIndex];
        avLon = longitude[avIndex];
        avBuffer[3] = mobiledoas::GPSBearing(avLat, avLon, srclat, srclon);
        avBuffer[0] = tmpColumn[avIndex];;
        avBuffer[1] = avLat;
        avBuffer[2] = avLon;
        avBuffer[4] = avIndex;
        return 1;
    }

    bool CTraverse::IsBadPoint(int spectrumIndex) {
        if (spectrumIndex < 0 || spectrumIndex >= m_recordNum)
            return true; // <-- out of bounds

        if (columnError[spectrumIndex] > 30)
            return true;

        if (intensArray[spectrumIndex] < 0.05)
            return true;

        if (intensArray[spectrumIndex] > 1.0 && intensArray[spectrumIndex] < 200)
            return true;

        return false;
    }


    double CTraverse::CalculateOffset() {

        // calculate the offset as the average of the 20% lowest column values 
        //    that are not considered as 'bad' values

        std::vector<double> testColumns;

        for (long i = 0; i < m_recordNum; ++i) {
            if (IsBadPoint(i)) {
                continue;
            }

            testColumns.push_back(columnArray[i]);
        }

        if (testColumns.size() <= 5) {
            m_Offset = 0;
            return 0.0; // <-- could not calculate the offset
        }

        // Calculate the offset
        this->m_Offset = CalculateOffset(testColumns, static_cast<long>(testColumns.size()));

        return m_Offset;
    }

    /** Calculates the offset of the given set of column data
        @return the offset value. This will be 0.0 if nothing
            could be calculated */
    double CTraverse::CalculateOffset(std::vector<double>& columnData, long nDataPoints) const {
        double avg = 0.0;

        // We need at least 5 data points to be able to calculate
        //	an offset
        if (nDataPoints <= 5) {
            return 0.0; // <-- could not calculate the offset
        }

        // calculate the offset as the average of the 20% lowest column values 

        // Find the N lowest column values
        int N = (int)(0.2 * nDataPoints);
        std::vector<double> m(N);

        FindNLowest(columnData, nDataPoints, m);

        avg = Average(m);

        return avg;
    }

    // --------------- OPERATORS ---------------------
    CTraverse& CTraverse::operator=(const CTraverse& t) {
        this->m_recordNum = t.m_recordNum;
        this->m_channelNum = t.m_channelNum;
        this->m_dynRange = t.m_dynRange;
        this->m_gasFactor = t.m_gasFactor;
        this->m_Offset = t.m_Offset;
        this->m_maxColumn = t.m_maxColumn;
        this->m_lowIndex = t.m_lowIndex;
        this->m_highIndex = t.m_highIndex;
        this->m_fileName = t.m_fileName;
        this->m_filePath = t.m_filePath;
        this->m_specieName = t.m_specieName;
        this->m_plumeWidth = t.m_plumeWidth;
        this->m_traverseLength = t.m_traverseLength;
        this->m_correctedTraverseLength = t.m_correctedTraverseLength;
        this->m_spectraInPlume = t.m_spectraInPlume;

        memcpy(this->latitude, t.latitude, MAX_TRAVERSELENGTH * sizeof(double));
        memcpy(this->longitude, t.longitude, MAX_TRAVERSELENGTH * sizeof(double));
        memcpy(this->altitude, t.altitude, MAX_TRAVERSELENGTH * sizeof(double));

        this->columnArray.resize(MAX_TRAVERSELENGTH);
        memcpy(this->columnArray.data(), t.columnArray.data(), MAX_TRAVERSELENGTH * sizeof(double));

        this->columnError.resize(MAX_TRAVERSELENGTH);
        memcpy(this->columnError.data(), t.columnError.data(), MAX_TRAVERSELENGTH * sizeof(double));

        memcpy(this->intensArray, t.intensArray, MAX_TRAVERSELENGTH * sizeof(double));
        memcpy(this->time, t.time, MAX_TRAVERSELENGTH * sizeof(mobiledoas::Time));
        memcpy(this->m_windDirection, t.m_windDirection, MAX_TRAVERSELENGTH * sizeof(double));
        memcpy(this->m_windSpeed, t.m_windSpeed, MAX_TRAVERSELENGTH * sizeof(double));

        this->m_useWindField = t.m_useWindField;
        this->m_hasGPS = t.m_hasGPS;
        this->m_nSpectra = t.m_nSpectra;
        this->m_expTime = t.m_expTime;
        this->m_fitRegion[0] = t.m_fitRegion[0];
        this->m_fitRegion[1] = t.m_fitRegion[1];
        this->m_refFile = t.m_refFile;

        this->m_fCreateAdditionalLog = t.m_fCreateAdditionalLog;
        this->m_additionalLogName = std::string(t.m_additionalLogName);

        return *this;
    }

}
