#pragma once

// Commonly used definitions througout the MobileDoas program


// ----------------------------------------------------------------
// ---------------- FUNCTION RETURN VALUES ------------------------
// ----------------------------------------------------------------

#define SUCCESS true

#define FAIL false

// ----------------------------------------------------------------
// --------------------- MAXIMUM SIZES ----------------------------
// ----------------------------------------------------------------

// the maximum number of channels on the spectrometer that we can handle 
#define MAX_N_CHANNELS 2

// The maximum number of fit-regions that we can use at any single time
#define MAX_FIT_WINDOWS 2

// the maximum length of any single spectrum
#define MAX_SPECTRUM_LENGTH 3648

// ----------------------------------------------------------------
// ---------------- MATHEMATICAL CONSTANTS ------------------------
// ----------------------------------------------------------------
#define DEGREETORAD   0.01745329251994 
#define RADTODEGREE   57.29577951308232
#define HALF_PI       1.57079632679490
#define M_PI          3.14159265358979
#define TWO_PI        6.28318530717959


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
