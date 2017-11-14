// Flux1.h: interface for the CFlux class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLUX1_H__AF49BE9D_D420_458C_8999_ABA43DE6BF9B__INCLUDED_)
#define AFX_FLUX1_H__AF49BE9D_D420_458C_8999_ABA43DE6BF9B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Flux/WindField.h"
#include "Flux/Traverse.h"
#include <afxtempl.h>

namespace Flux
{

	class CFlux{
	public:
		CFlux();
		virtual ~CFlux();

		/** The read traverse data */
		CArray <CTraverse *, CTraverse *> m_traverse;

		/** The number of traverses read so far */
		long      m_traverseNum;

		/** The current traverse */
		long      m_curTraverse;


		/* managing the data */
		void  Reset();
		bool  hasValidGPS(int file = -1);

		/* Reading the files */
		int       ReadSettingFile(CString filename, int &nChannels, double &fileVersion);
		int       ReadSettingFile(CString filename, long fileIndex, int &nChannels, double &fileVersion);
		long      ReadLogFile(CString filePath, CString fileName, int nChannels, double fileVersion);
		long      ReadLogFile(CString filePath, CString fileName, long fileIndex, int nChannels, double fileVersion);

		/* Output */
		bool      fCreateAdditionalLog;
		CString   additionalLogName;

		/* Calculating */
		void      SetParams(double pSpeed,double pAngle,double pLat,double pLon, long pLowIndex,long pHighIndex,double pOffset);

		/* Exporting data to other parts of the program */
		long	GetLat(double* pBuffer);
		long	GetLon(double* pBuffer);
		long	GetColumn(double* pBuffer);
		long	GetColumnError(double *pBuffer);
		long	GetIntensity(double *pBuffer);
		long	GetAltitude(double *pBuffer);
		long	GetTime(double *pBuffer);
		long	GetCurrentFileName(CString &str);
		long	GetDynamicRange();

		/* Calculating the flux */
		double    GetTotalFlux();

		/* Geometrical calculations */
		int       GetPlumeCenter(double srclat,double srclon,double *maxBuffer,double *avBuffer);

		/* The result */
		double    m_totalFlux;
		
		/** The upper limit for the total flux */
		double	m_totalFlux_High;
		
		/** The lower limit for the total flux */
		double	m_totalFlux_Low;

		/* parameters for calculating the flux */ 
		double    m_windAngle;
		double    m_windSpeed;

		/** A wind field, if one has been imported into the program */
		CWindField  *m_windField;
		bool        m_useWindField;

		/** Interpolates the wind field to the currently selected traverse */
		void        InterpolateWindField(int layer);

		/* the source */
		double    m_srcLatitude;
		double    m_srcLongitude;

		/* information about the traverse - calculated at the same time as the flux */
		double    plumeWidth;
		double    traverseLength;
		long      spectraInPlume;

	private:
		/** Temporary variables, used for reading and parsing the log files */
		char		m_FileType[100];
		CString		m_specieName[20];
		double		m_lastGasFactor;
		long		m_lastDynamicRange;
		long		m_lastFitFrom;
		long		m_lastFitTo;
		CString		m_lastRefFile[20];
		long		m_lastRefFileNum;
		

		/** Assign a read value to the corresponding array 
			(decides if the read value is a column value, a latitude, a longitude...) */
		bool      AssignValueToColumn(long fileIndex, int column, int row, double value[3], int nChannels, double fileVersion); // used when parsing the log files
		bool      AssignValueToColumn_ReEvaluationLog(long fileIndex, int column, int row, double value[3], int nChannels, double fileVersion); // used when parsing the reevaluation log files
	};

}
#endif // !defined(AFX_FLUX1_H__AF49BE9D_D420_458C_8999_ABA43DE6BF9B__INCLUDED_)

