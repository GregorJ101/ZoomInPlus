#include "stdafx.h"
#include "DibApi.h"

#define IS_WIN30_DIB(pbi)	((*(PDWORD)(pbi)) == sizeof(BITMAPINFOHEADER))
#define WIDTHBYTES(bits)	(((bits) + 31) / 32 * 4)

//***********************************************
bool CBitmapSave::Save (CDC *pDC, const CRect &rect, const CString &strFilename, const bool bForce24)
{
    CBitmap bitmap, *pOldBmp;
    CDC memDC;
    CPalette pal;
    DWORD dwDIB;

    // Create the bitmap and copy the image from the DC into it
    memDC.CreateCompatibleDC (pDC);
    bitmap.CreateCompatibleBitmap (pDC, rect.Width (), rect.Height ());
    pOldBmp = memDC.SelectObject (&bitmap);
    memDC.BitBlt (0, 0, rect.Width (), rect.Height (), pDC, 0, 0, SRCCOPY);
    memDC.SelectObject (pOldBmp);

    // Create logical palette if device support a palette
    if (pDC->GetDeviceCaps (RASTERCAPS) & RC_PALETTE)
    {
        unsigned int nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 256);
        LOGPALETTE *pLP = (LOGPALETTE*) new BYTE[nSize];
        pLP->palVersion = 0x300;
        pLP->palNumEntries = GetSystemPaletteEntries (*pDC, 0, 255, pLP->palPalEntry);

        // Create the palette
        pal.CreatePalette (pLP);

        delete[] pLP;
    }

    // Convert the bitmap to a DIB
    if ((dwDIB = DDBToDIB (bitmap, &pal, bForce24)) == NULL)
    {
        bitmap.DeleteObject ();
        return false;
    }

    // Write it to file
    WriteDIB (strFilename, dwDIB);

    // Free the memory allocated by DDBToDIB for the DIB
    VirtualFree ((PVOID)dwDIB, 0, MEM_RELEASE);

    bitmap.DeleteObject ();

    return true;
}

//***********************************************
DWORD CBitmapSave::DDBToDIB (CBitmap &bitmap, const CPalette *pPal, const bool bForce24)
{
    BOOL bSuccess;
    BITMAP bmp;
    BITMAPINFOHEADER bmpIHdr, *pbmpIHdr;
    DWORD dwDIB;
    DWORD dwLen;
    HPALETTE hPal, hPalOld;
    HDC hDC;

    // If a palette has not been supplied use defaul palette
    if ((hPal = (HPALETTE)pPal->GetSafeHandle ()) == NULL)
        hPal = (HPALETTE)GetStockObject (DEFAULT_PALETTE);

    // Get bitmap information
    bitmap.GetObject (sizeof(bmp), &bmp);

    // Initialize the BITMAPINFOHEADER
    ZeroMemory (&bmpIHdr, sizeof(BITMAPINFOHEADER));
    bmpIHdr.biSize        = sizeof(BITMAPINFOHEADER);
    bmpIHdr.biWidth       = bmp.bmWidth;
    bmpIHdr.biHeight      = bmp.bmHeight;
    bmpIHdr.biPlanes      = 1;
    bmpIHdr.biBitCount    = bmp.bmPlanes * bmp.bmBitsPixel;
    bmpIHdr.biCompression = BI_RGB;

    // Correct for 16/32 bit bitmaps, if the user wants us to
    if ((bmpIHdr.biBitCount == 16 || bmpIHdr.biBitCount == 32) && bForce24)
        bmpIHdr.biBitCount = 24;

    // Compute the size of BITMAPINFOHEADER and the color table
    dwLen = bmpIHdr.biSize + DIBNumColors (&bmpIHdr);

    // We need a device context to get the DIB from
    hDC = GetDC (NULL);
    hPalOld = SelectPalette (hDC, hPal, FALSE);
    RealizePalette (hDC);

    // Allocate enough memory to hold BITMAPINFOHEADER and color table
    if ((pbmpIHdr = (PBITMAPINFOHEADER) new BYTE[dwLen]) == NULL)
    {
        SelectPalette (hDC, hPalOld, FALSE);
        ReleaseDC (NULL, hDC);
        return NULL;
    }

    // Call GetDIBits with a NULL lpBits param, so the device driver
    // will calculate the biSizeImage field
    *pbmpIHdr = bmpIHdr;
    GetDIBits (hDC, (HBITMAP)bitmap.GetSafeHandle (), 0, bmpIHdr.biHeight, NULL, (PBITMAPINFO)pbmpIHdr, DIB_RGB_COLORS);
    bmpIHdr = *pbmpIHdr;

    // If the driver did not fill in the biSizeImage field, then compute it
    // Each scan line of the image is aligned on a DWORD (32bit) boundary
    if (bmpIHdr.biSizeImage == 0)
        bmpIHdr.biSizeImage = ((((bmpIHdr.biWidth * bmpIHdr.biBitCount) + 31) & ~31) / 8) * bmpIHdr.biHeight;

    // Alloc a buffer to hold all the bits
    delete[] pbmpIHdr;
    dwLen += bmpIHdr.biSizeImage;
    if ((dwDIB = (DWORD)VirtualAlloc (NULL, dwLen, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)) == NULL)
    {
        // Reselect the original palette
        SelectPalette (hDC, hPalOld, FALSE);
        ReleaseDC (NULL, hDC);
        return NULL;
    }

    // Get the bitmap bits
    pbmpIHdr = (PBITMAPINFOHEADER)dwDIB;
    *pbmpIHdr = bmpIHdr;

    // FINALLY get the DIB
    bSuccess = GetDIBits (hDC, (HBITMAP)bitmap.GetSafeHandle (), 0, bmpIHdr.biHeight, (PBYTE)pbmpIHdr + (bmpIHdr.biSize + DIBNumColors (pbmpIHdr)), (PBITMAPINFO)pbmpIHdr, DIB_RGB_COLORS);

    SelectPalette (hDC, hPalOld, FALSE);
    ReleaseDC (NULL, hDC);

    if (!bSuccess)
    {
        VirtualFree ((PVOID)dwDIB, 0, MEM_RELEASE);
        return NULL;
    }
    else
        return dwDIB;
}

//***********************************************
bool CBitmapSave::WriteDIB (const CString &strFilename, const DWORD dwDIB)
{
    _ASSERTE(dwDIB);

    BITMAPFILEHEADER bmpFHdr;
    PBITMAPINFOHEADER pbmpIHdr;
    DWORD dwDIBSize, dwBmBitsSize;

    int i = sizeof(BITMAPFILEHEADER);
    // Get a pointer to the DIB memory, the first of which contains
    // a BITMAPINFO structure
    pbmpIHdr = (PBITMAPINFOHEADER)dwDIB;

    if (!IS_WIN30_DIB(pbmpIHdr))
        return false;	// It's an other-style DIB (save not supported)

    // Fill in file type (first 2 bytes must be "BM" for a bitmap)
    ZeroMemory (&bmpFHdr, sizeof(BITMAPFILEHEADER));
    bmpFHdr.bfType = 0x4d42;	// 0x42 = "B" 0x4d = "M"

    // Calculating the size of the DIB is a bit tricky (if we want to
    // do it right).  The easiest way to do this is to call GlobalSize()
    // on our global handle, but since the size of our global memory may have
    // been padded a few bytes, we may end up writing out a few too
    // many bytes to the file (which may cause problems with some apps).
    //
    // So, instead let's calculate the size manually (if we can)
    //
    // First, find size of header plus size of color table.
    dwDIBSize = pbmpIHdr->biSize + DIBNumColors (pbmpIHdr);	// Partial Calculation

    // Now calculate the size of the image; Width (DWORD aligned) * Height
    dwBmBitsSize = WIDTHBYTES((pbmpIHdr->biWidth) * ((DWORD)pbmpIHdr->biBitCount)) * pbmpIHdr->biHeight;
    dwDIBSize += dwBmBitsSize;

    // Now, since we have calculated the correct size, why don't we
    // fill in the biSizeImage field (this will fix any .BMP files which
    // have this field incorrect).
    pbmpIHdr->biSizeImage = dwBmBitsSize;

    // Calculate the file size by adding the DIB size to sizeof(BITMAPFILEHEADER)
    bmpFHdr.bfSize = dwDIBSize + sizeof(BITMAPFILEHEADER);

    // Now, calculate the offset the actual bitmap bits will be in
    // the file -- It's the Bitmap file header plus the DIB header,
    // plus the size of the color table.
    bmpFHdr.bfOffBits = sizeof(BITMAPFILEHEADER) + pbmpIHdr->biSize + DIBNumColors (pbmpIHdr);

    // Save the bitmap
    // Open the file
    HANDLE hFile = CreateFile (strFilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // Write the file
    DWORD dwBytesWritten;
    WriteFile (hFile, &bmpFHdr, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile (hFile, pbmpIHdr, dwDIBSize, &dwBytesWritten, NULL);

    CloseHandle (hFile);

    return true;
}

//***********************************************
WORD CBitmapSave::DIBNumColors (const PBITMAPINFOHEADER pbmpIHdr)
{
    if (pbmpIHdr->biClrUsed != 0)
        return (WORD)pbmpIHdr->biClrUsed;

    switch (pbmpIHdr->biBitCount)
    {
    case 1:
        return (2 * sizeof(RGBQUAD));
    case 4:
        return (16 * sizeof(RGBQUAD));
    case 8:
        return (256 * sizeof(RGBQUAD));
    case 16:
    case 24:
    case 32:
    default:
        return 0;
    }
}
