#include <MobileDoasLib/Flux/Flux1.h>
#include <cmath>
#include <math.h>
#include <vector>
#include <MobileDoasLib/Definitions.h>
#include <MobileDoasLib/GpsData.h>
#include <SpectralEvaluation/VectorUtils.h>
#include <SpectralEvaluation/StringUtils.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4706) // Assignment within conditional expresssion
#endif // _MSC_VER

namespace mobiledoas {

    double GetWindFactor(double lat1, double lon1, double lat2, double lon2, double windAngle) {
        double windFactor, travelAngle, difAngle;

        travelAngle = DEGREETORAD * mobiledoas::GPSBearing(lat1, lon1, lat2, lon2);

        /* plumeAngle = pi + windAngle */
        difAngle = travelAngle + 1.5 * M_PI - windAngle * DEGREETORAD;	// this is the difference between travel direction and plume direction
        windFactor = std::cos(difAngle);

        return windFactor;
    }

    CFlux::CFlux() {
        m_curTraverse = 0;
        m_traverseNum = 0;

        // allocate space for one traverse...
        m_traverse.push_back(new CTraverse());

        m_windField = nullptr;
        m_useWindField = false;

        m_windSpeed = 5;
        m_windAngle = 210;
        plumeWidth = 0;
        traverseLength = 0;
        spectraInPlume = 0;

        fCreateAdditionalLog = false;
    }

    CFlux::~CFlux()
    {
        if (m_windField != nullptr) {
            delete(m_windField);
            m_windField = nullptr;
        }

        for (int i = 0; i < m_traverse.size(); ++i) {
            mobiledoas::CTraverse* tr = m_traverse[i];
            if (tr != nullptr) {
                delete tr;
            }
        }
    }

    /**Calculate flux and record every calculated flux value in "postFluxRecord.txt" file
    *
    */
    double CFlux::GetTotalFlux()
    {
        if (fCreateAdditionalLog) {
            additionalLogName = "AdditionalFluxLog.txt";
            m_traverse[m_curTraverse]->m_fCreateAdditionalLog = &fCreateAdditionalLog;
            m_traverse[m_curTraverse]->m_additionalLogName = std::string(additionalLogName);
        }

        // ----------------------- CALCULATING THE FLUX -------------------------
        // Calculate the flux
        if (m_traverse[m_curTraverse]->m_useWindField)
            m_totalFlux = m_traverse[m_curTraverse]->GetTotalFlux();
        else
            m_totalFlux = m_traverse[m_curTraverse]->GetTotalFlux(m_windSpeed, m_windAngle);

        // save some more data
        this->plumeWidth = m_traverse[m_curTraverse]->m_plumeWidth;
        this->traverseLength = m_traverse[m_curTraverse]->m_traverseLength;

        // -------------------- ESTIMATING THE ERROR IN FLUX -----------------------
        mobiledoas::CTraverse* upperTraverse = new mobiledoas::CTraverse();
        mobiledoas::CTraverse* lowerTraverse = new mobiledoas::CTraverse();
        mobiledoas::CTraverse* curTraverse = m_traverse[m_curTraverse];
        *upperTraverse = *curTraverse;
        *lowerTraverse = *curTraverse;

        // Get the offset that the user has chosen
        double userOffset = curTraverse->m_Offset;

        // Find the average column error
        double avgColErr = Average(begin(curTraverse->columnError), begin(curTraverse->columnError) + curTraverse->m_recordNum);

        int k;
        // Go through the traverse, for each data point with a column value less than
        //		(offset + avgColErr) decrease the column value with its column error
        //		and for each data point with a column value higher than (offset + avgColErr)
        //		increase the column value with its column error.
        for (k = 0; k < curTraverse->m_recordNum; ++k) {
            if (curTraverse->columnArray[k] < (userOffset + avgColErr)) {
                lowerTraverse->columnArray[k] = curTraverse->columnArray[k] + curTraverse->columnError[k];
                upperTraverse->columnArray[k] = curTraverse->columnArray[k] - curTraverse->columnError[k];
            }
            else {
                lowerTraverse->columnArray[k] = curTraverse->columnArray[k] - curTraverse->columnError[k];
                upperTraverse->columnArray[k] = curTraverse->columnArray[k] + curTraverse->columnError[k];
            }
        }
        // correct the offsets
        lowerTraverse->m_Offset = userOffset + avgColErr;
        upperTraverse->m_Offset = userOffset - avgColErr;

        // Calculate the fluxes
        if (m_traverse[m_curTraverse]->m_useWindField) {
            m_totalFlux_Low = lowerTraverse->GetTotalFlux();
            m_totalFlux_High = upperTraverse->GetTotalFlux();
        }
        else {
            m_totalFlux_Low = lowerTraverse->GetTotalFlux(m_windSpeed, m_windAngle);
            m_totalFlux_High = upperTraverse->GetTotalFlux(m_windSpeed, m_windAngle);
        }

        delete upperTraverse;
        delete lowerTraverse;

        return m_totalFlux;
    }

    /* called when reading a new evaluation log */
    long CFlux::ReadLogFile(std::string filePath, std::string fileName, int nChannels, double fileVersion) {
        return ReadLogFile(filePath, fileName, m_traverseNum, nChannels, fileVersion);
    }

    /* called when re-reading an old evaluation log */
    long CFlux::ReadLogFile(std::string filePath, std::string fileName, long fileIndex, int nChannels, double fileVersion) {
        char buf[4096];
        int n = 0;

        FILE* f = fopen(fileName.c_str(), "r");
        if (nullptr == f) {
            f = fopen((filePath + std::string("\\") + fileName).c_str(), "r");
            if (nullptr == f) {
                // MessageBox(NULL, TEXT("Can not read log file"), TEXT("Error"), MB_OK);
                return 0;
            }
        }

        m_traverse[fileIndex]->m_hasGPS = false;
        if (nChannels > 1) {
            m_traverse[fileIndex + 1]->m_hasGPS = false;
        }

        double fValue[3];
        int i = 0;

        // Read the data from the evaluation file into the correct columns
        while (fgets(buf, 4095, f)) { // fgets reads until the next newline character
            char* szToken = buf;

            i = 0;
            while (szToken = strtok(szToken, "\t")) {

                if (strstr(szToken, ":")) {
                    if (sscanf(szToken, "%lf:%lf:%lf", &fValue[0], &fValue[1], &fValue[2]) != 3)
                        break;
                }
                else {
                    if (sscanf(szToken, "%lf", &fValue[0]) != 1)
                        break;
                }

                AssignValueToColumn(fileIndex, i, n, fValue, nChannels, fileVersion);

                ++i;
                szToken = NULL;
            }
            if (i != 0)
                ++n;

            if (n == MAX_TRAVERSELENGTH) {
                // CString message;
                // message.Format("Traverses longer than %d data points cannot be read fully. Reading stopped at datapoint %d.", MAX_TRAVERSELENGTH, MAX_TRAVERSELENGTH);
                // MessageBox(NULL, message, "Too long traverse", MB_OK);
                break;
            }
        }

        fclose(f);

        if (n == 0) {
            nChannels = 1;
            return 0;
        }

        /* Remove all backslashes in the given fileName, if any */
        int stringIndex = ReverseFind(fileName, '\\');
        if (stringIndex != -1) {
            fileName = Right(fileName, static_cast<int>(fileName.size()) - stringIndex - 1);
        }

        for (int k = 0; k < nChannels; ++k) {
            m_traverse[fileIndex + k]->m_recordNum = n;
            m_traverse[fileIndex + k]->m_gasFactor = m_traverse[fileIndex]->m_gasFactor;

            if (m_specieName[k].size() == 0) {
                m_traverse[fileIndex + k]->m_fileName = fileName;
            }
            else {
                m_traverse[fileIndex + k]->m_fileName = m_specieName[k] + std::string(" * ") + fileName;
                if (0 == _strnicmp(m_specieName[k].c_str(), "SO2", 3 * sizeof(char)))
                    m_traverse[fileIndex + k]->m_gasFactor = GASFACTOR_SO2;
                if (0 == _strnicmp(m_specieName[k].c_str(), "O3", 2 * sizeof(char)))
                    m_traverse[fileIndex + k]->m_gasFactor = GASFACTOR_O3;
                if (0 == _strnicmp(m_specieName[k].c_str(), "NO2", 3 * sizeof(char)))
                    m_traverse[fileIndex + k]->m_gasFactor = GASFACTOR_NO2;
            }
            m_traverse[fileIndex + k]->m_filePath = filePath;

            m_traverse[fileIndex + k]->CalculateOffset();

            // for files with several channels the fitregion is only read into the first channel
            m_traverse[fileIndex + k]->m_fitRegion[0] = m_traverse[fileIndex]->m_fitRegion[0];
            m_traverse[fileIndex + k]->m_fitRegion[1] = m_traverse[fileIndex]->m_fitRegion[1];

            m_traverse[fileIndex + k]->m_dynRange = m_lastDynamicRange;

        }
        if (fileIndex == m_traverseNum) {
            m_traverseNum += nChannels;
            m_curTraverse = m_traverseNum - nChannels;
        }

        return n;
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
    int CFlux::GetPlumeCenter(double srclat, double srclon, double* maxBuffer, double* avBuffer)
    {
        return m_traverse[m_curTraverse]->GetPlumeCenter(srclat, srclon, maxBuffer, avBuffer);
    }

    long CFlux::GetColumn(double* pBuffer)
    {
        memcpy((void*)pBuffer, (void*)m_traverse[m_curTraverse]->columnArray.data(), sizeof(double) * m_traverse[m_curTraverse]->m_recordNum);
        return m_traverse[m_curTraverse]->m_recordNum;
    }

    long CFlux::GetTime(double* pBuffer) {
        for (long k = 0; k < m_traverse[m_curTraverse]->m_recordNum; ++k) {
            mobiledoas::Time& t = m_traverse[m_curTraverse]->time[k];
            pBuffer[k] = 3600 * t.hour + 60 * t.minute + t.second;
            if (k > 0) {
                if (pBuffer[k] < pBuffer[k - 1]) {
                    pBuffer[k] += 3600 * 24;
                }
            }
        }
        return m_traverse[m_curTraverse]->m_recordNum;
    }

    long CFlux::GetColumnError(double* pBuffer)
    {
        memcpy((void*)pBuffer, (void*)m_traverse[m_curTraverse]->columnError.data(), sizeof(double) * m_traverse[m_curTraverse]->m_recordNum);
        return m_traverse[m_curTraverse]->m_recordNum;
    }

    long CFlux::GetIntensity(double* pBuffer)
    {
        memcpy((void*)pBuffer, (void*)m_traverse[m_curTraverse]->intensArray, sizeof(double) * m_traverse[m_curTraverse]->m_recordNum);
        return m_traverse[m_curTraverse]->m_recordNum;
    }

    long CFlux::GetAltitude(double* pBuffer)
    {
        memcpy((void*)pBuffer, (void*)m_traverse[m_curTraverse]->altitude, sizeof(double) * m_traverse[m_curTraverse]->m_recordNum);
        return m_traverse[m_curTraverse]->m_recordNum;
    }

    long CFlux::GetLon(double* pBuffer)
    {
        memcpy((void*)pBuffer, (void*)m_traverse[m_curTraverse]->longitude, sizeof(double) * m_traverse[m_curTraverse]->m_recordNum);
        return m_traverse[m_curTraverse]->m_recordNum;
    }

    long CFlux::GetLat(double* pBuffer)
    {
        memcpy((void*)pBuffer, (void*)m_traverse[m_curTraverse]->latitude, sizeof(double) * m_traverse[m_curTraverse]->m_recordNum);
        return m_traverse[m_curTraverse]->m_recordNum;
    }

    long CFlux::GetDynamicRange() {
        return m_traverse[m_curTraverse]->m_dynRange;
    }

    void CFlux::SetParams(double pSpeed, double pAngle, double pLat, double pLon, long pLowIndex, long pHighIndex, double pOffset)
    {
        m_windSpeed = pSpeed;
        m_windAngle = pAngle;
        m_srcLatitude = pLat * DEGREETORAD;
        m_srcLongitude = pLon * DEGREETORAD;
        m_traverse[m_curTraverse]->m_lowIndex = pLowIndex;
        m_traverse[m_curTraverse]->m_highIndex = pHighIndex;
        m_traverse[m_curTraverse]->m_Offset = pOffset;
    }

    int CFlux::ReadSettingFile(std::string filename, int& nChannels, double& fileVersion) {
        return ReadSettingFile(filename, m_traverseNum, nChannels, fileVersion);
    }

    int CFlux::ReadSettingFile(std::string filename, long /*fileIndex*/, int& nChannels, double& fileVersion) {
        char* pt;
        char txt[256];
        char nl[2] = { 0x0a, 0 };
        char lf[2] = { 0x0d, 0 };
        int  nofSpecies = 0;
        bool hasMasterChannelData = false;
        std::string knownSpecie[] = { "SO2", "NO2", "O3", "O4", "HCHO", "RING", "H2O", "CLO", "BRO", "CHOCHO", "Glyoxal", "Formaldehyde", "FraunhoferRef" };
        int nKnownSpecies = 13;
        std::string columnLabel;

        // reset the 'm_specieName' buffer
        for (int i = 0; i < 20; ++i) {
            m_specieName[i] = "";
        }

        char msg[200];
        nChannels = 1;
        m_lastRefFileNum = 0;

        int result = 0;
        FILE* fil = fopen(filename.c_str(), "r");
        if (fil < (FILE*)1)
        {
            sprintf(msg, "Could not open file %s", filename.c_str());
            // MessageBox(NULL, msg, TEXT("Error"), MB_OK);
            return 0;
        }

        while (fgets(txt, sizeof(txt) - 1, fil)) {

            if (strlen(txt) > 4 && txt[0] != '%') {
                pt = txt;
                if (pt = strstr(txt, nl))
                    pt[0] = 0;
                pt = txt;
                if (pt = strstr(txt, lf))
                    pt[0] = 0;


                if (pt = strstr(txt, "GASFACTOR="))
                {
                    pt = strstr(txt, "=");
                    sscanf(&pt[1], "%lf", &m_lastGasFactor);
                }

                if (pt = strstr(txt, "DYNAMICRANGE="))
                {
                    pt = strstr(txt, "=");
                    sscanf(&pt[1], "%ld", &m_lastDynamicRange);
                }

                if ((pt = strstr(txt, "FITFROM=")) || (pt = strstr(txt, "FitFrom")))
                {
                    pt = strstr(txt, "=");
                    sscanf(&pt[1], "%ld", &m_lastFitFrom);
                }

                if ((pt = strstr(txt, "FITTO=")) || (pt = strstr(txt, "FitTo")))
                {
                    pt = strstr(txt, "=");
                    sscanf(&pt[1], "%ld", &m_lastFitTo);
                }

                if (pt = strstr(txt, "REFFILE="))
                {
                    char buffer[4096];
                    pt = strstr(txt, "=");
                    if (0 < sscanf(&pt[1], "%4095s", &buffer)) {
                        char* lastBackslash = strrchr(buffer, '\\');
                        if (lastBackslash == nullptr) {
                            m_lastRefFile[m_lastRefFileNum++] = buffer;
                        }
                        else {
                            m_lastRefFile[m_lastRefFileNum++] = lastBackslash + 1;
                        }
                    }
                }

                if (pt = strstr(txt, "VERSION="))
                {
                    pt = strstr(txt, "=");
                    sscanf(&pt[1], "%lf", &fileVersion);
                }

                if (pt = strstr(txt, "FILETYPE=")) {
                    pt = strstr(txt, "=");
                    sscanf(pt + 1, "%99s", m_FileType);
                    result = 1; /* the file is a correct evaluation log */
                }

                pt = txt;
                while (pt = strstr(pt, "Master_Column_")) {
                    if (EqualsIgnoringCase(pt + 14, "SO2", 3)) {
                        m_specieName[nofSpecies] = "Master_SO2";
                    }
                    else if (EqualsIgnoringCase(pt + 14, "NO2", 3)) {
                        m_specieName[nofSpecies] = "Master_NO2";
                    }
                    else if (EqualsIgnoringCase(pt + 14, "O3", 2)) {
                        m_specieName[nofSpecies] = "Master_O3";
                    }
                    else if (EqualsIgnoringCase(pt + 14, "RING", 4)) {
                        m_specieName[nofSpecies] = "Master_Ring";
                    }
                    else if (EqualsIgnoringCase(pt + 14, "O4", 2)) {
                        m_specieName[nofSpecies] = "Master_O4";
                    }
                    else if (EqualsIgnoringCase(pt + 14, "HCHO", 4)) {
                        m_specieName[nofSpecies] = "Master_HCHO";
                    }
                    ++nofSpecies;
                    ++pt;
                }

                pt = txt;
                while (pt = strstr(pt, "Slave_Column_")) {
                    if (EqualsIgnoringCase(pt + 14, "SO2", 3)) {
                        m_specieName[nofSpecies] = "Slave_SO2";
                    }
                    else if (EqualsIgnoringCase(pt + 14, "NO2", 3)) {
                        m_specieName[nofSpecies] = "Slave_NO2";
                    }
                    else if (EqualsIgnoringCase(pt + 14, "O3", 2)) {
                        m_specieName[nofSpecies] = "Slave_O3";
                    }
                    else if (EqualsIgnoringCase(pt + 14, "RING", 4)) {
                        m_specieName[nofSpecies] = "Slave_Ring";
                    }
                    else if (EqualsIgnoringCase(pt + 14, "O4", 2)) {
                        m_specieName[nofSpecies] = "Slave_O4";
                    }
                    else if (EqualsIgnoringCase(pt + 14, "HCHO", 4)) {
                        m_specieName[nofSpecies] = "Slave_HCHO";
                    }
                    ++nofSpecies;
                    ++pt;
                }

                if (nullptr != (pt = strstr(txt, "Column(Master)")) || nullptr != (pt = strstr(txt, "Master_Column"))) {
                    hasMasterChannelData = true;
                }

                if (hasMasterChannelData && (nullptr != (pt = strstr(txt, "Column(Slave)")) || nullptr != (pt = strstr(txt, "Slave_Column")))) {
                    nChannels = 2; /* if there are several channels */
                    m_specieName[0] = "Master";
                    m_specieName[1] = "Slave";
                    m_traverse.push_back(new mobiledoas::CTraverse());
                }

                if (pt = strstr(txt, "nSpecies=")) {
                    pt = strstr(txt, "=");
                    sscanf(&pt[1], "%d", &nChannels);
                }
                // Search for known species
                for (int k = 0; k < nKnownSpecies; ++k) {
                    columnLabel = knownSpecie[k] + "(column)";
                    pt = txt;
                    while (pt = strstr(pt, columnLabel.c_str())) {
                        m_specieName[nofSpecies] = knownSpecie[k];
                        ++nofSpecies;
                        ++pt;
                    }
                }
            }
        }
        fclose(fil);

        if (nofSpecies > 1)
        {
            for (int ii = 1; ii < nofSpecies; ++ii)
            {
                m_traverse.push_back(new mobiledoas::CTraverse());
            }
        }

        return result;
    }

    void CFlux::Reset()
    {
        /*	memset((void*)columnArray, 0, sizeof(double)*2*65536);
            memset((void*)latArray, 0, sizeof(double)*2*65536);
            memset((void*)longitude, 0, sizeof(double)*2*65536);
          memset((void*)altArray, 0, sizeof(double)*2*65536);*/
    }

    bool CFlux::hasValidGPS(int file) {
        if (file == -1)
            file = m_curTraverse;

        if (m_traverse[file]->m_hasGPS)
            return true;

        for (int k = 0; k < m_traverse[file]->m_recordNum; ++k) {
            if (m_traverse[file]->latitude[k] != 0 || m_traverse[file]->longitude[k] != 0) {
                m_traverse[file]->m_hasGPS = true;
                return true;
            }
        }

        return false;
    }

    // used when parsing the log files
    bool CFlux::AssignValueToColumn(long fileIndex, int column, int row, double value[3], int nChannels, double fileVersion) {

        // first make sure that this is not a reevaluation log, they are different
        if (0 == strncmp(m_FileType, "ReEvaluationlog", 15))
            return AssignValueToColumn_ReEvaluationLog(fileIndex, column, row, value, nChannels, fileVersion);

        // the columns that are same for all versions of ev.logs.
        switch (column) {
        case 0:
            for (int k = 0; k < nChannels; ++k) {
                m_traverse[fileIndex + k]->time[row].hour = (char)value[0];
                m_traverse[fileIndex + k]->time[row].minute = (char)value[1];
                m_traverse[fileIndex + k]->time[row].second = (char)value[2];
            }
            return true;
        case 1:
            for (int k = 0; k < nChannels; ++k)
                m_traverse[fileIndex + k]->latitude[row] = value[0];
            return true;
        case 2:
            for (int k = 0; k < nChannels; ++k)
                m_traverse[fileIndex + k]->longitude[row] = value[0];
            return true;
        }

        // the version specific
        if (fileVersion < 4.0) {
            switch (column) {
            case 3: m_traverse[fileIndex]->intensArray[row] = value[0]; return true;
            case 4: m_traverse[fileIndex]->m_nSpectra = (long)value[0]; return true;
            case 5: m_traverse[fileIndex]->columnArray[row] = value[0]; return true;
            case 6: m_traverse[fileIndex]->m_expTime = (long)value[0]; return true;
            }
        }
        else if (fileVersion == 4.0) {
            switch (column) {
            case 3:
                for (int k = 0; k < nChannels; ++k)
                    m_traverse[fileIndex + k]->altitude[row] = value[0];
                return true;
            case 4: m_traverse[fileIndex]->m_nSpectra = (long)value[0]; return true;
            case 5: m_traverse[fileIndex]->m_expTime = (long)value[0]; return true;
            case 6: m_traverse[fileIndex]->intensArray[row] = value[0]; return true;
            case 7: m_traverse[fileIndex]->columnArray[row] = value[0]; return true;
            case 8: m_traverse[fileIndex + 1]->intensArray[row] = value[0]; return true;
            case 9: m_traverse[fileIndex + 1]->columnArray[row] = value[0]; return true;

            }

        }
        else if (fileVersion >= 4.1) {// || fileVersion == 4.2){
            switch (column) {
            case 3:
                for (int k = 0; k < nChannels; ++k)
                    m_traverse[fileIndex + k]->altitude[row] = value[0];
                return true;
            case 4:  m_traverse[fileIndex]->m_nSpectra = (long)value[0]; return true;
            case 5:  m_traverse[fileIndex]->m_expTime = (long)value[0]; return true;
            case 6:  m_traverse[fileIndex]->intensArray[row] = value[0]; return true;
            case 7:  m_traverse[fileIndex]->columnArray[row] = value[0]; return true;
            case 8:	 m_traverse[fileIndex]->columnError[row] = value[0]; return true;
            case 9:  m_traverse[fileIndex + 1]->intensArray[row] = value[0]; return true;
            case 10: m_traverse[fileIndex + 1]->columnArray[row] = value[0]; return true;
            case 11: m_traverse[fileIndex + 1]->columnError[row] = value[0]; return true;
            }

        }

        // no column found
        return false;
    }

    bool CFlux::AssignValueToColumn_ReEvaluationLog(long fileIndex, int column, int row, double value[3], int nChannels, double /*fileVersion*/) {
        const int nCommonColumns = 7;
        const int nColumnsPerSpecie = 6;

        /* if any of the common columns */
        switch (column) {
        case 0:
            for (int k = 0; k < nChannels; ++k) {
                m_traverse[fileIndex + k]->time[row].hour = (char)value[0];
                m_traverse[fileIndex + k]->time[row].minute = (char)value[1];
                m_traverse[fileIndex + k]->time[row].second = (char)value[2];
            }
            return true;
        case 1:
            for (int k = 0; k < nChannels; ++k)
                m_traverse[fileIndex + k]->latitude[row] = value[0];
            return true;
        case 2:
            for (int k = 0; k < nChannels; ++k)
                m_traverse[fileIndex + k]->longitude[row] = value[0];
            return true;
        case 3:
            for (int k = 0; k < nChannels; ++k)
                m_traverse[fileIndex + k]->altitude[row] = value[0];
            return true;
        case 4:
            for (int k = 0; k < nChannels; ++k)
                m_traverse[fileIndex + k]->m_nSpectra = (long)value[0];
            return true;
        case 5:
            for (int k = 0; k < nChannels; ++k)
                m_traverse[fileIndex + k]->m_expTime = (long)value[0];
            return true;
        case 6:
            for (int k = 0; k < nChannels; ++k)
                m_traverse[fileIndex + k]->intensArray[row] = value[0];
            return true;
        }
        /* this column is a description of one specie.
            There are six columns for every specie; COLUMN, COLUMN_ERRROR, SHIFT, SHIFT_ERROR, SQUEEZE, SQUEEZE_ERROR */
        int specieNumber = (column - nCommonColumns) / nColumnsPerSpecie;
        int columnType = (column - nCommonColumns) % nColumnsPerSpecie;

        switch (columnType) {
        case 0: m_traverse[fileIndex + specieNumber]->columnArray[row] = value[0];
            return true;
        case 1:	m_traverse[fileIndex + specieNumber]->columnError[row] = value[0];
            return true;
        case 2:
            return true;
        case 3:
            return true;
        case 4:
            return true;
        case 5:
            return true;
        }
        return false;
    }

    long CFlux::GetCurrentFileName(std::string& str) {
        char buffer[4096];
        sprintf(buffer, "%s", m_traverse[m_curTraverse]->m_fileName.c_str());
        char* pt = strchr(buffer, '*');
        if (pt != 0)
            pt += 2;
        else
            pt = buffer;

        str = m_traverse[m_curTraverse]->m_filePath + std::string("\\") + pt;

        return 0;
    }

    void CFlux::InterpolateWindField(int layer) {
        if (m_curTraverse < 0 || m_traverseNum <= 0)
            return;
        if (m_windField == nullptr || m_useWindField == false)
            return;

        mobiledoas::CTraverse* tr = m_traverse[m_curTraverse];
        std::vector<double> ws(tr->m_recordNum, 0);
        std::vector<double> wd(tr->m_recordNum, 0);

        // do the actual interpolation
        int nPoints = m_windField->Interpolate(tr->latitude, tr->longitude, tr->time, layer, tr->m_recordNum,
            ws.data(), wd.data(), mobiledoas::CWindField::INTERPOLATION_NEAREST);

        if (nPoints < tr->m_recordNum) {
            // MessageBox(NULL, "Failed to interpolate all points in the traverse", "Error", MB_OK);
            return;
        }

        // save the result
        for (int i = 0; i < tr->m_recordNum; ++i) {
            tr->m_windDirection[i] = wd[i];
            tr->m_windSpeed[i] = ws[i];
        }
        tr->m_useWindField = true;
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

