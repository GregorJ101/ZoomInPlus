#pragma once

#define MIN_ZOOM	1
#define MAX_ZOOM	320

// Returns x bound by min and max. The returned value is guaranteed between min and max.
#define BOUND(x,min,max)	((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

class CSettings
{
public:
    CSettings ();
    CSettings (const CSettings &src);
    virtual ~CSettings () { }

    CSettings &operator= (const CSettings &src);

    void LoadDefaults ();
    bool Load (const CString& cstrThemeName = "");
    void Save (const CString& cstrThemeName = "") const;
    void Delete () const;

    void GetWindowPos (CPoint &point, CSize &size) const { point = m_pt; size = m_size; }
    void SetWindowPos (const CPoint &point, const CSize &size) { m_pt = point; m_size = size; }

    int   GetZoom () const { return m_nZoom; }
    float GetZoomF () const { return (float)((float)(m_nZoom) / 10.0); }
    void  SetZoom (const int nZoom);
    void  IncreaseZoom (const int nIncreaseFactor = 1);
    void  DecreaseZoom (const int nDecreaseFactor = 1);

    bool GetAutoRefresh () const { return m_bAutoRefresh; }
    void SetAutoRefresh (const bool bAutoRefresh) { m_bAutoRefresh = bAutoRefresh; }
    void ToggleAutoRefresh () { m_bAutoRefresh ^= true; }

    int GetRefreshInterval () const { return m_nRefreshInterval; }
    void SetRefreshInterval (const int nRefreshInterval) { m_nRefreshInterval = nRefreshInterval; }

    bool GetGrid () const { return m_bGrid; }
    void SetGrid (const bool bGrid) { m_bGrid = bGrid; }
    void ToggleGrid () { m_bGrid ^= true; }

    bool GetNegativeGrid () const { return m_bNegativeGrid; }
    void SetNegativeGrid (const bool bNegativeGrid) { m_bNegativeGrid = bNegativeGrid; }
    void ToggleNegativeGrid () { m_bNegativeGrid ^= true; }

    bool GetAbsoluteNumbering () const { return m_bAbsoluteNumbering; }
    void SetAbsoluteNumbering (const bool bAbsoluteNumbering) { m_bAbsoluteNumbering = bAbsoluteNumbering; }
    void ToggleAbsoluteNumbering () { m_bAbsoluteNumbering ^= true; }

    bool GetMinimizeToTray () const { return m_bMinimizeToTray; }
    void SetMinimizeToTray (const bool bMinimizeToTray) { m_bMinimizeToTray = bMinimizeToTray; }
    void ToggleMinimizeToTray () { m_bMinimizeToTray ^= true; }

    bool GetAlwaysOnTop () const { return m_bAlwaysOnTop; }
    void SetAlwaysOnTop (const bool bAlwaysOnTop) { m_bAlwaysOnTop = bAlwaysOnTop; }
    void ToggleAlwaysOnTop () { m_bAlwaysOnTop ^= true; }

    bool GetStartMinimized () const { return m_bStartMinimized; }
    void SetStartMinimized (bool bStartMinimized) { m_bStartMinimized = bStartMinimized; }
    void ToggleStartMinimized () { m_bStartMinimized ^= true; }

    bool GetUseSelectionRectangle () const { return m_bUseSelectionRectangle; }
    void SetUseSelectionRectangle (bool bUseSelectionRectangle) { m_bUseSelectionRectangle = bUseSelectionRectangle; }
    void ToggleUseSelectionRectangle () { m_bUseSelectionRectangle ^= true; }

    bool GetRecursiveMagnification () const { return m_bRecursiveMagnification; }
    void SetRecursiveMagnification (bool bRecursiveMagnification) { m_bRecursiveMagnification = bRecursiveMagnification; }
    void ToggleRecursiveMagnification () { m_bRecursiveMagnification ^= true; }

    void SetXZoomed (int iXZoomed) { m_nxZoomed = iXZoomed; }
    int  GetXZoomed () { return m_nxZoomed; }

    void SetYZoomed (int iYZoomed) { m_nyZoomed = iYZoomed; }
    int  GetYZoomed () { return m_nyZoomed; }

    void SetZoomPoint (CPoint ptZoom) { m_ptZoom = ptZoom; }
    CPoint GetZoomPoint () { return m_ptZoom; }

    void SetIsIconic (bool bIsIconic) { m_bIsIconic = bIsIconic; }

protected:
    void   ChangeZoomFactor (bool bIncrement, int bDeltaFactor);

    CPoint m_pt;
    CSize  m_size;
    int    m_nZoom;
    bool   m_bAutoRefresh;
    int    m_nRefreshInterval;
    bool   m_bGrid;
    bool   m_bNegativeGrid;
    bool   m_bAbsoluteNumbering;
    bool   m_bMinimizeToTray;
    bool   m_bAlwaysOnTop;
    bool   m_bStartMinimized;
    bool   m_bUseSelectionRectangle;
    bool   m_bRecursiveMagnification;
    int    m_nxZoomed;   // Client width in zoomed pixels
    int    m_nyZoomed;   // Client height in zoomed pixels
    CPoint m_ptZoom;
    bool  m_bIsIconic;
};
