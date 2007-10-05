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
#include <VideoSupport/VideoProcessorFactory.h>
#include <VideoSupport/VideoFormat.h>
#include <VideoSupport/VideoScaler.h>
#include <VideoSupport/VideoSurfaceConverter.h>
#include <VideoSupport/VideoSurfaceConverterFactory.h>

#define WIDTHBYTES(bits) ((DWORD)(((bits)+31) & (~31)) / 8)
#define DIBWIDTHBYTES(bi) (DWORD)WIDTHBYTES((DWORD)(bi).biWidth * (DWORD)(bi).biBitCount)
#define _DIBSIZE(bi) (DIBWIDTHBYTES(bi) * (DWORD)(bi).biHeight)
#define DIBSIZE(bi) ((bi).biHeight < 0 ? (-1)*(_DIBSIZE(bi)) : _DIBSIZE(bi))


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpvoGdi::MpvoGdi(HWND hWnd)
: mHwnd(hWnd)
// bitmap 
, mBitmap(NULL)
, mpBitmapBuffer(NULL)
, mBmpWidth(0)
, mBmpHeight(0)
// scaler
, mSclInHeight(0)
, mSclInWidth(0)
, mSclInSize(0)
, mSclOutHeight(0)
, mSclOutWidth(0)
, mSclOutSize(0)
, mSclFormat(0)
, mpSclBuffer(NULL)
, mpScaler(NULL)
// surface converter
, mScvHeight(0)
, mScvWidth(0)
, mScvInSize(0)
, mScvOutFormat(0)
, mScvInFormat(0)
, mScvOutSize(0)
, mpSurfaceConverter(NULL)
, mHighQualityScaling(false)
{
   initOffscreenBuffer();
}

MpvoGdi::~MpvoGdi()
{
   if (NULL != mBitmap)
   {
      ::DeleteObject(mBitmap);
      mBitmap = NULL;
      mpBitmapBuffer = NULL;
   }

   if (NULL != mpSclBuffer)
   {
      mpSclBuffer = realloc(mpSclBuffer, 0);
   }

   delete mpScaler;
   mpScaler = NULL;

   delete mpSurfaceConverter;
   mpSurfaceConverter = NULL;
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpvoGdi::render(MpVideoBufPtr pFrame)
{
   if (!pFrame.isValid())
      return OS_FAILED;

   if (mHwnd == NULL)
      return OS_FAILED;

   RECT dstRc;
   if (!::GetClientRect(mHwnd, &dstRc))
      return OS_FAILED;

   const int dspW = dstRc.right;
   const int dspH = dstRc.bottom;

   const int srcW = pFrame->getFrameWidth();
   const int srcH = pFrame->getFrameHeight();

   // performance hack: if display window is BIG (like fullscreen),
   // use high-quality resampling to make the video integral sub-multiply 
   // of the window size, then resize with low-quality Windows scaling.
   // in high resolutions software scaling is a CPU hog.
   int dstW = dstRc.right;
   int dstH = dstRc.bottom;

   if (!mHighQualityScaling)
   {
      // don't lower the scaling quality if size below the thresholds 
      static const int xThreshold = VideoFormat::width_4CIF;
      static const int yThreshold = VideoFormat::height_4CIF;

      int xRatio = dstW / srcW;
      if (dstW > xThreshold && xRatio >= 2)
      {
         // scale at most x4 with low-quality scaling - prevent
         // very large pixel blocks to appear (esp. with SQCIF)
         if (xRatio > 4)
            xRatio = 4;

         dstW /= xRatio;
         if (0 != (dstW % 2))
            --dstW;
      }

      int yRatio = dstH / srcH;
      if (dstH > yThreshold && yRatio >= 2)
      {
         if (yRatio > 4)
            yRatio = 4;

         dstH /= yRatio;
         if (0 != (dstH % 2))
            --dstH;
      }
   }

   const MpVideoBuf::ColorSpace inColorSpace = pFrame->getColorspace();

   const void* scvInBuffer = NULL;
   if (dstW != srcW || dstH != srcH)
   {
      void* sclBuffer = getScalerBuffer(inColorSpace, srcW, srcH, dstW, dstH);
      if (NULL == sclBuffer)
         return OS_FAILED;

      assert(NULL != mpScaler);
      mpScaler->Process(pFrame->getDataPtr(), mSclInSize, sclBuffer, mSclOutSize);
      scvInBuffer = sclBuffer;

   }
   else
   {
      scvInBuffer = pFrame->getDataPtr();
   }

   BITMAPINFOHEADER *p_header = &mBitmapInfo.bmiHeader;
   HDC hdc;
   HDC off_dc;

   // Get DC for video output window
   hdc = ::GetDC(mHwnd);
   assert(hdc != NULL);

   // Create DC for drawing picture
   off_dc = ::CreateCompatibleDC(hdc);
   assert(off_dc != NULL);
   ::GdiFlush();

   MpVideoBuf::ColorSpace outColorSpace;
   // Convert frame to screen color format and copy it to windows bitmap
   switch (p_header->biBitCount)
   {
   case 32:
      outColorSpace = MpVideoBuf::MP_COLORSPACE_BGR32;
      break;
   case 24:
      outColorSpace = MpVideoBuf::MP_COLORSPACE_BGR24;
      break;
   case 16:
      outColorSpace = MpVideoBuf::MP_COLORSPACE_BGR565;
      break;
   case 15:
      outColorSpace = MpVideoBuf::MP_COLORSPACE_BGR555;
      break;
   default:
      osPrintf( "screen depth %i not supported", p_header->biBitCount );
      return OS_FAILED;
      break;
   }

   void* scvOutBuffer = getBitmapBuffer(dstW, dstH);
   if (NULL == scvOutBuffer)
      return OS_FAILED;

   VideoSurfaceConverter* scv = getSurfaceConverter(inColorSpace, outColorSpace, dstW, dstH);
   if (NULL == scv)
      return OS_FAILED;

   scv->Convert(scvInBuffer, mScvInSize, scvOutBuffer, mScvOutSize);

   HGDIOBJ oldBitmap = ::SelectObject(off_dc, mBitmap);
   if (p_header->biHeight > 0)
      ::StretchBlt(hdc, 0, 0, dspW, dspH, off_dc, 0, dstH, dstW, -dstH, SRCCOPY);
   else
   {
      if (dstW == dspW && dstH == dspH)
         ::BitBlt(hdc, 0, 0, dstW, dstH, off_dc, 0, 0, SRCCOPY);
      else
         ::StretchBlt(hdc, 0, 0, dspW, dspH, off_dc, 0, 0, dstW, dstH, SRCCOPY);
   }

   ::SelectObject(off_dc, oldBitmap);

   ::DeleteDC(off_dc);
   ::ReleaseDC(mHwnd, hdc);
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
   osPrintf( "GDI depth is %i\n", i_depth );

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


void* MpvoGdi::getBitmapBuffer(const int width, const int height)
{
   if (NULL != mpBitmapBuffer && width == mBmpWidth && height == mBmpHeight)
      return mpBitmapBuffer;

   if (NULL != mpBitmapBuffer)
   {
      assert(NULL != mBitmap);
      ::DeleteObject(mBitmap);
      mBitmap = NULL;
      mpBitmapBuffer = NULL;
   }

   if (NULL == mHwnd)
      return NULL;

   BITMAPINFOHEADER *p_header = &mBitmapInfo.bmiHeader;
   HDC dc = NULL, offDc = NULL;
   
   if (NULL == (dc = ::GetDC(mHwnd)))
      goto Finish;

   p_header->biWidth = width;
   // we certainly prefer to have a top-down DIB to bottom-up one. this saves us 
   // flipping the image during StretchBlt
   p_header->biHeight = -height;
   p_header->biSizeImage = DIBSIZE(*p_header);

   mBitmap = ::CreateDIBSection(dc, (BITMAPINFO*)p_header, DIB_RGB_COLORS,
                                  &mpBitmapBuffer, NULL, 0);
   if (NULL == mBitmap)
      goto Finish;

   assert(mpBitmapBuffer != NULL);
   mBmpWidth = width;
   mBmpHeight = height;

Finish:
   if (NULL != dc)
      ::ReleaseDC(mHwnd, dc);

   return mpBitmapBuffer;
}

void* MpvoGdi::getScalerBuffer(int format, int srcW, int srcH, int dstW, int dstH)
{
   if (mSclFormat == format && 
       srcW == mSclInWidth && 
       srcH == mSclInHeight &&
       dstW == mSclOutWidth &&
       dstH == mSclOutHeight
      )
   {
      assert(NULL != mpScaler);
      assert(NULL != mpSclBuffer);
      return mpSclBuffer;
   }

   mSclInWidth = 0;
   mSclInHeight = 0;
   mSclOutWidth = 0;
   mSclOutHeight = 0;
   delete mpScaler;
   mpScaler = NULL;

   PixelFormat fmt = (PixelFormat)MpVideoBuf::getFFMpegColorspace((MpVideoBuf::ColorSpace)format);
   VideoSurface surface = PixelFormatToVideoSurface(fmt);
   assert(videoSurfaceUnknown != surface);
   if (videoSurfaceUnknown == surface)
      return NULL;

   VideoProcessorFactory* factory = VideoProcessorFactory::GetInstance();
   if (NULL == factory)
      return NULL;

   VideoFrameProcessorAutoPtr ptr = factory->CreateProcessor(videoScaler, surface);
   if (NULL == ptr.get())
      return NULL;

   mpScaler = dynamic_cast<VideoScaler*>(ptr.get());
   assert(NULL != mpScaler);

   if (mpScaler->Initialize(surface, srcW, srcH, dstW, dstH))
      ptr.release();
   else
   {
      mpScaler = NULL;
      return NULL;
   }

   mSclInSize = GetVideoFrameByteSize(surface, srcW, srcH);
   mSclOutSize = GetVideoFrameByteSize(surface, dstW, dstH);

   assert(0 != mSclOutSize);
   mpSclBuffer = realloc(mpSclBuffer, mSclOutSize);
   if (NULL == mpSclBuffer)
      return NULL;

   mSclInWidth = srcW;
   mSclInHeight = srcH;
   mSclOutWidth = dstW;
   mSclOutHeight = dstH;
   mSclFormat = format;
   return mpSclBuffer;
}

VideoSurfaceConverter* MpvoGdi::getSurfaceConverter(int inFormat, int outFormat, int w, int h)
{
   if (mScvInFormat == inFormat &&
       mScvOutFormat == outFormat &&
       mScvWidth == w &&
       mScvHeight == h
      )
   {
      assert(NULL != mpSurfaceConverter);
      return mpSurfaceConverter;
   }

   mScvInFormat = 0;
   mScvOutFormat = 0;
   mScvWidth = 0;
   mScvHeight = 0;
   delete mpSurfaceConverter;
   mpSurfaceConverter = NULL;

   PixelFormat inFmt = (PixelFormat)MpVideoBuf::getFFMpegColorspace((MpVideoBuf::ColorSpace)inFormat);
   VideoSurface inSf = PixelFormatToVideoSurface(inFmt);
   if (!IsVideoSurfaceValid(inSf))
      return NULL;

   PixelFormat outFmt = (PixelFormat)MpVideoBuf::getFFMpegColorspace((MpVideoBuf::ColorSpace)outFormat);
   VideoSurface outSf = PixelFormatToVideoSurface(outFmt);
   if (!IsVideoSurfaceValid(outSf))
      return NULL;

   VideoSurfaceConverterFactory* factory = VideoSurfaceConverterFactory::GetInstance();
   if (NULL == factory)
      return NULL;

   VideoSurfaceConverterAutoPtr ptr = factory->CreateConverter(w, h, inSf, outSf);
   if (NULL == ptr.get())
      return NULL;

   mpSurfaceConverter = ptr.release();
   mScvInFormat = inFormat;
   mScvOutFormat = outFormat;
   mScvWidth = w;
   mScvHeight = h;
   mScvInSize = GetVideoFrameByteSize(inSf, w, h);
   mScvOutSize = GetVideoFrameByteSize(outSf, w, h);
   return mpSurfaceConverter;
}

#endif // SIPX_VIDEO ]
