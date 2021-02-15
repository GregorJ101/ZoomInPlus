#pragma once

class CRefreshRateDlg : public CDialog
{
// Construction
public:
    CRefreshRateDlg (CWnd *pParent = NULL);

// Dialog Data
    //{{AFX_DATA(CRefreshRateDlg)
    enum { IDD = IDD_REFRESH_RATE };
    BOOL	m_bEnableAutoRefresh;
    int		m_nRefreshInterval;
    //}}AFX_DATA

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CRefreshRateDlg)
    protected:
    virtual void DoDataExchange (CDataExchange *pDX);
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CRefreshRateDlg)
    virtual BOOL OnInitDialog ();
    afx_msg void OnAutoRefresh ();
    afx_msg void OnKillFocusRefreshRate ();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
