#pragma once

// DMSpecDoc.h : interface of the CDMSpecDoc class
//
/////////////////////////////////////////////////////////////////////////////


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
