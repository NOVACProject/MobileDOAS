// DMSpecDoc.cpp : implementation of the CDMSpecDoc class
//

#include "stdafx.h"
#include "DMSpec.h"

#include "DMSpecDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDMSpecDoc

IMPLEMENT_DYNCREATE(CDMSpecDoc, CDocument)

BEGIN_MESSAGE_MAP(CDMSpecDoc, CDocument)
	//{{AFX_MSG_MAP(CDMSpecDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDMSpecDoc construction/destruction

CDMSpecDoc::CDMSpecDoc()
{
	// TODO: add one-time construction code here

}

CDMSpecDoc::~CDMSpecDoc()
{
}

BOOL CDMSpecDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CDMSpecDoc serialization

void CDMSpecDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDMSpecDoc diagnostics

#ifdef _DEBUG
void CDMSpecDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDMSpecDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDMSpecDoc commands
