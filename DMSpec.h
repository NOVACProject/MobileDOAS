// DMSpec.h : main header file for the DMSPEC application
//

#if !defined(AFX_DMSPEC_H__0F666198_4C40_4AD8_A962_1D2E8B60D1CA__INCLUDED_)
#define AFX_DMSPEC_H__0F666198_4C40_4AD8_A962_1D2E8B60D1CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include "Version.h"

/////////////////////////////////////////////////////////////////////////////
// CDMSpecApp:
// See DMSpec.cpp for the implementation of this class
//

class CDMSpecApp : public CWinApp
{
public:
    CDMSpecApp();

    // Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CDMSpecApp)
public:
    virtual BOOL InitInstance();
    //}}AFX_VIRTUAL

// Implementation
    //{{AFX_MSG(CDMSpecApp)
    afx_msg void OnAppAbout();
    // NOTE - the ClassWizard will add and remove member functions here.
    //    DO NOT EDIT what you see in these blocks of generated code !
//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    virtual int ExitInstance();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DMSPEC_H__0F666198_4C40_4AD8_A962_1D2E8B60D1CA__INCLUDED_)
