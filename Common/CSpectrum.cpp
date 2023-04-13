#include "StdAfx.h"
#include "CSpectrum.h"
#include <MobileDoasLib/DateTime.h>
#include <algorithm>
#include <string>

#undef min
#undef max

CSpectrum::CSpectrum(void)
{
    this->Clear();
}

CSpectrum::~CSpectrum(void)
{
}

CSpectrum::CSpectrum(const CSpectrum& other) {
    this->isDark = other.isDark;
    this->length = other.length;
    this->exposureTime = other.exposureTime;
    this->date = other.date;
    this->lat = other.lat;
    this->lon = other.lon;
    this->altitude = other.altitude;
    this->gpsStatus = other.gpsStatus;
    this->speed = other.speed;
    this->course = other.course;
    this->scans = other.scans;
    this->spectrometerSerial.Format(other.spectrometerSerial);
    this->spectrometerModel.Format(other.spectrometerModel);
    this->name.Format(other.name);
    this->boardTemperature = other.boardTemperature;
    this->detectorTemperature = other.detectorTemperature;
    this->fitHigh = other.fitHigh;
    this->fitLow = other.fitLow;

    memcpy(this->startTime, other.startTime, 3 * sizeof(int));
    memcpy(this->stopTime, other.stopTime, 3 * sizeof(int));

    memcpy(this->I, other.I, MAX_SPECTRUM_LENGTH * sizeof(double));
}

CSpectrum& CSpectrum::operator=(CSpectrum other)
{
    swap(*this, other);
    return *this;
}

void swap(CSpectrum& first, CSpectrum& second)
{
    using std::swap;

    swap(first.isDark, second.isDark);
    swap(first.length, second.length);
    swap(first.exposureTime, second.exposureTime);
    swap(first.date, second.date);
    swap(first.lat, second.lat);
    swap(first.lon, second.lon);
    swap(first.altitude, second.altitude);
    swap(first.gpsStatus, second.gpsStatus);
    swap(first.speed, second.speed);
    swap(first.course, second.course);
    swap(first.scans, second.scans);
    swap(first.spectrometerSerial, second.spectrometerSerial);
    swap(first.spectrometerModel, second.spectrometerModel);
    swap(first.name, second.name);
    swap(first.boardTemperature, second.boardTemperature);
    swap(first.boardTemperature, second.detectorTemperature);
    swap(first.fitHigh, second.fitHigh);
    swap(first.fitLow, second.fitLow);

    for (int ii = 0; ii < 3; ++ii) {
        swap(first.startTime[ii], second.startTime[ii]);
        swap(first.stopTime[ii], second.stopTime[ii]);
    }

    for (int ii = 0; ii < first.length; ++ii) {
        swap(first.I[ii], second.I[ii]);
    }
}

void CSpectrum::GetMinMax(double& minValue, double& maxValue) const
{
    if (0 == this->length)
    {
        minValue = 0.0;
        maxValue = 0.0;
        return;
    }

    minValue = I[0];
    maxValue = I[0];

    for (int ii = 0; ii < this->length; ++ii)
    {
        minValue = std::min(minValue, I[ii]);
        maxValue = std::max(maxValue, I[ii]);
    }
}

double CSpectrum::GetMax() const {
    double maxValue = 0;

    for (int i = 0; i < this->length; ++i)
    {
        maxValue = std::max(maxValue, I[i]);
    }

    return maxValue;
}

double CSpectrum::GetAverage() const {

    if (this->length == 0)
        return 0;
    else {
        double sum = this->GetSum();
        return (sum / this->length);
    }
}

double CSpectrum::GetAverage(int low, int high) const {
    low = std::max(low, 0);
    high = std::min(high, this->length);

    if (this->length == 0) {
        return 0;
    }

    if (high == low) {
        return 0;
    }

    double sum = this->GetSum(low, high);
    return (sum / (high - low));
}

double CSpectrum::GetSum() const {
    double sum = 0;

    for (int i = 0; i < this->length; ++i) {
        sum += I[i];
    }

    return sum;
}

double CSpectrum::GetSum(int low, int high) const {
    double sum = 0;

    if (low > high) {
        return 0;
    }

    low = std::max(low, 0);
    high = std::min(high, this->length);

    for (int i = low; i < high; ++i) {
        sum += I[i];
    }

    return sum;
}

bool CSpectrum::Add(CSpectrum& spec2) {
    if (this->length != spec2.length) {
        return false;
    }

    for (int i = 0; i < this->length; ++i) {
        this->I[i] += spec2.I[i];
    }

    scans += spec2.scans;

    return true;
}

bool CSpectrum::Div(CSpectrum& spec2) {
    if (this->length != spec2.length) {
        return false;
    }

    for (int i = 0; i < this->length; ++i) {
        if (spec2.I[i] == 0) {
            this->I[i] = 0;
        }
        else {
            this->I[i] /= spec2.I[i];
        }
    }
    return true;
}

bool CSpectrum::Sub(CSpectrum& spec2) {
    if (this->length != spec2.length) {
        return false;
    }

    for (int i = 0; i < this->length; ++i) {
        this->I[i] -= spec2.I[i];
    }
    return true;
}

bool CSpectrum::Add(double value) {
    for (int i = 0; i < this->length; ++i) {
        this->I[i] += value;
    }
    return true;
}

bool CSpectrum::Div(double value) {
    if (value == 0) {
        return false;
    }

    for (int i = 0; i < this->length; ++i) {
        this->I[i] /= value;
    }
    return true;
}

bool CSpectrum::Mult(double value) {
    if (value == 0) {
        return false;
    }

    for (int i = 0; i < this->length; ++i) {
        this->I[i] *= value;
    }
    return true;
}

bool CSpectrum::Sub(double value) {
    for (int i = 0; i < this->length; ++i) {
        this->I[i] -= value;
    }
    return true;
}

// clearing out the information in the spectrum
void CSpectrum::Clear() {
    length = 0;
    scans = 0;
    exposureTime = 0;
    lat = 0.0;
    lon = 0.0;
    altitude = 0.0;
    gpsStatus = "NA";
    speed = 0.0;
    course = 0.0;
    spectrometerSerial.Format("");
    spectrometerModel.Format("");
    name.Format("");
    fitHigh = std::numeric_limits<int>::quiet_NaN();
    fitLow = std::numeric_limits<int>::quiet_NaN();
    boardTemperature = std::numeric_limits<double>::quiet_NaN();
    detectorTemperature = std::numeric_limits<double>::quiet_NaN();

    date = "00.00.00";
    startTime[0] = startTime[1] = startTime[2] = 0;
    stopTime[0] = stopTime[1] = stopTime[2] = 0;

    isDark = false;

    memset(I, 0, MAX_SPECTRUM_LENGTH * sizeof(double));
}

void CSpectrum::SetStartTime(long time) {
    mobiledoas::GetHrMinSec(time, startTime[0], startTime[1], startTime[2]);
}

void CSpectrum::SetStopTime(long time) {
    mobiledoas::GetHrMinSec(time, stopTime[0], stopTime[1], stopTime[2]);
}

