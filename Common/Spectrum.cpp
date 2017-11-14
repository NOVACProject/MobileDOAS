/* The following two definitions are necesarry since
	VC6 (and VC7) does not handle the long long data-type 
	used by the OceanOptics OmniDriver
*/
#define HIGHRESTIMESTAMP_H
//#define SPECTROMETERCHANNEL_H


#include "StdAfx.h"
#include "spectrum.h"

CSpectrum::CSpectrum(void)
{
	this->Clear();
}


CSpectrum::~CSpectrum(void)
{
}

double CSpectrum::GetMax() const{
  double ret = 0;

  for(int i = 0; i < this->length; ++i){
    if(I[i] > ret)
      ret = I[i];
  }
  return ret;
}

double CSpectrum::GetAverage() const{
  
  if(this->length == 0)
    return 0;
  else{
    double sum = this->GetSum();
    return (sum / this->length);
  }
}

double CSpectrum::GetAverage(int low, int high) const{
  low   = max(low, 0);
  high  = min(high, this->length);
  
  if(this->length == 0)
    return 0;
  if(high == low)
    return 0;

  double sum = this->GetSum(low, high);
  return (sum / (high - low));
}

double CSpectrum::GetSum() const{
  double ret = 0;
  for(int i = 0; i < this->length; ++i)
    ret += I[i];

  return ret;
}

double CSpectrum::GetSum(int low, int high) const{
  double ret = 0;

  if(low > high)
    return 0;
  low   = max(low, 0);
  high  = min(high, this->length);

  for(int i = low; i < high; ++i)
    ret += I[i];

  return ret;
}

// add two spectra together
bool CSpectrum::Add(CSpectrum &spec2){
  if(this->length != spec2.length)
    return false;

  for(int i = 0; i < this->length; ++i){
    this->I[i] += spec2.I[i];
  }

  scans += spec2.scans;

  return true;
}

// divide one spectrum with another
bool CSpectrum::Div(CSpectrum &spec2){
  if(this->length != spec2.length)
    return false;

  for(int i = 0; i < this->length; ++i){
    if(spec2.I[i] == 0)
      this->I[i] = 0;
    else
      this->I[i] /= spec2.I[i];
  }
  return true;
}

// subtract one spectrum from another
bool CSpectrum::Sub(CSpectrum &spec2){
  if(this->length != spec2.length)
    return false;

  for(int i = 0; i < this->length; ++i){
    this->I[i] -= spec2.I[i];
  }
  return true;
}

// add a constant to a spectrum
bool CSpectrum::Add(double value){
  for(int i = 0; i < this->length; ++i){
    this->I[i] += value;
  }
  return true;
}

// divide a spectrum with a constant
bool CSpectrum::Div(double value){
  if(value == 0)
    return false;

  for(int i = 0; i < this->length; ++i){
    this->I[i] /= value;
  }
  return true;
}

// subtract a constant from a spectrum
bool CSpectrum::Sub(double value){
  for(int i = 0; i < this->length; ++i){
    this->I[i] -= value;
  }
  return true;
}

bool CSpectrum::Copy(const CSpectrum &spec2){
  this->isDark    = spec2.isDark;
  this->length    = spec2.length;
  this->intTime   = spec2.intTime;
  this->lat       = spec2.lat;
  this->lon       = spec2.lon;
  this->scans     = spec2.scans;
  this->spectrometer.Format(spec2.spectrometer);

  memcpy(this->date, spec2.date, 3*sizeof(int));
  memcpy(this->startTime, spec2.startTime, 3*sizeof(int));
  memcpy(this->stopTime,  spec2.stopTime,  3*sizeof(int));

  memcpy(this->I, spec2.I, MAX_SPECTRUM_LENGTH*sizeof(double));

  return true;
}

// clearing out the information in the spectrum
void	CSpectrum::Clear(){
  length	= 0;
	scans		= 0;
	intTime = 0;
	lat			= 0.0;
	lon			= 0.0;
	spectrometer.Format("");

	date[0]				= date[1]				= date[2] = 0;
	startTime[0]	= startTime[1]	= startTime[2] = 0;
	stopTime[0]		= stopTime[1]		= stopTime[2] = 0;

	isDark = false;

	memset(I, 0, MAX_SPECTRUM_LENGTH * sizeof(double));
}
