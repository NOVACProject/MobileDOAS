#include "StdAfx.h"
#include "SpectrumIO.h"
#include <MobileDoasLib/GpsData.h>
#include <MobileDoasLib/DateTime.h>
#include <vector>
#include <cmath>

CSpectrumIO::CSpectrumIO()
{
}

CSpectrumIO::~CSpectrumIO()
{
}

int CSpectrumIO::readSTDFile(CString filename, CSpectrum* curSpec) {
    char tmpStr[1024];
    int tmpInt, tmpInt2, tmpInt3, i;
    double tmpDouble;

    FILE* f = fopen(filename, "r");

    if (0 == f) {
        return 1;
    }

    fscanf(f, "%1022s\n", tmpStr);
    if (0 != strncmp(tmpStr, "GDBGMNUP", 8)) {
        fclose(f);
        return 1;
    }

    fscanf(f, "%d\n", &tmpInt);
    if (tmpInt != 1) {
        fclose(f);
        return 1;
    }

    fscanf(f, "%d\n", &tmpInt);
    if (tmpInt > MAX_SPECTRUM_LENGTH || tmpInt < 1) {
        fclose(f);
        return 1;
    }

    curSpec->length = tmpInt;

    /* Read the actual data */
    for (i = 0; i < curSpec->length; ++i) {
        if (0 == fscanf(f, "%lf\n", &tmpDouble)) {
            fclose(f);
            return 1;
        }
        else {
            curSpec->I[i] = tmpDouble;
        }
    }

    // the name of the spectrum, this is ignored at the moment
    if (0 == fgets(tmpStr, 1024, f)) {
        fclose(f);
        return 1;
    }

    // the name of the spectrometer
    if (0 == fgets(tmpStr, 1024, f)) {
        fclose(f);
        return 1;
    }
    else {
        curSpec->spectrometerSerial.Format(tmpStr);
        curSpec->spectrometerSerial.Trim(" \t\r\n");
    }

    // the name of the detector, this is ignored at the moment
    if (0 == fgets(tmpStr, 1024, f)) {
        fclose(f);
        return 1;
    }

    /* Read the date */
    if (3 != fscanf(f, "%d.%d.%d\n", &tmpInt, &tmpInt2, &tmpInt3)) {

    }
    else {
        curSpec->date[0] = tmpInt;
        curSpec->date[1] = tmpInt2;
        curSpec->date[2] = tmpInt3;
    }

    /* Read the start time */
    if (3 != fscanf(f, "%d:%d:%d\n", &tmpInt, &tmpInt2, &tmpInt3)) {

    }
    else {
        curSpec->startTime[0] = tmpInt;
        curSpec->startTime[1] = tmpInt2;
        curSpec->startTime[2] = tmpInt3;
    }

    /* Read the stop time */
    if (3 != fscanf(f, "%d:%d:%d\n", &tmpInt, &tmpInt2, &tmpInt3)) {

    }
    else {
        curSpec->stopTime[0] = tmpInt;
        curSpec->stopTime[1] = tmpInt2;
        curSpec->stopTime[2] = tmpInt3;
    }

    /* some information that is ignored for the moment s*/
    for (i = 0; i < 2; ++i) {
        if (0 == fgets(tmpStr, 1024, f)) {
            fclose(f);
            return 1;
        }
    }

    if (0 == fscanf(f, "SCANS %d\n", &tmpInt)) {
        fclose(f);
        return 1;
    }
    else {
        curSpec->scans = tmpInt;
    }

    if (0 == fscanf(f, "INT_TIME %lf\n", &tmpDouble)) {
        fclose(f);
        return 1;
    }
    else {
        curSpec->exposureTime = (int)tmpDouble;
    }

    /* The site is ignored */
    if ((0 == fgets(tmpStr, 1024, f)) && (0 == strstr(tmpStr, "SITE"))) {
        //  if(0 == fscanf(f, "SITE %s\n", tmpStr)){
        fclose(f);
        return 1;
    }
    else {
    }

    if (0 == fscanf(f, "LONGITUDE %lf\n", &tmpDouble)) {
        fclose(f);
        return 1;
    }
    else {
        curSpec->lon = tmpDouble;
    }

    if (0 == fscanf(f, "LATITUDE %lf\n", &tmpDouble)) {
        fclose(f);
        return 1;
    }
    else {
        curSpec->lat = tmpDouble;
    }

    // ----------- EXTENDED STD ------------------
    // - if the file is in the extended STD-format then we can continue here... -
    char szLine[8192];
    char* pt;
    while (fgets(szLine, 8192, f))
    {
        // Read in altitude
        if (pt = strstr(szLine, "Altitude ="))
        {
            pt = strstr(szLine, "=");
            if (0 < sscanf(&pt[1], "%lf", &tmpDouble)) {
                curSpec->altitude = tmpDouble;
            }
        }

        // Read in GPS status
        if (pt = strstr(szLine, "GPSStatus ="))
        {
            pt = strstr(szLine, "=");
            if (0 < sscanf(&pt[1], "%s", &tmpStr)) {
                curSpec->gpsStatus = tmpStr;
            }
        }

        // Read in GPS speed
        if (pt = strstr(szLine, "Speed ="))
        {
            pt = strstr(szLine, "=");
            if (0 < sscanf(&pt[1], "%lf", &tmpDouble)) {
                curSpec->speed = tmpDouble;
            }
        }

        // Read in GPS course

        if (pt = strstr(szLine, "Course ="))
        {
            pt = strstr(szLine, "=");
            if (0 < sscanf(&pt[1], "%lf", &tmpDouble)) {
                curSpec->course = tmpDouble;
            }
        }
    }

    fclose(f);
    return 0;
}

[[deprecated]]
int CSpectrumIO::readTextFile(CString filename, CSpectrum* curSpec) {
    double tmpDouble1, tmpDouble2;
    int tmpInt = 0;
    int ret;
    CFileException exceFile;
    CStdioFile fileRef;

    if (!fileRef.Open(filename, CFile::modeRead | CFile::typeText, &exceFile)) {
        MessageBox(NULL, TEXT("Can not read log file"), TEXT("Error"), MB_OK);
        return FALSE;
    }

    CString szLine2;
    while (fileRef.ReadString(szLine2)) {
        char* szToken = (char*)(LPCSTR)szLine2;

        while (szToken = strtok(szToken, "\n")) {

            ret = sscanf(szToken, "%lf\t%lf", &tmpDouble1, &tmpDouble2);

            if (ret == 1) {
                curSpec->I[tmpInt] = tmpDouble1;
            }
            else if (ret == 2) {
                curSpec->I[tmpInt] = tmpDouble2;
            }
            else {

            }
            curSpec->length = ++tmpInt;

            szToken = NULL;
        }
    }

    curSpec->scans = 1;

    //  fclose(f);
    return 0;

}

bool CSpectrumIO::WriteStdFile(const CString& fileName, const CSpectrum& spectrum)
{
    int extendedFormat = 1;
    FILE* f = fopen(fileName, "w");
    if (f < (FILE*)1) {
        return FAIL;
    }

    fprintf(f, "GDBGMNUP\n");
    fprintf(f, "1\n");
    fprintf(f, "%ld\n", spectrum.length);

    for (long ii = 0; ii < spectrum.length; ++ii)
    {
        fprintf(f, "%.9lf\n", spectrum.I[ii]);
    }

    // Find the name of the file itself (removing the path)
    CString name;
    name.Format(fileName);
    Common::GetFileName(name);

    fprintf(f, "%s\n", name);                /* The name of the spectrum */
    fprintf(f, "%s\n", spectrum.spectrometerModel);  /* The name of the spectrometer */
    fprintf(f, "%s\n", spectrum.spectrometerSerial);
    fprintf(f, "%s\n", spectrum.date.c_str());
    //fprintf(f, "%02d.%02d.%02d\n", spectrum.date[2], spectrum.date[1], spectrum.date[0]);
    fprintf(f, "%02d:%02d:%02d\n", spectrum.startTime[0], spectrum.startTime[1], spectrum.startTime[2]);
    fprintf(f, "%02d:%02d:%02d\n", spectrum.stopTime[0], spectrum.stopTime[1], spectrum.stopTime[2]);
    fprintf(f, "0.0\n");
    fprintf(f, "0.0\n");
    fprintf(f, "SCANS %ld\n", spectrum.scans);
    fprintf(f, "INT_TIME %ld\n", spectrum.exposureTime);
    fprintf(f, "SITE %s\n", spectrum.name);
    fprintf(f, "LONGITUDE %f\n", spectrum.lon);
    fprintf(f, "LATITUDE %f\n", spectrum.lat);

    if (extendedFormat) {
        double minValue, maxValue;
        spectrum.GetMinMax(minValue, maxValue);
        double average = spectrum.GetAverage();

        fprintf(f, "Altitude = %.1lf\n", spectrum.altitude);
        //fprintf(f, "Author = \"\"\n");
        fprintf(f, "Average = %.1f\n", average);
        //fprintf(f, "AzimuthAngle = 0\n");
        if (!std::isnan(spectrum.course)) {
            fprintf(f, "Course = %.1lf\n", spectrum.course);
        }
        //fprintf(f, "Delta = 0\n");
        //fprintf(f, "DeltaRel = 0\n");
        //fprintf(f, "Deviation = 0\n");
        //fprintf(f, "Device = \"\"\n");
        //fprintf(f, "ElevationAngle = 90\n");
        fprintf(f, "ExposureTime = %ld\n", spectrum.exposureTime);
        fprintf(f, "FileName = %s\n", fileName);
        fprintf(f, "FitHigh = %d\n", spectrum.fitHigh);
        fprintf(f, "FitLow = %d\n", spectrum.fitLow);
        //fprintf(f, "Gain = 0\n");
        if (spectrum.gpsStatus.c_str() != "") {
            fprintf(f, "GPSStatus = %s\n", spectrum.gpsStatus.c_str());
        }
        fprintf(f, "IntegrationMethod = Average\n");
        fprintf(f, "Latitude = %.6lf\n", spectrum.lat);
        //fprintf(f, "LightPath = 0\n");
        //fprintf(f, "LightSource = \"\"\n");
        fprintf(f, "Longitude = %.6lf\n", spectrum.lon);
        fprintf(f, "Marker = %ld\n", spectrum.length / 2);
        fprintf(f, "MathHigh = %ld\n", spectrum.length - 1);
        //fprintf(f, "MathLow = 0\n");
        fprintf(f, "Max = %.1lf\n", maxValue);
        //fprintf(f, "MaxChannel = %ld\n", spectrum.length);
        fprintf(f, "Min = %.1lf\n", minValue);
        //fprintf(f, "MinChannel = 0\n");
        //fprintf(f, "MultiChannelCounter = 0\n");
        fprintf(f, "Name = \"%s\"\n", spectrum.name);
        fprintf(f, "NumScans = %ld\n", spectrum.scans);
        //fprintf(f, "OpticalDensity = 0\n");
        //fprintf(f, "OpticalDensityCenter = %ld\n", spectrum.length / 2);
        //fprintf(f, "OpticalDensityLeft = 0\n");
        //fprintf(f, "OpticalDensityRight = %ld\n", spectrum.length - 1);
        //fprintf(f, "Pressure = 0\n");
        //fprintf(f, "Remark = \"\"\n");
        //fprintf(f, "ScanGeometry = 0\n"); //(DoasCore.Math.ScanGeometry)SAZ: 137.41237083135 SZA: 31.5085943481828 LAZ: 298.523110145623 LAZ: 129.285101310559 Date: 1/5/2007 10:35:07 Lat.: 0 Lon.: 0\n");
        //fprintf(f, "ScanMax = 0\n");	
        if (!std::isnan(spectrum.speed)) {
            fprintf(f, "Speed = %.1lf\n", spectrum.speed);
        }
        if (!std::isnan(spectrum.boardTemperature)) {
            fprintf(f, "BoardTemperature = %lf\n", spectrum.boardTemperature);
        }
        if (!std::isnan(spectrum.boardTemperature)) {
            fprintf(f, "DetectorTemperature = %lf\n", spectrum.detectorTemperature);
        }
        //fprintf(f, "Variance = 0\n");
    }

    fclose(f);

    return SUCCESS;
}

