#include "StdAfx.h"
#include "fitwindow.h"
#include "../Common.h"

using namespace Evaluation;

CFitWindow::CFitWindow(void)
{
	Clear();
}

CFitWindow::~CFitWindow(void)
{
}

void CFitWindow::Clear(){
	fitHigh = 460;
	fitLow = 320;
	channel = 0;
	specLength = MAX_SPECTRUM_LENGTH;
	fitType = FIT_HP_DIV;
	interlaced = false;
	name.Format("NEW");
	nRef = 0;
	polyOrder = 5;
	offsetFrom	= 50;
	offsetTo		= 200;
	for(int i = 0; i < MAX_N_REFERENCES; ++i){
		ref[i].m_path.Format("");
		ref[i].m_specieName.Format("");
	}
}

CFitWindow &CFitWindow::operator =(const CFitWindow &w2){
	this->fitHigh     = w2.fitHigh;
	this->fitLow      = w2.fitLow;
	this->fitType     = w2.fitType;
	this->channel     = w2.channel;
	this->interlaced  = w2.interlaced;
	this->name.Format("%s", (LPCTSTR)w2.name);
	this->nRef        = w2.nRef;
	this->polyOrder   = w2.polyOrder;
	this->offsetFrom  = w2.offsetFrom;
	this->offsetTo    = w2.offsetTo;
	for(int i = 0; i < w2.nRef; ++i){
		this->ref[i] = w2.ref[i];
	}

	return *this;
}
