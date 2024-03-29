// Common.cpp: implementation of the Common class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DMSpec.h"
#include "Common.h"
#include <utility>
#include <vector>
#include <string>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------
// ---------------- GLOBAL FUNCTIONS -----------------------------
// ---------------------------------------------------------------
int Equals(const CString& str1, const CString& str2) {
    return (0 == _tcsnicmp(str1, str2, max(strlen(str1), strlen(str2))));
}

int Equals(const CString& str1, const CString& str2, unsigned int nCharacters) {
    return (0 == _tcsnicmp(str1, str2, min(nCharacters, max(strlen(str1), strlen(str2)))));
}

int IsExistingFile(const CString* fileName) {
    FILE* f = fopen(*fileName, "r");
    if (f < (FILE*)1)
        return(0);

    fclose(f);
    return(1);
}

int IsExistingFile(const CString& fileName) {
    return IsExistingFile(&fileName);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Common::Common()
{

}

Common::~Common()
{

}
void Common::WriteLogFile(CString filename, CString txt)
{
    FILE* f;
    f = fopen(filename, "a+");

    if (f < (FILE*)1) {
        CString tmpStr;
        tmpStr.Format("Could not write to log file: %s. Not enough free space?", (LPCTSTR)filename);
        MessageBox(NULL, tmpStr, "Big Error", MB_OK);
        return;
    }

    fprintf(f, txt + "\n");

    fclose(f);
}

void Common::GetTimeText(char* txt)
{
    struct tm* tim;
    time_t t;

    txt[0] = 0;
    time(&t);
    tim = localtime(&t);
    sprintf(txt, "%02d:%02d:%02d", tim->tm_hour, tim->tm_min, tim->tm_sec);
}

void Common::GetTimeText(CString& str)
{
    char txt[512];
    GetTimeText(txt);
    str.Format(txt);
}


void Common::GetDateText(char* txt)
{
    struct tm* tim;
    time_t t;

    txt[0] = 0;
    time(&t);
    tim = localtime(&t);
    sprintf(txt, "%04d.%02d.%02d", tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday);
}

void Common::GetDateText(CString& str) {
    char txt[512];
    GetDateText(txt);
    str.Format(txt);
}

void Common::GetDateTimeText(char* txt)
{
    struct tm* tim;
    time_t t;

    txt[0] = 0;
    time(&t);
    tim = localtime(&t);
    sprintf(txt, "%04d.%02d.%02d  %02d:%02d:%02d", tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday, tim->tm_hour, tim->tm_min, tim->tm_sec);
}

void Common::GetDateTimeText(CString& str)
{
    char txt[512];
    GetDateTimeText(txt);
    str.Format(txt);
}

void Common::GetDateTimeTextPlainFormat(char* txt)
{
    struct tm* tim;
    time_t t;

    txt[0] = 0;
    time(&t);
    tim = localtime(&t);
    sprintf(txt, "%04d.%02d.%02d_%02d%02d%02d", tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday, tim->tm_hour, tim->tm_min, tim->tm_sec);
}

void Common::GetDateTimeTextPlainFormat(CString& str)
{
    char txt[512];
    GetDateTimeTextPlainFormat(txt);
    str.Format(txt);
}

void Common::GetExePath()
{
    TCHAR exeFullPath[MAX_PATH];
    GetModuleFileName(NULL, exeFullPath, MAX_PATH);
    m_exePath = (CString)exeFullPath;
    int position = m_exePath.ReverseFind('\\');
    m_exePath = m_exePath.Left(position + 1);
}

/*  adapts parameters k and m so that y = k*x + m, in a
    least square sense.
    Algorithm found at: http://mathworld.wolfram.com/LeastSquaresFittingPolynomial.html */
int Common::AdaptStraightLine(double* x, double* y, unsigned int l, double* k, double* m) {
    double sx = 0, sy = 0, sx2 = 0, sxy = 0, det_inv;
    double M_inv[2][2], XTy[2]; /*M=X^T * X, M_inv = M^-1, XTy = X^T * y */
    unsigned int i;

    if ((x == 0) || (y == 0) || (l == 0) || (k == 0) || (m == 0)) {
        return 1;
    }

    for (i = 0; i < l; ++i) {
        sx += x[i];
        sy += y[i];
        sx2 += x[i] * x[i];
        sxy += x[i] * y[i];
    }

    det_inv = 1 / (sx2 * l - sx * sx);
    M_inv[0][0] = sx2 * det_inv;
    M_inv[0][1] = -sx * det_inv;
    M_inv[1][0] = -sx * det_inv;
    M_inv[1][1] = l * det_inv;

    XTy[0] = sy;
    XTy[1] = sxy;

    *(m) = M_inv[0][0] * XTy[0] + M_inv[0][1] * XTy[1];
    *(k) = M_inv[1][0] * XTy[0] + M_inv[1][1] * XTy[1];

    return 0;
}

void Common::GuessSpecieName(const CString& fileName, CString& specie) {
    specie.Format("");
    CString spc[] = { "SO2", "NO2", "O3", "O4", "HCHO", "RING", "H2O", "CLO", "BRO", "CHOCHO", "Glyoxal", "Formaldehyde" };
    int nSpecies = 12;

    int index = fileName.ReverseFind('\\');
    if (index == 0)
        return;

    CString fil;
    fil.Format("%s", (LPCTSTR)fileName.Right((int)strlen(fileName) - index - 1));
    fil.MakeUpper();

    for (int i = 0; i < nSpecies; ++i) {
        if (strstr(fil, spc[i])) {
            specie.Format("%s", (LPCTSTR)spc[i]);
            return;
        }
    }

    // nothing found
    return;
}


bool FormatErrorCode(DWORD error, CString& string) {
    /* from System Error Codes */
    switch (error) {
    case ERROR_FILE_NOT_FOUND:
        string.Format("File not found"); return true;
    case ERROR_PATH_NOT_FOUND:
        string.Format("Path not found"); return true;
    case ERROR_TOO_MANY_OPEN_FILES:
        string.Format("Too many open files"); return true;
    case ERROR_ACCESS_DENIED:
        string.Format("Access denied"); return true;
    case ERROR_NOT_ENOUGH_MEMORY:
        string.Format("Not enough memory"); return true;
    case ERROR_OUTOFMEMORY:
        string.Format("Out of memory"); return true;
    case ERROR_WRITE_PROTECT:
        string.Format("The media is write protected"); return true;
    case ERROR_SEEK:
        string.Format("The drive cannot locate a specific area or track on the disk."); return true;
    case ERROR_WRITE_FAULT:
        string.Format("The system cannot write to the specified device"); return true;
    case ERROR_READ_FAULT:
        string.Format("The system cannot read from the specified device"); return true;
    case ERROR_HANDLE_DISK_FULL:
    case ERROR_DISK_FULL:
        string.Format("The disk is full"); return true;
    case ERROR_CANNOT_MAKE:
        string.Format("The directory or file cannot be created"); return true;
    case ERROR_BUFFER_OVERFLOW:
        string.Format("The file name is too long"); return true;
    case ERROR_INVALID_NAME:
        string.Format("The filename, directory name, or volume label syntax is incorrect"); return true;
    case ERROR_DIRECTORY:
        string.Format("The directory name is invalid"); return true;
    case ERROR_DISK_TOO_FRAGMENTED:
        string.Format("The volume is too fragmented to complete this operation"); return true;
    case ERROR_ARITHMETIC_OVERFLOW:
        string.Format("Arithmetic result exceeded 32 bits"); return true;
    case ERROR_ALREADY_EXISTS:
        string.Format("The file already exists"); return true;

    }

    return false;
}

/* Calculates the distance in meters between the point (lat1, lon1) and the point
  (lat2, lon2).
    @lat1 - latitude of position 1  [degrees]
    @lon1 - longitude of position 1 [degrees]
    @lat2 - latitude of position 2  [degrees]
    @lon2 - longitude of position 2 [degrees]
*/
double GPSDistance(double lat1, double lon1, double lat2, double lon2) {
    const double R_Earth = 6367000;
    double distance, dlon, dlat, a, c;
    lat1 = lat1 * DEGREETORAD;
    lat2 = lat2 * DEGREETORAD;
    lon1 = lon1 * DEGREETORAD;
    lon2 = lon2 * DEGREETORAD;
    dlon = lon2 - lon1;
    dlat = lat2 - lat1;
    a = pow((sin(dlat / 2)), 2) + cos(lat1) * cos(lat2) * pow((sin(dlon / 2)), 2);
    c = 2 * asin(min(1, sqrt(a)));
    distance = R_Earth * c;

    return distance;

}

/** Calculate the bearing from point 1 to point 2.
  Bearing is here defined as the angle between the direction to point 2 (at point 1)
    and the direction to north (at point 1).
*@lat1 - the latitude of beginning point,   [degree]
*@lon1 - the longitude of beginning point,  [degree]
*@lat2 - the latitude of ending point,      [degree]
*@lon2 - the longitude of ending point,     [degree]
*/
double GPSBearing(double lat1, double lon1, double lat2, double lon2) {
    double angle, dLat, dLon;

    lat1 = lat1 * DEGREETORAD;
    lat2 = lat2 * DEGREETORAD;
    lon1 = lon1 * DEGREETORAD;
    lon2 = lon2 * DEGREETORAD;

    dLat = lat1 - lat2;
    dLon = lon1 - lon2;

    if ((dLon == 0) && (dLat == 0))
        angle = 0;
    else
        angle = atan2(-sin(dLon) * cos(lat2),
            cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon));

    /*  	angle = atan2(lon1*cos(lat1)-lon2*cos(lat2), lat1-lat2); */

    if (angle < 0)
        angle = TWO_PI + angle;

    angle = RADTODEGREE * angle;
    return angle;
}

/** This function calculates the latitude and longitude for a point
        which is the distance 'dist' m and bearing 'az' degrees from
        the point defied by 'lat1' and 'lon1' */
void CalculateDestination(double lat1, double lon1, double dist, double az, double& lat2, double& lon2) {
    const double R_Earth = 6367000; // radius of the earth

    double dR = dist / R_Earth;

    // convert to radians
    lat1 = lat1 * DEGREETORAD;
    lon1 = lon1 * DEGREETORAD;
    az = az * DEGREETORAD;

    // calculate the second point
    lat2 = asin(sin(lat1) * cos(dR) + cos(lat1) * sin(dR) * cos(az));

    lon2 = lon1 + atan2(sin(az) * sin(dR) * cos(lat1), cos(dR) - sin(lat1) * sin(lat2));

    // convert back to degrees
    lat2 = lat2 * RADTODEGREE;
    lon2 = lon2 * RADTODEGREE;
}


double GetWindFactor(double lat1, double lon1, double lat2, double lon2, double windAngle) {
    double windFactor, travelAngle, difAngle;

    travelAngle = DEGREETORAD * GPSBearing(lat1, lon1, lat2, lon2);

    /* plumeAngle = pi + windAngle */
    difAngle = travelAngle + 1.5 * M_PI - windAngle * DEGREETORAD;	// this is the difference between travel direction and plume direction
    windFactor = cos(difAngle);

    return windFactor;
}

bool Common::BrowseForDirectory(CString& folder)
{
    IFileOpenDialog* pfd;
    // CoCreate the dialog object.
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pfd));
    bool success = false;

    if (SUCCEEDED(hr))
    {
        DWORD dwOptions;
        hr = pfd->GetOptions(&dwOptions);

        if (SUCCEEDED(hr))
        {
            hr = pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }

        if (SUCCEEDED(hr))
        {
            // Show the Open dialog.
            if (!folder.IsEmpty()) {
                IShellItem* dir;
                hr = SHCreateItemFromParsingName(CT2CW(folder), NULL, IID_IShellItem, (void**)&dir);
                if (SUCCEEDED(hr)) {
                    pfd->SetFolder(dir);
                }
            }
            hr = pfd->Show(NULL);

            if (SUCCEEDED(hr))
            {
                // Obtain the result of the user interaction.
                IShellItem* file;
                hr = pfd->GetResult(&file);

                if (SUCCEEDED(hr))
                {
                    LPWSTR dn;
                    file->GetDisplayName(SIGDN_FILESYSPATH, &dn);
                    folder = dn;
                    success = true;
                    file->Release();
                }
            }
        }
        pfd->Release();
    }
    return success;
}

std::vector<std::string> Common::ListFilesInDirectory(const char* directory, const char* fileNameFilter)
{
    std::vector<std::string> result;

    char searchPattern[MAX_PATH];
    if (fileNameFilter == nullptr)
    {
        sprintf(searchPattern, "%s*", directory);
    }
    else
    {
        sprintf(searchPattern, "%s%s", directory, fileNameFilter);
    }

    WIN32_FIND_DATA FindFileData;
    HANDLE hFile = FindFirstFile(searchPattern, &FindFileData);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return result; // no files found
    }

    do {
        if (Equals(FindFileData.cFileName, ".") || Equals(FindFileData.cFileName, ".."))
        {
            continue;
        }

        if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // This is a directory that we've found. Skip
        }
        else
        {
            // Get the full name of the file that we've just found...
            CString fileName;
            fileName.Format("%s%s", directory, FindFileData.cFileName);
            result.push_back(std::string(fileName));
        }
    } while (0 != FindNextFile(hFile, &FindFileData));

    FindClose(hFile);

    return result;
}

std::vector<CString> Common::BrowseForFiles()
{
    std::vector<CString> filenames;
    IFileOpenDialog* pfd;
    // CoCreate the dialog object.
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pfd));

    if (SUCCEEDED(hr))
    {
        DWORD dwOptions;
        // Specify multiselect.
        hr = pfd->GetOptions(&dwOptions);

        if (SUCCEEDED(hr))
        {
            hr = pfd->SetOptions(dwOptions | FOS_ALLOWMULTISELECT);
        }

        if (SUCCEEDED(hr))
        {

            COMDLG_FILTERSPEC aFileTypes[] = {
                { L"Text files", L"*.txt" }
            };
            hr = pfd->SetFileTypes(ARRAYSIZE(aFileTypes), aFileTypes);
        }

        if (SUCCEEDED(hr))
        {
            // Show the Open dialog.
            hr = pfd->Show(NULL);

            if (SUCCEEDED(hr))
            {
                // Obtain the result of the user interaction.
                IShellItemArray* results;
                hr = pfd->GetResults(&results);

                if (SUCCEEDED(hr))
                {
                    DWORD numFiles;
                    hr = results->GetCount(&numFiles);
                    filenames.reserve(numFiles);
                    if (SUCCEEDED(hr)) {
                        for (DWORD j = 0; j < numFiles; j++) {
                            IShellItem* file;
                            results->GetItemAt(j, &file);
                            LPWSTR dn;
                            file->GetDisplayName(SIGDN_FILESYSPATH, &dn);
                            CString name(dn);
                            filenames.push_back(name);
                            CoTaskMemFree(dn);
                        }
                        return filenames;
                    }
                    //
                    // You can add your own code here to handle the results.
                    //
                    results->Release();
                }
            }
        }
        pfd->Release();
    }
    return filenames;
}

// open a browser window and let the user search for a file
bool Common::BrowseForFile(const char* filter, CString& fileName) {
    static char szFile[4096];
    szFile[0] = 0;

    OPENFILENAME ofn;       // common dialog box structure
    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = nullptr;
    ofn.hInstance = AfxGetInstanceHandle();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    if (GetOpenFileName(&ofn) == TRUE) {
        fileName.Format(szFile);
        return SUCCESS;
    }
    fileName.Format("");
    return FAIL;
}

bool Common::BrowseForEvaluationLog(CString& fileName)
{
    TCHAR filter[512];
    int n = _stprintf(filter, "Evaluation Logs\0");
    n += _stprintf(filter + n + 1, "*.txt;\0");
    filter[n + 2] = 0;

    return Common::BrowseForFile(filter, fileName);
}

// open a browser window and let the user search for a file
bool Common::BrowseForFile_SaveAs(TCHAR* filter, CString& fileName)
{
    return BrowseForFile_SaveAs(filter, fileName, nullptr);
}

bool Common::BrowseForFile_SaveAs(TCHAR* filter, CString& fileName, int* filterType)
{
    static TCHAR szFile[4096];
    sprintf(szFile, "%s", (LPCTSTR)fileName);

    OPENFILENAME ofn;       // common dialog box structure
    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = nullptr;
    ofn.hInstance = AfxGetInstanceHandle();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER;

    if (GetSaveFileName(&ofn) == TRUE)
    {
        fileName.Format(szFile);

        if (filterType != nullptr)
        {
            *filterType = ofn.nFilterIndex;
        }

        return true;
    }
    fileName.Format("");
    return false;
}

/** Take out the exe name from a long path
      @param fileName path of the exe file	*/
void Common::GetFileName(CString& fileName)
{
    int position = fileName.ReverseFind('\\');
    int length = CString::StringLength(fileName);
    fileName = fileName.Right(length - position - 1);

}

/** Take out the directory from a long path name.
    @param fileName - the complete path of the file */
void Common::GetDirectory(CString& fileName) {
    int position = fileName.ReverseFind('\\');
    fileName = fileName.Left(position + 1);
}

unsigned short Common::Swp(unsigned short in)
{
    unsigned char* p1, * p2;
    unsigned short ut;

    p1 = (unsigned char*)&ut;
    p2 = (unsigned char*)&in;
    p1[0] = p2[1];
    p1[1] = p2[0];
    return(ut);
}

/** Takes a given year and month and returns the number of days in that month. */
int	Common::DaysInMonth(int year, int month) {
    static int nDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    // detect non-existing months.
    if (month < 1 || month > 12)
        return 0;

    // If the month is not february, then it's easy!!!
    if (month != 2)
        return nDays[month - 1];

    // If february, then check for leap-years
    if (year % 4 != 0)
        return 28; // not a leap-year

    if (year % 400 == 0) // every year dividable by 400 is a leap-year
        return 29;

    if (year % 100 == 0) // years diviable by 4 and by 100 are not leap-years
        return 28;
    else
        return 29;		// years dividable by 4 and not by 100 are leap-years
}