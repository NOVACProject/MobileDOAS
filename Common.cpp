// Common.cpp: implementation of the Common class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DMSpec.h"
#include "Common.h"
#include <utility>
#include <vector>
#include <string>
#include <MobileDoasLib/GpsData.h>

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

void Common::GetExePath()
{
    TCHAR exeFullPath[MAX_PATH];
    GetModuleFileName(NULL, exeFullPath, MAX_PATH);
    m_exePath = (CString)exeFullPath;
    int position = m_exePath.ReverseFind('\\');
    m_exePath = m_exePath.Left(position + 1);
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
