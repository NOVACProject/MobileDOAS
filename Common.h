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

// ----------------------------------------------------------------
// ---------------- MATHEMATICAL CONSTANTS ------------------------
// ----------------------------------------------------------------
#define DEGREETORAD   0.01745329251994 
#define RADTODEGREE   57.29577951308232
#define HALF_PI       1.57079632679490
#define M_PI          3.14159265358979
#define TWO_PI        6.28318530717959

// ----------------------------------------------------------------
// ------------ SIMPLE MATHEMATICAL FUNCTIONS  --------------------
// ----------------------------------------------------------------
// #define round(x) (x < 0 ? ceil((x)-0.5) : floor((x)+0.5))

// ---------------------------------------------------------------
// ---------------- DEFINED CONSTANTS ----------------------------
// ---------------------------------------------------------------
// conversion from ppmm to mg/m^2 for SO2
#define GASFACTOR_SO2 2.66

// conversion from ppmm to mg/m^2 for O3
#define GASFACTOR_O3 1.99

// conversion from ppmm to mg/m^2 for NO2
#define GASFACTOR_NO2 1.93

// conversion from ppmm to mg/m^2 for HCHO
#define GASFACTOR_HCHO 1.25

// the maximum number of channels on the spectrometer that we can handle 
#define MAX_N_CHANNELS 2

// The maximum number of fit-regions that we can use at any single time
#define MAX_FIT_WINDOWS 2

// the maximum length of any single spectrum
#define MAX_SPECTRUM_LENGTH 3648

// function returns
#define SUCCESS true
#define FAIL false

struct plotRange{
	double maxLat;
	double maxLon;
	double minLat;
	double minLon;
};

typedef struct gpsPosition{
	double latitude = 0.0;
	double longitude = 0.0;
	double altitude = 0.0;
}gpsPosition;




// ---------------------------------------------------------------
// ---------------- GLOBAL FUNCTIONS -----------------------------
// ---------------------------------------------------------------
/** Compares two strings without regard to case.
    @return 1 if the strings are equal. @return 0 if the strings are not equal. */
int Equals(const CString &str1, const CString &str2);

/** Compares at most 'nCharacters' of two strings without regard to case.
    @param nCharacters - The number of characters to compare
    @return 1 if the strings are equal. @return 0 if the strings are not equal. */
int Equals(const CString &str1, const CString &str2, unsigned int nCharacters);

/** A simple function to find out wheather a given file exists or not.
    @param - The filename (including path) to the file.
    @return 0 if the file does not exist.
    @return 1 if the file exist. */
int IsExistingFile(const CString *fileName);
int IsExistingFile(const CString &fileName);

class Common  
{
public:
	CString m_exePath;
	Common();
	virtual ~Common();
	void WriteLogFile(CString filename, CString txt);
	static void GetDateText(char *txt);
	static void GetDateText(CString &str);
	static void GetTimeText(char *txt);
	static void GetTimeText(CString &str);
	static void GetDateTimeText(char *txt);
	static void GetDateTimeText(CString &str);
	static void GetDateTimeTextPlainFormat(char *txt);
	static void GetDateTimeTextPlainFormat(CString &str);

	// ---------------- PATH MANAGING FUNCTIONS -----------------

	/** Get the path of the executable */
	void GetExePath();

	/** Take out the file name from a long path 
	    @param fileName path of the file	*/
	static void GetFileName(CString& fileName);

	/** Take out the directory from a long path name.
		@param fileName - the complete path of the file */
	static void GetDirectory(CString &fileName);

	/** Opens a browser window and lets the user select any number of files */
	static std::vector<CString> BrowseForFiles();

	/** Opens a browser window and lets the user select one file which matches the provided filter. */
	static bool BrowseForFile(char *filter, CString &fileName);

	/** Specialization of BrowseForFile, only allows for browsing of evaluation-logs (.txt) */
	static bool BrowseForEvaluationLog(CString &fileName);

	/** Opens a dialog window and lets the user browse for a filename to save to */
	static bool BrowseForFile_SaveAs(TCHAR *filter, CString &fileName);

	/** Opens a dialog window and lets the user browse for a directory.
			@return true if all is ok,
			@return false otherwise */
	static bool BrowseForDirectory(CString &folder);
	
	// ---------------- MATHEMATICAL FUNCTIONS --------------------
	/** Rounds a given float value to the nearest integer. */
	long  Round(double d);

	/*  adapts parameters k and m so that y = k*x + m, in a least square sense.
		  Algorithm found at: http://mathworld.wolfram.com/LeastSquaresFittingPolynomial.html */
	static int AdaptStraightLine(double *x, double *y, unsigned int l, double *k, double *m);

	/** Guesses the name of the specie in the reference-file whos filename
			is given in 'fileName'. */
	static void GuessSpecieName(const CString &fileName, CString &specie);

	// ------------- BINARY FUNCTIONS ------------------

	/** This function swaps the place of the MostSignificantByte and 
			the LeastSignificantByte of the given number */
	static unsigned short Swp(unsigned short in);
	
	// --------------------------------------------------------------------
	// ---------------------- DATE & TIME ---------------------------------
	// --------------------------------------------------------------------

	/** Takes a given year and month and returns the number of days in that month. 
			The month ranges from 1 to 12. Any illegal values in the month will return 0. */
	static int DaysInMonth(int year, int month);
	
};

/* this function returns a string describing the error code given in 'error'
  return value is true if the errorCode can be found, else false. */
bool FormatErrorCode(DWORD error, CString &string);

/* This function returns the distance in meters between the two points defined
  by (lat1,lon1) and (lat2, lon2). All angles must be in degrees */
double GPSDistance(double lat1, double lon1, double lat2, double lon2);

/* This function returns the initial bearing (degrees) when travelling from
  the point defined by (lat1, lon1) to the point (lat2, lon2). 
  All angles must be in degrees */
double GPSBearing(double lat1, double lon1, double lat2, double lon2);

/** This function calculates the latitude and longitude for point
		which is the distance 'dist' m and bearing 'az' degrees from 
		the point defied by 'lat1' and 'lon1' */
void	CalculateDestination(double lat1, double lon1, double dist, double az, double &lat2, double &lon2);

 /* This function calculates the wind factor when travelling from point
  1 to point 2 and the wind is defined by 'windAngle' */
double GetWindFactor(double lat1, double lon1, double lat2, double lon2, double windAngle);


// --------------------------------------------------------------------
// ------------------- ARRAY FUNCTIONS --------------------------------
// --------------------------------------------------------------------

/** Searches for the maximum element in the array.
    @param pBuffer - The array in which to search for an element.
    @param bufLen - The length of the array.
    @return - The maximum value in the array */
template <class T> T Max(T *pBuffer, long bufLen){
	T maxValue = pBuffer[0];
	for(long i = 1; i < bufLen; ++i){
		if(pBuffer[i] > maxValue)
			maxValue = pBuffer[i];
	}
	return maxValue;
}

/** Searches for the minimum element in the array.
    @param pBuffer - The array in which to search for an element.
    @param bufLen - The length of the array.
    @return - The minimum value in the array */
template <class T> T Min(T *pBuffer, long bufLen){
	T minValue = pBuffer[0];
	for(long i = 1; i < bufLen; i++){
		if(pBuffer[i] < minValue)
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
template <class T> T Average(T *pBuffer, long bufLen){
	T sumValue = pBuffer[0];
	for(long i = 1; i < bufLen; ++i){
		sumValue = sumValue + pBuffer[i];
	}
	return sumValue / bufLen;
}

/** Calculates the sum of the elements in the array.
    It is required that addition is defined for the elements in the array.
    @param pBuffer - The array of which to calculate the average.
    @param bufLen - The length of the array.
    @return - The sum of the elements in the array. */
template <class T> T Sum(T *pBuffer, long bufLen){
	T sumValue = pBuffer[0];
	for(long i = 1; i < bufLen; ++i){
		sumValue = sumValue + pBuffer[i];
	}
	return sumValue;
}

/** This function finds the 'N' highest values in the supplied array. 
		On successfull return the array 'output' will be filled with the N highest
		values in the input array, sorted in descending order.
		@param array - the array to look into
		@param nElements - the number of elements in the supplied array
		@param output - the output array, must be at least 'N'- elements long
		@param N - the number of values to take out. 	
		@param indices - if specified, this will on return the indices for the highest elements. Must have length 'N' */
template <class T> bool FindNHighest(const T array[], long nElements, T output[], int N, int *indices = NULL){
	for(int i = 0; i < N; ++i)
		output[i] = array[0]; // to get some initial value

	// loop through all elements in the array
	for(int i = 0; i < nElements; ++i){

		// compare this element with all elements in the output array.
		for(int j = 0; j < N; ++j){
			if(array[i] > output[j]){
				// If we found a higher value, shift all other values down one step...
				for(int k = N-1; k > j; --k){
					output[k] = output[k-1];
					if(indices)							indices[k] = indices[k-1];
				}
				output[j] = array[i];
				if(indices)								indices[j] = i;
				break;
			}
		}
	}
	return true;
}

/** This function finds the 'N' lowest values in the supplied array. 
		On successfull return the array 'output' will be filled with the N lowest
		values in the input array, sorted in ascending order.
		@param array - the array to look into
		@param nElements - the number of elements in the supplied array
		@param output - the output array, must be at least 'N'- elements long
		@param N - the number of values to take out. 	*/
template <class T> bool FindNLowest(const T array[], long nElements, T output[], int N, int *indices = NULL){
	for(int i = 0; i < N; ++i)
		output[i] = 1e16; // to get some initial value

	// loop through all elements in the array
	for(int i = 0; i < nElements; ++i){

		// compare this element with all elements in the output array.
		for(int j = 0; j < N; ++j){
			if(array[i] < output[j]){
				// If we found a higher value, shift all other values down one step...
				for(int k = N-1; k > j; --k){
					output[k] = output[k-1];
					if(indices)							indices[k] = indices[k-1];
				}
				output[j] = array[i];
				if(indices)								indices[j] = i;
				break;
			}
		}
	}
	return true;
}


#endif // !defined(AFX_COMMON_H__D37EDB58_63D7_4142_9CCB_67370BD06621__INCLUDED_)
