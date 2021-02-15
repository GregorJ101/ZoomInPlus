#include "stdafx.h"
#include "ZoomIn++.h"
#include "RefreshRateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//***********************************************
void DialogEnableWindow (CDialog *pDlg, CWnd *pWnd, const bool bEnable)
{
    _ASSERTE(pDlg);
    _ASSERTE(pWnd);

    // If we're disabling a control with focus, move focus to the next control first
    if (!bEnable && pWnd == CWnd::GetFocus ())
        pDlg->NextDlgCtrl ();

    // Disable the control
    pWnd->EnableWindow (bEnable);

    // If we enabled a default button, reset it as the default button (this forces the UI to repaint the darker frame)
    LRESULT n = pDlg->GetDefID ();
    if (bEnable && HIWORD(n) && LOWORD(n) == LOWORD(pWnd->GetDlgCtrlID ()))
        pDlg->SetDefID (LOWORD(n));
}

//***********************************************
CRefreshRateDlg::CRefreshRateDlg (CWnd *pParent)
    : CDialog(CRefreshRateDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CRefreshRateDlg)
    m_bEnableAutoRefresh = FALSE;
    m_nRefreshInterval = 6;
    //}}AFX_DATA_INIT
}

//***********************************************
void CRefreshRateDlg::DoDataExchange (CDataExchange *pDX)
{
    CDialog::DoDataExchange (pDX);
    //{{AFX_DATA_MAP(CRefreshRateDlg)
    DDX_Check (pDX, IDC_CHECK_AUTO_REFRESH, m_bEnableAutoRefresh);
    DDX_Text (pDX, IDC_EDIT_REFRESH_RATE, m_nRefreshInterval);
    DDV_MinMaxInt (pDX, m_nRefreshInterval, 1, 50);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRefreshRateDlg, CDialog)
    //{{AFX_MSG_MAP(CRefreshRateDlg)
    ON_BN_CLICKED(IDC_CHECK_AUTO_REFRESH, OnAutoRefresh)
    ON_EN_KILLFOCUS(IDC_EDIT_REFRESH_RATE, OnKillFocusRefreshRate)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//***********************************************
BOOL CRefreshRateDlg::OnInitDialog ()
{
    CDialog::OnInitDialog ();

    ((CSpinButtonCtrl*)GetDlgItem (IDC_SPIN_REFRESH_RATE))->SetRange (1, 50);

    OnAutoRefresh ();

    return TRUE;
}

//***********************************************
void CRefreshRateDlg::OnAutoRefresh ()
{
    UpdateData ();

    DialogEnableWindow (this, GetDlgItem (IDC_STATIC_MSG), (m_bEnableAutoRefresh == TRUE));
    DialogEnableWindow (this, GetDlgItem (IDC_EDIT_REFRESH_RATE), (m_bEnableAutoRefresh == TRUE));
    DialogEnableWindow (this, GetDlgItem (IDC_SPIN_REFRESH_RATE), (m_bEnableAutoRefresh == TRUE));
}

//***********************************************
void CRefreshRateDlg::OnKillFocusRefreshRate ()
{
    UpdateData ();

    if (m_nRefreshInterval < 1)
        SetDlgItemText (IDC_EDIT_REFRESH_RATE, _T("1"));
    else if (m_nRefreshInterval > 50)
        SetDlgItemText (IDC_EDIT_REFRESH_RATE, _T("50"));
}
