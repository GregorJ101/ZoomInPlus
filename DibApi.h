#pragma once

class CBitmapSave
{
public:
    CBitmapSave () { }
    virtual ~CBitmapSave () { }

    static bool Save (CDC *pDC, const CRect &rect, const CString &strFilename, const bool bForce24 = true);

protected:
    static DWORD DDBToDIB (CBitmap &bitmap, const CPalette *pPal, const bool force24);
    static bool WriteDIB (const CString &strFilename, const DWORD dwDIB);
    static WORD DIBNumColors (const PBITMAPINFOHEADER pbmpIHdr);
};
