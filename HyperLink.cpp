#include "stdafx.h"
#include "HyperLink.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//***********************************************
CHyperLink::CHyperLink ()
{
    char szUserName[MAX_PATH];
    char szComputerName[MAX_PATH];
    DWORD dwUserBufferSize = MAX_PATH;
    DWORD dwComputerBufferSize = MAX_PATH;
    bool bUseRudeHand = false;

    if (GetUserName     ((LPSTR)&szUserName,     &dwUserBufferSize) &&
        GetComputerName ((LPSTR)&szComputerName, &dwComputerBufferSize))
    {
        if (strcmp (szUserName,      "Gregor")   == 0 &&
            (_stricmp (szComputerName, "JukeBox")  == 0  ||
             _stricmp (szComputerName, "PhotoBox") == 0))
        {
            m_hCursorHand = (HCURSOR)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_RUDE_HYPERLINK), IMAGE_CURSOR, GetSystemMetrics (SM_CXCURSOR), GetSystemMetrics (SM_CYCURSOR), 0);
        }
        else
        {
            m_hCursorHand = (HCURSOR)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_HYPERLINK), IMAGE_CURSOR, GetSystemMetrics (SM_CXCURSOR), GetSystemMetrics (SM_CYCURSOR), 0);
        }
    }
}

//***********************************************
CHyperLink::~CHyperLink ()
{
    if (m_hCursorHand != NULL)
        DestroyCursor (m_hCursorHand);
}

BEGIN_MESSAGE_MAP(CHyperLink, CStatic)
    //{{AFX_MSG_MAP(CHyperLink)
    ON_WM_CTLCOLOR_REFLECT()
    ON_WM_NCHITTEST()
    ON_WM_SETCURSOR()
    ON_WM_LBUTTONDOWN()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//***********************************************
HBRUSH CHyperLink::CtlColor (CDC *pDC, UINT nCtlColor)
{
    _ASSERTE(nCtlColor == CTLCOLOR_STATIC);
    DWORD dwStyle = GetStyle ();

    HBRUSH hbr = NULL;
    if ((dwStyle & 0xFF) <= SS_RIGHT)
    {	// This is a text control: set up font and colors
        if (!(HFONT)m_font)
        {	// First time init: create font
            LOGFONT lf;
            GetFont ()->GetObject (sizeof(lf), &lf);
            lf.lfUnderline = TRUE;
            m_font.CreateFontIndirect (&lf);
        }

        // Use underline font and visited/unvisited colors
        pDC->SelectObject (&m_font);
        pDC->SetTextColor (0xFF0000);
        pDC->SetBkMode (TRANSPARENT);

        // Return hollow brush to preserve parent background color
        hbr = (HBRUSH)::GetStockObject (HOLLOW_BRUSH);
    }
    return hbr;
}

//***********************************************
LRESULT CHyperLink::OnNcHitTest (CPoint point)
{
    return HTCLIENT;
}

//***********************************************
BOOL CHyperLink::OnSetCursor (CWnd *pWnd, UINT nHitTest, UINT message)
{
    _ASSERTE(m_hCursorHand);

    ::SetCursor (m_hCursorHand);
    return TRUE;
}

//***********************************************
void CHyperLink::OnLButtonDown (UINT nFlags, CPoint point)
{
    CString strLink;
    GetWindowText (strLink); _ASSERTE(!strLink.IsEmpty ());

    // Attempt to spawn the web browser through ShellExecute
    if ((DWORD)ShellExecute (0, _T("open"), strLink, 0, 0, SW_SHOWNORMAL) <= 32)
    {	// The function failed, instead copy the URL to the clipboard
        if (OpenClipboard ())
        {
            if (EmptyClipboard ())
            {
                int nSize = (strLink.GetLength () + 1) * sizeof(TCHAR);	// The extra char is for the null term
                HGLOBAL hMem = GlobalAlloc (GMEM_MOVEABLE | GMEM_DDESHARE, nSize);
                PTSTR szData = (PTSTR)GlobalLock (hMem);
                lstrcpy (szData, strLink);
                GlobalUnlock (hMem);

                // Add to the clipboard
#if defined(UNICODE) || defined(_UNICODE)
                SetClipboardData (CF_UNICODETEXT, hMem);
#else
                SetClipboardData (CF_TEXT, hMem);
#endif
            }

            CloseClipboard ();
        }
    }

    CStatic::OnLButtonDown (nFlags, point);
}
