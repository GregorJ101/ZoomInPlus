#include "stdafx.h"
#include "ZoomIn++.h"
#include "Settings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const static PCTSTR g_szZoomInRegKey                 = _T("SOFTWARE\\Sacred Cat Software\\ZoomIn++");
const static PCTSTR g_szRegValuePosition             = _T("Position");
const static PCTSTR g_szRegValueZoom                 = _T("Zoom");
const static PCTSTR g_szRegValueAutoRefresh          = _T("Auto Refresh");
const static PCTSTR g_szRegValueRefreshInterval      = _T("Refresh Interval");
const static PCTSTR g_szRegValueGrid                 = _T("Grid");
const static PCTSTR g_szRegValueNegativeGrid         = _T("Negative Grid");
const static PCTSTR g_szRegValueAbsoluteNumbering    = _T("Absolute Numbering");
const static PCTSTR g_szRegValueMinimizeToTray       = _T("Minimize To Tray");
const static PCTSTR g_szRegValueAlwaysOnTop          = _T("Always On Top");
const static PCTSTR g_szRegValueZoomPointX           = _T("ZoomPointX");
const static PCTSTR g_szRegValueZoomPointY           = _T("ZoomPointY");
const static PCTSTR g_szRegValueStartMinimized       = _T("StartMinimized");
const static PCTSTR g_szRegValueUseRectangle         = _T("UseSelectionRectangle");
const static PCTSTR g_szRegValueRecurseMagnification = _T("RecursiveMagnification");

//***********************************************
CSettings::CSettings ()
{
    m_ptZoom = CPoint (0, 0);
    m_bIsIconic = false;
    LoadDefaults ();
}

//***********************************************
CSettings::CSettings (const CSettings &src)
{
    LoadDefaults ();
    *this = src;
}

//***********************************************
CSettings &CSettings::operator= (const CSettings &src)
{
    _ASSERTE(this != &src);

    m_pt                      = src.m_pt;
    m_size                    = src.m_size;
    m_nZoom                   = src.m_nZoom;
    m_bAutoRefresh            = src.m_bAutoRefresh;
    m_nRefreshInterval        = src.m_nRefreshInterval;
    m_bGrid                   = src.m_bGrid;
    m_bNegativeGrid           = src.m_bNegativeGrid;
    m_bAbsoluteNumbering      = src.m_bAbsoluteNumbering;
    m_bMinimizeToTray         = src.m_bMinimizeToTray;
    m_bAlwaysOnTop            = src.m_bAlwaysOnTop;
    m_bStartMinimized         = src.m_bStartMinimized;
    m_bUseSelectionRectangle  = src.m_bUseSelectionRectangle;
    m_bRecursiveMagnification = src.m_bRecursiveMagnification;

    return *this;
}

//***********************************************
void CSettings::LoadDefaults ()
{
    m_pt                      = CPoint (0, 0);
    m_size                    = CSize (0, 0);
    m_nZoom                   = 40;
    m_bAutoRefresh            = false;
    m_nRefreshInterval        = 6;
    m_bGrid                   = false;
    m_bNegativeGrid           = false;
    m_bAbsoluteNumbering      = true;
    m_bMinimizeToTray         = false;
    m_bAlwaysOnTop            = true;
    m_bStartMinimized         = false;
    m_bUseSelectionRectangle  = false;
    m_bRecursiveMagnification = true;
}

//***********************************************
bool CSettings::Load (const CString& cstrThemeName)
{
    CString cstrKey (cstrThemeName);
    if (cstrKey.IsEmpty ())
    {
        cstrKey = "default";
    }

    TRACE ("Load (%s)\n", cstrKey);

    HKEY hKey;
    LONG lRegRtn;
    CString cstrZoomInRegKey (g_szZoomInRegKey);
    cstrZoomInRegKey.Append ("\\");
    cstrZoomInRegKey.Append (cstrKey);
    TRACE ("Load reg key: %s\n", cstrZoomInRegKey);

    lRegRtn = RegOpenKeyEx (HKEY_CURRENT_USER, cstrZoomInRegKey, 0, KEY_READ, &hKey);
    if (lRegRtn != ERROR_SUCCESS)
    {
        lRegRtn = RegOpenKeyEx (HKEY_CURRENT_USER, g_szZoomInRegKey, 0, KEY_READ, &hKey);
    }
    if (lRegRtn == ERROR_SUCCESS)
    {
        DWORD dwData, dwType, dwSize, dwPos[4];

        // Load 'Position'
        dwSize = sizeof (dwPos);
        lRegRtn = RegQueryValueEx (hKey, g_szRegValuePosition, 0, &dwType, (PBYTE)&dwPos, &dwSize);
        if (lRegRtn == ERROR_SUCCESS && dwType == REG_BINARY)
        {
            m_pt.x = dwPos[0];
            m_pt.y = dwPos[1];
            m_size.cx = dwPos[2];
            m_size.cy = dwPos[3];

            CRect rectScreen;
            SystemParametersInfo (SPI_GETWORKAREA, NULL, &rectScreen, 0);
            if (m_pt.x < 1)
                m_pt.x = 1;
            if (m_pt.y < 1)
                m_pt.y = 1;
            if (m_pt.x + m_size.cx > rectScreen.right)
                m_pt.x = rectScreen.right - m_size.cx;
            if (m_pt.y + m_size.cy > rectScreen.bottom)
                m_pt.y = rectScreen.bottom - m_size.cy;
        }

        // Load 'Zoom'
        dwSize = sizeof(dwData);
        lRegRtn = RegQueryValueEx (hKey, g_szRegValueZoom, 0, &dwType, (PBYTE)&dwData, &dwSize);
        if (lRegRtn == ERROR_SUCCESS && dwType == REG_DWORD)
            m_nZoom = BOUND(dwData, 1, 32);

        // Load 'AutoRefresh'
        dwSize = sizeof(dwData);
        lRegRtn = RegQueryValueEx (hKey, g_szRegValueAutoRefresh, 0, &dwType, (PBYTE)&dwData, &dwSize);
        if (lRegRtn == ERROR_SUCCESS && dwType == REG_DWORD)
            m_bAutoRefresh = (dwData != 0);

        // Load 'RefreshInterval'
        dwSize = sizeof(dwData);
        lRegRtn = RegQueryValueEx (hKey, g_szRegValueRefreshInterval, 0, &dwType, (PBYTE)&dwData, &dwSize);
        if (lRegRtn == ERROR_SUCCESS && dwType == REG_DWORD)
            m_nRefreshInterval = BOUND(dwData, 1, 50);

        // Load 'Grid'
        dwSize = sizeof(dwData);
        lRegRtn = RegQueryValueEx (hKey, g_szRegValueGrid, 0, &dwType, (PBYTE)&dwData, &dwSize);
        if (lRegRtn == ERROR_SUCCESS && dwType == REG_DWORD)
            m_bGrid = (dwData != 0);

        // Load 'NegativeGrid'
        dwSize = sizeof(dwData);
        lRegRtn = RegQueryValueEx (hKey, g_szRegValueNegativeGrid, 0, &dwType, (PBYTE)&dwData, &dwSize);
        if (lRegRtn == ERROR_SUCCESS && dwType == REG_DWORD)
            m_bNegativeGrid = (dwData != 0);

        // Load 'AbsoluteNumbering'
        dwSize = sizeof(dwData);
        lRegRtn = RegQueryValueEx (hKey, g_szRegValueAbsoluteNumbering, 0, &dwType, (PBYTE)&dwData, &dwSize);
        if (lRegRtn == ERROR_SUCCESS && dwType == REG_DWORD)
            m_bAbsoluteNumbering = (dwData != 0);

        // Load 'MinimizeToTray'
        dwSize = sizeof(dwData);
        lRegRtn = RegQueryValueEx (hKey, g_szRegValueMinimizeToTray, 0, &dwType, (PBYTE)&dwData, &dwSize);
        if (lRegRtn == ERROR_SUCCESS && dwType == REG_DWORD)
            m_bMinimizeToTray = (dwData != 0);

        // Load 'AlwaysOnTop'
        dwSize = sizeof(dwData);
        lRegRtn = RegQueryValueEx (hKey, g_szRegValueAlwaysOnTop, 0, &dwType, (PBYTE)&dwData, &dwSize);
        if (lRegRtn == ERROR_SUCCESS && dwType == REG_DWORD)
            m_bAlwaysOnTop = (dwData != 0);

        // Load 'StartMinimized'
        dwSize = sizeof(dwData);
        lRegRtn = RegQueryValueEx (hKey, g_szRegValueStartMinimized, 0, &dwType, (PBYTE)&dwData, &dwSize);
        if (lRegRtn == ERROR_SUCCESS && dwType == REG_DWORD)
            m_bStartMinimized = (dwData != 0);

        // Load 'UseRectangle'
        dwSize = sizeof(dwData);
        lRegRtn = RegQueryValueEx (hKey, g_szRegValueUseRectangle, 0, &dwType, (PBYTE)&dwData, &dwSize);
        if (lRegRtn == ERROR_SUCCESS && dwType == REG_DWORD)
            m_bUseSelectionRectangle = (dwData != 0);

        // Load 'RecursiveMagnification'
        dwSize = sizeof(dwData);
        lRegRtn = RegQueryValueEx (hKey, g_szRegValueRecurseMagnification, 0, &dwType, (PBYTE)&dwData, &dwSize);
        if (lRegRtn == ERROR_SUCCESS && dwType == REG_DWORD)
            m_bRecursiveMagnification = (dwData != 0);

        CPoint pt;
        // Load 'ZoomPointX'
        dwSize = sizeof(dwData);
        lRegRtn = RegQueryValueEx (hKey, g_szRegValueZoomPointX, 0, &dwType, (PBYTE)&dwData, &dwSize);
        if (lRegRtn == ERROR_SUCCESS && dwType == REG_DWORD)
            pt.x = (int)dwData;

        // Load 'ZoomPointY'
        dwSize = sizeof(dwData);
        lRegRtn = RegQueryValueEx (hKey, g_szRegValueZoomPointY, 0, &dwType, (PBYTE)&dwData, &dwSize);
        if (lRegRtn == ERROR_SUCCESS && dwType == REG_DWORD)
            pt.y = (int)dwData;

        m_ptZoom = pt;
        TRACE ("Load m_ptZoom: %d, %d\n", m_ptZoom.x, m_ptZoom.y);

        RegCloseKey (hKey);
    }

    return true;
}

//***********************************************
void CSettings::Save (const CString& cstrThemeName) const
{
    CString cstrKey (cstrThemeName);
    if (cstrKey.IsEmpty ())
    {
        cstrKey = "default";
    }

    TRACE ("Save (%s)\n", cstrKey);
    HKEY hKey;
    LONG lRegRtn;
    CString cstrZoomInRegKey (g_szZoomInRegKey);
    cstrZoomInRegKey.Append ("\\");
    cstrZoomInRegKey.Append (cstrKey);
    TRACE ("Load reg key: %s\n", cstrZoomInRegKey);

    lRegRtn = RegCreateKeyEx (HKEY_CURRENT_USER, cstrZoomInRegKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    if (lRegRtn == ERROR_SUCCESS)
    {
        DWORD dwData;

        // Size 'Position'
        if (!m_bIsIconic)
        {
            DWORD dwPos[4];

            dwPos[0] = m_pt.x;
            dwPos[1] = m_pt.y;
            dwPos[2] = m_size.cx;
            dwPos[3] = m_size.cy;
            //TRACE ("%d, %d, %d, %d\n", dwPos[0], dwPos[1], dwPos[2], dwPos[3]);
            VERIFY (RegSetValueEx (hKey, g_szRegValuePosition, 0, REG_BINARY, (PCBYTE)dwPos, sizeof(dwPos)) == ERROR_SUCCESS);
        }

        // Save 'Zoom'
        VERIFY (RegSetValueEx (hKey, g_szRegValueZoom, 0, REG_DWORD, (PCBYTE)&m_nZoom, sizeof(m_nZoom)) == ERROR_SUCCESS);

        // Save 'AutoRefresh'
        dwData = m_bAutoRefresh;
        VERIFY (RegSetValueEx (hKey, g_szRegValueAutoRefresh, 0, REG_DWORD, (PCBYTE)&dwData, sizeof(dwData)) == ERROR_SUCCESS);

        // Save 'RefreshInterval'
        VERIFY (RegSetValueEx (hKey, g_szRegValueRefreshInterval, 0, REG_DWORD, (PCBYTE)&m_nRefreshInterval, sizeof(m_nRefreshInterval)) == ERROR_SUCCESS);

        // Save 'Grid'
        dwData = m_bGrid;
        VERIFY (RegSetValueEx (hKey, g_szRegValueGrid, 0, REG_DWORD, (PCBYTE)&dwData, sizeof(dwData)) == ERROR_SUCCESS);

        // Save 'NegativeGrid'
        dwData = m_bNegativeGrid;
        VERIFY (RegSetValueEx (hKey, g_szRegValueNegativeGrid, 0, REG_DWORD, (PCBYTE)&dwData, sizeof(dwData)) == ERROR_SUCCESS);

        // Save 'AbsoluteNumbering'
        dwData = m_bAbsoluteNumbering;
        VERIFY (RegSetValueEx (hKey, g_szRegValueAbsoluteNumbering, 0, REG_DWORD, (PCBYTE)&dwData, sizeof(dwData)) == ERROR_SUCCESS);

        // Save 'MinimizeToTray'
        dwData = m_bMinimizeToTray;
        VERIFY (RegSetValueEx (hKey, g_szRegValueMinimizeToTray, 0, REG_DWORD, (PCBYTE)&dwData, sizeof(dwData)) == ERROR_SUCCESS);

        // Save 'AlwaysOnTop'
        dwData = m_bAlwaysOnTop;
        VERIFY (RegSetValueEx (hKey, g_szRegValueAlwaysOnTop, 0, REG_DWORD, (PCBYTE)&dwData, sizeof(dwData)) == ERROR_SUCCESS);

        // Save 'StartMinimized'
        dwData = m_bStartMinimized;
        VERIFY (RegSetValueEx (hKey, g_szRegValueStartMinimized, 0, REG_DWORD, (PCBYTE)&dwData, sizeof(dwData)) == ERROR_SUCCESS);

        // Save 'UseRectangle'
        dwData = m_bUseSelectionRectangle;
        VERIFY (RegSetValueEx (hKey, g_szRegValueUseRectangle, 0, REG_DWORD, (PCBYTE)&dwData, sizeof(dwData)) == ERROR_SUCCESS);

        // Save 'UseRectangle'
        dwData = m_bRecursiveMagnification;
        VERIFY (RegSetValueEx (hKey, g_szRegValueRecurseMagnification, 0, REG_DWORD, (PCBYTE)&dwData, sizeof(dwData)) == ERROR_SUCCESS);

        //TRACE ("Save m_ptZoom: %d, %d\n", m_ptZoom.x, m_ptZoom.y);
        // Save 'ZoomPointX'
        dwData = m_ptZoom.x;
        VERIFY (RegSetValueEx (hKey, g_szRegValueZoomPointX, 0, REG_DWORD, (PCBYTE)&dwData, sizeof(dwData)) == ERROR_SUCCESS);

        // Save 'ZoomPoint'
        dwData = m_ptZoom.y;
        VERIFY (RegSetValueEx (hKey, g_szRegValueZoomPointY, 0, REG_DWORD, (PCBYTE)&dwData, sizeof(dwData)) == ERROR_SUCCESS);

        RegCloseKey (hKey);
    }
}

//***********************************************
void CSettings::Delete () const
{
    RegDeleteKey (HKEY_CURRENT_USER, g_szZoomInRegKey);
}

//***********************************************
void CSettings::SetZoom (const int nZoom)
{
    m_nZoom = BOUND(nZoom, MIN_ZOOM, MAX_ZOOM);
}

//***********************************************
void CSettings::IncreaseZoom (const int nIncreaseFactor)
{
    ChangeZoomFactor (true, nIncreaseFactor);
}

//***********************************************
void CSettings::DecreaseZoom (const int nDecreaseFactor)
{
    ChangeZoomFactor (false, nDecreaseFactor);
}

//***********************************************
void CSettings::ChangeZoomFactor (bool bIncrement, int iDeltaFactor)
{
    if (bIncrement && m_nZoom >= MAX_ZOOM)
        return;

    if (!bIncrement && m_nZoom <= MIN_ZOOM)
        return;

    int iZoomDelta = m_nZoom >= 240 ? 10 :
                     m_nZoom >= 160 ? 5  :
                     m_nZoom >=  80 ? 2  : 1;

    int iZoomStep = iDeltaFactor + (bIncrement && (m_nZoom % iZoomDelta) ? 1 : 0);
    iZoomStep *= bIncrement ? 1 : -1;

    m_nZoom /= iZoomDelta;
    m_nZoom += iZoomStep;
    m_nZoom *= iZoomDelta;
}
