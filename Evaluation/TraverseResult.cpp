#include "stdafx.h"
#include "traverseresult.h"

CTraverseResult::CTraverseResult(void)
{
    m_speciesNum = 0;
    m_spectrumNum = 0;

    // Initially reserve space for 200 spectra
    m_measurement.SetSize(200);
}

CTraverseResult::~CTraverseResult(void)
{
}

/** Returns the number of spectra in this traverse */
unsigned long CTraverseResult::GetSpectrumNum() {
    return m_spectrumNum;
}

/** Returns the number of species that these spectra were
        evaluated for */
unsigned long CTraverseResult::GetSpecieNum() {
    return m_speciesNum;
}

/** Returns a pointer to a suitably allocated SpectrumInfo-object.
        if spectrumIndex is out of bounds, then NULL is returned. */
CTraverseResult::SpectrumInfo* CTraverseResult::GetSpectrumInfo(int spectrumIndex) {
    if (spectrumIndex < 0)
        return NULL;

    if (m_measurement.GetSize() < spectrumIndex)
        m_measurement.SetSize(spectrumIndex + 50);

    return &m_measurement.GetAt(spectrumIndex).info;
}

/** Returns a pointer to a suitably allocated gpsPosition-object.
        if spectrumIndex is out of bounds, then NULL is returned. */
mobiledoas::gpsPosition* CTraverseResult::GetGPS(int spectrumIndex) {
    if (spectrumIndex < 0)
        return NULL;

    if (m_measurement.GetSize() < spectrumIndex)
        m_measurement.SetSize(spectrumIndex + 50);

    return &m_measurement.GetAt(spectrumIndex).gps;
}


/** Returns a pointer to a suitably allocated EvaluationResult-object.
        if spectrumIndex is out of bounds, then NULL is returned. */
CTraverseResult::EvaluationResult* CTraverseResult::GetEvaluationResult(int spectrumIndex) {
    if (spectrumIndex < 0)
        return NULL;

    if (m_measurement.GetSize() < spectrumIndex)
        m_measurement.SetSize(spectrumIndex + 50);

    return &m_measurement.GetAt(spectrumIndex).result;
}

/** Returns a pointer to a suitably allocated Time-object.
        if spectrumIndex is out of bounds, then NULL is returned. */
mobiledoas::Time* CTraverseResult::GetTime(int spectrumIndex) {
    if (spectrumIndex < 0)
        return NULL;

    if (m_measurement.GetSize() < spectrumIndex)
        m_measurement.SetSize(spectrumIndex + 50);

    return &m_measurement.GetAt(spectrumIndex).gmtTime;
}

/** Sets the evaluated column for the given spectrum and specie */
void CTraverseResult::SetColumn(int spectrumIndex, int specieIndex, double column) {
    EvaluationResult* result = GetEvaluationResult(spectrumIndex);
    if (result != nullptr)
        result->column[specieIndex] = column;
}

/** Sets the evaluated column Error for the given spectrum and specie */
void CTraverseResult::SetColumnError(int spectrumIndex, int specieIndex, double columnError) {
    EvaluationResult* result = GetEvaluationResult(spectrumIndex);
    if (result != nullptr)
        result->columnError[specieIndex] = columnError;
}

/** Sets the gps-altitude for the given spectrum */
void CTraverseResult::SetAltitude(int spectrumIndex, double altitude) {
    mobiledoas::gpsPosition* gps = GetGPS(spectrumIndex);
    if (gps != nullptr)
        gps->altitude = altitude;
}

/** Sets the gps-latitude for the given spectrum */
void CTraverseResult::SetLatitude(int spectrumIndex, double latitude) {
    mobiledoas::gpsPosition* gps = GetGPS(spectrumIndex);
    if (gps != nullptr)
        gps->latitude = latitude;
}

/** Sets the gps-longitude for the given spectrum */
void CTraverseResult::SetLongitude(int spectrumIndex, double longitude) {
    mobiledoas::gpsPosition* gps = GetGPS(spectrumIndex);
    if (gps != nullptr)
        gps->longitude = longitude;
}

/** Sets the exposure-time for the given spectrum */
void CTraverseResult::SetExptime(int spectrumIndex, long exptime) {
    SpectrumInfo* info = GetSpectrumInfo(spectrumIndex);
    if (info != nullptr)
        info->expTime = (long)exptime;
}

/** Sets the number of exposures for the given spectrum */
void CTraverseResult::SetNumExposures(int spectrumIndex, int expNum) {
    SpectrumInfo* info = GetSpectrumInfo(spectrumIndex);
    if (info != nullptr)
        info->numExposures = expNum;
}

/** Sets the intensity for the given spectrum */
void CTraverseResult::SetIntensity(int spectrumIndex, double intensity) {
    SpectrumInfo* info = GetSpectrumInfo(spectrumIndex);
    if (info != nullptr)
        info->intensity = intensity;
}

/** Sets the start-time for the given spectrum */
void CTraverseResult::SetStartTime(int spectrumIndex, const mobiledoas::Time& startTime) {
    mobiledoas::Time* tid = GetTime(spectrumIndex);
    if (tid != nullptr) {
        tid->hour = startTime.hour;
        tid->minute = startTime.minute;
        tid->second = startTime.second;
    }
}
