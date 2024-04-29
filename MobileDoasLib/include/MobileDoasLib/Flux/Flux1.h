#pragma once

#include <vector>
#include <MobileDoasLib/Flux/Traverse.h>
#include <MobileDoasLib/Flux/WindField.h>

namespace mobiledoas
{
    /* This function calculates the wind factor when travelling from point 1 to point 2
        and the wind is defined by 'windAngle' */
    double GetWindFactor(double lat1, double lon1, double lat2, double lon2, double windAngle);

    class CFlux {
    public:
        CFlux();
        virtual ~CFlux();

        /** The read traverse data */
        std::vector <mobiledoas::CTraverse*> m_traverse;

        /** The number of traverses read so far */
        long m_traverseNum;

        /** The current traverse */
        long m_curTraverse;


        /* managing the data */
        void  Reset();
        bool  hasValidGPS(int file = -1);

        // --------  Reading the files --------

        /** This reads the properties in the evaluation log file and checks that all data is valid.
            @return 1 if the file is ok, otherwise 0. */
        int       ReadSettingFile(std::string filename, int& nChannels, double& fileVersion);
        int       ReadSettingFile(std::string filename, long fileIndex, int& nChannels, double& fileVersion);
        long      ReadLogFile(std::string filePath, std::string fileName, int nChannels, double fileVersion);
        long      ReadLogFile(std::string filePath, std::string fileName, long fileIndex, int nChannels, double fileVersion);

        /* Output */
        bool      fCreateAdditionalLog;
        std::string additionalLogName;

        /* Calculating */
        void      SetParams(double pSpeed, double pAngle, double pLat, double pLon, long pLowIndex, long pHighIndex, double pOffset);

        /* Exporting data to other parts of the program */
        long	GetLat(double* pBuffer);
        long	GetLon(double* pBuffer);
        long	GetColumn(double* pBuffer);
        long	GetColumnError(double* pBuffer);
        long	GetIntensity(double* pBuffer);
        long	GetAltitude(double* pBuffer);
        long	GetTime(double* pBuffer);
        long	GetCurrentFileName(std::string& str);
        long	GetDynamicRange();

        /* Calculating the flux */
        double    GetTotalFlux();

        /* Geometrical calculations */
        int       GetPlumeCenter(double srclat, double srclon, double* maxBuffer, double* avBuffer);

        /* The result */
        double    m_totalFlux;

        /** The upper limit for the total flux */
        double	m_totalFlux_High;

        /** The lower limit for the total flux */
        double	m_totalFlux_Low;

        /* parameters for calculating the flux */
        double    m_windAngle;
        double    m_windSpeed;

        /** A wind field, if one has been imported into the program */
        mobiledoas::CWindField* m_windField;
        bool        m_useWindField;

        /** Interpolates the wind field to the currently selected traverse */
        void        InterpolateWindField(int layer);

        /* the source */
        double    m_srcLatitude;
        double    m_srcLongitude;

        /* information about the traverse - calculated at the same time as the flux */
        double    plumeWidth;
        double    traverseLength;
        long      spectraInPlume;

    private:
        /** Temporary variables, used for reading and parsing the log files */
        char		m_FileType[100];
        std::string m_specieName[20];
        double		m_lastGasFactor;
        long		m_lastDynamicRange;
        long		m_lastFitFrom;
        long		m_lastFitTo;
        std::string m_lastRefFile[20];
        long		m_lastRefFileNum;


        /** Assign a read value to the corresponding array
            (decides if the read value is a column value, a latitude, a longitude...) */
        bool AssignValueToColumn(long fileIndex, int column, int row, double value[3], int nChannels, double fileVersion); // used when parsing the log files
        bool AssignValueToColumn_ReEvaluationLog(long fileIndex, int column, int row, double value[3], int nChannels, double fileVersion); // used when parsing the reevaluation log files
    };
}
