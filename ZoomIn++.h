#pragma once

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"	// main symbols

class CZoomInApp : public CWinApp
{
public:
    CZoomInApp ();

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CZoomInApp)
    public:
    virtual BOOL InitInstance ();
    //}}AFX_VIRTUAL

// Implementation
    //{{AFX_MSG(CZoomInApp)
    afx_msg void OnAppAbout ();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
