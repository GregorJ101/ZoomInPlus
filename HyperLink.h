#pragma once

class CHyperLink : public CStatic
{
public:
    CHyperLink ();
    virtual ~CHyperLink ();

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CHyperLink)
    //}}AFX_VIRTUAL

protected:
    CFont m_font;
    HCURSOR m_hCursorHand;

    //{{AFX_MSG(CHyperLink)
    afx_msg HBRUSH CtlColor (CDC *pDC, UINT nCtlColor);
    afx_msg LRESULT OnNcHitTest (CPoint point);
    afx_msg BOOL OnSetCursor (CWnd *pWnd, UINT nHitTest, UINT message);
    afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
