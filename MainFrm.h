#pragma once

#include "Settings.h"

#define WM_USER_GET_START_MINIMIZED  WM_USER + 1

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
    DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:
    CMainFrame ();
    virtual ~CMainFrame ();

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMainFrame)
    public:
    virtual BOOL DestroyWindow ();
    //}}AFX_VIRTUAL

// Implementation
public:
    bool IsStartMinimized () { return m_settings.GetStartMinimized (); }

#ifdef _DEBUG
    virtual void AssertValid () const { CFrameWnd::AssertValid (); }
    virtual void Dump (CDumpContext &dc) const { CFrameWnd::Dump (dc); }
#endif

protected:
    static UINT s_wmTaskbarCreated;

    CSettings   m_settings;

    HICON       m_hIconLarge,
                m_hIconSmall;
    HCURSOR     m_hCursorScan,
                m_hCursorPrev;
    CMenu       m_menu;
    CStatusBar  m_wndStatusBar;
    CScrollBar  m_wndScrollBar;
    CPalette    m_palPhysical;
    CString     m_cstrCaption;

    bool        m_bSaveSettings;
    bool        m_bIsLooking;
    bool        m_bCaptionValid; 
    int         m_nxScreenMax;		// Width of the screen (less 1)
    int         m_nyScreenMax;		// Height of the screen (less 1)
    int         m_nTimerID;

    ULONG       m_ulMouseMoveCount;
    ULONG_PTR   m_gdiplusToken;

    bool  ValidateCaption (const char* szCaption);
    CRect GetDesktopSize ();
    void  CreatePhysicalPalette ();
    int   GetStatusBarHeight () const;
    void  CalcZoomedSize ();
    void  DrawZoomRect (CPoint point = CPoint (-1, -1));
    void  DoTheGrid (CDC *pDC);
    void  DoTheZoom (CDC *pDC = NULL);
    void  MoveView (int nDirectionCode, bool bFast, bool bPeg);
    void  EnableAutoRefresh (const bool bEnable);
    void  AddTrayIcon () const;
    void  RemoveTrayIcon () const;

// Generated message map functions
protected:
    //{{AFX_MSG(CMainFrame)
    afx_msg int OnCreate (CREATESTRUCT *pCreateStruct);
    afx_msg void OnSysCommand (UINT nID, LPARAM lParam);
    afx_msg void OnGetMinMaxInfo (MINMAXINFO *pMMI);
    afx_msg void OnCopy ();
    afx_msg void OnExit ();
    afx_msg void OnRestore ();
    afx_msg void OnRefresh ();
    afx_msg void OnRefreshRate ();
    afx_msg void OnGrid ();
    afx_msg void OnNegativeGrid ();
    afx_msg void OnUpdateNegativeGrid (CCmdUI *pCmdUI);
    afx_msg void OnAbsoluteNumbering ();
    afx_msg void OnSaveSettings ();
    afx_msg void OnAlwaysOnTop ();
    afx_msg void OnWindowPosChanging (WINDOWPOS *pwndpos);
    afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp (UINT nFlags, CPoint point);
    afx_msg void OnCaptureChanged (CWnd *pWnd);
    afx_msg void OnMouseMove (UINT nFlags, CPoint point);
    afx_msg BOOL OnMouseWheel (UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnPaint ();
    afx_msg void OnSize (UINT nType, int cx, int cy);
    afx_msg void OnVScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnKeyDown (UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnTimer (UINT nIDEvent);
    afx_msg void OnSave ();
    afx_msg void OnDeleteSettings ();
    afx_msg void OnCopyArgumentsToClipboard ();
    afx_msg void OnSaveSettingsInRegistry ();
    afx_msg void OnRestoreSavedSettings ();
    afx_msg BOOL OnEraseBkgnd (CDC *pDC);
    afx_msg LRESULT OnMouseMessage (WPARAM wParam, LPARAM lParam);
    afx_msg void OnMinimizeToTray ();
    afx_msg void OnStartWindowMinimized ();
    afx_msg void OnUseSelectionRectangle ();
    afx_msg void OnRecursiveMagnification ();
    afx_msg LRESULT OnIsStartMinimized (WPARAM wParam, LPARAM lParam);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
