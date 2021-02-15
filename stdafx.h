#pragma once

// Exclude rarely-used stuff from Windows headers
#define VC_EXTRALEAN

#include <afxwin.h>		// MFC core and standard components
#include <afxext.h>		// MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>		// MFC support for Windows Common Controls
#endif

typedef const BYTE *PCBYTE;
typedef LPCTSTR PCTSTR;

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

//#include <GdiPlusGraphics.h>

//{{AFX_INSERT_LOCATION}}
