#include "stdafx.h"
#include "reevaluator.h"
#include "../Version.h"
#include <MobileDoasLib/DateTime.h>

using namespace ReEvaluation;
using namespace Evaluation;

CReEvaluator::CReEvaluator(void)
{
    fRun = false;
    m_pause = 0;
    m_sleeping = false;

    m_evalLogFileName.Format("");
    m_specFileDir.Format("");
    m_outputDir.Format("");
    m_outputLog.Format("");

    // The information about the spectrometer
    m_spectrometerDynRange = 4096;
    m_detectorSize = 2048;
    m_spectrometerName.Format("unknown");
    m_spectrometerModel.Format("unknown");

    // the data from the evaluation log
    m_nChannels = 1;
    m_nSpecies = 1;
    m_recordNum[0] = m_recordNum[1] = 0;

    memset(m_time, 0, sizeof(int) * MAX_TRAVERSE_LENGTH);
    memset(m_nspec, 0, sizeof(int) * MAX_TRAVERSE_LENGTH);
    memset(m_exptime, 0, sizeof(int) * MAX_TRAVERSE_LENGTH);
    memset(m_lat, 0, sizeof(double) * MAX_TRAVERSE_LENGTH);
    memset(m_lon, 0, sizeof(double) * MAX_TRAVERSE_LENGTH);
    memset(m_alt, 0, sizeof(int) * MAX_TRAVERSE_LENGTH);
    memset(m_int, 0, sizeof(double) * 2 * MAX_TRAVERSE_LENGTH);
    memset(m_oldCol, 0, sizeof(double) * 2 * MAX_TRAVERSE_LENGTH);
    memset(m_offset, 0, sizeof(double) * 2 * MAX_TRAVERSE_LENGTH);


    // the information about which spectra are dark.
    m_darkSpecListLength[0] = 0;
    m_darkSpecListLength[1] = 0;

    // handling the shift and squeeze
    optimumShift = 1;   optimumShiftError = 0;
    optimumSqueeze = 1;   optimumSqueezeError = 0;

    // the wavelength calibration
    m_includePreCalSky = false;
    m_calibSky.Format("");

    m_progress = 0;
    m_mainView = nullptr;
    m_mode = MODE_NOTHING;
    m_statusMsg.Format("");
}

CReEvaluator::~CReEvaluator(void)
{
}

int CReEvaluator::ReadEvaluationLog()
{
    CFileException exceFile;
    CStdioFile fileRef;
    int n = 0, i = 0;
    double fValue;
    CString szLine2;
    int nColumns;
    const int BUF_SIZE = 512;
    char buf[BUF_SIZE];
    int tmpInt1, tmpInt2, tmpInt3;

    /** Reset the already existing information in this ReEvaluator... */
    m_darkSpecListLength[0] = m_darkSpecListLength[1] = 0;
    m_spectrometerDynRange = 4096;
    m_detectorSize = 2048;
    m_spectrometerName.Format("unknown");
    m_spectrometerModel.Format("unknown");
    m_nChannels = 1;
    m_nSpecies = 1;
    m_recordNum[0] = m_recordNum[1] = 0;
    memset(m_time, 0, sizeof(int) * MAX_TRAVERSE_LENGTH);
    memset(m_nspec, 0, sizeof(int) * MAX_TRAVERSE_LENGTH);
    memset(m_exptime, 0, sizeof(int) * MAX_TRAVERSE_LENGTH);
    memset(m_lat, 0, sizeof(double) * MAX_TRAVERSE_LENGTH);
    memset(m_lon, 0, sizeof(double) * MAX_TRAVERSE_LENGTH);
    memset(m_alt, 0, sizeof(int) * MAX_TRAVERSE_LENGTH);
    memset(m_int, 0, sizeof(double) * 2 * MAX_TRAVERSE_LENGTH);
    memset(m_oldCol, 0, sizeof(double) * 2 * MAX_TRAVERSE_LENGTH);
    memset(m_offset, 0, sizeof(double) * 2 * MAX_TRAVERSE_LENGTH);

    if (ReadSettings())
        return 1;

    if (!fileRef.Open(m_evalLogFileName, CFile::modeRead | CFile::typeText, &exceFile))
    {
        MessageBox(NULL, TEXT("Can not read log file"), TEXT("Error"), MB_OK);
        return 1;
    }

    /* in fileVersion 4.0 the 'altitude' column was added */
    /*  also possibility of several channels was added */
    /* in fileVersion 4.1 the 'columnError' column was added */
    if (m_nChannels == 1)
    {
        nColumns = (m_fileVersion >= 4.1) ? 7 + 2 * m_nSpecies : (m_fileVersion < 4) ? 7 : 8;
    }
    else
    {
        nColumns = (m_fileVersion >= 4.1) ? 12 : 10;
    }

    // Read the data from the evaluation file into the correct columns
    while (fileRef.ReadString(buf, BUF_SIZE - 1))
    {
        char* szToken = buf;

        while (szToken = strtok(szToken, "\t"))
        {

            if (sscanf(szToken, "%lf", &fValue) != 1)
                break;

            // init to get next token
            if (m_fileVersion == 4.0)
            {
                switch (i)
                {
                case 0: sscanf(szToken, "%d:%d:%d", &tmpInt1, &tmpInt2, &tmpInt3);
                    m_time[n] = tmpInt1 * 10000 + tmpInt2 * 100 + tmpInt3; break;
                case 1: m_lat[n] = fValue; break;
                case 2: m_lon[n] = fValue; break;
                case 3: m_alt[n] = (int)fValue; break;
                case 4: m_nspec[n] = (int)fValue; break;
                case 5: m_exptime[n] = (int)fValue; break;
                case 6: m_int[0][n] = fValue; break;
                case 7: m_oldCol[0][n] = fValue; break;
                case 8: m_int[1][n] = fValue; break;
                case 9: m_oldCol[1][n] = fValue; break;
                }
            }
            else
            {
                if (m_fileVersion >= 4.1)
                {
                    switch (i)
                    {
                    case 0: sscanf(szToken, "%d:%d:%d", &tmpInt1, &tmpInt2, &tmpInt3);
                        m_time[n] = tmpInt1 * 10000 + tmpInt2 * 100 + tmpInt3; break;
                    case 1: m_lat[n] = fValue; break;
                    case 2: m_lon[n] = fValue; break;
                    case 3: m_alt[n] = (int)fValue; break;
                    case 4: m_nspec[n] = (int)fValue; break;
                    case 5: m_exptime[n] = (int)fValue; break;
                    case 6: m_int[0][n] = fValue; break;
                    case 7: m_oldCol[0][n] = fValue; break;
                    case 9: m_int[1][n] = fValue; break;
                    case 10: m_oldCol[1][n] = fValue; break;
                    }
                }
                else
                {
                    if (i == 0)
                        m_time[n] = (int)fValue;
                    else if (i == 1)
                        m_lat[n] = fValue;
                    else if (i == 2)
                        m_lon[n] = fValue;
                    else if (i == 3)
                        m_int[0][n] = fValue;
                    else if (i == 4)
                        m_nspec[n] = (int)fValue;
                    else if (i == 5)
                        m_oldCol[0][n] = fValue;
                    else if (i == 6)
                        m_exptime[n] = (int)fValue;
                }
            }

            ++i;
            if (i == nColumns)
            {
                i = 0;
                ++n;
                if (n == MAX_TRAVERSE_LENGTH)
                    break;
            }
            szToken = NULL;
        }

        if (n == MAX_TRAVERSE_LENGTH)
        {
            break;
        }
    }

    fileRef.Close();

    m_recordNum[0] = n;
    m_recordNum[1] = (m_nChannels > 1) ? n : 0;

    return 0;
}

int CReEvaluator::ReadSettings()
{
    char* pt;
    FILE* fil;
    const int BUF_SIZE = 256;
    char txt[BUF_SIZE];
    char nl[2] = { 0x0a, 0 };
    char lf[2] = { 0x0d, 0 };
    bool foundRefFile = false;

    char msg[512];
    int result = 1;

    fil = fopen(m_evalLogFileName, "r");
    if (fil < (FILE*)1)
    {
        sprintf(msg, "Could not open file %s", (LPCTSTR)m_evalLogFileName);
        MessageBox(NULL, msg, TEXT("Error"), MB_OK);
        return 1;
    }

    while (fgets(txt, BUF_SIZE - 1, fil))
    {

        if (strlen(txt) > 4 && txt[0] != '%')
        {
            pt = txt;
            if (pt = strstr(txt, nl))
                pt[0] = 0;
            pt = txt;
            if (pt = strstr(txt, lf))
                pt[0] = 0;

            if (pt = strstr(txt, "VERSION="))
            {
                pt = strstr(txt, "=");
                sscanf(&pt[1], "%lf", &m_fileVersion);
            }

            if (pt = strstr(txt, "DYNAMICRANGE="))
            {
                pt = strstr(txt, "=");
                sscanf(&pt[1], "%ld", &m_spectrometerDynRange);
            }

            if (pt = strstr(txt, "DETECTORSIZE="))
            {
                pt = strstr(txt, "=");
                sscanf(&pt[1], "%ld", &m_detectorSize);
            }

            if (pt = strstr(txt, "SERIAL="))
            {
                char strbuffer[4096];
                pt = strstr(txt, "=");
                sscanf(&pt[1], "%4095s", strbuffer);
                this->m_spectrometerName.Format("%s", strbuffer);
            }

            if (pt = strstr(txt, "MODEL="))
            {
                char strbuffer[4096];
                pt = strstr(txt, "=");
                sscanf(&pt[1], "%4095s", strbuffer);
                this->m_spectrometerModel.Format("%s", strbuffer);
            }

            if (pt = strstr(txt, "FILETYPE="))
            {
                result = 0; /* the file is a correct evaluation log */
            }

            if (pt = strstr(txt, "Column(Slave1)"))
            {
                m_nChannels = 2; /* if there are several channels */
                break; // done reading the file. Quit.
            }
            if (pt = strstr(txt, "Slave1_Column_"))
            {
                m_nChannels = 2; /* if there are several channels */
                break; // done reading the file. Quit.
            }

            if (pt = strstr(txt, "nSpecies="))
            {
                pt = strstr(txt, "=");
                sscanf(&pt[1], "%d", &m_nSpecies);
            }

            if (pt = strstr(txt, "REFFILE="))
            {
                if (foundRefFile)
                {
                    ++m_nSpecies;
                }
                else
                {
                    foundRefFile = true;
                }
            }
        }
    }
    fclose(fil);

    return result;
}

int CReEvaluator::ReadAllOffsets()
{
    int i, chn;
    long nDone = 0;
    double nToDo = (double)(m_nChannels == 1) ? m_recordNum[0] : m_recordNum[0] + m_recordNum[1];
    CSpectrum curSpectrum;
    m_darkSpecListLength[0] = m_darkSpecListLength[1] = 0; // reset the length of the darkspeclist

    m_progress = 0;

    for (chn = 0; chn < m_nChannels; ++chn)
    {
        for (i = 0; i < m_recordNum[chn]; ++i)
        {
            if (!fRun)
                return true;

            GetSpectrum(curSpectrum, i, chn);

            // get the offset
            m_offset[chn][i] = GetOffset(curSpectrum);

            // check if the spectrum is dark
            if (IsDark(curSpectrum))
            {
                m_darkSpecList[chn][m_darkSpecListLength[chn]++] = i;
            }

            // tell the user about the progress
            m_progress = (nDone++) / nToDo;
            if (m_mainView != nullptr && (i % 10) == 0)
                m_mainView->PostMessage(WM_PROGRESS, (WPARAM)(int)(100.0 * m_progress));
        }
    }

    if (m_mainView != nullptr)
        m_mainView->PostMessage(WM_DONE);

    return 0;
}

double CReEvaluator::GetOffset(CSpectrum& spectrum)
{
    return (spectrum.GetAverage(2, 20));
}

bool CReEvaluator::IsDark(CSpectrum& spectrum)
{
    double average = spectrum.GetAverage(1130, 1158);
    double offset = GetOffset(spectrum);

    if (fabs(average - offset) < 4)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CReEvaluator::DoEvaluation()
{
    static double sky[MAX_SPECTRUM_LENGTH];
    Evaluation::CEvaluation evaluator;

    // adaptive mode flag
    bool adaptiveMode = false;

    // go into re-evaluation 'mode'
    m_mode = MODE_REEVALUATION;

    /* Check the settings before we start */
    if (!MakeInitialSanityCheck())
        return false;

    /* Check the reference files */
    if (!ReadReferences(&evaluator))
        return false;

    /* create the output directory and the evaluation file */
    if (!CreateOutputDirectory())
        return false;

    /* if we are to interpolate the dark spectra, start by reading the spectra and register the offsets */
    if (m_settings.m_fInterpolateDark)
    {
        m_mode = MODE_READING_OFFSETS; // tell the world that we're reading offsets
        ReadAllOffsets();
        m_mode = MODE_REEVALUATION; // tell the world that we're re-evaluating data
    }

    /* Check if there are several exposure times present in this traverse,
            if so then we must find the appropriate dark spectra to use. */
    if (!AllSpectraHaveSameExpTime())
    {
        m_mode = MODE_READING_OFFSETS; // tell the world that we're reading offsets

        if (m_mainView != nullptr)
        {
            m_statusMsg.Format("Not all spectra have the same exp.time - Reading offsets");
            m_mainView->PostMessage(WM_STATUS);
        }

        ReadAllOffsets();
        m_mode = MODE_REEVALUATION; // tell the world that we're re-evaluating data

        if (m_mainView != nullptr)
        {
            m_statusMsg.Format("All offsets read. ");
            m_mainView->PostMessage(WM_STATUS);
        }
    }

    /* reset the average residuum */
    memset(m_avgResidual, 0, MAX_SPECTRUM_LENGTH * sizeof(double));
    m_nAveragedInResidual = 0;

    /* evaluate the spectra */
    {
        int chn;
        long nDone = 0;
        double nToDo = (double)(m_nChannels == 1) ? m_recordNum[0] : m_recordNum[0] + m_recordNum[1];
        if (m_settings.m_nAverageSpectra != 1)
        {
            nToDo /= (double)m_settings.m_nAverageSpectra;
        }
        CString message;
        CSpectrum curSpectrum;
        CSpectrum darkSpectrum, skySpectrum;
        CSpectrum darkcurSpectrum, offsetSpectrum;

        m_progress = 0;

        for (chn = 0; chn < m_nChannels; ++chn)
        {

            // Tell the user that we're starting the evaluation
            if (m_mainView != nullptr)
            {
                m_statusMsg.Format("Evaluating Channel %d", chn);
                m_mainView->PostMessage(WM_STATUS);
            }

            // start by reading dark and sky
            if (!ReadSkySpectrum(skySpectrum, chn))
            {
                MessageBox(NULL, "Cannot read sky spectrum. Evaluation stopped.", "Error", MB_OK);
                return false;
            }
            if (!ReadSpectrumFromFile(darkSpectrum, "dark", chn))
            {
                // no dark file found. read darkcur and offset file
                if (!ReadSpectrumFromFile(darkcurSpectrum, "darkcur", chn))
                {
                    MessageBox(NULL, "Cannot read dark spectrum. Evaluation stopped.", "Error", MB_OK);
                    return false;
                }
                if (!ReadSpectrumFromFile(offsetSpectrum, "offset", chn))
                {
                    MessageBox(NULL, "Cannot read offst spectrum. Evaluation stopped.", "Error", MB_OK);
                    return false;
                }
                darkcurSpectrum.Sub(offsetSpectrum);	// subtract offset from darkcur
                adaptiveMode = true;
            }

            // Get the size of the spectra
            m_settings.m_window.specLength = skySpectrum.length;

            // initialize the evaluator
            evaluator.SetFitWindow(m_settings.m_window);

            // save the sky, dark, and offset spectra
            SaveSpectra(skySpectrum, "sky", chn);
            if (!adaptiveMode)
            {
                SaveSpectra(darkSpectrum, "dark", chn);
            }
            else
            {
                SaveSpectra(darkSpectrum, "darkcur", chn);
                SaveSpectra(offsetSpectrum, "offset", chn);
            }

            // Subtract the dark from the sky, and don't do it again.
            // note: dark already subtracted if USE_SKY_USER mode.
            if (m_settings.m_skySelection != USE_SKY_USER)
            {
                // Rebuild darkSpectrum if adaptive mode
                // darkspec = offset + (skySpectrum.exposureTime)*darkcur/(darkcurSpectrum.exposureTime)
                if (adaptiveMode)
                {
                    darkSpectrum = CSpectrum(darkcurSpectrum);
                    darkSpectrum.Mult(skySpectrum.exposureTime);
                    darkSpectrum.Div(darkcurSpectrum.exposureTime);
                    darkSpectrum.Add(offsetSpectrum);
                }
                skySpectrum.Sub(darkSpectrum);
            }
            evaluator.m_subtractDarkFromSky = false;

            // if wanted, include the sky spectrum into the fit
            if (m_settings.m_window.fitType == FIT_HP_SUB || m_settings.m_window.fitType == FIT_POLY)
            {
                IncludeSkySpecInFit(evaluator, skySpectrum, m_settings.m_window);
            }

            // set the shift and squeeze to be used in the evaluation
            for (int k = 0; k < m_settings.m_window.nRef; ++k)
            {
                if (m_settings.m_window.ref[k].m_shiftOption == novac::SHIFT_TYPE::SHIFT_OPTIMAL)
                {
                    if (!FindOptimalShiftAndSqueeze(evaluator, chn, skySpectrum, darkSpectrum))
                    {
                        return false;
                    }
                }
            }
            evaluator.SetFitWindow(m_settings.m_window);

            int fitLow = m_settings.m_window.fitLow;
            int fitHigh = m_settings.m_window.fitHigh;

            // create the evaluation log and write its header
            if (!WriteEvaluationLogHeader(chn))
                return false;

            // now evaluate all the spectra
            for (m_curSpec = 0; m_curSpec < (m_recordNum[chn]); m_curSpec += m_settings.m_nAverageSpectra)
            {

                // Check if the user has pressed the 'cancel' button, if so return...
                if (!fRun)
                {
                    // if the sky was included into the fit it should now be removed from the list of references used
                    if (m_settings.m_window.fitType == FIT_HP_SUB || m_settings.m_window.fitType == FIT_POLY)
                        --m_settings.m_window.nRef;
                    return true;
                }

                // Get the next spectrum
                GetSpectrum(curSpectrum, m_curSpec, chn);

                // check if we should ignore the current spectrum
                if (int ret = Ignore(curSpectrum, m_curSpec))
                {
                    if (m_mainView != nullptr)
                    {
                        if (ret == INTENSITY_SATURATED)
                        {
                            m_statusMsg.Format("Ignoring spectrum number: %d - saturated", m_curSpec);
                        }
                        else
                        {
                            m_statusMsg.Format("Ignoring spectrum number: %d - too dark", m_curSpec);
                        }
                        m_mainView->PostMessage(WM_STATUS);
                    }
                    continue;
                }

                // Get the dark spectrum to use with this spectrum
                if (!adaptiveMode)
                {
                    GetDarkSpectrum(darkSpectrum, m_curSpec, chn); // don't do if adaptive mode
                }
                else
                {
                    // darkspec = offset + (curSpectrum.intTime)*darkcur/(darkcurSpectrum.intTime)
                    darkSpectrum = CSpectrum(darkcurSpectrum);
                    darkSpectrum.Mult(curSpectrum.exposureTime);
                    darkSpectrum.Div(darkcurSpectrum.exposureTime);
                    darkSpectrum.Add(offsetSpectrum);
                }

                memcpy(sky, skySpectrum.I, MAX_SPECTRUM_LENGTH * sizeof(double));

                // do the evaluation
                evaluator.Evaluate(darkSpectrum.I, sky, curSpectrum.I);

                // sum the residuals togheter to find enable us to discover if some reference has been forgotten
                if (evaluator.m_residual.GetSize() > 0)
                {
                    for (int tmpCounter = 0; tmpCounter < (fitHigh - fitLow); ++tmpCounter)
                    {
                        m_residual[tmpCounter] = evaluator.m_residual.GetAt(tmpCounter);
                        m_avgResidual[tmpCounter] += evaluator.m_residual.GetAt(tmpCounter);
                        ++m_nAveragedInResidual;
                    }
                }

                // Get the result of the evaluation and write them to file
                AppendResultToEvaluationLog(m_curSpec, chn, evaluator);

                // update the fit on the screen
                if (m_mainView != nullptr)
                {
                    // the measured spectrum
                    memcpy(m_spectrum, evaluator.m_filteredSpectrum.data(), evaluator.m_filteredSpectrum.size() * sizeof(double));
                    if (m_settings.m_window.fitType == FIT_HP_SUB || m_settings.m_window.fitType == FIT_POLY)
                    {
                        // remove the fitted sky spectrum
                        for (int tmpCounter = fitLow; tmpCounter < fitHigh; ++tmpCounter)
                        {
                            m_spectrum[tmpCounter] -= evaluator.m_fitResult[m_settings.m_window.nRef - 1].GetAt(tmpCounter);
                        }
                    }

                    // the fitted references
                    memset(m_fitResult, 0, MAX_SPECTRUM_LENGTH * sizeof(double));
                    int nRef = (m_settings.m_window.fitType == FIT_HP_DIV) ? m_settings.m_window.nRef : m_settings.m_window.nRef - 1;

                    // add the fitted references
                    for (int r = 0; r < nRef; ++r)
                    {
                        for (int tmpCounter = fitLow; tmpCounter < fitHigh; ++tmpCounter)
                        {
                            this->m_fitResult[tmpCounter] += evaluator.m_fitResult[r].GetAt(tmpCounter);
                        }
                    }
                    // also add the polynomial
                    int a = (m_settings.m_window.fitType != FIT_HP_DIV) ? 1 : 0;
                    for (int tmpCounter = fitLow; tmpCounter < fitHigh; ++tmpCounter)
                    {
                        this->m_fitResult[tmpCounter] += evaluator.m_fitResult[nRef + a].GetAt(tmpCounter);
                    }
                    m_mainView->PostMessage(WM_EVAL);
                }

                // If the user wants a pause, sleep
                if (m_pause)
                {
                    CWinThread* thread = AfxGetThread();
                    m_sleeping = true;
                    if (m_mainView != 0)
                        m_mainView->PostMessage(WM_GOTO_SLEEP);
                    thread->SuspendThread();
                    m_sleeping = false;
                }

                // tell the user about the progress
                m_progress = (nDone++) / nToDo;
                if (m_mainView != nullptr && (m_curSpec % 10) == 0)
                    m_mainView->PostMessage(WM_PROGRESS, (WPARAM)(int)(100.0 * m_progress));
            } // end for(m_curspec...

            // if the sky was included into the fit it should now be removed from the list of references used
            if (m_settings.m_window.fitType == FIT_HP_SUB || m_settings.m_window.fitType == FIT_POLY)
                --m_settings.m_window.nRef;

            for (int tmpCounter = 0; tmpCounter < (fitHigh - fitLow); ++tmpCounter)
                m_avgResidual[tmpCounter] /= m_nAveragedInResidual;

            WriteAverageResidualToFile();
        }//end for chn..

        if (m_mainView != nullptr)
            m_mainView->PostMessage(WM_DONE);
    }


    fRun = false;
    return true;
}

bool CReEvaluator::ReadReferences(Evaluation::CEvaluation* evaluator)
{
    int i;
    CString refFileList[MAX_N_REFERENCES];
    CString message;

    if (m_settings.m_window.nRef == 0)
    {
        MessageBox(NULL, "No reference files selected, cannot reevaluate", "Error", MB_OK);
        return false;
    }

    for (i = 0; i < m_settings.m_window.nRef; ++i)
    {
        refFileList[i] = (LPCSTR)m_settings.m_window.ref[i].m_path.c_str();
    }

    // read the reference files
    if (!(evaluator->ReadRefList(refFileList, m_settings.m_window.nRef, MAX_SPECTRUM_LENGTH)))
    {
        message.Format("Can not read one or more of the reference files\n Please check the files.");
        MessageBox(NULL, message, TEXT("Error"), MB_OK);
        return 1;
    }

    return true;
}

/* GetSpectrum returns spectrum number @number collected with channel number
  @channel. This function takes the global parameter 'm_nAverageSpectra' into
  account, and averages the spectra if 'm_nAverageSpectra' > 1
  */
bool CReEvaluator::GetSpectrum(CSpectrum& spec, int number, int channel)
{
    // check for illegal values
    if (m_settings.m_nAverageSpectra <= 0)
    {
        return false;
    }

    CSpectrum spectrum[2];

    // in the simplest case, just read one spectrum from the file
    if (m_settings.m_nAverageSpectra == 1)
    {
        return ReadSpectrum(spec, number, channel);
    }

    for (int i = 0; i < m_settings.m_nAverageSpectra; ++i)
    {
        int specNo = number * m_settings.m_nAverageSpectra + i;
        ReadSpectrum(spectrum[(i != 0)], specNo, channel);

        if (i > 0)
        {
            spectrum[0].Add(spectrum[(i != 0)]);
        }
    }

    spectrum[0].Div(m_settings.m_nAverageSpectra);

    // copy spectrum[0] to @spec
    spec = CSpectrum(spectrum[0]);

    return true;
}

/* ReadSpectrum reads the spectrum file with the given number and channel
  from file (or from the internal buffer).*/
bool CReEvaluator::ReadSpectrum(CSpectrum& spec, int number, int channel)
{
    CString specFileName;

    specFileName.Format("%s\\%05d_%1d.STD", m_specFileDir, number, channel); // the file name
    if (CSpectrumIO::readSTDFile(specFileName, &spec))
    {
        specFileName.Format("%s\\%05d.STD", m_specFileDir, number); // the file name
        if (CSpectrumIO::readSTDFile(specFileName, &spec))
        {
            CString message;
            message.Format("Cannot read spectrum %s. Evaluation stopped.", specFileName);
            MessageBox(NULL, message, "Error", MB_OK);
            return false;
        }
    }

    return true;
}

/* Reads spectrum from file.  Used for sky, dark, darkcur, and offset. */
bool CReEvaluator::ReadSpectrumFromFile(CSpectrum& spec, CString filename, int channel)
{
    CString specFileName;
    specFileName.Format("%s\\%s_%1d.STD", m_specFileDir, filename, channel); // the file name
    if (CSpectrumIO::readSTDFile(specFileName, &spec))
    {
        specFileName.Format("%s\\%s.STD", m_specFileDir, filename); // the file name
        if (CSpectrumIO::readSTDFile(specFileName, &spec))
        {
            return false;
        }
    }

    if (m_mainView != nullptr)
    {
        m_statusMsg.Format("Read %s", specFileName);
        m_mainView->PostMessage(WM_STATUS);
    }

    return true;
}

/* Reads the sky spectrum */
bool CReEvaluator::ReadSkySpectrum(CSpectrum& spec, int channel)
{


    //// if the sky spectrum is 'sky_0.STD'
    if (m_settings.m_skySelection == USE_SKY_FIRST)
    {
        //CString specFileName;
        //specFileName.Format("%s\\sky_%1d.STD", m_specFileDir, channel); // the file name
        //if(CSpectrumIO::readSTDFile(specFileName, &spec)){
        //	specFileName.Format("%s\\sky.STD", m_specFileDir); // the file name
        //	if(CSpectrumIO::readSTDFile(specFileName, &spec)){
        //		CString message;
        //		message.Format("Cannot read spectrum %s. Evaluation stopped.", specFileName);
        //		MessageBox(NULL, message, "Error", MB_OK);
        //		return false;
        //	}
        //}
        //if(m_mainView != nullptr){
        //	m_statusMsg.Format("Read Sky spectrum");
        //	m_mainView->PostMessage(WM_STATUS);
        //}
        //return true;
        return ReadSpectrumFromFile(spec, "sky", channel);
    }

    // set the sky spectrum to zero
    spec.Clear();
    CSpectrum curSpectrum;
    GetSpectrum(curSpectrum, 0, channel);
    spec.length = curSpectrum.length;
    long nAdded = 0;

    // if the average of all spectra is to be used as sky
    if (USE_SKY_ALL == m_settings.m_skySelection)
    {
        for (int i = 0; i < m_recordNum[channel]; ++i)
        {
            GetSpectrum(curSpectrum, i, channel);
            if (!IsDark(curSpectrum))
            {
                spec.Add(curSpectrum);
                ++nAdded;
            }
        }
        spec.Div(nAdded);

        if (m_mainView != nullptr)
        {
            m_statusMsg.Format("Created Sky spectrum as average of: %ld spectra", nAdded);
            m_mainView->PostMessage(WM_STATUS);
        }
        return true;
    }

    // if the user has supplied a custom sky spectrum
    if (USE_SKY_USER == m_settings.m_skySelection)
    {
        // read the sky spectrum
        if (CSpectrumIO::readSTDFile(m_settings.m_skySpectrumFile, &spec))
        {
            if (CSpectrumIO::readTextFile(m_settings.m_skySpectrumFile, &spec))
            {
                CString message;
                message.Format("Cannot read sky spectrum: %s. Evaluation stopped", m_settings.m_skySpectrumFile);
                MessageBox(NULL, message, "Error", MB_OK);
                return false;
            }
        }
        CSpectrum darkSpec;
        // read the dark spectrum that comes with the sky
        if (CSpectrumIO::readSTDFile(m_settings.m_skySpectrumDark, &darkSpec))
        {
            if (CSpectrumIO::readTextFile(m_settings.m_skySpectrumDark, &darkSpec))
            {
                CString message;
                message.Format("Cannot read spectrum: %s. Evaluation stopped", m_settings.m_skySpectrumDark);
                MessageBox(NULL, message, "Error", MB_OK);
                return false;
            }
        }
        spec.Sub(darkSpec); // <-- Remove the dark from the sky

        if (m_mainView != nullptr)
        {
            m_statusMsg.Format("Read Sky spectrum");
            m_mainView->PostMessage(WM_STATUS);
        }
        return true;
    }

    // nothing selected
    return false;
}

bool CReEvaluator::CreateOutputDirectory()
{
    CString cDateTime;
    struct tm* tim;
    time_t t;

    time(&t);
    tim = localtime(&t);
    cDateTime.Format("%04d%02d%02d_%02d%02d", tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday, tim->tm_hour, tim->tm_min);

    m_outputDir = m_specFileDir + "\\ReEvaluation_" + cDateTime;

    if (0 == CreateDirectory(m_outputDir, NULL))
    {
        DWORD errorCode = GetLastError();
        if (errorCode != ERROR_ALREADY_EXISTS)
        { /* We shouldn't quit just because the directory that we want to create already exists. */
            CString tmpStr, errorStr;
            if (FormatErrorCode(errorCode, errorStr))
            {
                tmpStr.Format("Could not create output directory. Reason: %s", errorStr);
            }
            else
            {
                tmpStr.Format("Could not create output directory, not enough free disk space?. Error code returned %ld", errorCode);
            }
            MessageBox(NULL, tmpStr, "ERROR", MB_OK);
            return false;
        }
    }
    return true;
}

bool CReEvaluator::WriteEvaluationLogHeader(int channel)
{
    int i;

    if (this->m_nChannels == 1)
    {
        m_outputLog = m_outputDir + "\\ReEvaluationLog.txt";
    }
    else
    {
        if (channel == 0)
        {
            m_outputLog = m_outputDir + "\\ReEvaluationLog_Master.txt";
        }
        else
        {
            m_outputLog.Format(m_outputDir + "\\ReEvaluationLog_Slave%02d.txt", channel - 1);
        }
    }

    FILE* f = fopen(m_outputLog, "w");
    if (f == 0)
    {
        return false;
    }
    fprintf(f, "***Desktop Mobile Program***\nVERSION=%1d.%1d.%1d\nFILETYPE=ReEvaluationlog\n", CVersion::majorNumber, CVersion::minorNumber, CVersion::patchNumber);
    fprintf(f, "Original EvaluationLog=%s\n", (LPCTSTR)m_evalLogFileName);
    fprintf(f, "***Settings Used in the Evaluation***\n");
    fprintf(f, "FitFrom=%d\nFitTo=%d\nPolynom=%d\n", m_settings.m_window.fitLow, m_settings.m_window.fitHigh, m_settings.m_window.polyOrder);
    fprintf(f, "Number of averaged spectra=%ld\n", m_settings.m_nAverageSpectra);
    switch (m_settings.m_ignoreDark.selection)
    {
    case IGNORE_DARK:  fprintf(f, "Ignore Dark Spectra=1\n"); break;
    case IGNORE_LIMIT: fprintf(f, "Ignore Spectra with intensity below=%.1lf @ channel %d\n", m_settings.m_ignoreDark.intensity, m_settings.m_ignoreDark.channel); break;
    case IGNORE_LIST:  fprintf(f, "Ignore Spectrum number: ");
        for (i = 0; i < MAX_IGNORE_LIST_LENGTH; ++i)
        {
            if (m_settings.m_ignoreList_Lower[i] != -1)
                fprintf(f, "%ld\t", m_settings.m_ignoreList_Lower[i]);
        }
        break;
    }
    if (m_settings.m_ignoreSaturated.selection == IGNORE_LIMIT)
    {
        fprintf(f, "Ignore Spectra with intensity above=%.1lf @ channel %d\n", m_settings.m_ignoreSaturated.intensity, m_settings.m_ignoreSaturated.channel);
    }
    fprintf(f, "Sky Spectrum=%s\n", "sky_0.STD");
    fprintf(f, "Dark Spectrum=%s\n", "dark_0.STD");
    fprintf(f, "nSpecies=%d\n", m_settings.m_window.nRef);
    fprintf(f, "Specie\tShift\tSqueeze\tReferenceFile\n");
    for (i = 0; i < m_settings.m_window.nRef; ++i)
    {
        novac::CReferenceFile& ref = m_settings.m_window.ref[i];

        fprintf(f, "%s\t", ref.m_specieName.c_str());
        switch (ref.m_shiftOption)
        {
        case novac::SHIFT_TYPE::SHIFT_FIX:
            fprintf(f, "%0.3lf\t", ref.m_shiftValue); break;
        case novac::SHIFT_TYPE::SHIFT_LINK:
            fprintf(f, "linked to %s\t", m_settings.m_window.ref[(int)ref.m_shiftValue].m_specieName.c_str()); break;
        default:
            fprintf(f, "free\t"); break;
        }
        switch (ref.m_squeezeOption)
        {
        case novac::SHIFT_TYPE::SHIFT_FIX:
            fprintf(f, "%0.3lf\t", ref.m_squeezeValue); break;
        case novac::SHIFT_TYPE::SHIFT_LINK:
            fprintf(f, "linked to %s\t", m_settings.m_window.ref[(int)ref.m_squeezeValue].m_specieName.c_str()); break;
        default:
            fprintf(f, "free\t"); break;
        }
        fprintf(f, "%s\n", ref.m_path.c_str());
    }
    fprintf(f, "\n");
    fprintf(f, "#Time\tLat\tLong\tAlt\tNSpec\tExpTime\tIntens\t");
    for (i = 0; i < m_settings.m_window.nRef; ++i)
    {
        const char* name = m_settings.m_window.ref[i].m_specieName.c_str();
        fprintf(f, "%s(column)\t%s(columnError)\t%s(shift)\t%s(shiftError)\t%s(squeeze)\t%s(squeezeError)\t",
            name, (LPCTSTR)name, (LPCTSTR)name, (LPCTSTR)name, (LPCTSTR)name, (LPCTSTR)name);
    }

    fprintf(f, "Delta\tChi²\n");

    // Write the information about the spectrometer
    fprintf(f, "***Spectrometer Information***\n");
    fprintf(f, "SERIAL=%s\n", (LPCTSTR)m_spectrometerName);
    fprintf(f, "DETECTORSIZE=%ld\n", m_detectorSize);
    fprintf(f, "DYNAMICRANGE=%ld\n", m_spectrometerDynRange);
    fprintf(f, "MODEL=%s\n", (LPCTSTR)m_spectrometerModel);

    fclose(f);
    return true;
}

bool CReEvaluator::AppendResultToEvaluationLog(int specIndex, int channel, Evaluation::CEvaluation& evaluator)
{
    GetResult(evaluator);

    FILE* f = fopen(m_outputLog, "a+");
    if (0 == f)
        return false;

    int hr, mi, se;
    mobiledoas::GetHrMinSec(m_time[specIndex], hr, mi, se);
    fprintf(f, "%02d:%02d:%02d\t", hr, mi, se);
    fprintf(f, "%.6lf\t%.6lf\t%d\t%d\t%d\t%d\t",
        m_lat[specIndex], m_lon[specIndex], m_alt[specIndex], m_nspec[specIndex], m_exptime[specIndex], (int)m_int[channel][specIndex]);

    int nRef = 0;
    for (int i = 0; i < m_settings.m_window.nRef; ++i)
    {
        const auto evResult = evaluator.GetResult(nRef);
        ++nRef;
        fprintf(f, "%0.6G\t", evResult.column);
        fprintf(f, "%0.6G\t", evResult.columnError);
        fprintf(f, "%0.6G\t", evResult.shift);
        fprintf(f, "%0.6G\t", evResult.shiftError);
        fprintf(f, "%0.6G\t", evResult.squeeze);
        fprintf(f, "%0.6G\t", evResult.squeezeError);

        m_evResult[i][0] = evResult.column;
        m_evResult[i][1] = evResult.columnError;
        m_evResult[i][2] = evResult.shift;
        m_evResult[i][3] = evResult.shiftError;
        m_evResult[i][4] = evResult.squeeze;
        m_evResult[i][5] = evResult.squeezeError;
    }

    // finally write the quality of the fit
    fprintf(f, "%.2e\t", m_delta);
    fprintf(f, "%.2e", m_chiSquare);

    fprintf(f, "\n");
    fclose(f);

    return true;
}
bool CReEvaluator::GetDarkSpectrum(CSpectrum& dark, int number, int channel)
{

    // If there are only one dark spectrum, don't do anything
    if (m_darkSpecListLength[channel] <= 0)
        return true;

    // Find all dark spectra with the same exp-time as the supplied one. 
    int expTime = m_exptime[number];
    int foundDarkSpectra[256];
    int	nFoundDarkSpectra = 0;
    for (int k = 0; k < m_darkSpecListLength[channel]; ++k)
    {
        if (m_exptime[m_darkSpecList[channel][k]] == expTime)
        {
            foundDarkSpectra[nFoundDarkSpectra] = m_darkSpecList[channel][k];
            ++nFoundDarkSpectra;
        }
    }
    // If none found, return the first one...
    if (nFoundDarkSpectra == 0)
    {
        FILE* f = fopen(m_outputDir + "\\DarkLog.txt", "a+");
        if (f != nullptr)
        {
            fprintf(f, "Spec # %d - using default dark\n", number);
            fclose(f);
        }
        return false; // <-- error!
    }

    // If only one dark spectrum with the same exp-time was found, return it
    if (nFoundDarkSpectra == 1)
    {
        FILE* f = fopen(m_outputDir + "\\DarkLog.txt", "a+");
        if (f != nullptr)
        {
            fprintf(f, "Spec # %d - using spectrum %d as dark\n", number, foundDarkSpectra[0]);
            fclose(f);
        }
        if (m_mainView != nullptr)
        {
            m_statusMsg.Format("Spec # %d - using spectrum %d as dark\n", number, foundDarkSpectra[0]);
            m_mainView->PostMessage(WM_STATUS);
        }
        return GetSpectrum(dark, foundDarkSpectra[0], channel);
    }

    // If several found, find out if we should interpolate or take the closest one
    int closestAbove = -1;	// <-- the dark spectrum with the nearest higher offset
    int closestBelow = -1;	// <-- the dark spectrum with the nearest lower offset
    double specOffset = m_offset[channel][number]; // <-- the offset of the measured spectrum
    double closestHigherOffset = 1e16, closestLowerOffset = -1e16;

    // loop through all dark spectra with the same exposure-time
    for (int k = 0; k < nFoundDarkSpectra; ++k)
    {
        // get the offset of dark spectrum number 'k' 
        double darkOffset = m_offset[channel][foundDarkSpectra[k]];

        if (darkOffset > specOffset)
        {
            if (darkOffset < closestHigherOffset)
            {
                closestHigherOffset = darkOffset;
                closestAbove = foundDarkSpectra[k];
            }
        }
        else
        {
            if (darkOffset > closestLowerOffset)
            {
                closestLowerOffset = darkOffset;
                closestBelow = foundDarkSpectra[k];
            }
        }
    }

    if (closestBelow == -1 && closestAbove != -1)
    {
        // 1. If only a dark spectrum with higher offset was found...
        FILE* f = fopen(m_outputDir + "\\DarkLog.txt", "a+");
        if (f != nullptr)
        {
            fprintf(f, "Spec # %d - using spectrum %d as dark\n", number, closestAbove);
            fclose(f);
        }
        if (m_mainView != nullptr)
        {
            m_statusMsg.Format("Spec # %d - using spectrum %d as dark\n", number, closestAbove);
            m_mainView->PostMessage(WM_STATUS);
        }
        return GetSpectrum(dark, closestAbove, channel);

    }
    else if (closestBelow != -1 && closestAbove == -1)
    {
        // 2. If only a dark spectrum with lower offset was found...
        FILE* f = fopen(m_outputDir + "\\DarkLog.txt", "a+");
        if (f != nullptr)
        {
            fprintf(f, "Spec # %d - using spectrum %d as dark\n", number, closestBelow);
            fclose(f);
        }
        if (m_mainView != nullptr)
        {
            m_statusMsg.Format("Spec # %d - using spectrum %d as dark\n", number, closestBelow);
            m_mainView->PostMessage(WM_STATUS);
        }
        return GetSpectrum(dark, closestBelow, channel);

    }
    else
    {
        // 3. If we found one dark with higher offset and one dark with lower offset...
        CSpectrum dark2;
        GetSpectrum(dark, closestAbove, channel);
        GetSpectrum(dark2, closestBelow, channel);

        double alpha = (specOffset - closestLowerOffset) / (closestHigherOffset - closestLowerOffset);
        for (int k = 0; k < dark2.length; ++k)
        {
            dark.I[k] = alpha * dark.I[k] + (1.0 - alpha) * dark2.I[k];
        }
        FILE* f = fopen(m_outputDir + "\\DarkLog.txt", "a+");
        if (f != nullptr)
        {
            fprintf(f, "Spec # %d - using average of spectra %d and %d as dark\n", number, closestBelow, closestAbove);
            fclose(f);
        }
        if (m_mainView != nullptr)
        {
            m_statusMsg.Format("Spec # %d - using average of spectra %d and %d as dark\n", number, closestBelow, closestAbove);
            m_mainView->PostMessage(WM_STATUS);
        }
        return true;
    }

    // should not get here...
    return false;
}

bool CReEvaluator::Stop()
{
    this->fRun = false;
    return true;
}

bool CReEvaluator::FindOptimalShiftAndSqueeze(Evaluation::CEvaluation& evaluator, int channel, CSpectrum& skySpectrum, CSpectrum& darkSpectrum)
{
    double		sky[MAX_SPECTRUM_LENGTH];// temporary storage for the sky vector
    CSpectrum curSpectrum; // the read spectrum
    double		maxColumn = -1e6;
    int				indexOfMaxColumn = -1;
    int				nLinked = 0;
    int				linkTo = 0;

    // first find the spectrum with the highest (credible) column value
    for (int i = 0; i < m_recordNum[channel]; ++i)
    {
        if (m_oldCol[channel][i] <= maxColumn)
            continue;

        if (m_int[channel][i] > MINIMUM_CREDIBLE_INTENSITY)
        {
            GetSpectrum(curSpectrum, i, channel);
            if (!Ignore(curSpectrum, i))
            {
                maxColumn = m_oldCol[channel][i];
                indexOfMaxColumn = i;
            }
        }
    }
    if (indexOfMaxColumn == -1)
        return false; // no credible column value found...

    // start by setting the shift and squeeze of the first referenceFile to SHIFT_FREE
    //  and link the other references to this referenceFile
    for (int i = 0; i < m_settings.m_window.nRef; ++i)
    {
        novac::CReferenceFile& ref = m_settings.m_window.ref[i];

        if (nLinked == 0)
        {
            // the first reference file
            ref.m_shiftOption = novac::SHIFT_TYPE::SHIFT_FREE;
            ref.m_shiftValue = 0;
            ref.m_squeezeOption = novac::SHIFT_TYPE::SHIFT_FREE;
            ref.m_squeezeValue = 1;
            linkTo = i;
        }
        else
        {
            // all other reference files
            ref.m_shiftOption = novac::SHIFT_TYPE::SHIFT_LINK;
            ref.m_squeezeOption = novac::SHIFT_TYPE::SHIFT_LINK;
            ref.m_shiftValue = linkTo;
            ref.m_squeezeValue = linkTo;
        }
        ++nLinked;
    } // done setting shift and squeezes...

    // if the sky is included in the fit, it should not be linked to the others
    if (m_settings.m_window.fitType == FIT_HP_SUB || m_settings.m_window.fitType == FIT_POLY)
    {
        m_settings.m_window.ref[m_settings.m_window.nRef - 1].m_shiftOption = novac::SHIFT_TYPE::SHIFT_FREE;
        m_settings.m_window.ref[m_settings.m_window.nRef - 1].m_squeezeOption = novac::SHIFT_TYPE::SHIFT_FIX;
        m_settings.m_window.ref[m_settings.m_window.nRef - 1].m_squeezeValue = 1;
    }

    // initialize the evaluator to our newly changed values
    evaluator.SetFitWindow(m_settings.m_window);

    // send a message about the progress
    if (m_mainView != nullptr)
    {
        m_statusMsg.Format("Determining shift & squeeze from spec #%d", indexOfMaxColumn);
        m_mainView->PostMessage(WM_STATUS);
    }

    // get the spectrum
    GetSpectrum(curSpectrum, indexOfMaxColumn, channel);

    memcpy(sky, skySpectrum.I, MAX_SPECTRUM_LENGTH * sizeof(double));

    // do the evaluation
    evaluator.Evaluate(darkSpectrum.I, sky, curSpectrum.I, 5000);

    // Check if the result is reasonable, if not then only allow the shift to wary - not the squeeze
    if (evaluator.GetChiSquare() > 0.9)
    {
        for (int i = 0; i < m_settings.m_window.nRef; ++i)
        {
            novac::CReferenceFile& ref = m_settings.m_window.ref[i];
            ref.m_squeezeOption = novac::SHIFT_TYPE::SHIFT_FIX;
            ref.m_squeezeValue = 1;
        } // done setting shift and squeezes...

        // do the evaluation again
        evaluator.SetFitWindow(m_settings.m_window);
        evaluator.Evaluate(darkSpectrum.I, sky, curSpectrum.I, 5000);
    }

    // The result
    Evaluation::EvaluationResult evResult = evaluator.GetResult();

    // extract the result
    optimumShift = evResult.shift;
    optimumShiftError = evResult.shiftError;
    optimumSqueeze = evResult.squeeze;
    optimumSqueezeError = evResult.squeezeError;

    // send a message about the progress
    if (m_mainView != nullptr)
    {
        m_statusMsg.Format("Shift set to %.2f; Squeeze set to %.2f", optimumShift, optimumSqueeze);
        m_mainView->PostMessage(WM_STATUS);
    }

    // set the shift and squeeze of the reference files to the
    //  newly found optimum value
    for (int i = 0; i < m_settings.m_window.nRef; ++i)
    {
        novac::CReferenceFile& ref = m_settings.m_window.ref[i];

        ref.m_shiftOption = novac::SHIFT_TYPE::SHIFT_FIX;
        ref.m_squeezeOption = novac::SHIFT_TYPE::SHIFT_FIX;
        ref.m_shiftValue = (double)optimumShift;
        ref.m_squeezeValue = (double)optimumSqueeze;
    } // done setting shift and squeezes...

    // if the fit is included in the fit, it should not be linked to the others
    if (m_settings.m_window.fitType == FIT_HP_SUB || m_settings.m_window.fitType == FIT_POLY)
    {
        evResult = evaluator.GetResult(m_settings.m_window.nRef - 1);
        m_settings.m_window.ref[m_settings.m_window.nRef - 1].m_shiftOption = novac::SHIFT_TYPE::SHIFT_FREE;
        m_settings.m_window.ref[m_settings.m_window.nRef - 1].m_shiftValue = evResult.shift;
        m_settings.m_window.ref[m_settings.m_window.nRef - 1].m_squeezeOption = novac::SHIFT_TYPE::SHIFT_FIX;
        m_settings.m_window.ref[m_settings.m_window.nRef - 1].m_squeezeValue = evResult.squeeze;
    }

    return true;
}

/* Check the settings before we start */
bool CReEvaluator::MakeInitialSanityCheck()
{

    if (this->m_recordNum[0] == 0)
    {
        MessageBox(NULL, "No spectra found. You have either not chosen an evaluation log or evaluation log is empty. Please choose a proper evaluation log", "Error", MB_OK);
        return false;
    }

    bool findOptimum = false;
    int i;
    for (i = 0; i < m_settings.m_window.nRef; ++i)
    {
        if (m_settings.m_window.ref[i].m_shiftOption == novac::SHIFT_TYPE::SHIFT_OPTIMAL)
        {
            findOptimum = true;
            break;
        }
    }
    if (findOptimum)
    {
        for (int k = 0; k < m_settings.m_window.nRef; ++k)
        {
            if (m_settings.m_window.ref[k].m_shiftOption != novac::SHIFT_TYPE::SHIFT_LINK && k != i)
            {
                CString msg;
                msg.Format("You have selected 'find optimum' for reference %s. This means that all references will be linked to this reference and the optimum shift and squeeze for all references will be searched for", m_settings.m_window.ref[k].m_specieName);
                MessageBox(NULL, msg, "Info", MB_OK);
            }
        }

    }

    return true;
}

/* write the average residual to a file */
bool CReEvaluator::WriteAverageResidualToFile()
{
    int fitWidth = m_settings.m_window.fitHigh - m_settings.m_window.fitLow;

    CString fileName = m_outputDir + "\\AverageResidual.txt";
    FILE* f = fopen(fileName, "w");
    if (f == nullptr)
        return false;

    for (int i = 0; i < fitWidth; ++i)
    {
        fprintf(f, "%le\n", m_avgResidual[i]);
    }
    fclose(f);
    return true;
}

/* saves a copy of the spectra */
bool CReEvaluator::SaveSpectra(CSpectrum& spec, CString filename, int channel)
{
    CString fileName;
    fileName.Format("%s\\%s_%1d.txt", m_outputDir, filename, channel);
    FILE* f = fopen(fileName, "w");
    if (f != nullptr)
    {
        for (int i = 0; i < spec.length; ++i)
        {
            fprintf(f, "%lg\n", spec.I[i]);
        }
        fclose(f);
    }
    else
    {
        return false;
    }

    return true;
}

int CReEvaluator::Ignore(CSpectrum& spec, int spectrumNumber)
{

    // if the user wants to ignore completely dark spectra
    if (m_settings.m_ignoreDark.selection == IGNORE_DARK)
    {
        if (IsDark(spec))
            return INTENSITY_DARK;
    }

    // if the user wants to ignore spectra with intensity lower than a certain limit
    if (m_settings.m_ignoreDark.selection == IGNORE_LIMIT)
    {
        if (spec.GetAverage(m_settings.m_ignoreDark.channel - 10, m_settings.m_ignoreDark.channel + 10) < m_settings.m_ignoreDark.intensity)
            return INTENSITY_DARK;
    }

    // if the user has a list of spectra to ignore
    if (m_settings.m_ignoreDark.selection == IGNORE_LIST)
    {
        for (int i = 0; i < MAX_IGNORE_LIST_LENGTH; ++i)
        {
            if (m_settings.m_ignoreList_Lower[i] == spectrumNumber)
                return INTENSITY_DARK;
        }
    }

    // if the user wants to ignore spectra with intensity higher than a certain limit
    if (m_settings.m_ignoreSaturated.selection == IGNORE_LIMIT)
    {
        if (spec.GetAverage(m_settings.m_ignoreSaturated.channel - 10, m_settings.m_ignoreSaturated.channel + 10) > m_settings.m_ignoreSaturated.intensity)
            return INTENSITY_SATURATED;
    }

    // no reason has been found why we should ignore the following spectrum
    return 0;
}

bool CReEvaluator::GetResult(const Evaluation::CEvaluation& evaluator)
{

    m_delta = evaluator.GetDelta();
    m_chiSquare = evaluator.GetChiSquare();

    return true;
}

/** Includes the sky spectrum into the fit */
bool  CReEvaluator::IncludeSkySpecInFit(Evaluation::CEvaluation& eval, const CSpectrum& skySpectrum, Evaluation::CFitWindow& window)
{
    double sky[MAX_SPECTRUM_LENGTH];

    // first make a local copy of the sky spectrum
    CSpectrum tmpSpec = skySpectrum;

    int specLen = tmpSpec.length;

    memcpy(sky, tmpSpec.I, specLen * sizeof(double));
    eval.RemoveOffset(sky, specLen, window.offsetFrom, window.offsetTo);
    if (window.fitType == FIT_HP_SUB)
        eval.HighPassBinomial(sky, specLen, 500);

    eval.Log(sky, specLen);
    eval.IncludeAsReference(sky, specLen, window.nRef);

    // set the shift and squeeze
    window.ref[window.nRef].m_shiftOption = novac::SHIFT_TYPE::SHIFT_FREE;
    window.ref[window.nRef].m_squeezeOption = novac::SHIFT_TYPE::SHIFT_FIX;
    window.ref[window.nRef].m_squeezeValue = 1;

    // set the column value to 1
    window.ref[window.nRef].m_columnOption = novac::SHIFT_TYPE::SHIFT_FIX;
    window.ref[window.nRef].m_columnValue = (window.fitType == FIT_POLY) ? -1.0 : 1.0;

    // set the name of the reference
    window.ref[window.nRef].m_specieName = "FraunhoferRef";

    ++window.nRef;
    return true;
}

/** Checks if all spectra have the same exposure time,
        returns true if all spectra have same exp-time, otherwise return false */
bool CReEvaluator::AllSpectraHaveSameExpTime()
{

    int expT = m_exptime[0];

    // loop through all results and check the exposure times
    for (int k = 1; k < m_recordNum[0]; ++k)
    {
        if (m_exptime[k] != expT)
            return false;
    }

    return true;
}
