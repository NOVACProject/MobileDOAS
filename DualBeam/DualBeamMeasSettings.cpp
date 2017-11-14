#include "StdAfx.h"
#include "dualbeammeassettings.h"
#include "../Common.h"

using namespace DualBeamMeasurement;

CDualBeamMeasSettings::CDualBeamMeasSettings(void)
{
	lowPassFilterAverage	= 20;
	shiftMax							= 90;		// maximum shift is 1.5 minutes
	testLength						= 300;	// compare over 5-minute intervals
	columnMin							= -300; // normally not used...
	sigmaMin							= 0.1;
	plumeHeight						= 1000.0;
	angleSeparation				= 0.12 / DEGREETORAD;	// in degrees
}

CDualBeamMeasSettings::~CDualBeamMeasSettings(void)
{
}
