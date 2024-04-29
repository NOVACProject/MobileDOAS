// Communication.cpp: implementation of the CSpectrometer class.
//
//////////////////////////////////////////////////////////////////////

#undef min
#undef max

#include "stdafx.h" 
#include <Mmsystem.h> // used for PlaySound
#include "DMSpec.h"
#include "Spectrometer.h"
#include <MobileDoasLib/DateTime.h>
#include "Common/SpectrumIO.h"
#include <algorithm>
#include "Dialogs/SelectionDialog.h"
#include <SpectralEvaluation/StringUtils.h>
#include <SpectralEvaluation/Spectra/SpectrumInfo.h>
#include "Evaluation/RealTimeCalibration.h"
#include <MobileDoasLib/Measurement/SpectrumUtils.h>
#include <MobileDoasLib/Flux/Flux1.h>
#include <SpectralEvaluation/StringUtils.h>
#include <sstream>

extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CSpectrometer::CSpectrometer(
    CView& mainForm,
    std::unique_ptr<mobiledoas::SpectrometerInterface> spectrometerInterface,
    std::unique_ptr<Configuration::CMobileConfiguration> configuration)
    : m_mainForm(mainForm), m_useGps(true), m_scanNum(0), m_spectrumCounter(0)
{
    sprintf(m_GPSPort, "COM5");

    m_spectrometer = std::move(spectrometerInterface);
    m_conf = std::move(configuration);

    m_isRunning = true;
    m_spectrometerMode = MODE_TRAVERSE; // default mode
    m_posFlag = 1;
    m_zeroPosNum = 0;
    m_timeResolution = 0;
    m_windSpeed = 1.0;
    m_windAngle = 0;
    m_detectorSize = 2048;
    m_spectrometerDynRange = 4095;
    m_spectrometerModel = "Unknown";
    m_flux = 0; //initiate flux value in this variable
    m_maxColumn = 100;
    m_integrationTime = 100;
    m_adjustIntegrationTime = FALSE;
    m_gasFactor = GASFACTOR_SO2;

    m_fitRegionNum = 1;
    for (int k = 0; k < MAX_FIT_WINDOWS; ++k)
    {
        m_fitRegion[k].eval[0] = nullptr;
        m_fitRegion[k].eval[1] = nullptr;
    }

    m_spectrometerName = TEXT("..........");

    m_NChannels = 1;
    m_spectrometerChannel = 0;

    m_lastDarkOffset[0] = 0;
    m_lastDarkOffset[1] = 0;

    m_timeDiffToUtc = 0;

    m_fitRegion[0].window.nRef = 0;
    m_fitRegion[0].window.specLength = MAX_SPECTRUM_LENGTH;

    m_gps = nullptr;

    // Make an initial guess of the spectrometer wavelengths
    m_wavelength.Resize(2, MAX_SPECTRUM_LENGTH);
    for (int k = 0; k < MAX_SPECTRUM_LENGTH; ++k)
    {
        m_wavelength.data[0][k] = k;
        m_wavelength.data[1][k] = k;
    }
}

CSpectrometer::~CSpectrometer()
{
    for (int k = 0; k < MAX_FIT_WINDOWS; ++k)
    {
        delete m_fitRegion[k].eval[0];
        m_fitRegion[k].eval[0] = nullptr;

        delete m_fitRegion[k].eval[1];
        m_fitRegion[k].eval[1] = nullptr;
    }

    if (m_gps != nullptr)
    {
        delete m_gps;
        m_gps = nullptr;
    }
}


long CSpectrometer::GetTimeValue_UMT()
{
    struct tm* tim;
    long timeValue;
    time_t t;
    time(&t);
    tim = localtime(&t);

    /* tim is the local time according to the computer clock
        We now need to adjust it to UMT using the time difference between the
        GPS-time and computer time that we (should have) retrieved
        at the first GPS-reading */
    long tid = m_timeDiffToUtc + 3600 * tim->tm_hour + 60 * tim->tm_min + tim->tm_sec;
    int hr = (int)tid / 3600;
    int mi = (int)(tid / 60 - 60 * hr);
    int se = (int)(tid - 3600 * hr - 60 * mi);

    timeValue = hr * 10000 + mi * 100 + se;
    return timeValue;
}

int CSpectrometer::Scan(int sumInComputer, int sumInSpectrometer, mobiledoas::MeasuredSpectrum& result)
{
    result.SetToZero();

    // set point temperature for CCD if supported.
    if (m_spectrometer->SupportsDetectorTemperatureControl())
    {
        m_spectrometer->EnableDetectorTemperatureControl(true, m_conf->m_setPointTemperature);
    }

    // Set the parameters for acquiring the spectrum
    m_spectrometer->SetIntegrationTime(m_integrationTime * 1000);
    m_spectrometer->SetScansToAverage(sumInSpectrometer);

    // Get the spectrum
    for (int readoutNumber = 0; readoutNumber < sumInComputer; ++readoutNumber)
    {
        if (!m_isRunning)
        {
            return 1; // abort the spectrum collection
        }

        // Retreives the spectra from the spectrometer, one vector per channel.
        std::vector<std::vector<double>> spectrumData;
        const int spectrumLength = m_spectrometer->GetNextSpectrum(spectrumData);

        // Handle errors while reading out the spectrum
        if (spectrumLength == 0)
        {
            if (IsSpectrometerDisconnected())
            {
                ReconnectWithSpectrometer();
            }
            return 0;
        }

        // copies the spectrum-values to the output array
        result.Resize((int)spectrumData.size(), (int)spectrumData[0].size());
        for (size_t chn = 0; chn < spectrumData.size(); ++chn)
        {
            for (size_t pixelIdx = 0; pixelIdx < spectrumData[chn].size(); ++pixelIdx)
            {
                result.data[chn][pixelIdx] += spectrumData[chn][pixelIdx];
            }
        }
    }

    // make the spectrum an average 
    if (sumInComputer > 0)
    {
        for (int chn = 0; chn < result.NumberOfChannels(); ++chn)
        {
            for (int pixelIdx = 0; pixelIdx < result.SpectrumLength(); ++pixelIdx)
            {
                result.data[chn][pixelIdx] /= sumInComputer;
            }
        }
    }

    // Check the status of the last readout
    // TODO: Check how to do this with the SpectrometerInterface
    // WrapperExtensions ext = m_wrapper.getWrapperExtensions();
    // if (!ext.isSpectrumValid(m_spectrometerIndex))
    // {
    //     if (IsSpectrometerDisconnected())
    //     {
    //         ReconnectWithSpectrometer();
    //     }
    //     return 0;
    // }

    return 0;
}

//-----------------------------------------------------------------
void CSpectrometer::SetUserParameters(double windspeed, double winddirection, char* baseName)
{

    m_windSpeed = windspeed;
    m_windAngle = winddirection;

    m_measurementBaseName.Format("%s", baseName);
    m_measurementBaseName.TrimLeft(' ');
    m_measurementBaseName.TrimRight(' ');
}

void CSpectrometer::ApplyEvaluationSettings()
{
    for (int fitRegionIdx = 0; fitRegionIdx < MAX_FIT_WINDOWS; ++fitRegionIdx)
    {
        m_fitRegion[fitRegionIdx].window = m_conf->m_fitWindow[fitRegionIdx];

        for (int channelIdx = 0; channelIdx < 2; ++channelIdx)
        {
            if (m_fitRegion[fitRegionIdx].eval[channelIdx] != nullptr)
            {
                delete m_fitRegion[fitRegionIdx].eval[channelIdx];
            }
            m_fitRegion[fitRegionIdx].eval[channelIdx] = new Evaluation::CEvaluation();
        }

        // the offset
        m_fitRegion[fitRegionIdx].window.offsetFrom = m_conf->m_offsetFrom;
        m_fitRegion[fitRegionIdx].window.offsetTo = m_conf->m_offsetTo;
    }

    this->m_fitRegionNum = m_conf->m_nFitWindows;
}

void CSpectrometer::ApplySettings()
{
    CString msg;

    m_NChannels = m_conf->m_nChannels;
    bool error = false;
    if (m_NChannels > MAX_N_CHANNELS)
    {
        msg.Format("Cannot handle more than %d channels. Changed number of channels to %d", MAX_N_CHANNELS, MAX_N_CHANNELS);
        m_NChannels = MAX_N_CHANNELS;   error = true;
    }
    if (m_NChannels < 0)
    {
        msg.Format("A negative amount of channels defined in the configuration file. This does not make sense, will change number of channels to 1");
        m_NChannels = 1;  error = true;
    }
    if (m_NChannels == 0)
    {
        msg.Format("Zero channels defined in the configuration file. This does not make sense, will change number of channels to 1");
        m_NChannels = 1; error = true;
    }
    if (m_spectrometerMode == MODE_WIND && m_NChannels < 2)
    {
        msg.Format("%d channels defined in the configuration file, this does not agree with wind-measurements. Will change number of channels to 2", m_NChannels);
        m_NChannels = 2; error = true;
    }
    if (error)
    {
        ShowMessageBox(msg, "Error in settings");
    }

    // The GPS-settings
    m_GPSBaudRate = m_conf->m_gpsBaudrate;
    sprintf(m_GPSPort, "%s", m_conf->m_gpsPort.c_str());
    m_useGps = (m_conf->m_useGPS != 0);

    // The exposure-time settings
    if (m_conf->m_expTimeMode == Configuration::CMobileConfiguration::EXPOSURETIME_FIXED)
    {
        m_fixexptime = m_conf->m_fixExpTime;
    }
    else if (m_conf->m_expTimeMode == Configuration::CMobileConfiguration::EXPOSURETIME_ADAPTIVE)
    {
        m_fixexptime = -1;
    }
    else
    {
        m_fixexptime = 0;
    }
    this->m_desiredSaturationRatio = std::max(0.0, std::min(1.0, m_conf->m_percent / 100.0));
    this->m_timeResolution = m_conf->m_timeResolution;

    // The settings for the evaluation
    ApplyEvaluationSettings();

    // Audio Settings
    this->m_useAudio = m_conf->m_useAudio;
    this->m_maxColumn = m_conf->m_maxColumn;
}

/* makes a test of the settings, returns 0 if all is ok, else 1 */
int CSpectrometer::CheckSettings()
{
    CString msgStr, fileName;
    double tmpDouble1, tmpDouble2, tmpDouble3;
    char buf[512];
    int  nColumns;

    // 1. Check so that there are at least one reference-file defined
    if (m_fitRegion[0].window.nRef <= 0)
    {
        ShowMessageBox("There are no reference-files defined in the configuration file. Please check settings and restart", "Error");
        return 1;
    }

    //2. Check so that the reference-files exists, can be read and
    //  have the correct format.
    for (int k = 0; k < m_fitRegion[0].window.nRef; ++k)
    {
        int nRows = 0;
        FILE* f = fopen(m_fitRegion[0].window.ref[k].m_path.c_str(), "r");
        if (nullptr == f)
        {
            fileName.Format("%s%s", (LPCSTR)g_exePath, m_fitRegion[0].window.ref[k].m_path.c_str());
            f = fopen(fileName, "r");
            if (nullptr == f)
            {
                msgStr.Format("Cannot open reference file : %s for reading.", m_fitRegion[0].window.ref[k].m_path.c_str());
                ShowMessageBox(msgStr, "Error");
                return 1;
            }
        }

        while (fgets(buf, 511, f))
        {
            nColumns = sscanf(buf, "%lf\t%lf\t%lf", &tmpDouble1, &tmpDouble2, &tmpDouble3);
            if (nColumns == 0 || nColumns == EOF)
                break;

            ++nRows;
            if (nColumns == 3)
            {
                msgStr.Format("There are %d columns in the reference file. This programs wants reference files with one or two columns", nColumns);
                ShowMessageBox(msgStr, "Error");
                fclose(f);
                return 1;
            }
        }

        fclose(f);

        if (nRows > MAX_SPECTRUM_LENGTH)
        {
            msgStr.Format("Length of the reference file is: %d values. Cannot handle references with more than %d data-points.", nRows, MAX_SPECTRUM_LENGTH);
            ShowMessageBox(msgStr, "Error");
            return 1;
        }
    }

    // 3. If there is a reference file with the name SO2, the it should
    //  be the first reference file (since the main window will only
    //  show the result of evaluating the first reference-file)
    if (!EqualsIgnoringCase(m_fitRegion[0].window.ref[0].m_specieName, "SO2"))
    {
        int k;
        for (k = 1; k < m_fitRegion[0].window.nRef; ++k)
        {
            if (EqualsIgnoringCase(m_fitRegion[0].window.ref[k].m_specieName, "SO2"))
                break;
        }
        if (k != m_fitRegion[0].window.nRef)
        {
            novac::CReferenceFile tmpRef = m_fitRegion[0].window.ref[0];
            m_fitRegion[0].window.ref[0] = m_fitRegion[0].window.ref[k];
            m_fitRegion[0].window.ref[k] = tmpRef;
        }
    }

    return 0;
}



/**This function is to write the log file to record evaluation result and time
**
*/

void CSpectrometer::WriteEvFile(CString filename, FitRegion* fitRegion)
{
    int channel = fitRegion->window.channel;

    FILE* f;
    CString wholePath = m_subFolder + "\\" + m_measurementBaseName + "_" + m_measurementStartTimeStr + filename;
    f = fopen(wholePath, "a+");
    if (f < (FILE*)1)
    {
        ShowMessageBox("Could not open evaluation log file. No data was written!", "Error");
        return;
    }

    int hr, min, sec;
    mobiledoas::ExtractTime(m_spectrumGpsData[m_spectrumCounter], hr, min, sec);

    // 1. Write the time of the spectrum
    fprintf(f, "%02d:%02d:%02d\t", hr, min, sec);

    // 2. Write the GPS-information about the spectrum
    fprintf(f, "%f\t%f\t%.1f\t", m_spectrumGpsData[m_spectrumCounter].latitude, m_spectrumGpsData[m_spectrumCounter].longitude, m_spectrumGpsData[m_spectrumCounter].altitude);

    // 3. The number of spectra averaged and the exposure-time
    fprintf(f, "%ld\t%d\t", NumberOfSpectraToAverage(), m_integrationTime);

    // 4. The intensity
    fprintf(f, "%ld\t", m_averageSpectrumIntensity[channel]);

    // 5. The evaluated column values
    for (int k = 0; k < fitRegion->window.nRef; ++k)
    {
        Evaluation::EvaluationResult result = fitRegion->eval[channel]->GetResult(k);
        fprintf(f, "%lf\t%lf\t", result.column, result.columnError);
    }

    // 6. The std-file
    fprintf(f, "%s\n", (LPCTSTR)m_stdfileName[channel]);

    fclose(f);
}

/** This function is to adjust the integration time
**@pSky, @pDark - the average intensity in the sky and dark spectra
@intT - the integration time used when measuring the sky and dark spectra
*/
long CSpectrometer::GetInttime(long pSky, long pDark, int intT)
{
    double xRate;
    long intTime = intT;

    if (m_fixexptime <= 0)
    {
        xRate = (m_spectrometerDynRange * m_desiredSaturationRatio - (double)pDark) / (double)pSky;

        if (xRate == 0.0)
            intTime = MAX_EXPOSURETIME;
        else
        {
            intTime = (long)(xRate * intT);
            if (intTime > MAX_EXPOSURETIME)
            {
                intTime = MAX_EXPOSURETIME;
            }
            if (intTime < MIN_EXPOSURETIME)
                intTime = MIN_EXPOSURETIME;
        }

    }
    else
    {
        intTime = m_fixexptime;
        if (intTime < MIN_EXPOSURETIME)
            intTime = MIN_EXPOSURETIME;
        else if (intTime > 65535)
            intTime = 65535;
    }


    return intTime;
}



/** This function is to count the flux between two points
** if the position is (0,0), calculate average distance when the coordinates go above 0.
** @windAngle - wind angle with the North
** @windSpeed - wind speed
*/

double CSpectrometer::CountFlux(double windSpeed, double windAngle)
{
    double flux;
    double column, windFactor, distance, lat1, lat2, lon1, lon2;
    long   columnSize;
    double  accColumn = 0; //total column when gps position is 0
    if (m_posFlag == 0)  // when the gps coordinate is (0,0)
    {
        m_zeroPosNum++;
        columnSize = m_fitRegion[0].vColumn[0].GetSize();
        accColumn += 1E-6 * (m_fitRegion[0].vColumn[0].GetAt(columnSize - 1)) * m_gasFactor;
        flux = 0;
        m_flux += flux;
        WriteFluxLog();
        return flux;
    }
    else
    {
        columnSize = m_fitRegion[0].vColumn[0].GetSize();
        lat1 = m_spectrumGpsData[m_spectrumCounter - 1].latitude;
        lat2 = m_spectrumGpsData[m_spectrumCounter - 2].latitude;
        lon1 = m_spectrumGpsData[m_spectrumCounter - 1].longitude;
        lon2 = m_spectrumGpsData[m_spectrumCounter - 2].longitude;

        if ((lat2 == 0) && (lon2 == 0)) // when the gps coordinate just become not equal to (0,0)
        {
            lat2 = m_spectrumGpsData[m_spectrumCounter - 2 - m_zeroPosNum].latitude;
            lon2 = m_spectrumGpsData[m_spectrumCounter - 2 - m_zeroPosNum].longitude;
            distance = mobiledoas::GPSDistance(lat1, lon1, lat2, lon2) / (m_zeroPosNum + 1);
            column = 1E-6 * (m_fitRegion[0].vColumn[0].GetAt(columnSize - 1)) * m_gasFactor + accColumn;
        }
        else     // the gps coordinate is not (0,0)
        {
            distance = mobiledoas::GPSDistance(lat1, lon1, lat2, lon2);
            column = 1E-6 * (m_fitRegion[0].vColumn[0].GetAt(columnSize - 1)) * m_gasFactor;
        }
        accColumn = 0;
        m_zeroPosNum = 0;
        windFactor = mobiledoas::GetWindFactor(lat1, lon1, lat2, lon2, windAngle);
        flux = column * distance * windSpeed * windFactor;

        m_flux += flux;
        WriteFluxLog();
        return flux;
    }
}

void CSpectrometer::GetLastColumn(mobiledoas::ReferenceFitResult& evaluationResult)
{

    evaluationResult.m_column = evaluateResult[0][0][0];
    evaluationResult.m_columnError = evaluateResult[0][0][1];
    evaluationResult.m_shift = evaluateResult[0][0][2];
    evaluationResult.m_shiftError = evaluateResult[0][0][3];
    evaluationResult.m_squeeze = evaluateResult[0][0][4];
    evaluationResult.m_squeezeError = evaluateResult[0][0][5];
}

void CSpectrometer::GetDark()
{
    m_NChannels = std::max(std::min(m_NChannels, MAX_N_CHANNELS), 1);

    if (m_fixexptime >= 0)
    {
        m_dark.CopyTo(m_tmpDark);
    }
    else
    {
        // Adaptive exposure-time
        m_offset.CopyTo(m_tmpDark);
        for (int j = 0; j < m_NChannels; ++j)
        {
            for (int i = 0; i < m_offset.SpectrumLength(); ++i)
            {
                m_tmpDark.data[j][i] += m_darkCur.data[j][i] * (m_integrationTime / DARK_CURRENT_EXPTIME);
            }
        }
    }
}

void CSpectrometer::GetSky()
{
    m_NChannels = std::max(std::min(m_NChannels, MAX_N_CHANNELS), 1);

    m_sky.CopyTo(m_tmpSky);
}

int CSpectrometer::Stop()
{
    m_spectrometer->Close();

    // Stop this thread
    m_isRunning = false;

    // Also stop the GPS-reading thread
    if (m_gps != nullptr)
    {
        m_gps->Stop();
    }

    return 1;
}

int CSpectrometer::Start()
{
    m_isRunning = true;
    return 1;
}

/* Return value = 0 if all is ok, else 1 */
int CSpectrometer::ReadReferenceFiles()
{
    UpdateStatusBarMessage("Reading References");

    for (int fitRegionIdx = 0; fitRegionIdx < m_fitRegionNum; ++fitRegionIdx)
    {
        CString refFileList[10];
        for (int referenceIdx = 0; referenceIdx < m_fitRegion[fitRegionIdx].window.nRef; ++referenceIdx)
        {
            if (IsExistingFile(m_fitRegion[fitRegionIdx].window.ref[referenceIdx].m_path.c_str()))
            {
                refFileList[referenceIdx].Format("%s", m_fitRegion[fitRegionIdx].window.ref[referenceIdx].m_path.c_str());
            }
            else
            {
                CString tmpFileName;
                tmpFileName.Format("%s\\%s", (LPCSTR)g_exePath, m_fitRegion[fitRegionIdx].window.ref[referenceIdx].m_path.c_str());
                if (IsExistingFile(tmpFileName))
                {
                    refFileList[referenceIdx].Format("%s", (LPCSTR)tmpFileName);
                }
                else
                {
                    CString errorMessage;
                    errorMessage.Format("Can not read reference file\n %s\n Please check the file location and restart collection", m_fitRegion[fitRegionIdx].window.ref[referenceIdx].m_path.c_str());
                    ShowMessageBox(errorMessage, "Error");
                    return 1;
                }
            }
        }

        /* Init the Master Channel Evaluator */
        if (m_fitRegion[fitRegionIdx].eval[0] == nullptr)
        {
            m_fitRegion[fitRegionIdx].eval[0] = new Evaluation::CEvaluation(); // should in fact not happen as this should have been done in ApplyEvaluationSettings()
        }
        if (!(m_fitRegion[fitRegionIdx].eval[0]->ReadRefList(refFileList, m_fitRegion[fitRegionIdx].window.nRef, m_detectorSize)))
        {
            ShowMessageBox("Can not read reference file\n Please check the file location and restart collection", "Error");
            return 1;
        }

        /* Init the 1:st Slave Channel Evaluator */
        if (m_fitRegion[fitRegionIdx].eval[1] == nullptr)
        {
            m_fitRegion[fitRegionIdx].eval[1] = new Evaluation::CEvaluation(); // should in fact not happen as this should have been done in ApplyEvaluationSettings()
        }
        if (!(m_fitRegion[fitRegionIdx].eval[1]->ReadRefList(refFileList, m_fitRegion[fitRegionIdx].window.nRef, m_detectorSize)))
        {
            ShowMessageBox("Can not read reference file\n Please check the file location and restart collection", "Error");
            return 1;
        }
    }

    return 0;
}

void CSpectrometer::DoEvaluation(mobiledoas::MeasuredSpectrum& sky, mobiledoas::MeasuredSpectrum& dark, mobiledoas::MeasuredSpectrum& spectrum)
{
    double curColumn[8];
    double curColumnError[8];

    m_fitResult.SetToZero();

    for (int fitRegionIdx = 0; fitRegionIdx < m_fitRegionNum; ++fitRegionIdx)
    {
        const int chn = m_fitRegion[fitRegionIdx].window.channel;

        // Evaluate
        m_fitRegion[fitRegionIdx].eval[chn]->Evaluate(dark[chn].data(), sky[chn].data(), spectrum[chn].data());

        // Store the results
        const auto evaluationResult = m_fitRegion[fitRegionIdx].eval[chn]->GetResult();
        evaluateResult[fitRegionIdx][chn][0] = evaluationResult.column;
        evaluateResult[fitRegionIdx][chn][1] = evaluationResult.columnError;
        evaluateResult[fitRegionIdx][chn][2] = evaluationResult.shift;
        evaluateResult[fitRegionIdx][chn][3] = evaluationResult.shiftError;
        evaluateResult[fitRegionIdx][chn][4] = evaluationResult.squeeze;
        evaluateResult[fitRegionIdx][chn][5] = evaluationResult.squeezeError;

        curColumn[chn] = evaluateResult[fitRegionIdx][chn][0];
        curColumnError[chn] = evaluateResult[fitRegionIdx][chn][1];

        // Save the results in the lists
        for (int i = 0; i < 6; ++i)
        {
            m_fitRegion[fitRegionIdx].vColumn[i].Append(evaluateResult[fitRegionIdx][chn][i]);
        }

        // copy the high pass filtered spectrum
        m_spectrum.data[chn] = m_fitRegion[fitRegionIdx].eval[chn]->m_filteredSpectrum;

        // copy the fitted reference
        for (int referenceIdx = 0; referenceIdx < m_fitRegion[fitRegionIdx].window.nRef + 1; ++referenceIdx)
        {
            for (int pixel = m_fitRegion[fitRegionIdx].window.fitLow; pixel < m_fitRegion[fitRegionIdx].window.fitHigh; ++pixel)
            {
                m_fitResult[fitRegionIdx][pixel] += m_fitRegion[fitRegionIdx].eval[chn]->m_fitResult[referenceIdx].GetAt(pixel);
            }
        }

        CString fileName;
        fileName.Format("evaluationLog_%s.txt", (LPCSTR)m_fitRegion[fitRegionIdx].window.name);
        WriteEvFile(fileName, &m_fitRegion[fitRegionIdx]);

    }
    if (m_useAudio)
    {
        Sing(curColumn[0] / m_maxColumn);
    }
    OnNewColumnMeasurement();

    ++m_spectrumCounter;
    if (m_spectrumCounter == 65535)
    {
        memset((void*)m_spectrumGpsData, 0, sizeof(struct mobiledoas::GpsData) * 65536);
        m_spectrumCounter = 0;
        m_zeroPosNum = 0;
    }
}


/**Create directories for the data files
*
*/

void CSpectrometer::CreateDirectories()
{
    char dateText[11];
    char cDateTime[14];
    struct tm* tim;
    time_t t;

    int strLength = m_measurementBaseName.GetLength();
    if (strLength == 0)
    {
        m_measurementBaseName = TEXT("base1");
    }

    time(&t);
    tim = localtime(&t);
    sprintf(cDateTime, "%04d%02d%02d_%02d%02d", tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday, tim->tm_hour, tim->tm_min);

    mobiledoas::GetDateText(dateText);

    CString folderName(g_exePath + dateText);

    if (0 == CreateDirectory(folderName, NULL))
    {
        DWORD errorCode = GetLastError();
        if (errorCode != ERROR_ALREADY_EXISTS)
        { /* We shouldn't quit just because the directory that we want to create already exists. */
            CString tmpStr, errorStr;
            if (FormatErrorCode(errorCode, errorStr))
            {
                tmpStr.Format("Could not create output directory. Reason: %s", (LPCSTR)errorStr);
            }
            else
            {
                tmpStr.Format("Could not create output directory, not enough free disk space?. Error code returned %ld", errorCode);
            }
            ShowMessageBox(tmpStr, "ERROR");
            exit(EXIT_FAILURE);
        }
    }

    m_measurementStartTimeStr = TEXT(cDateTime);
    m_subFolder = folderName + TEXT("\\") + m_measurementBaseName + TEXT("_") + TEXT(cDateTime);

    if (GetFileAttributes(m_subFolder) == INVALID_FILE_ATTRIBUTES
        && 0 == CreateDirectory(m_subFolder, NULL))
    {
        DWORD errorCode = GetLastError();
        CString tmpStr, errorStr;
        if (FormatErrorCode(errorCode, errorStr))
        {
            tmpStr.Format("Could not create output directory. Reason: %s", errorStr);
        }
        else
        {
            tmpStr.Format("Could not create output directory, not enough free disk space?. Error code returned %ld", errorCode);
        }
        ShowMessageBox(tmpStr, "ERROR");
        exit(EXIT_FAILURE);
    }
}




void CSpectrometer::SetFileName()
{
    static long lastidx = 0; /* The number of the last std-file written */

    int i, j;

    if (m_fixexptime >= 0)
    {
        // Normal mode, fixed exposure time
        for (j = 0; j < m_NChannels; ++j)
        {
            i = lastidx;
            do
            {
                if (m_scanNum == 1)
                    m_stdfileName[j].Format("\\dark_%d.STD", j);
                else if (m_scanNum == 2)
                    m_stdfileName[j].Format("\\sky_%d.STD", j);
                else
                    m_stdfileName[j].Format("\\%05d_%d.STD", i, j);

                m_stdfileName[j] = m_subFolder + m_stdfileName[j];
                i++;
            } while (IsExistingFile(m_stdfileName[j]));
        }
    }
    else
    {
        // Automatic adjustment of the exposure-time
        for (j = 0; j < m_NChannels; ++j)
        {
            i = lastidx;
            do
            {
                if (m_scanNum == 1)
                    m_stdfileName[j].Format("\\offset_%d.STD", j);
                else if (m_scanNum == 2)
                    m_stdfileName[j].Format("\\darkcur_%d.STD", j);
                else if (m_scanNum == 3)
                    m_stdfileName[j].Format("\\sky_%d.STD", j);
                else
                    m_stdfileName[j].Format("\\%05d_%d.STD", i, j);

                m_stdfileName[j] = m_subFolder + m_stdfileName[j];
                i++;
            } while (IsExistingFile(m_stdfileName[j]));
        }
    }

    lastidx = i - 1;
}

long CSpectrometer::GetColumns(std::vector<double>& list, long maxNumberOfValues, int fitRegion) const
{

    const long numberOfValuesAvailable = m_fitRegion[fitRegion].vColumn[0].GetSize();
    const long numberOfValuesToRead = std::min(numberOfValuesAvailable, maxNumberOfValues);

    if (numberOfValuesAvailable > 0)
    {
        for (int i = 0; i < numberOfValuesToRead; ++i)
        {
            list[i] = m_fitRegion[fitRegion].vColumn[0].GetAt(numberOfValuesAvailable - numberOfValuesToRead + i);
        }
    }

    return numberOfValuesToRead;
}

long CSpectrometer::GetColumnErrors(std::vector<double>& list, long maxNumberOfValues, int fitRegion) const
{

    const long numberOfValuesAvailable = m_fitRegion[fitRegion].vColumn[0].GetSize();
    const long numberOfValuesToRead = std::min(numberOfValuesAvailable, maxNumberOfValues);

    if (numberOfValuesAvailable > 0)
    {
        for (int i = 0; i < numberOfValuesToRead; ++i)
        {
            list[i] = m_fitRegion[fitRegion].vColumn[1].GetAt(numberOfValuesAvailable - numberOfValuesToRead + i);
        }
    }

    return numberOfValuesToRead;
}

long CSpectrometer::GetLatLongAlt(double* la, double* lo, double* al, long sum)
{
    int i = 0;

    /* Check that we dont return more values than what we've got */
    long columnSize = m_fitRegion[0].vColumn[0].GetSize();
    if (columnSize > 0)
    {
        if (sum > columnSize)
            sum = columnSize;

        for (i = 0; i < sum; i++)
        {
            if (la != 0)
                la[i] = m_spectrumGpsData[i].latitude;
            if (lo != 0)
                lo[i] = m_spectrumGpsData[i].longitude;
            if (al != 0)
                al[i] = m_spectrumGpsData[i].altitude;
        }
    }

    return i;
}

std::vector<double> CSpectrometer::GetIntensityRegion() const
{
    const auto region = mobiledoas::GetIntensityMeasurementRegion(m_conf->m_specCenter, m_conf->m_specCenterHalfWidth, this->m_detectorSize);

    std::vector<double> result;
    for (long ii = region.first; ii < region.second; ++ii)
    {
        result.push_back(static_cast<double>(ii));
    }
    return result;
}

int CSpectrometer::GetGpsPos(mobiledoas::GpsData& data) const
{
    const int c = this->m_spectrumCounter; // local buffer, to avoid race conditions

    data = m_spectrumGpsData[c];

    return c;
}

bool CSpectrometer::GpsGotContact() const
{

    if (m_gps == nullptr)
    {
        return false;
    }

    return this->m_gps->GotContact();
}

long CSpectrometer::GetIntensity(std::vector<double>& list, long sum) const
{
    const long size = static_cast<long>(m_intensityOfMeasuredSpectrum.size());
    const long numberOfValues = std::min(sum, size);

    for (long i = 0; i < numberOfValues; i++)
    {
        list[i] = m_intensityOfMeasuredSpectrum[size - sum + i];
    }

    return numberOfValues;
}

long CSpectrometer::GetColumnNumber()
{
    long columnSize = m_fitRegion[0].vColumn[0].GetSize();
    return columnSize;
}

void CSpectrometer::GetNSpecAverage(int& averageInSpectrometer, int& averageInComputer)
{
    averageInSpectrometer = this->m_sumInSpectrometer;
    averageInComputer = this->m_sumInComputer;
}

void CSpectrometer::WriteFluxLog()
{
    FILE* f;
    CString fileName = m_subFolder + "\\" + m_measurementBaseName + "_" + m_measurementStartTimeStr + TEXT("FluxLog.txt");
    f = fopen(fileName, "a+");

    if (f < (FILE*)1)
        return;

    fprintf(f, "%f\n", m_flux);

    fclose(f);
}

void CSpectrometer::Sing(double factor)
{
    CString fileToPlay;
    TCHAR windowsDir[MAX_PATH + 1];
    GetWindowsDirectory(windowsDir, MAX_PATH + 1);
    DWORD multiplier = (DWORD)(0xFFFF);
    if (factor > 0)
    {
        multiplier = (DWORD)(0xFFFF * factor);
    }
    else
    {
        multiplier = (DWORD)(0xFFFF * (fabs(factor) + 0.0000001));
    }

    WAVEOUTCAPS pwoc;
    waveOutGetDevCaps(0, &pwoc, sizeof(WAVEOUTCAPS));
    if (pwoc.dwSupport & WAVECAPS_PITCH)  // check if device can set pitch
    {
        // See https://msdn.microsoft.com/en-us/library/Dd743872(v=VS.85).aspx
        MMRESULT res = waveOutSetPitch(0, multiplier);
    }
    else
    {
        MMRESULT res = waveOutSetVolume(0, multiplier); // nope, change volume instead
    }
    fileToPlay.Format("%s\\Media\\ringout.wav", windowsDir);

    PlaySound(fileToPlay, 0, SND_SYNC);
}

bool CSpectrometer::UpdateGpsData(mobiledoas::GpsData& gpsInfo)
{
    // If GPS thread does not exist or is not running
    if (nullptr == m_gps)
    {
        return false;
    }

    // Read the data from the GPS
    m_gps->Get(gpsInfo);
    m_spectrumGpsData[m_spectrumCounter] = gpsInfo;

    // check for valid lat/lon
    bool gpsDataIsValid = IsValidGpsData(gpsInfo);

    if (!m_gps->GotContact())
    {
        gpsDataIsValid = false;
    }

    // Check if the gps-readout seems to be stuck, which can happen at times.
    // Previously this check was done on two consecutive data-points, however that does not work if the time resolution is 
    // so high that multiple spectra are read out on a given second. Current check should be good for readouts up to 10 spectra/second
    if (m_spectrumCounter >= 10 && (m_spectrumGpsData[m_spectrumCounter].time == m_spectrumGpsData[m_spectrumCounter - 10].time))
    {
        gpsDataIsValid = false;
    }

    this->UpdateGpsLocation();

    return gpsDataIsValid;
}

void CSpectrometer::GetCurrentDateAndTime(std::string& currentDate, long& currentTime)
{
    mobiledoas::GpsData currentGpsInfo;
    const bool couldReadValidGPSData = (m_useGps) ? UpdateGpsData(currentGpsInfo) : false;
    if (couldReadValidGPSData)
    {
        currentDate = GetDate(currentGpsInfo, '.');
        currentTime = GetTime(currentGpsInfo);
    }
    else
    {
        currentDate = mobiledoas::GetCurrentDateFromComputerClock('.');
        currentTime = GetCurrentTimeFromComputerClock();
    }

    if (currentGpsInfo.date == 0)
    {
        currentDate = mobiledoas::GetCurrentDateFromComputerClock('.');
    }
}

void CSpectrometer::GetCurrentDateAndTime(novac::CDateTime& currentDateAndTime)
{
    mobiledoas::GpsData currentGpsInfo;
    const bool couldReadValidGPSData = (m_useGps) ? UpdateGpsData(currentGpsInfo) : false;
    if (couldReadValidGPSData)
    {
        ExtractDateAndTime(currentGpsInfo, currentDateAndTime);
    }
    else
    {
        mobiledoas::GetCurrentDateFromComputerClock(currentDateAndTime);
        GetCurrentTimeFromComputerClock(currentDateAndTime);
    }

    if (currentGpsInfo.date == 0)
    {
        mobiledoas::GetCurrentDateFromComputerClock(currentDateAndTime);
    }
}

void CSpectrometer::WriteLogFile(CString filename, CString txt)
{
    FILE* f;
    f = fopen(filename, "a+");

    if (f < (FILE*)1)
    {
        CString tmpStr;
        tmpStr.Format("Could not write log file: %s. Not enough free space?", (LPCSTR)filename);
        ShowMessageBox(tmpStr, "Big Error");
        return;
    }

    fprintf(f, txt + "\n");

    fclose(f);
}

void CSpectrometer::WriteBeginEvFile(int fitRegion)
{

    // Write a copy of the old cfg-file into the evaluation-log
    CString evPath = m_subFolder + "\\" + m_measurementBaseName + "_" + m_measurementStartTimeStr + TEXT("evaluationLog_" + m_fitRegion[fitRegion].window.name + ".txt");
    CString str1, str2, str3, str4, str5, str6, str7, channelName;

    str1.Format("***Desktop Mobile Program***\nVERSION=%1d.%1d.%1d\nFILETYPE=evaluationlog\n", CVersion::majorNumber, CVersion::minorNumber, CVersion::patchNumber);
    str2.Format("BASENAME=%s\nWINDSPEED=%f\nWINDDIRECTION=%f\n", (LPCSTR)m_measurementBaseName, m_windSpeed, m_windAngle);
    str3 = TEXT("***copy of related configuration file ***\n");

    str4.Format("GPSBAUD=%d\nGPSPORT=%s\nTIMERESOLUTION=%d\n",
        m_GPSBaudRate, m_GPSPort, m_timeResolution);

    str5.Format("FIXEXPTIME=%d\nFITFROM=%d\nFITTO=%d\nPOLYNOM=%d\n",
        m_fixexptime, m_fitRegion[fitRegion].window.fitLow, m_fitRegion[fitRegion].window.fitHigh, m_fitRegion[fitRegion].window.polyOrder);
    str5.AppendFormat("FIXSHIFT=%d\nFIXSQUEEZE=%d\n",
        m_fitRegion[fitRegion].window.ref[0].m_shiftOption == novac::SHIFT_TYPE::SHIFT_FIX, m_fitRegion[fitRegion].window.ref[0].m_squeezeOption == novac::SHIFT_TYPE::SHIFT_FIX);
    str6.Format("SPECCENTER=%d\nPERCENT=%f\nMAXCOLUMN=%f\nGASFACTOR=%f\n",
        m_conf->m_specCenter, m_desiredSaturationRatio, m_maxColumn, m_gasFactor);
    for (int k = 0; k < m_fitRegion[fitRegion].window.nRef; ++k)
    {
        str6.AppendFormat("REFFILE=%s\n", m_fitRegion[fitRegion].window.ref[k].m_path.c_str());
    }
    WriteLogFile(evPath, str1 + str2 + str3 + str4 + str5 + str6);

    // Write some additional information about the spectrometer
    str1.Format("***Spectrometer Information***\n");
    str1.AppendFormat("SERIAL=%s\n", m_spectrometerName.c_str());
    str1.AppendFormat("DETECTORSIZE=%d\n", m_detectorSize);
    str1.AppendFormat("DYNAMICRANGE=%d\n", m_spectrometerDynRange);
    str1.AppendFormat("MODEL=%s\n", m_spectrometerModel.c_str());
    WriteLogFile(evPath, str1);

    // The header-line
    if (m_fitRegion[fitRegion].window.channel == 0)
    {
        channelName.Format("Master");
    }
    else
    {
        channelName.Format("Slave");
    }

    str7.Format("\n#Time\tLat\tLong\tAlt\tNSpec\tExpTime\tIntens(%s)\t", (LPCSTR)channelName);
    for (int referenceIdx = 0; referenceIdx < m_fitRegion[fitRegion].window.nRef; ++referenceIdx)
    {
        str7.AppendFormat("%s_Column_%s\t%s_ColumnError_%s\t",
            channelName,
            m_fitRegion[fitRegion].window.ref[referenceIdx].m_specieName.c_str(),
            channelName,
            m_fitRegion[fitRegion].window.ref[referenceIdx].m_specieName.c_str());
    }
    str7.AppendFormat("STD-File(%s)\n", (LPCSTR)channelName);

    WriteLogFile(evPath, str7);
}

int CSpectrometer::CountRound(long timeResolution, mobiledoas::SpectrumSummation& result) const
{

    int maximumNumberToAddInDevice = 15;
    if (m_spectrometerModel == "USB2000+" || m_spectrometerModel == "AVANTES")
    {
        maximumNumberToAddInDevice = 1000;
    }

    return mobiledoas::CountRound(timeResolution, m_integrationTime, m_spectrometer->GetReadoutDelay(), maximumNumberToAddInDevice, result);
}

std::vector<double> CSpectrometer::GetSpectrum(int channel) const
{
    if ((unsigned int)channel > m_curSpectrum.NumberOfChannels())
    {
        return std::vector<double>(MAX_SPECTRUM_LENGTH, 0.0);
    }
    return m_curSpectrum.data[channel];
}

std::vector<std::string> CSpectrometer::GetConnectedSpectrometers() const
{
    return m_spectrometer->ListDevices();
}

int CSpectrometer::TestSpectrometerConnection()
{
    m_spectrometerIndex = 0; // assume that we will use spectrometer #1

    UpdateStatusBarMessage("Searching for attached spectrometers");

    // List the serials of all spectrometers attached to the computer
    const auto connectedSpectrometers = m_spectrometer->ScanForDevices();

    m_numberOfSpectrometersAttached = static_cast<int>(connectedSpectrometers.size());

    // Check the number of spectrometers attached
    if (m_numberOfSpectrometersAttached == 0)
    {
        ShowMessageBox("No spectrometer found. Make sure that the spectrometer is attached properly to the USB-port and the driver is installed.", "Error");
        return 0;
    }
    else if (m_numberOfSpectrometersAttached > 1)
    {
        Dialogs::CSelectionDialog dlg;
        CString selectedSerial;

        UpdateStatusBarMessage("Several spectrometers found.");

        dlg.m_windowText.Format("Select which spectrometer to use");
        for (int k = 0; k < m_numberOfSpectrometersAttached; ++k)
        {
            dlg.m_option[k].Format("%s", connectedSpectrometers[k].c_str());
        }
        dlg.m_currentSelection = &selectedSerial;
        dlg.DoModal();

        for (int k = 0; k < m_numberOfSpectrometersAttached; ++k)
        {
            if (Equals(selectedSerial, connectedSpectrometers[k].c_str()))
            {
                m_spectrometerIndex = k;
            }
        }
        m_spectrometerName = (LPCSTR)selectedSerial;
    }
    else
    {
        m_spectrometerName = connectedSpectrometers[0];
    }

    UpdateStatusBarMessage("Will use spectrometer %s.", m_spectrometerName.c_str());

    // Setup the channels to use
    std::vector<int> channelIndices;
    if (m_NChannels == 1)
    {
        channelIndices.push_back(m_spectrometerChannel);
    }
    else
    {
        for (int channelIdx = 0; channelIdx < m_NChannels; ++channelIdx)
        {
            channelIndices.push_back(channelIdx);
        }
    }

    // Change the selected spectrometer. This will also fill in the parameters about the spectrometer
    this->m_spectrometerIndex = ChangeSpectrometer(m_spectrometerIndex, channelIndices);

    return (this->m_spectrometerIndex >= 0);
}

bool CSpectrometer::IsSpectrometerDisconnected()
{
    const std::string lastErrorMsg = m_spectrometer->GetLastError();

    // This search string isn't really documented by Ocean Optics but has been found through experimentation.
    // Full error message returned from the driver was: "java.io.IOException: Bulk failed."
    // TODO: This is VERY OceanOptics specific!
    return (lastErrorMsg.size() > 0 && nullptr != strstr(lastErrorMsg.c_str(), "Bulk failed."));
}

void CSpectrometer::ReconnectWithSpectrometer()
{
    UpdateStatusBarMessage("Connection with spectrometer lost! Reconnecting.");

    m_spectrometer->Close();

    std::vector<std::string> spectrometersFound;
    int attemptNumber = 1;
    while (spectrometersFound.size() != m_numberOfSpectrometersAttached)
    {
        // Make the user aware of the problems here...
        Sing(1.0);

        Sleep(500);

        spectrometersFound = m_spectrometer->ScanForDevices();

        UpdateStatusBarMessage("Connection with spectrometer lost! Reconnecting, attempt #%d", attemptNumber++);
    }
}

int CSpectrometer::ChangeSpectrometer(int selectedspec, const std::vector<int>& channelsToUse)
{
    if (selectedspec < 0)
    {
        return m_spectrometerIndex;
    }

    // Check the number of spectrometers attached
    if (m_numberOfSpectrometersAttached == 0)
    {
        selectedspec = 0; // here it doesn't matter what the user wanted to have. there's only one spectrometer let's use it.
        return 0;
    }

    const bool wasSuccessfullyChanged = m_spectrometer->SetSpectrometer(selectedspec, channelsToUse);
    if (!wasSuccessfullyChanged)
    {
        CString msg;
        msg.Format("Failed to set the spectrometer to use. Error message: %s", m_spectrometer->GetLastError().c_str());
        ShowMessageBox(msg, "Error");
        return 0;
    }

    OnChangedSpectrometer();

    // Get the spectrometer model
    m_spectrometerModel = m_spectrometer->GetModel();
    m_spectrometerDynRange = m_spectrometer->GetSaturationIntensity();

    const int nofChannelsAvailable = m_spectrometer->GetNumberOfChannels();

    if (m_NChannels > nofChannelsAvailable)
    {
        CString msg;
        msg.Format("Cfg.txt specifies %d channels to be used but spectrometer can only handle %d. Changing configuration to handle only %d channel(s)", m_NChannels, nofChannelsAvailable, nofChannelsAvailable);
        ShowMessageBox(msg, "Error");
        m_NChannels = nofChannelsAvailable;
    }

    if (m_spectrometerDynRange < 0)
    {
        if (EqualsIgnoringCase(m_spectrometerModel, "USB4000") || EqualsIgnoringCase(m_spectrometerModel, "HR4000") ||
            EqualsIgnoringCase(m_spectrometerModel, "USB2000+") || EqualsIgnoringCase(m_spectrometerModel, "QE65000"))
        {
            m_spectrometerDynRange = 65536;
        }
        else
        {
            m_spectrometerDynRange = 4096;
        }
    }

    UpdateStatusBarMessage("Will use spectrometer #%d (%s).", m_spectrometerIndex, m_spectrometerModel.c_str());

    // Get a spectrum
    UpdateStatusBarMessage("Attempting to retrieve a spectrum from %s", m_spectrometerName.c_str());

    m_spectrometer->SetIntegrationTime(3000); // use 3 ms exp-time
    m_spectrometer->SetScansToAverage(1);  // only retrieve one single spectrum

    std::vector<std::vector<double>> spectrumData;
    int returnCode = m_spectrometer->GetNextSpectrum(spectrumData);
    if (returnCode == 0)
    {
        std::string errorMessage = m_spectrometer->GetLastError();
        if (errorMessage.size() > 0)
        {
            ShowMessageBox((std::string("Failed to retrieve spectrum, error was: ") + errorMessage).c_str(), "Error getting spectrum");
        }
        else
        {
            ShowMessageBox("Failed to retrieve spectrum, unknown error", "Error getting spectrum");
        }
        return -1;
    }

    ASSERT(spectrumData.size() == m_NChannels);
    ASSERT(spectrumData[0].size() > 0);
    m_detectorSize = static_cast<int>(spectrumData[0].size());

    // Get the wavelength calibration from the spectrometer
    std::vector<std::vector<double>> wavelengthData;
    m_spectrometer->GetWavelengths(wavelengthData);
    ASSERT(wavelengthData.size() == m_NChannels);

    m_wavelength.data = wavelengthData;

    UpdateStatusBarMessage("Detector size is %d", m_detectorSize);

    return m_spectrometerIndex;
}


void CSpectrometer::CloseSpectrometerConnection()
{
    m_spectrometer->Close();
}

void CSpectrometer::GetSpectrumInfo(const mobiledoas::MeasuredSpectrum& spectrum)
{
    /* The nag flag makes sure that we dont remind the user to take a new dark
        spectrum too many times in a row. */
    static bool nagFlag = false;

    if (m_fixexptime < 0)
    {
        nagFlag = true;
    }

    /* Cut from the S2000 manual:
        pixel   Description
            0-1     Not usable
            2-24    Optical dark pixels
            24-25   Transition pixels
            26-2074 Optical active pixels

            Cut from the USB4000 manual:
             Pixel   Description
                 15    Not usable
                 618    Optical black pixels
                 1921   Transition pixels
                 223669  Optical active pixels
                 36703681 Not usable
  */

  /* The offset is judged as the average intensity in pixels 6 - 18 */
    for (int n = 0; n < m_NChannels; ++n)
    {
        m_specInfo[n].offset = mobiledoas::GetOffset(spectrum.data[n]);

        // Check if this spectrum is dark
        const bool isDark = mobiledoas::CheckIfDark(spectrum.data[n]);

        if (isDark)
        {
            m_specInfo[n].isDark = true;
            m_lastDarkOffset[n] = m_specInfo[n].offset;
            nagFlag = false;

            // use the new spectrum as dark spectrum
            spectrum.CopyTo(m_dark);
        }
        else
        {
            m_specInfo[n].isDark = false;

            /* If the offset level has changed alot since the last dark, encourage the user to take a new dark spectrum */
            if (m_lastDarkOffset[n] > 10)
            {/* Make sure this test is not run on the first spectrum collected */
                if (!nagFlag)
                {
                    // TODO: Investigate what is a reasonable threshold here for the different supported spectrometer models.
                    //  The threshold of 20 counts is used for S2000/USB2000 and the value is a rather basic scaling of this to the dynamic range of the spectrometer.
                    const double threshold = (20 / 4095.0) * m_spectrometerDynRange;
                    if (threshold > 0.0 && fabs(m_lastDarkOffset[n] - m_specInfo[n].offset) > threshold)
                    {
                        DisplayDialog(DARK_DIALOG);
                        nagFlag = true;
                    }
                }
            }
        }
    }

    /** If possible, get the board temperature of the spectrometer */
    if (m_spectrometer->SupportsBoardTemperature())
    {
        m_boardTemperature = m_spectrometer->GetBoardTemperature();
    }
    else
    {
        m_boardTemperature = std::numeric_limits<double>::quiet_NaN();
    }

    /** If possible, get the detector temperature of the spectrometer */

    if (m_spectrometer->SupportsDetectorTemperatureControl())
    {
        detectorTemperature = m_spectrometer->GetDetectorTemperature();;
        if (abs(detectorTemperature - m_conf->m_setPointTemperature) <= 2.0)
        {
            detectorTemperatureIsSetPointTemp = true;
        }
        else
        {
            detectorTemperatureIsSetPointTemp = false;
        }
    }
    else
    {
        detectorTemperature = std::numeric_limits<double>::quiet_NaN();
        detectorTemperatureIsSetPointTemp = false;
    }

    /* Print the information to a file */
    CString fileName = m_subFolder + "\\" + m_measurementBaseName + "_" + m_measurementStartTimeStr + "AdditionalLog.txt";
    FILE* f = nullptr;

    if (!IsExistingFile(fileName))
    {
        f = fopen(fileName, "w");
        if (f == nullptr)
        {
            return;
        }
        fprintf(f, "#--Additional log file to the Mobile DOAS program---\n");
        fprintf(f, "#This file has only use as test file for further development of the Mobile DOAS program\n\n");
        fprintf(f, "#SpectrumNumber\t");
        if (!std::isnan(m_boardTemperature))
        {
            fprintf(f, "BoardTemperature\t");
        }
        if (!std::isnan(detectorTemperature))
        {
            fprintf(f, "DetectorTemperature\t");
        }
        if (m_NChannels == 1)
        {
            fprintf(f, "Offset\tSpecCenterIntensity\tisDark\n");
        }
        else
        {
            fprintf(f, "#SpectrumNumber\tOffset(Master)\tSpecCenterIntensity(Master)\tisDark(Master)\tOffset(Slave)\tSpecCenterIntensity\tisDark(Slave)\t\n");
        }
        fclose(f);
    }

    f = fopen(fileName, "a+");
    if (f == nullptr)
    {
        UpdateStatusBarMessage("ERROR! Could not open the Additional Log file - Information has been lost!");
    }
    else
    {
        // Spectrum number
        fprintf(f, "%ld\t", m_spectrumCounter);
        // The board temperature 
        if (!std::isnan(m_boardTemperature))
        {
            fprintf(f, "%.3lf\t", m_boardTemperature);
        }
        // The detector temperature 
        if (!std::isnan(detectorTemperature))
        {
            fprintf(f, "%.3lf\t", detectorTemperature);
        }
        // The master-channel
        fprintf(f, "%lf\t%ld\t%d", m_specInfo[0].offset, m_averageSpectrumIntensity[0], m_specInfo[0].isDark);
        if (m_NChannels > 1)
        {
            fprintf(f, "\t%lf\t%ld\t%d", m_specInfo[1].offset, m_averageSpectrumIntensity[1], m_specInfo[1].isDark);
        }
        fprintf(f, "\n");

        fclose(f);
    }
}


void CSpectrometer::UpdateMobileLog()
{
    char txt[256];

    /* Open the mobile log file */
    FILE* f = fopen(g_exePath + "MobileLog.txt", "r");

    if (0 == f)
    {
        /* File might not exist */
        f = fopen(g_exePath + "MobileLog.txt", "w");
        if (0 == f)
        {
            /* File cannot be opened */
            return;
        }
        else
        {
            fprintf(f, "BASENAME=%s\n", (LPCSTR)m_measurementBaseName);
            fprintf(f, "WINDSPEED=%.2lf\n", m_windSpeed);
            fprintf(f, "WINDDIRECTION=%.2lf\n", m_windAngle);
            fclose(f);
            return;
        }
    }
    else
    {
        CString tmpStr;
        /* read in all the funny strings we dont understand */
        while (fgets(txt, sizeof(txt) - 1, f))
        {
            if ((0 == strstr(txt, "BASENAME=")) && (0 == strstr(txt, "WINDSPEED=")) && (0 == strstr(txt, "WINDDIRECTION=")))
            {
                tmpStr.AppendFormat("%s", txt);
            }
        }
        fclose(f);
        f = fopen(g_exePath + "MobileLog.txt", "w");
        fprintf(f, "%s", (LPCTSTR)tmpStr);
        fprintf(f, "BASENAME=%s\n", (LPCSTR)m_measurementBaseName);
        fprintf(f, "WINDSPEED=%.2lf\n", m_windSpeed);
        fprintf(f, "WINDDIRECTION=%.2lf\n", m_windAngle);

        fclose(f);
    }
}

short CSpectrometer::AdjustIntegrationTime()
{

    mobiledoas::MeasuredSpectrum skySpec;
    m_sumInSpectrometer = 1;
    m_sumInComputer = 1;
    int darkInt = 0;
    int skyInt = 0;
    int oldIntTime = m_integrationTime;

    int highLimit = MAX_EXPOSURETIME; // maximum exposure time
    int lowLimit = MIN_EXPOSURETIME; // minimum exposure time

    // if the exposure time is set by the user, don't even bother to calculate it
    if (m_fixexptime > 0)
    {
        m_integrationTime = (short)m_fixexptime;
        return m_integrationTime;
    }

    // First make a try at setting the integration time by collecting one 
    // spectrum at 10 ms exposure-time and one at 50 ms exposure-time
    if (-1 != AdjustIntegrationTime_Calculate(10, 50))
    {
        return m_integrationTime;
    }

    // The clever setting failed... revert to simple trial and error...
    m_integrationTime = 100;
    while (1)
    {
        UpdateStatusBarMessage("Measuring the intensity");

        // measure the intensity
        if (Scan(1, m_sumInSpectrometer, skySpec))
        {
            return -1;
        }

        // Get the intensity of the sky and the dark spectra
        skyInt = mobiledoas::AverageIntensity(skySpec.data[0], m_conf->m_specCenter, m_conf->m_specCenterHalfWidth);
        darkInt = (long)mobiledoas::GetOffset(skySpec.data[0]);

        // Draw the measured sky spectrum on the screen.
        skySpec.CopyTo(m_curSpectrum);

        if (m_fixexptime >= 0)
        {
            UpdateDisplayedSpectrum();
        }
        OnUpdatedIntegrationTime();

        // Check if we have reached our goal
        if (fabs(skyInt - m_spectrometerDynRange * m_desiredSaturationRatio) < 200)
        {
            return m_integrationTime;
        }

        // Adjust the exposure time so that we come closer to the desired value
        if (skyInt - m_spectrometerDynRange * m_desiredSaturationRatio > 0)
        {
            highLimit = m_integrationTime;
        }
        else
        {
            lowLimit = m_integrationTime;
        }

        // calculate the neccessary exposure time
        oldIntTime = m_integrationTime;
        m_integrationTime = (short)GetInttime(skyInt, darkInt, m_integrationTime);

        // if we don't change the exposure time, just quit.
        if (fabs(oldIntTime - m_integrationTime) < 0.1 * m_integrationTime)
        {
            return m_integrationTime;
        }

        // check if we can reach the desired level at all
        if ((m_integrationTime == MAX_EXPOSURETIME) || (m_integrationTime == MIN_EXPOSURETIME))
        {
            return m_integrationTime;
        }
    }

    return m_integrationTime;
}

short CSpectrometer::AdjustIntegrationTime_Calculate(long minExpTime, long maxExpTime)
{
    mobiledoas::MeasuredSpectrum skySpec;
    m_sumInSpectrometer = 1;
    m_sumInComputer = 1;

    // TODO: This setup does not seem to work to well together with the Avantes spectrometers.
    // Possibly because the (minExpTime, maxExpTime) is too narrow ?

    // Sanity check
    if (maxExpTime < minExpTime || (maxExpTime - minExpTime) < 2)
    {
        return -1;
    }

    m_integrationTime = (short)minExpTime;

    // measure the intensity
    if (Scan(1, m_sumInSpectrometer, skySpec))
    {
        return -1;
    }
    const int intensityAtShortIntegrationTime = mobiledoas::AverageIntensity(skySpec.data[0], m_conf->m_specCenter, m_conf->m_specCenterHalfWidth);

    m_integrationTime = (short)maxExpTime;
    // measure the intensity
    if (Scan(1, m_sumInSpectrometer, skySpec))
    {
        return -1;
    }
    const int intensityAtLongIntegrationTime = mobiledoas::AverageIntensity(skySpec.data[0], m_conf->m_specCenter, m_conf->m_specCenterHalfWidth);

    // This will only work if the spectrum is not saturated at the maximum exposure-time
    if (intensityAtLongIntegrationTime > 0.9 * m_spectrometerDynRange)
    {
        return AdjustIntegrationTime_Calculate(minExpTime, (maxExpTime - minExpTime) / 2);
    }

    // Calculate the exposure-time
    m_integrationTime = mobiledoas::EstimateNewIntegrationTime(
        mobiledoas::SpectrumIntensityMeasurement(intensityAtShortIntegrationTime, m_spectrometerDynRange, minExpTime),
        mobiledoas::SpectrumIntensityMeasurement(intensityAtLongIntegrationTime, m_spectrometerDynRange, maxExpTime),
        m_desiredSaturationRatio,
        MIN_EXPOSURETIME,
        m_timeResolution);


    // Try out the calculated intensity to see if it works...
    if (Scan(1, m_sumInSpectrometer, skySpec))
    {
        return -1;
    }
    int finalInt = mobiledoas::AverageIntensity(skySpec.data[0], m_conf->m_specCenter, m_conf->m_specCenterHalfWidth);

    // TODO: There is a range of allowed intensities, use that instead!
    int desiredInt = (int)(m_spectrometerDynRange * m_desiredSaturationRatio);
    if ((finalInt - desiredInt) / desiredInt > 0.2)
    {
        return -1;
    }
    else
    {
        // Draw the measured sky spectrum on the screen.
        skySpec.CopyTo(m_curSpectrum);

        if (m_fixexptime >= 0)
        {
            UpdateDisplayedSpectrum();
        }
        OnUpdatedIntegrationTime();

        return m_integrationTime;
    }
}

long CSpectrometer::GetCurrentTimeFromComputerClock()
{
    const int currentSpectrumCounter = this->m_spectrumCounter; // local copy to prevent race conditions

    long startTime = GetTimeValue_UMT();

    if (m_timeDiffToUtc == 0)
    {
        time_t t;
        time(&t);
        struct tm* localTime = localtime(&t);

        const mobiledoas::GpsData curGpsInfo = m_spectrumGpsData[currentSpectrumCounter];

        int hr, min, sec;
        ExtractTime(curGpsInfo, hr, min, sec);

        /* get the difference between the local time and the GPS-time */
        m_timeDiffToUtc = 3600 * (hr - localTime->tm_hour) + 60 * (min - localTime->tm_min) + (sec - localTime->tm_sec);
    }

    return startTime;
}

void CSpectrometer::GetCurrentTimeFromComputerClock(novac::CDateTime& result)
{
    long currentTime = GetCurrentTimeFromComputerClock();

    int hours, minutes, seconds;
    mobiledoas::GetHrMinSec(currentTime, hours, minutes, seconds);

    result.hour = hours;
    result.minute = minutes;
    result.second = seconds;
}

unsigned int CSpectrometer::GetProcessedSpectrum(double* dst, unsigned int maxNofElements, int chn) const
{
    if (chn < 0 || chn >= m_spectrum.NumberOfChannels())
    {
        return 0U; // invalid input parameter
    }

    const unsigned int length = std::min(maxNofElements, (unsigned int)m_spectrum.SpectrumLength());
    memcpy(dst, this->m_spectrum.data[chn].data(), MAX_SPECTRUM_LENGTH * sizeof(double));
    return length;
}

void CSpectrometer::ShowMessageBox(CString message, CString label) const
{
    if (m_isRunning)
    {
        // the point here is that we only allow the messagebox to be shown when the user interface is ready for receiving message boxes (i.e. when the program is running!)
        MessageBox(this->m_mainForm.m_hWnd, message, label, MB_OK);
    }
}

void CSpectrometer::CreateSpectrum(CSpectrum& spectrum, const std::vector<double>& spec, const std::string& startDate, long startTime, long elapsedSecond)
{
    memcpy((void*)spectrum.I, (void*)spec.data(), sizeof(double) * std::min((size_t)MAX_SPECTRUM_LENGTH, spec.size()));
    spectrum.length = m_detectorSize;
    spectrum.exposureTime = m_integrationTime;
    spectrum.date = startDate;
    spectrum.spectrometerModel = m_spectrometerModel.c_str();
    spectrum.spectrometerSerial = m_spectrometerName.c_str();
    spectrum.scans = NumberOfSpectraToAverage();
    spectrum.name = m_measurementBaseName;
    spectrum.fitHigh = m_conf->m_fitWindow->fitHigh;
    spectrum.fitLow = m_conf->m_fitWindow->fitLow;
    spectrum.boardTemperature = m_boardTemperature;
    spectrum.detectorTemperature = detectorTemperature;
    spectrum.gpsStatus = "NA";
    if (m_useGps)
    {
        spectrum.SetStartTime(m_spectrumGpsData[m_spectrumCounter].time);
        spectrum.SetStopTime(m_spectrumGpsData[m_spectrumCounter].time + elapsedSecond);
        spectrum.lat = m_spectrumGpsData[m_spectrumCounter].latitude;
        spectrum.lon = m_spectrumGpsData[m_spectrumCounter].longitude;
        spectrum.altitude = m_spectrumGpsData[m_spectrumCounter].altitude;
        spectrum.gpsStatus = m_spectrumGpsData[m_spectrumCounter].status;
        spectrum.speed = m_spectrumGpsData[m_spectrumCounter].speed;
        spectrum.course = m_spectrumGpsData[m_spectrumCounter].course;
    }
    else
    {
        spectrum.SetStartTime(startTime);
        spectrum.SetStopTime(startTime + elapsedSecond);
    }
}

void CSpectrometer::InitializeEvaluators(bool skySpectrumIsDarkCorrected)
{
    for (int fitRgnIdx = 0; fitRgnIdx < m_fitRegionNum; ++fitRgnIdx)
    {
        for (int channelIdx = 0; channelIdx < m_NChannels; ++channelIdx)
        {
            m_fitRegion[fitRgnIdx].window.specLength = this->m_detectorSize;
            m_fitRegion[fitRgnIdx].eval[channelIdx]->SetFitWindow(m_fitRegion[fitRgnIdx].window);

            m_fitRegion[fitRgnIdx].eval[channelIdx]->m_subtractDarkFromSky = !skySpectrumIsDarkCorrected;
        }
    }
}

void CSpectrometer::WriteEvaluationLogFileHeaders()
{
    for (int fitRgnIdx = 0; fitRgnIdx < m_fitRegionNum; ++fitRgnIdx)
    {
        WriteBeginEvFile(fitRgnIdx);
    }
}

bool CSpectrometer::RunInstrumentCalibration(const double* measuredSpectrum, const double* darkSpectrum, size_t spectrumLength)
{
    UpdateStatusBarMessage("Performing instrument calibration");

    bool referencesUpdated = false;

    try
    {
        novac::CSpectrumInfo spectrumInfo;
        spectrumInfo.m_device = m_spectrometerName;
        spectrumInfo.m_specModelName = m_spectrometerModel;
        GetCurrentDateAndTime(spectrumInfo.m_startTime); // TODO: This should in fact be the time the spectrum was measured, which may be just before now

        std::string outputDirectory = std::string(m_subFolder + "/");

        referencesUpdated = Evaluation::CRealTimeCalibration::RunInstrumentCalibration(
            measuredSpectrum,
            darkSpectrum,
            spectrumLength,
            spectrumInfo,
            outputDirectory,
            *m_conf,
            m_spectrometerDynRange);

        if (referencesUpdated)
        {
            // Make sure to update the relevant settings here as well.
            ApplyEvaluationSettings();
        }
    }
    catch (std::exception& e)
    {
        std::stringstream message;
        message << "Failed to perform instrument calibration: " << e.what();
        ShowMessageBox(CString(message.str().c_str()), "Error");
    }

    return referencesUpdated;
}

void CSpectrometer::UpdateSpectrumAverageIntensity(mobiledoas::MeasuredSpectrum& scanResult)
{
    for (int i = 0; i < m_NChannels; ++i)
    {
        m_averageSpectrumIntensity[i] = mobiledoas::AverageIntensity(scanResult[i], m_conf->m_specCenter, m_conf->m_specCenterHalfWidth);
    }
}

void CSpectrometer::UpdateUserAboutSpectrumAverageIntensity(const std::string& spectrumName, bool checkIfDark)
{
    std::string fullSpectrumName = spectrumName.length() > 0 ? "(" + spectrumName + ")" : "";
    if (checkIfDark && m_specInfo->isDark)
    {
        fullSpectrumName = fullSpectrumName + "(Dark)";
    }

    UpdateStatusBarMessage(
        "Average value around center channel%s %d: %d (%.0lf%%)",
        fullSpectrumName.c_str(),
        m_conf->m_specCenter,
        m_averageSpectrumIntensity[0],
        100.0 * m_averageSpectrumIntensity[0] / m_spectrometerDynRange);
}

void CSpectrometer::OnUpdatedIntegrationTime() const
{
    this->m_mainForm.PostMessage(WM_SHOWINTTIME);
}

void CSpectrometer::UpdateStatusBarMessage(const std::string& newMessage)
{
    this->m_statusMsg.Format("%s", newMessage.c_str());
    this->m_mainForm.PostMessage(WM_STATUSMSG);
}

void CSpectrometer::UpdateStatusBarMessage(const char* format, ...)
{
    std::vector<char> localBuffer(65535);

    va_list args;
    va_start(args, format);
    vsprintf(localBuffer.data(), format, args);
    va_end(args);

    this->m_statusMsg.Format("%s", localBuffer.data());

    this->m_mainForm.PostMessage(WM_STATUSMSG);
}

void CSpectrometer::UpdateGpsLocation() const
{
    this->m_mainForm.PostMessage(WM_READGPS);
}

void CSpectrometer::UpdateDisplayedSpectrum() const
{
    this->m_mainForm.PostMessage(WM_DRAWSPECTRUM);
}

void CSpectrometer::OnNewColumnMeasurement() const
{
    this->m_mainForm.PostMessage(WM_DRAWCOLUMN);
}

void CSpectrometer::OnChangedSpectrometer() const
{
    this->m_mainForm.PostMessage(WM_CHANGEDSPEC);
}

void CSpectrometer::DisplayDialog(int dialogToDisplay) const
{
    this->m_mainForm.PostMessage(WM_SHOWDIALOG, dialogToDisplay);
}
