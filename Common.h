// Common.h: interface for the Common class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMMON_H__D37EDB58_63D7_4142_9CCB_67370BD06621__INCLUDED_)
#define AFX_COMMON_H__D37EDB58_63D7_4142_9CCB_67370BD06621__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <math.h>
#include <vector>
#include <string>
#include <MobileDoasLib/Definitions.h>

#define MAX_TRAVERSE_SHOWN 16384

/** Not initialized values are set to 'NOT_A_NUMBER' to indicate
    that a value is missing.
    This can e.g. be the case if only the wind-direction (and not the wind speed)
        is known by an element in the database (which can be the case if a the
        wind direction was calculated by combining two scans).
    */
#define NOT_A_NUMBER -9999.0

    // ----------------------------------------------------------------
    // ---------------- MESSAGES USED IN THE PROGRAM ------------------
    // ----------------------------------------------------------------
#define WM_PROGRESS     WM_USER + 2
#define WM_DONE         WM_USER + 3
#define WM_STATUS       WM_USER + 4
#define WM_CANCEL       WM_USER + 5
#define WM_EVAL         WM_USER + 6
#define WM_GOTO_SLEEP   WM_USER + 7

struct plotRange {
    double maxLat;
    double maxLon;
    double minLat;
    double minLon;
};





// ---------------------------------------------------------------
// ---------------- GLOBAL FUNCTIONS -----------------------------
// ---------------------------------------------------------------
/** Compares two strings without regard to case.
    @return 1 if the strings are equal. @return 0 if the strings are not equal. */
int Equals(const CString& str1, const CString& str2);

/** Compares at most 'nCharacters' of two strings without regard to case.
    @param nCharacters - The number of characters to compare
    @return 1 if the strings are equal. @return 0 if the strings are not equal. */
int Equals(const CString& str1, const CString& str2, unsigned int nCharacters);

/** A simple function to find out whether a given file exists or not.
    @param - The filename (including path) to the file.
    @return 0 if the file does not exist.
    @return 1 if the file exist. */
int IsExistingFile(const CString* fileName);
int IsExistingFile(const CString& fileName);

class Common
{
public:
    CString m_exePath;
    Common();
    virtual ~Common();
    void WriteLogFile(CString filename, CString txt);

    // ---------------- PATH MANAGING FUNCTIONS -----------------

    /** Get the path of the executable */
    void GetExePath();

    /** Take out the file name from a long path
        @param fileName path of the file	*/
    static void GetFileName(CString& fileName);

    /** Take out the directory from a long path name.
        @param fileName - the complete path of the file */
    static void GetDirectory(CString& fileName);

    /** Opens a browser window and lets the user select any number of files */
    static std::vector<CString> BrowseForFiles();

    /** Opens a browser window and lets the user select one file which matches the provided filter. */
    static bool BrowseForFile(const char* filter, CString& fileName);

    /** Specialization of BrowseForFile, only allows for browsing of evaluation-logs (.txt) */
    static bool BrowseForEvaluationLog(CString& fileName);

    /** Opens a dialog window and lets the user browse for a filename to save to */
    static bool BrowseForFile_SaveAs(TCHAR* filter, CString& fileName);

    /** Opens a dialog window and lets the user browse for a filename to save to.
        If filterType is not nullptr then it will be filled with the selected filter. */
    static bool BrowseForFile_SaveAs(TCHAR* filter, CString& fileName, int* filterType);

    /** Opens a dialog window and lets the user browse for a directory.
            @return true if all is ok,
            @return false otherwise */
    static bool BrowseForDirectory(CString& folder);

    /** Lists all files in the provided directory.
        If a fileNameFilter is supplied, then only files with names matching the given filter are returned.
        Possible filters are '*.*', '*.xs' or 'SO2_*.txt'
        This will not recurse into sub-directories.
        @param directory The name of the directory to search. Must end with a backslash or forwardslash.
        @return a vector containing the full file path to each file found (i.e. including directory and extension) */
    static std::vector<std::string> ListFilesInDirectory(const char* directory, const char* fileNameFilter = nullptr);

    // ---------------- MATHEMATICAL FUNCTIONS --------------------

    /** Guesses the name of the specie in the reference-file whos filename
            is given in 'fileName'. */
    static void GuessSpecieName(const CString& fileName, CString& specie);

};

/* this function returns a string describing the error code given in 'error'
  return value is true if the errorCode can be found, else false. */
bool FormatErrorCode(DWORD error, CString& string);



// --------------------------------------------------------------------
// ------------------- ARRAY FUNCTIONS --------------------------------
// --------------------------------------------------------------------

/** Searches for the maximum element in the array.
    @param pBuffer - The array in which to search for an element.
    @param bufLen - The length of the array.
    @return - The maximum value in the array */
template <class T> T MaxValue(T* pBuffer, long bufLen) {
    T maxValue = pBuffer[0];
    for (long i = 1; i < bufLen; ++i) {
        if (pBuffer[i] > maxValue)
            maxValue = pBuffer[i];
    }
    return maxValue;
}

/** Searches for the minimum element in the array.
    @param pBuffer - The array in which to search for an element.
    @param bufLen - The length of the array.
    @return - The minimum value in the array */
template <class T> T MinValue(T* pBuffer, long bufLen) {
    T minValue = pBuffer[0];
    for (long i = 1; i < bufLen; i++) {
        if (pBuffer[i] < minValue)
            minValue = pBuffer[i];
    }
    return minValue;
}

/** Calculates the average value of the elements in the array.
    It is required that addition and division are defined for
    the elements in the array.
    @param pBuffer - The array of which to calculate the average.
    @param bufLen - The length of the array.
    @return - The average value of the elements in the array. */
template <class T> T Average(T* pBuffer, long bufLen) {
    T sumValue = pBuffer[0];
    for (long i = 1; i < bufLen; ++i) {
        sumValue = sumValue + pBuffer[i];
    }
    return sumValue / bufLen;
}

/** This function finds the 'N' lowest values in the supplied array.
        On successfull return the array 'output' will be filled with the N lowest
        values in the input array, sorted in ascending order.
        @param array - the array to look into
        @param nElements - the number of elements in the supplied array
        @param output - the output array, must be at least 'N'- elements long
        @param N - the number of values to take out. 	*/
template <class T> bool FindNLowest(const T array[], long nElements, T output[], int N, int* indices = NULL) {
    for (int i = 0; i < N; ++i)
        output[i] = 1e16; // to get some initial value

    // loop through all elements in the array
    for (int i = 0; i < nElements; ++i) {

        // compare this element with all elements in the output array.
        for (int j = 0; j < N; ++j) {
            if (array[i] < output[j]) {
                // If we found a higher value, shift all other values down one step...
                for (int k = N - 1; k > j; --k) {
                    output[k] = output[k - 1];
                    if (indices)							indices[k] = indices[k - 1];
                }
                output[j] = array[i];
                if (indices)								indices[j] = i;
                break;
            }
        }
    }
    return true;
}


#endif // !defined(AFX_COMMON_H__D37EDB58_63D7_4142_9CCB_67370BD06621__INCLUDED_)
