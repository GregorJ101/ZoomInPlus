#include "stdafx.h"
#include "ZoomIn++.h"
#include "MainFrm.h"
#include "HyperLink.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CZoomInApp, CWinApp)
    //{{AFX_MSG_MAP(CZoomInApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//***********************************************
CZoomInApp::CZoomInApp () { }

// The one and only CZoomInApp object
CZoomInApp theApp;

//***********************************************
BOOL CZoomInApp::InitInstance ()
{
#ifdef _AFXDLL
    //Enable3dControls ();			// Call this when using MFC in a shared DLL
#else
    Enable3dControlsStatic ();	// Call this when linking to MFC statically
#endif

    m_pMainWnd = new CMainFrame ();
    DWORD dwStyle = SW_SHOW;
    LRESULT lres = m_pMainWnd->SendMessage (WM_USER_GET_START_MINIMIZED);
    if (lres != FALSE)
    {
        dwStyle |= SW_MINIMIZE;
    }
    m_pMainWnd->ShowWindow (dwStyle);
    m_pMainWnd->UpdateWindow ();

    return TRUE;
}

//***********************************************
class CAboutDlg : public CDialog
{
public:
    CAboutDlg ();

// Dialog Data
    //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_ABOUTBOX };
    CHyperLink	m_ctrlHyperLink;
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CAboutDlg)
    protected:
    virtual void DoDataExchange (CDataExchange *pDX);
    virtual BOOL OnInitDialog ();
    //}}AFX_VIRTUAL

// Implementation
protected:
    //{{AFX_MSG(CAboutDlg)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    CTime ConvertFrom__DATE__ ();
    CString GetBuildTimeStamp (const CTime& ctDate);
public:
};

//***********************************************
CAboutDlg::CAboutDlg () : CDialog (CAboutDlg::IDD)
{
    //{{AFX_DATA_INIT(CAboutDlg)
    //}}AFX_DATA_INIT
}

//***********************************************
BOOL CAboutDlg::OnInitDialog ()
{
    CDialog::OnInitDialog ();

    GetDlgItem (IDC_STATIC_BUILD)->SetWindowText (GetBuildTimeStamp (ConvertFrom__DATE__ ()));

    return TRUE;
}

//***********************************************
void CAboutDlg::DoDataExchange (CDataExchange *pDX)
{
    CDialog::DoDataExchange (pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    DDX_Control (pDX, IDC_STATIC_URL, m_ctrlHyperLink);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//***********************************************
void CZoomInApp::OnAppAbout ()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal ();
}

//***********************************************
CTime CAboutDlg::ConvertFrom__DATE__ ()
{
    const char *szaMonths[] = {"jan", "feb", "mar", "apr", "may", "jun",
                               "jul", "aug", "sep", "oct", "nov", "dec"};

    CString cstrDateString (__DATE__);
    int iFirstSpaceIdx = cstrDateString.Find (' ');
    int iSecondSpaceIdx = -1;
    if (iFirstSpaceIdx + 1 < cstrDateString.GetLength ())
    {
        iSecondSpaceIdx = cstrDateString.Find (' ', iFirstSpaceIdx + 2);
    }
    if (iFirstSpaceIdx  > 0              &&
        iSecondSpaceIdx > iFirstSpaceIdx &&
        iSecondSpaceIdx < cstrDateString.GetLength () + 4)
    {
        CString cstrYear  (cstrDateString.Mid (iSecondSpaceIdx + 1));
        CString cstrDay   (cstrDateString.Mid (iFirstSpaceIdx + 1, iSecondSpaceIdx - iFirstSpaceIdx - 1));
        CString cstrMonth (cstrDateString.Left (iFirstSpaceIdx));
        CString cstrMonthLower (cstrMonth);
        cstrMonthLower.MakeLower ();

        int iYear  = atoi (cstrDateString.Mid (iSecondSpaceIdx + 1));
        int iDay   = atoi (cstrDateString.Mid (iFirstSpaceIdx + 1, iSecondSpaceIdx - iFirstSpaceIdx));
        int iMonth = -1;

        for (int iIdx = 0; iIdx < sizeof (szaMonths); ++iIdx)
        {
            if (_strnicmp (cstrMonthLower, szaMonths[iIdx], cstrMonthLower.GetLength ()) == 0)
            {
                iMonth = iIdx + 1;
                break;
            }
        }

        return CTime (iYear, iMonth, iDay, 0, 0, 0);
    }

    return CTime ();
}

//***********************************************
CString CAboutDlg::GetBuildTimeStamp (const CTime& ctDate)
{
    const char *szaWeekDays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    CString cstrTimeStamp;
    CString cstrWeekDay;
    int iDayOfWeek = ctDate.GetDayOfWeek ();
    int iYear      = ctDate.GetYear ();
    int iMonth     = ctDate.GetMonth ();
    int iDay       = ctDate.GetDay ();

    if (iDayOfWeek > 0 &&
        iDayOfWeek <= 7)
    {
        cstrWeekDay = szaWeekDays[iDayOfWeek - 1];
    }

    cstrTimeStamp.Format ("Built: %s %d-%02d-%02d %s", cstrWeekDay, iYear, iMonth, iDay, __TIME__);
    return cstrTimeStamp;
}
