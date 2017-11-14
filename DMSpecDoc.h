// DMSpecDoc.h : interface of the CDMSpecDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DMSPECDOC_H__EDB59288_DAD1_47AF_B02E_7F70AA31C773__INCLUDED_)
#define AFX_DMSPECDOC_H__EDB59288_DAD1_47AF_B02E_7F70AA31C773__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDMSpecDoc : public CDocument
{
protected: // create from serialization only
	CDMSpecDoc();
	DECLARE_DYNCREATE(CDMSpecDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDMSpecDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDMSpecDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CDMSpecDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DMSPECDOC_H__EDB59288_DAD1_47AF_B02E_7F70AA31C773__INCLUDED_)
