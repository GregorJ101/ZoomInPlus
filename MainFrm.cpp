#include "stdafx.h"
#include "ZoomIn++.h"
#include "MainFrm.h"
#include "RefreshRateDlg.h"
#include "DibApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define X_SNAP          15
#define Y_SNAP          15

#define ABS(i)          (((i) < 0) ? -(i) : (i))

#define TIMER_ID        1234

#define WM_NOTIFYICON   (WM_USER + 1000)

// Tenths of a millimeter per inch
#define MM10PERINCH     254

// Register a Window Message for the Taskbar Creation
// This message is passed to us if Explorer crashes and reloads
UINT CMainFrame::s_wmTaskbarCreated = RegisterWindowMessage (_T("TaskbarCreated"));

#ifndef MONITOR_DEFAULTTONULL
#define MONITOR_DEFAULTTONULL   0x00000000
#endif

//***********************************************
BOOL APIENTRY MonitorEnumProc (HMONITOR hMon, HDC hdcMon, PRECT pRectMon, LPARAM lParam)
{
    _ASSERTE(lParam);
    CRect *pRect = (CRect*)lParam;
    if (pRectMon->left < pRect->left)
        pRect->left = pRectMon->left;
    if (pRectMon->right > pRect->right)
        pRect->right = pRectMon->right;
    if (pRectMon->top < pRect->top)
        pRect->top = pRectMon->top;
    if (pRectMon->bottom > pRect->bottom)
        pRect->bottom = pRectMon->bottom;
    return TRUE;
}

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    //{{AFX_MSG_MAP(CMainFrame)
    ON_WM_CREATE()
    ON_WM_SYSCOMMAND()
    ON_WM_GETMINMAXINFO()
    ON_COMMAND(ID_EDIT_COPY, OnCopy)
    ON_COMMAND(ID_MENU_EXIT, OnExit)
    ON_COMMAND(ID_MENU_RESTORE, OnRestore)
    ON_COMMAND(ID_MENU_REFRESH, OnRefresh)
    ON_COMMAND(ID_MENU_REFRESH_RATE, OnRefreshRate)
    ON_COMMAND(ID_MENU_GRID, OnGrid)
    ON_COMMAND(ID_MENU_NEGATIVE_GRID, OnNegativeGrid)
    ON_UPDATE_COMMAND_UI(ID_MENU_NEGATIVE_GRID, OnUpdateNegativeGrid)
    ON_COMMAND(ID_MENU_ABSOLUTE_NUMBERING, OnAbsoluteNumbering)
    ON_COMMAND(ID_MENU_SAVE_SETTINGS, OnSaveSettings)
    ON_COMMAND(ID_MENU_MINIMIZETOTRAY, OnMinimizeToTray)
    ON_COMMAND(ID_MENU_ALWAYSONTOP, OnAlwaysOnTop)
    ON_COMMAND(ID_START_WINDOW_MINIMIZED,OnStartWindowMinimized)
    ON_COMMAND(ID_USE_SELECTION_RECTANGLE,OnUseSelectionRectangle)
    ON_COMMAND(ID_RECURSIVE_MAGNIFICATION,OnRecursiveMagnification)
    ON_COMMAND(ID_DELETE_SETTINGS,OnDeleteSettings)
    ON_COMMAND(ID_COPY_ARGUMENTS_TO_CLIPBOARD,OnCopyArgumentsToClipboard)
    ON_COMMAND(ID_SAVE_SETTINGS_IN_REGISTRY,OnSaveSettingsInRegistry)
    ON_COMMAND(ID_RESTORE_SAVED_SETTINGS,OnRestoreSavedSettings)
    ON_COMMAND(ID_COPY_ARGUMENTS_TO_CLIPBOARD,OnCopyArgumentsToClipboard)
    ON_WM_WINDOWPOSCHANGING()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_CAPTURECHANGED()
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSEWHEEL()
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_WM_VSCROLL()
    ON_WM_KEYDOWN()
    ON_WM_TIMER()
    ON_COMMAND(ID_FILE_SAVE, OnSave)
    ON_WM_ERASEBKGND()
    ON_MESSAGE(WM_NOTIFYICON, OnMouseMessage)
    ON_MESSAGE(WM_USER_GET_START_MINIMIZED, OnIsStartMinimized)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//***********************************************
CMainFrame::CMainFrame ()
{
    // Load the icons and cursors
    m_hIconLarge =  (HICON)LoadImage   (AfxGetInstanceHandle (), MAKEINTRESOURCE(IDR_MAINFRAME),     IMAGE_ICON,   GetSystemMetrics (SM_CXICON),   GetSystemMetrics (SM_CYICON),   0);
    m_hIconSmall =  (HICON)LoadImage   (AfxGetInstanceHandle (), MAKEINTRESOURCE(IDR_MAINFRAME),     IMAGE_ICON,   GetSystemMetrics (SM_CXSMICON), GetSystemMetrics (SM_CYSMICON), 0);
    m_hCursorScan = (HCURSOR)LoadImage (AfxGetInstanceHandle (), MAKEINTRESOURCE(IDC_CROSSHAIR_CUR), IMAGE_CURSOR, GetSystemMetrics (SM_CXCURSOR), GetSystemMetrics (SM_CYCURSOR), 0);
    m_hCursorPrev = NULL;

    m_bCaptionValid = ValidateCaption (AfxGetApp ()->m_lpCmdLine);
    m_settings.Load (m_cstrCaption);
    m_bSaveSettings = true;                     // By default it's true, OnSaveSettings negates this
    m_settings.ToggleGrid ();                   // By default it's false, OnGrid negates this
    m_settings.ToggleAbsoluteNumbering ();      // By default it's true, OnAbsoluteNumbering negates this
    m_settings.ToggleMinimizeToTray ();         // By default it's false, OnMinimizeToTray negates this
    m_settings.ToggleAlwaysOnTop ();            // By default it's true, OnAlwaysOnTop negates this
    m_settings.ToggleStartMinimized ();         // By default it's false, OnStartWindowMinimized negates this
    m_settings.ToggleUseSelectionRectangle ();  // By default it's false, OnUseSelectionRectangle negates this
    m_settings.ToggleRecursiveMagnification (); // By default it's true, OnRecursiveMagnification negates this

    CPoint point;
    CSize size;

    m_settings.GetWindowPos (point, size);

    m_menu.LoadMenu (IDR_MAINFRAME);

    CString strWndClass = AfxRegisterWndClass (0, AfxGetApp ()->LoadStandardCursor (IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), m_hIconSmall);
    DWORD dwStyle = WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX;
    CreateEx (WS_EX_TOPMOST, strWndClass, _T("ZoomIn"), dwStyle,
             point.x, point.y, size.cx, size.cy, NULL, m_menu.GetSafeHmenu ());
    LoadAccelTable (MAKEINTRESOURCE(IDR_MAINFRAME));

    SetIcon (m_hIconLarge, TRUE);       // Set big icon
    SetIcon (m_hIconSmall, FALSE);      // Set small icon

    CRect rectDesktop  = GetDesktopSize ();
    m_nxScreenMax      = rectDesktop.Width () - 1;
    m_nyScreenMax      = rectDesktop.Height () - 1;
    m_bIsLooking       = false;
    m_nTimerID         = 0;
    m_ulMouseMoveCount = 0;

    CreatePhysicalPalette ();
    OnGrid ();
    OnAbsoluteNumbering ();
    OnSaveSettings ();
    OnMinimizeToTray ();
    OnAlwaysOnTop ();
    OnStartWindowMinimized ();
    OnUseSelectionRectangle ();
    OnRecursiveMagnification ();

    if (m_settings.GetAutoRefresh ())
        EnableAutoRefresh (true);

    CString strMsg;
    strMsg.Format (_T("%2.1fx"), m_settings.GetZoomF ());
    m_wndStatusBar.SetPaneText (0, strMsg);
    m_wndScrollBar.SetScrollPos (m_settings.GetZoom (), TRUE);
    CString cstrCaption ("ZoomIn++");
    if (m_cstrCaption.GetLength () > 0)
    {
        cstrCaption.Append (" (");
        cstrCaption.Append (m_cstrCaption);
        cstrCaption.Append (")");
    }
    SetWindowText (cstrCaption);
}

//***********************************************
CMainFrame::~CMainFrame ()
{
    if (m_bSaveSettings)
        m_settings.Save (m_cstrCaption);
    if (m_hIconSmall != NULL)
        DestroyIcon (m_hIconSmall);
    if (m_hIconLarge != NULL)
        DestroyIcon (m_hIconLarge);
    if (m_hCursorScan != NULL)
        DestroyCursor (m_hCursorScan);
}

//***********************************************
int CMainFrame::OnCreate (CREATESTRUCT *pCreateStruct)
{
    if (CFrameWnd::OnCreate (pCreateStruct) == -1)
        return -1;

    static UINT indicators[] =
    {
        ID_INDICATOR_ZOOM,
        ID_INDICATOR_CURSOR,
        ID_INDICATOR_RGB
    };

    if (!m_wndStatusBar.Create (this) || !m_wndStatusBar.SetIndicators (indicators, sizeof(indicators) / sizeof(UINT)))
        return -1;

    m_wndStatusBar.SetPaneInfo (0, ID_INDICATOR_ZOOM,   SBPS_NORMAL, 26);
    m_wndStatusBar.SetPaneInfo (1, ID_INDICATOR_CURSOR, SBPS_NORMAL, 62);
    m_wndStatusBar.SetPaneInfo (2, ID_INDICATOR_RGB,    SBPS_NORMAL, 100);

    // Setup the scroll bar
    RECT rect;
    GetClientRect (&rect);
    rect.bottom -= GetStatusBarHeight ();

    m_wndScrollBar.Create (WS_CHILD | WS_VISIBLE | SBS_VERT | SBS_RIGHTALIGN, rect, this, IDC_VSCROLLBAR);
    m_wndScrollBar.SetScrollRange (MIN_ZOOM, MAX_ZOOM, FALSE);

    return 0;
}

//***********************************************
BOOL CMainFrame::DestroyWindow ()
{
    if (m_bSaveSettings)
    {
        CRect rect;
        GetWindowRect (rect);
        CPoint point (rect.left, rect.top);
        CSize size (rect.Width (), rect.Height ());
        m_settings.SetWindowPos (point, size);
        m_settings.SetIsIconic (IsIconic () != FALSE);
    }

    if (m_nTimerID != 0)
        KillTimer (m_nTimerID);
    return CFrameWnd::DestroyWindow ();
}

//***********************************************
void CMainFrame::OnSysCommand (UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == SC_MINIMIZE && m_settings.GetMinimizeToTray ())
    {
        ShowWindow (SW_HIDE);
        AddTrayIcon ();
    }
    else
    {
        CFrameWnd::OnSysCommand (nID, lParam);
    }
}

//***********************************************
bool CMainFrame::ValidateCaption (const char* szCaption)
{
    if (strlen (szCaption) == 0)
    {
        return true;             
    }
    else if (strlen (szCaption) > 30)
    {
        MessageBox ("Caption string too long; max length is 30 characters.\nCaption string ignored.",
                    "Zoomin++", MB_ICONSTOP | MB_OK);
        return false;
    }

    m_cstrCaption = szCaption;
    for (int iIdx = 0; iIdx < m_cstrCaption.GetLength (); ++iIdx)
    {
        char cTest = m_cstrCaption[iIdx];
        if (!(cTest >= '0' && cTest <= '9' ||
              cTest >= 'a' && cTest <= 'z' ||
              cTest >= 'A' && cTest <= 'Z' ||
              cTest == '!'                 ||
              cTest == '@'                 ||
              cTest == '#'                 ||
              cTest == '$'                 ||
              cTest == '%'                 ||
              cTest == '&'                 ||
              cTest == '*'                 ||
              cTest == '+'                 ||
              cTest == '-'))
        {
            MessageBox ("Invalid caption string: \"" + m_cstrCaption + "\"."
                        "\nCharacters must be alphanumeric or one of \"@ # $ % & * + -\"."
                        "\nCaption string ignored.",
                        "Zoomin++", MB_ICONSTOP | MB_OK);
            m_cstrCaption.Empty ();

            return false;
        }
    }

    return true;
}

//***********************************************
CRect CMainFrame::GetDesktopSize ()
{
    CRect rect (0, 0, 0, 0);

    HMODULE hUser32 = GetModuleHandle (_T("user32.dll"));
    if (hUser32)
    {
        typedef BOOL (APIENTRY *PFNMONITORENUMPROC)(HMONITOR, HDC, PRECT, LPARAM);
        typedef BOOL (APIENTRY *PFNENUMDISPLAYMONITORS)(HDC, PRECT, PFNMONITORENUMPROC, LPARAM);

        PFNENUMDISPLAYMONITORS pfnEnumDisplayMonitors = (PFNENUMDISPLAYMONITORS)GetProcAddress (hUser32, "EnumDisplayMonitors");
        if (pfnEnumDisplayMonitors)
            pfnEnumDisplayMonitors (NULL, NULL, MonitorEnumProc, (LPARAM)&rect);
    }

    if (rect.Width () == 0 || rect.Height () == 0)
    {
        rect.right  = GetSystemMetrics (SM_CXSCREEN);
        rect.bottom = GetSystemMetrics (SM_CYSCREEN);
    }

    return rect;
}

//***********************************************
void CMainFrame::CreatePhysicalPalette ()
{
    PLOGPALETTE plp = (PLOGPALETTE)new BYTE[sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 0xFF)];
    plp->palVersion = 0x300;
    plp->palNumEntries = 0xFF;  // 256 palette entries

    for (int nIndex = 0; nIndex < 0xFF; nIndex++)
    {
        plp->palPalEntry[nIndex].peFlags = PC_EXPLICIT;
        plp->palPalEntry[nIndex].peRed   = nIndex;
        plp->palPalEntry[nIndex].peGreen = 0;
        plp->palPalEntry[nIndex].peBlue  = 0;
    }

    m_palPhysical.CreatePalette (plp);
    delete[] plp;
}

//***********************************************
void CMainFrame::OnGetMinMaxInfo (MINMAXINFO *pMMI)
{
    pMMI->ptMinTrackSize = CPoint (219, 196);
    CFrameWnd::OnGetMinMaxInfo (pMMI);
}

//***********************************************
void CMainFrame::OnSave ()
{
    CFileDialog dlg (FALSE, _T(".bmp"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Bitmap files (*.bmp)|*.bmp|All Files (*.*)|*.*||"), this);

    if (dlg.DoModal () == IDOK)
    {
        RECT rect;
        GetClientRect (&rect);
        rect.right  -= ::GetSystemMetrics (SM_CXVSCROLL);
        rect.bottom -= GetStatusBarHeight ();

        CClientDC dc (this);
        DoTheZoom (&dc);
        CBitmapSave::Save (&dc, rect, dlg.GetPathName ());
    }
}

//***********************************************
void CMainFrame::OnCopy ()
{
    if (OpenClipboard ())
    {
        EmptyClipboard ();

        CClientDC dcSrc (this);
        CDC dcDest;

        CRect rect;
        GetClientRect (&rect);
        rect.right  -= ::GetSystemMetrics (SM_CXVSCROLL);
        rect.bottom -= GetStatusBarHeight ();

        int nWidth  = rect.Width (),
            nHeight = rect.Height ();

        HBITMAP hBmp;
        if (hBmp = CreateCompatibleBitmap (dcSrc.GetSafeHdc (), nWidth, nHeight))
        {
            if (dcDest.CreateCompatibleDC (&dcSrc))
            {
                // Calculate the dimensions of the bitmap and convert them to
                // tenths of a millimeter for setting the size with the
                // SetBitmapDimensionEx call.  This allows programs like Word
                // to retrieve the bitmap and know what size to display it as.
                SetBitmapDimensionEx (hBmp, (nWidth * MM10PERINCH) / dcSrc.GetDeviceCaps (LOGPIXELSX), (nHeight * MM10PERINCH) / dcSrc.GetDeviceCaps (LOGPIXELSY), NULL);

                dcDest.SelectObject (CBitmap::FromHandle (hBmp));
                dcDest.BitBlt (0, 0, nWidth, nHeight, &dcSrc, rect.left, rect.top, SRCCOPY);
                dcDest.DeleteDC ();
                SetClipboardData (CF_BITMAP, hBmp);
            }
            else
            {
                DeleteObject (hBmp);
            }
        }
        CloseClipboard ();
    }
}

//***********************************************
void CMainFrame::OnRefresh ()
{
    DoTheZoom ();
}

//***********************************************
void CMainFrame::OnRefreshRate ()
{
    CRefreshRateDlg dlg;
    dlg.m_nRefreshInterval = m_settings.GetRefreshInterval ();
    dlg.m_bEnableAutoRefresh = m_settings.GetAutoRefresh ();

    if (dlg.DoModal () == IDOK)
    {
        m_settings.SetRefreshInterval (dlg.m_nRefreshInterval);
        m_settings.SetAutoRefresh (dlg.m_bEnableAutoRefresh == TRUE);

        // First disable the timer, then turn it back on.
        // This ensures the new timer rate is used.
        EnableAutoRefresh (false);
        if (m_settings.GetAutoRefresh ())
            EnableAutoRefresh (true);
    }
}

//***********************************************
void CMainFrame::OnGrid ()
{
    m_settings.ToggleGrid ();
    DoTheZoom ();

    // Check/uncheck the menu item
    UINT nFlags = MF_BYCOMMAND | (m_settings.GetGrid () ? MF_CHECKED : MF_UNCHECKED);
    m_menu.CheckMenuItem (ID_MENU_GRID, nFlags);
}

//***********************************************
void CMainFrame::OnNegativeGrid ()
{
    m_settings.ToggleNegativeGrid ();
    DoTheZoom ();

    // Check/uncheck the menu item
    UINT nFlags = MF_BYCOMMAND | (m_settings.GetNegativeGrid () ? MF_CHECKED : MF_UNCHECKED);
    m_menu.CheckMenuItem (ID_MENU_NEGATIVE_GRID, nFlags);
}

//***********************************************
void CMainFrame::OnUpdateNegativeGrid (CCmdUI *pCmdUI)
{
    pCmdUI->Enable (m_settings.GetGrid ());
}

//***********************************************
void CMainFrame::OnAbsoluteNumbering ()
{
    m_settings.ToggleAbsoluteNumbering ();

    // Check/uncheck the menu item
    UINT nFlags = MF_BYCOMMAND | (m_settings.GetAbsoluteNumbering () ? MF_CHECKED : MF_UNCHECKED);
    m_menu.CheckMenuItem (ID_MENU_ABSOLUTE_NUMBERING, nFlags);
}

//***********************************************
void CMainFrame::OnStartWindowMinimized ()
{
    m_settings.ToggleStartMinimized ();

    // Check/uncheck the menu item
    UINT nFlags = MF_BYCOMMAND | (m_settings.GetStartMinimized () ? MF_CHECKED : MF_UNCHECKED);
    m_menu.CheckMenuItem (ID_START_WINDOW_MINIMIZED, nFlags);
}

//***********************************************
void CMainFrame::OnUseSelectionRectangle ()
{
    m_settings.ToggleUseSelectionRectangle ();

    // Check/uncheck the menu item
    UINT nFlags = MF_BYCOMMAND | (m_settings.GetUseSelectionRectangle () ? MF_CHECKED : MF_UNCHECKED);
    m_menu.CheckMenuItem (ID_USE_SELECTION_RECTANGLE, nFlags);
}

//***********************************************
void CMainFrame::OnRecursiveMagnification ()
{
    m_settings.ToggleRecursiveMagnification ();

    // Check/uncheck the menu item
    UINT nFlags = MF_BYCOMMAND | (m_settings.GetRecursiveMagnification () ? MF_CHECKED : MF_UNCHECKED);
    m_menu.CheckMenuItem (ID_RECURSIVE_MAGNIFICATION, nFlags);
}

//***********************************************
void CMainFrame::OnSaveSettings ()
{
    m_bSaveSettings ^= true;

    // Check/uncheck the menu item
    UINT nFlags = MF_BYCOMMAND | m_bSaveSettings ? MF_CHECKED : MF_UNCHECKED;
    m_menu.CheckMenuItem (ID_MENU_SAVE_SETTINGS, nFlags);
}

//***********************************************
void CMainFrame::OnMinimizeToTray ()
{
    m_settings.ToggleMinimizeToTray ();

    // Check/uncheck the menu item
    UINT nFlags = MF_BYCOMMAND | m_settings.GetMinimizeToTray () ? MF_CHECKED : MF_UNCHECKED;
    m_menu.CheckMenuItem (ID_MENU_MINIMIZETOTRAY, nFlags);
}

//***********************************************
void CMainFrame::OnAlwaysOnTop ()
{
    m_settings.ToggleAlwaysOnTop ();
    ::SetWindowPos (GetSafeHwnd (), m_settings.GetAlwaysOnTop () ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    // Check/uncheck the menu item
    UINT nFlags = MF_BYCOMMAND | m_settings.GetAlwaysOnTop () ? MF_CHECKED : MF_UNCHECKED;
    m_menu.CheckMenuItem (ID_MENU_ALWAYSONTOP, nFlags);
}

//***********************************************
int CMainFrame::GetStatusBarHeight () const
{
    if (!m_wndStatusBar.IsWindowVisible ())
        return 0;

    CRect rect;
    m_wndStatusBar.GetWindowRect (&rect);
    return rect.Height ();
}

//***********************************************
void CMainFrame::OnWindowPosChanging (WINDOWPOS *pwndpos)
{
    if (!(GetKeyState (VK_CONTROL) & 0x8000))
    {
        CRect rectScreen (0, 0, 0, 0);

        HMODULE hUser32 = GetModuleHandle (_T("user32.dll"));
        if (hUser32)
        {
            typedef HMONITOR (APIENTRY *PFNMONITORFROMWINDOW)(HWND, DWORD);
            typedef BOOL     (APIENTRY *PFNGETMONITORINFO)(HMONITOR, LPMONITORINFO);

            PFNMONITORFROMWINDOW pfnMonitorFromWindow = (PFNMONITORFROMWINDOW)GetProcAddress (hUser32, "MonitorFromWindow");
#if defined(UNICODE) || defined(_UNICODE)
            PFNGETMONITORINFO pfnGetMonitorInfo = (PFNGETMONITORINFO)GetProcAddress (hUser32, "GetMonitorInfoW");
#else
            PFNGETMONITORINFO pfnGetMonitorInfo = (PFNGETMONITORINFO)GetProcAddress (hUser32, "GetMonitorInfoA");
#endif
            if (pfnMonitorFromWindow && pfnGetMonitorInfo)
            {
                HMONITOR hMon = pfnMonitorFromWindow (GetSafeHwnd (), MONITOR_DEFAULTTONULL);
                if (hMon)
                {
                    MONITORINFO mi = {sizeof(MONITORINFO), 0};
                    pfnGetMonitorInfo (hMon, &mi);
                    rectScreen = mi.rcWork;
                }
            }
        }

        if (rectScreen.Width () == 0 || rectScreen.Height () == 0)
            SystemParametersInfo (SPI_GETWORKAREA, NULL, &rectScreen, 0);

        if (ABS(pwndpos->x - rectScreen.left) <= X_SNAP)
            pwndpos->x = rectScreen.left;
        else if (ABS(pwndpos->x + pwndpos->cx - rectScreen.right) <= X_SNAP)
            pwndpos->x = rectScreen.right - pwndpos->cx;

        if (ABS(pwndpos->y - rectScreen.top) <= Y_SNAP)
            pwndpos->y = rectScreen.top;
        else if (ABS(pwndpos->y + pwndpos->cy - rectScreen.bottom) <= Y_SNAP)
            pwndpos->y = rectScreen.bottom - pwndpos->cy;
    }
}

//***********************************************
void CMainFrame::OnLButtonDown (UINT nFlags, CPoint point)
{
    m_settings.SetZoomPoint (point);

    m_wndStatusBar.SetPaneText (1, _T(""));
    m_wndStatusBar.SetPaneText (2, _T(""));

    ClientToScreen (&point);
    if (m_settings.GetUseSelectionRectangle ())
    {
        DrawZoomRect (point);
    }
    DoTheZoom ();

    SetCapture ();
    m_bIsLooking = true;
    if (!m_settings.GetUseSelectionRectangle ())
    {
        m_hCursorPrev = SetCursor (m_hCursorScan);
    }
    else
    {
#ifdef _DEBUG
        m_hCursorPrev = SetCursor (m_hCursorScan);
#else
        m_hCursorPrev = SetCursor (NULL);
#endif
    }

    CFrameWnd::OnLButtonDown (nFlags, point);
}

//***********************************************
void CMainFrame::OnLButtonUp (UINT nFlags, CPoint point)
{
    if (m_bIsLooking)
    {
        if (m_settings.GetUseSelectionRectangle ())
        {
            DrawZoomRect (point);
        }
        ReleaseCapture ();
        m_bIsLooking = false;

        SetCursor (m_hCursorPrev);

        // Redraw the whole screen
        ::InvalidateRect (NULL, NULL, FALSE);
    }

    CFrameWnd::OnLButtonUp (nFlags, point);
}

//***********************************************
void CMainFrame::OnCaptureChanged (CWnd *pWnd)
{
    //TRACE ("OnCaptureChanged ()\n");
    if (m_bIsLooking)
    {
        if (m_settings.GetUseSelectionRectangle ())
        {
            DrawZoomRect ();
        }
        ReleaseCapture ();
        m_bIsLooking = false;

        SetCursor (m_hCursorPrev);
    }

    CFrameWnd::OnCaptureChanged (pWnd);
}

//***********************************************
void CMainFrame::OnMouseMove (UINT nFlags, CPoint point)
{
    //TRACE ("OnMouseMove () %d x %d\n", point.x, point.y);
    if (m_bIsLooking)
    {
        ClientToScreen (&point);

        if (m_settings.GetUseSelectionRectangle ())
        {
            if (!(m_ulMouseMoveCount++ % 5))
            {
                DrawZoomRect (point);
            }
        }
        else
        {
            // This takes care of the extremely sluggish response causes by drawing the selection rectangle
            m_hCursorPrev = SetCursor (m_hCursorScan);
        }

        m_settings.SetZoomPoint (point);
        DoTheZoom ();
    }
    else
    {
        CString strMsg;
        int nXRef = 0, nYRef = 0;

        if (m_settings.GetAbsoluteNumbering ())
        {
            CPoint pt;
            pt.x = BOUND(m_settings.GetZoomPoint ().x, m_settings.GetXZoomed () / 2, m_nxScreenMax - (m_settings.GetXZoomed () / 2));
            pt.y = BOUND(m_settings.GetZoomPoint ().y, m_settings.GetYZoomed () / 2, m_nyScreenMax - (m_settings.GetYZoomed () / 2));
            nXRef = pt.x - m_settings.GetXZoomed () / 2;
            nYRef = pt.y - m_settings.GetYZoomed () / 2;
        }
        strMsg.Format (_T("(%3d, %3d)"),
                       nXRef + (int)(point.x / m_settings.GetZoomF ()),
                       nYRef + (int)(point.y / m_settings.GetZoomF ()));
        //TRACE ("Format: (%3d, %3d)\n",
        //               nXRef + (int)(point.x / m_settings.GetZoomF ()),
        //               nYRef + (int)(point.y / m_settings.GetZoomF ()));
        m_wndStatusBar.SetPaneText (1, strMsg);

        COLORREF color;
        CClientDC dc (this);
        color = GetPixel (dc.GetSafeHdc (), point.x, point.y);
        strMsg.Format (_T("RGB (%3d, %3d, %3d)"), GetRValue(color), GetGValue(color), GetBValue(color));
        m_wndStatusBar.SetPaneText (2, strMsg);
    }

    CFrameWnd::OnMouseMove (nFlags, point);
}

//***********************************************
void CMainFrame::OnPaint ()
{
    CPaintDC dc (this);
    DoTheZoom (&dc);
}

//***********************************************
void CMainFrame::OnSize (UINT nType, int cx, int cy)
{
    CFrameWnd::OnSize (nType, cx, cy);

    // Move the ScrollBar
    CRect rect;
    GetClientRect (&rect);
    rect.bottom -= GetStatusBarHeight ();

    int nSBWidth = ::GetSystemMetrics (SM_CXVSCROLL);
    m_wndScrollBar.SetWindowPos (NULL, rect.right - nSBWidth, rect.top, nSBWidth, rect.Height (), SWP_NOZORDER);

    CalcZoomedSize ();
    DoTheZoom ();
}

//***********************************************
void CMainFrame::CalcZoomedSize ()
{
    RECT rect;
    GetClientRect (&rect);

    m_settings.SetXZoomed ((int)(((rect.right  - GetSystemMetrics (SM_CXVSCROLL)) / m_settings.GetZoomF ()) + 1));
    m_settings.SetYZoomed ((int)(((rect.bottom - GetStatusBarHeight ())           / m_settings.GetZoomF ()) + 1));
}

//***********************************************
void CMainFrame::DoTheZoom (CDC *pDC)
{
    if (!IsWindowVisible ())
        return;

    CDC *pdcScreen   = NULL;    // CDC ptr to entire screen
    CDC *pdcZoom      = NULL;   // CDC ptr to client area of Zoomin window
    CPalette *ppalOld = NULL;   // CPallette ptr to palette of Zoomin window

    CDC dcMem;                      // Memory CDC for off-screen drawing
    CBitmap bmpMem;                 // Memory BMP for off-screen drawing
    CBitmap *pbmpMemOld  = NULL;    // CBitmap ptr to old memory bitmap
    CPalette *ppalMemOld = NULL;    // CPalette ptr to old memory palette
    int nSBWidth = 0, nSBHeight = 0;

    // Establish a pointer to the screen's DC
    pdcZoom = (pDC == NULL) ? GetDC () : pDC;

    // Set clip areas to exclude status bar and scroll bar areas
    CRect rect;
    m_wndStatusBar.GetWindowRect (&rect);
    ScreenToClient (&rect);
    if (!m_settings.GetRecursiveMagnification ())
    {
        pdcZoom->ExcludeClipRect (&rect);
    }

    // Get ScrollBar Width
    nSBWidth = ::GetSystemMetrics (SM_CXVSCROLL);
    GetClientRect (&rect);
    rect.left = rect.right - nSBWidth;
    if (!m_settings.GetRecursiveMagnification ())
    {
        pdcZoom->ExcludeClipRect (&rect);
    }

    // Setup off-screen dc (Use an off-screen dc to avoid flickering)
    dcMem.CreateCompatibleDC (pdcZoom);
    bmpMem.CreateCompatibleBitmap (pdcZoom,
                                   (int)(m_settings.GetZoomF () * m_settings.GetXZoomed ()),
                                   (int)(m_settings.GetZoomF () * m_settings.GetYZoomed ()));
    pbmpMemOld = dcMem.SelectObject (&bmpMem);

    // If a palette has been created, set it up for use
    if ((HPALETTE)m_palPhysical)
    {
        ppalOld = pdcZoom->SelectPalette (&m_palPhysical, FALSE);
        pdcZoom->RealizePalette ();

        ppalMemOld = dcMem.SelectPalette (&m_palPhysical, FALSE);
        dcMem.RealizePalette ();
    }

    // Establish point to use for bitmapping
    // The point must not include areas outside the screen dimensions.
    CPoint pt;
    pt.x = BOUND(m_settings.GetZoomPoint ().x, m_settings.GetXZoomed () / 2, m_nxScreenMax - (m_settings.GetXZoomed () / 2));
    pt.y = BOUND(m_settings.GetZoomPoint ().y, m_settings.GetYZoomed () / 2, m_nyScreenMax - (m_settings.GetYZoomed () / 2));

    // Draw in the off-screen dc
    // Obtain pointer to screen dc
    pdcScreen = CDC::FromHandle (::GetDC (NULL));

    // Copy screen image to memory dc
    dcMem.FloodFill (1, 1, RGB(0, 0, 0));
    dcMem.SetStretchBltMode (COLORONCOLOR);
    dcMem.StretchBlt(0, 0,
                     (int)(m_settings.GetZoomF () * m_settings.GetXZoomed ()),
                     (int)(m_settings.GetZoomF () * m_settings.GetYZoomed ()),
                     pdcScreen,
                     pt.x - m_settings.GetXZoomed () / 2,
                     pt.y - m_settings.GetYZoomed () / 2,
                     m_settings.GetXZoomed (),
                     m_settings.GetYZoomed (),
                     SRCCOPY);

    // Does the zoomed rect intersect with the client window?
    // If so fill the area with black to prevent scanning the scan area.
    CRect rectClip, rectZoom (CPoint (pt.x - m_settings.GetXZoomed () / 2, pt.y - m_settings.GetYZoomed () / 2),
                              CSize  (m_settings.GetXZoomed (), m_settings.GetYZoomed ()));
    GetClientRect (&rect);
    rect.right  -= nSBWidth;
    rect.bottom -= GetStatusBarHeight ();
    ClientToScreen (&rect);
    if (!m_settings.GetRecursiveMagnification ())
    {
        if (rectClip.IntersectRect (rect, rectZoom))
        {
            CRect rectIntersect;
            rectIntersect.left   = (long)(((float)(rectClip.left - rectZoom.left) / (float)m_settings.GetXZoomed ()) * rect.Width ()) + rect.left;
            rectIntersect.right  = (long)(rectIntersect.left + rectClip.Width() * m_settings.GetZoomF ());
            rectIntersect.top    = (long)(((float)(rectClip.top - rectZoom.top)   / (float)m_settings.GetYZoomed ()) * rect.Height ()) + rect.top;
            rectIntersect.bottom = (long)(rectIntersect.top + rectClip.Height() * m_settings.GetZoomF ());

            ScreenToClient (&rectIntersect);
            dcMem.FillSolidRect (rectIntersect, RGB(0, 0, 0));
        }
    }

    // Finished with screen dc
    ReleaseDC (pdcScreen);

    // Draw in gridlines
    if (m_settings.GetGrid ())
        DoTheGrid (&dcMem);

    // Copy off-screen dc to windows dc
    SetStretchBltMode (pdcZoom->GetSafeHdc (), COLORONCOLOR);
    BitBlt (pdcZoom->GetSafeHdc (),
            0,
            0,
            (int)(m_settings.GetZoomF () * m_settings.GetXZoomed ()),
            (int)(m_settings.GetZoomF () * m_settings.GetYZoomed ()),
            dcMem.GetSafeHdc (),
            0,
            0,
            SRCCOPY);

    // Perform cleanup
    if (ppalOld != NULL)
    {
        pdcZoom->SelectPalette (ppalOld, FALSE);
        dcMem.SelectPalette (ppalMemOld, FALSE);
    }

    dcMem.SelectObject (pbmpMemOld);
    dcMem.DeleteDC ();

    if (pDC == NULL)
        ReleaseDC (pdcZoom);

    // Force scroll bar and status bar to repaint after being clobbered by StretchBlt
    m_wndScrollBar.Invalidate ();
    m_wndStatusBar.Invalidate ();
}

//***********************************************
void CMainFrame::DoTheGrid (CDC *pDC)
{
    _ASSERTE(pDC);

    if (m_settings.GetZoomF () < 6.0)
        return;

    RECT rect;
    GetClientRect (&rect);

    if (m_settings.GetNegativeGrid ())
    {   // Draw a "negative" grid
        for (int nX = 0; nX < m_settings.GetXZoomed (); nX++)
            pDC->PatBlt ((int)(nX * m_settings.GetZoomF ()),
                         0,
                         1,
                         rect.bottom,
                         DSTINVERT);

        for (int nY = 0; nY < m_settings.GetYZoomed (); nY++)
            pDC->PatBlt (0,
                         (int)(nY * m_settings.GetZoomF ()),
                         rect.right,
                         1,
                         DSTINVERT);
    }
    else
    {   // Draw a normal black grid
        for (int nX = 0; nX < m_settings.GetXZoomed (); nX++)
        {
            pDC->MoveTo((int)(nX * m_settings.GetZoomF ()), rect.top);
            pDC->LineTo((int)(nX * m_settings.GetZoomF ()), rect.bottom);
        }

        for (int nY = 0; nY < m_settings.GetYZoomed (); nY++)
        {
            pDC->MoveTo(rect.left,  (int)(nY * m_settings.GetZoomF ()));
            pDC->LineTo(rect.right, (int)(nY * m_settings.GetZoomF ()));
        }
    }
}

//***********************************************
void CMainFrame::DrawZoomRect (CPoint point)
{
    if (point.x < 0 ||
        point.y < 0)
    {
        point.x = m_settings.GetZoomPoint ().x;
        point.y = m_settings.GetZoomPoint ().y;
    }
    int nX = BOUND(point.x, m_settings.GetXZoomed () / 2, m_nxScreenMax - (m_settings.GetXZoomed () / 2));
    int nY = BOUND(point.y, m_settings.GetYZoomed () / 2, m_nyScreenMax - (m_settings.GetYZoomed () / 2));

    CRect rect;
    rect.left   = nX - m_settings.GetXZoomed () / 2;
    rect.top    = nY - m_settings.GetYZoomed () / 2;
    rect.right  = rect.left + m_settings.GetXZoomed ();
    rect.bottom = rect.top  + m_settings.GetYZoomed ();
    InflateRect (&rect, 1, 1);

    HDC hdc = ::GetDC (NULL);
    PatBlt (hdc, rect.left,      rect.top,        rect.Width (),    1,                 DSTINVERT);
    PatBlt (hdc, rect.left,      rect.bottom,     1,                -(rect.Height ()), DSTINVERT);
    PatBlt (hdc, rect.right - 1, rect.top,        1,                rect.Height (),    DSTINVERT);
    PatBlt (hdc, rect.right,     rect.bottom - 1, -(rect.Width ()), 1,                 DSTINVERT);
    ::ReleaseDC (NULL, hdc);
}

//***********************************************
void CMainFrame::OnVScroll (UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
    //TRACE ("OnVScroll (%d, %d, 0x%08X)\n", nSBCode, nPos, pScrollBar);
    switch (nSBCode)
    {
    case SB_LINEDOWN:
        m_settings.IncreaseZoom ();
        break;

    case SB_LINEUP:
        m_settings.DecreaseZoom ();
        break;

    case SB_PAGEDOWN:
        m_settings.IncreaseZoom (2);
        break;

    case SB_PAGEUP:
        m_settings.DecreaseZoom (2);
        break;

    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        m_settings.SetZoom (nPos);
        break;
    }

    m_wndScrollBar.SetScrollPos (m_settings.GetZoom (), TRUE);

    CString strMsg;
    strMsg.Format(_T("%2.1fx"), m_settings.GetZoomF ());
    m_wndStatusBar.SetPaneText (0, strMsg);

    CalcZoomedSize ();
    DoTheZoom ();

    CFrameWnd::OnVScroll (nSBCode, nPos, pScrollBar);
}

//***********************************************
void CMainFrame::OnKeyDown (UINT nChar, UINT nRepCnt, UINT nFlags)
{
    bool bControlPressed;

    switch (nChar)
    {
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
        MoveView (nChar, (GetKeyState (VK_SHIFT) & 0x8000) ? true : false, (GetKeyState (VK_CONTROL) & 0x8000) ? true : false);
        break;
    case VK_ADD:
        bControlPressed = (GetKeyState (VK_CONTROL) & 0x8000) ? true : false;
        OnVScroll (bControlPressed ? SB_THUMBPOSITION : SB_LINEDOWN, MAX_ZOOM, &m_wndScrollBar);
        break;
    case VK_SUBTRACT:
        bControlPressed = (GetKeyState (VK_CONTROL) & 0x8000) ? true : false;
        OnVScroll (bControlPressed ? SB_THUMBPOSITION : SB_LINEUP, MIN_ZOOM, &m_wndScrollBar);
        break;
    }

    CFrameWnd::OnKeyDown (nChar, nRepCnt, nFlags);
}

//***********************************************
void CMainFrame::MoveView (int nDirectionCode, bool bFast, bool bPeg)
{
    // If bFast is true move the view faster (more pixels)
    int nDelta = (bFast) ? 8 : 1;
    CPoint ptZoom = m_settings.GetZoomPoint ();
    //TRACE ("MoveView () %d x %d\n", ptZoom.x, ptZoom.y);

    switch (nDirectionCode)
    {
    case VK_UP:
        if (bPeg)
            ptZoom.y = m_settings.GetYZoomed () / 2;
        else
            ptZoom.y -= nDelta;
        ptZoom.y = BOUND(ptZoom.y, 0, m_nyScreenMax);
        break;

    case VK_DOWN:
        if (bPeg)
            ptZoom.y = m_nyScreenMax - (m_settings.GetYZoomed () / 2);
        else
            ptZoom.y += nDelta;
        ptZoom.y = BOUND(ptZoom.y, 0, m_nyScreenMax);
        break;

    case VK_LEFT:
        if (bPeg)
            ptZoom.x = m_settings.GetXZoomed () / 2;
        else
            ptZoom.x -= nDelta;
        ptZoom.x = BOUND(ptZoom.x, 0, m_nxScreenMax);
        break;

    case VK_RIGHT:
        if (bPeg)
            ptZoom.x = m_nxScreenMax - (m_settings.GetXZoomed () / 2);
        else
            ptZoom.x += nDelta;
        ptZoom.x = BOUND(ptZoom.x, 0, m_nxScreenMax);
        break;
    }

    m_settings.SetZoomPoint (ptZoom);

    DoTheZoom ();
}

//***********************************************
void CMainFrame::EnableAutoRefresh (const bool bEnable)
{
    if (bEnable)
    {   // Enable
        if (m_nTimerID == 0)
            m_nTimerID = SetTimer (TIMER_ID, m_settings.GetRefreshInterval () * 100, NULL);
    }
    else
    {   // Disable
        if (m_nTimerID != 0)
        {
            KillTimer (m_nTimerID);
            m_nTimerID = 0;
        }
    }
}

//***********************************************
void CMainFrame::OnTimer (UINT nIDEvent)
{
    DoTheZoom ();
    CFrameWnd::OnTimer (nIDEvent);
}

//***********************************************
void CMainFrame::OnDeleteSettings ()
{
    m_settings.Delete ();
}

//***********************************************
void CMainFrame::OnCopyArgumentsToClipboard ()
{
}

//***********************************************
void CMainFrame::OnSaveSettingsInRegistry ()
{
    CRect rect;
    GetWindowRect (rect);
    CPoint point (rect.left, rect.top);
    CSize size (rect.Width (), rect.Height ());
    m_settings.SetWindowPos (point, size);

    m_settings.Save (m_cstrCaption);
}

//***********************************************
void CMainFrame::OnRestoreSavedSettings ()
{
    m_settings.Load (m_cstrCaption);
    CPoint ptWindowTopLeft;
    CSize szWindowSize;
    m_settings.GetWindowPos (ptWindowTopLeft, szWindowSize);
    SetWindowPos (&wndTopMost, ptWindowTopLeft.x, ptWindowTopLeft.y, szWindowSize.cx, szWindowSize.cy, SWP_NOZORDER | SWP_SHOWWINDOW);
}

//***********************************************
BOOL CMainFrame::OnEraseBkgnd (CDC *pDC)
{
    return TRUE;
}

//***********************************************
void CMainFrame::AddTrayIcon () const
{
    // Add the tray icon
    CString strTip;
    strTip.LoadString (IDS_SYSTRAY_TIP);
    NOTIFYICONDATA nid = {0};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = GetSafeHwnd ();
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.hIcon = m_hIconSmall;
    nid.uCallbackMessage = WM_NOTIFYICON;
    lstrcpyn (nid.szTip, strTip, sizeof(nid.szTip));

    Shell_NotifyIcon (NIM_ADD, &nid);
}

//***********************************************
void CMainFrame::RemoveTrayIcon () const
{
    // Remove the tray icon
    NOTIFYICONDATA nid = {0};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = GetSafeHwnd ();

    Shell_NotifyIcon (NIM_DELETE, &nid);
}

//***********************************************
LRESULT CMainFrame::OnMouseMessage (WPARAM wParam, LPARAM lParam)
{
    // OnMouseMessage - processes callback messages for taskbar icons
    // wParam - first message parameter of the callback message
    // lParam - second message parameter of the callback message

    if (lParam == WM_LBUTTONDBLCLK)
    {
        OnRestore ();
    }
    else if (lParam == WM_RBUTTONUP)
    {
        CMenu menu;
        menu.LoadMenu (IDR_POPUP);
        CMenu *pPopup = menu.GetSubMenu (0);

        SetMenuDefaultItem (pPopup->GetSafeHmenu (), ID_MENU_RESTORE, FALSE);

        POINT pt;
        GetCursorPos (&pt);
        SetForegroundWindow ();
        pPopup->TrackPopupMenu (TPM_RIGHTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
        PostMessage (WM_NULL, 0, 0);
    }

    return 0;
}

//***********************************************
LRESULT CMainFrame::OnIsStartMinimized (WPARAM wParam, LPARAM lParam)
{
    return m_settings.GetStartMinimized ();
}

//***********************************************
void CMainFrame::OnExit ()
{
    RemoveTrayIcon ();
    PostMessage (WM_CLOSE);
}

//***********************************************
void CMainFrame::OnRestore ()
{
    RemoveTrayIcon ();
    ShowWindow (SW_SHOW);
}

//***********************************************
BOOL CMainFrame::OnMouseWheel (UINT nFlags, short zDelta, CPoint pt)
{
    //TRACE ("OnMouseWheel (%d, %d, %d x %d)\n", nFlags, zDelta, pt.x, pt.y);
    OnVScroll (zDelta > 0 ? SB_LINEDOWN : SB_LINEUP, 0, &m_wndScrollBar);
    return CFrameWnd::OnMouseWheel (nFlags, zDelta, pt);
}
