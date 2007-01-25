//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef SIPX_VIDEO // [

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/video/MpvoGdi.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpvoGdi::MpvoGdi(HWND hWnd)
: mHwnd(hWnd)
{
   initOffscreenBuffer();
}

MpvoGdi::~MpvoGdi()
{
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpvoGdi::render(MpVideoBufPtr pFrame)
{
   if (!pFrame.isValid())
      return OS_FAILED;

   if (mHwnd == NULL)
      return OS_FAILED;

   BITMAPINFOHEADER *p_header = &mBitmapInfo.bmiHeader;
   HDC hdc;
   HDC off_dc;
   HBITMAP  off_bitmap;
   void *pDibBuffer;

   // Get DC for video output window
   hdc = GetDC( mHwnd );
   assert(hdc != NULL);

   // Create DC for drawing picture
   off_dc = CreateCompatibleDC( hdc );
   assert(off_dc != NULL);

   // Fill in bitmap information that could change from frame to frame.
   p_header->biWidth = pFrame->getFrameWidth();
   p_header->biHeight = pFrame->getFrameHeight();

   off_bitmap = CreateDIBSection( hdc, (BITMAPINFO *)p_header, DIB_RGB_COLORS,
                                  (void**)&pDibBuffer, NULL, 0 );
 
   // Convert frame to screen color format and copy it to windows bitmap
   switch (p_header->biBitCount)
   {
   case 32:
      pFrame->convertToColorSpace(MpVideoBuf::MP_COLORSPACE_BGR32,
                                  (char*)pDibBuffer,
                                  p_header->biWidth*p_header->biHeight*32/8);
      break;
   case 24:
      pFrame->convertToColorSpace(MpVideoBuf::MP_COLORSPACE_BGR24,
                                  (char*)pDibBuffer,
                                  p_header->biWidth*p_header->biHeight*24/8);
      break;
   case 16:
      pFrame->convertToColorSpace(MpVideoBuf::MP_COLORSPACE_BGR565,
                                  (char*)pDibBuffer,
                                  p_header->biWidth*p_header->biHeight*16/8);
      break;
   case 15:
      pFrame->convertToColorSpace(MpVideoBuf::MP_COLORSPACE_BGR555,
                                  (char*)pDibBuffer,
                                  p_header->biWidth*p_header->biHeight*16/8);
      break;
   default:
      osPrintf( "screen depth %i not supported", p_header->biBitCount );
      return OS_FAILED;
      break;
   }

   SelectObject( off_dc, off_bitmap );

   StretchBlt( hdc, 0, 0,
               p_header->biWidth, p_header->biHeight,
               off_dc, 0, p_header->biHeight,
               p_header->biWidth, -p_header->biHeight, SRCCOPY );

   ReleaseDC( mHwnd, hdc );

   DeleteDC( off_dc );
   DeleteObject( off_bitmap );

   return OS_SUCCESS;
}

OsStatus MpvoGdi::setWindow(HWND hwnd)
{
   mHwnd = hwnd;
   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

OsStatus MpvoGdi::initOffscreenBuffer()
{
   BITMAPINFOHEADER *p_header = &mBitmapInfo.bmiHeader;
   BITMAPINFO *p_info = &mBitmapInfo;
   HDC window_dc = GetDC( mHwnd );
   int  i_depth;

   // Get screen properties
   i_depth = GetDeviceCaps( window_dc, PLANES ) *
             GetDeviceCaps( window_dc, BITSPIXEL );
   osPrintf( "GDI depth is %i", i_depth );

   // Initialize offscreen bitmap
   memset( p_info, 0, sizeof( BITMAPINFO ) + 3 * sizeof( RGBQUAD ) );

   p_header->biSize = sizeof( BITMAPINFOHEADER );
   p_header->biSizeImage = 0;
   p_header->biPlanes = 1;
   switch( i_depth )
   {
   case 15:
      p_header->biBitCount = 15;
      p_header->biCompression = BI_BITFIELDS;
      ((DWORD*)p_info->bmiColors)[0] = 0x00007c00;
      ((DWORD*)p_info->bmiColors)[1] = 0x000003e0;
      ((DWORD*)p_info->bmiColors)[2] = 0x0000001f;
      break;
   case 16:
      p_header->biBitCount = 16;
      p_header->biCompression = BI_BITFIELDS;
      ((DWORD*)p_info->bmiColors)[0] = 0x0000f800;
      ((DWORD*)p_info->bmiColors)[1] = 0x000007e0;
      ((DWORD*)p_info->bmiColors)[2] = 0x0000001f;
      break;
   case 24:
      p_header->biBitCount = 24;
      p_header->biCompression = BI_RGB;
      ((DWORD*)p_info->bmiColors)[0] = 0x00ff0000;
      ((DWORD*)p_info->bmiColors)[1] = 0x0000ff00;
      ((DWORD*)p_info->bmiColors)[2] = 0x000000ff;
      break;
   case 32:
      p_header->biBitCount = 32;
      p_header->biCompression = BI_RGB;
      ((DWORD*)p_info->bmiColors)[0] = 0x00ff0000;
      ((DWORD*)p_info->bmiColors)[1] = 0x0000ff00;
      ((DWORD*)p_info->bmiColors)[2] = 0x000000ff;
      break;
   default:
      osPrintf( "screen depth %i not supported", i_depth );
      return OS_FAILED;
      break;
   }
   p_header->biWidth = 0;
   p_header->biHeight = 0;
   p_header->biClrImportant = 0;
   p_header->biClrUsed = 0;
   p_header->biXPelsPerMeter = 0;
   p_header->biYPelsPerMeter = 0;

   ReleaseDC( 0, window_dc );

   return OS_SUCCESS;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

#endif // SIPX_VIDEO ]
