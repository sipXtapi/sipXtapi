//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpVideoBuf.h"
#ifdef DO_COLORSPACE_CONVERSION // [
#  ifdef WIN32 // [
#     define EMULATE_INTTYPES
#  endif // WIN32 ]

extern "C" {
#  include <avcodec.h>
}

#endif // SIPX_VIDEO ]

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
MpBufPool *MpVideoBuf::smpDefaultPool = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ UTLILITES ================================== */

unsigned MpVideoBuf::getColospacePixelDepth(ColorSpace colorspace)
{
   switch(colorspace) {
   case MP_COLORSPACE_RGB32:
   case MP_COLORSPACE_BGR32:
      return 32;

   case MP_COLORSPACE_RGB24:
   case MP_COLORSPACE_BGR24:
/*
   case MP_COLORSPACE_YUV:
   case MP_COLORSPACE_RGB24p:
   case MP_COLORSPACE_BGR24p:
   case MP_COLORSPACE_YUVp:
*/
      return 24;

   case MP_COLORSPACE_RGB555:   // We assume one pixel unused in this mode
   case MP_COLORSPACE_RGB565:
   case MP_COLORSPACE_BGR555:   // We assume one pixel unused in this mode
   case MP_COLORSPACE_BGR565:
/*   case MP_COLORSPACE_YUV422:
   case MP_COLORSPACE_YUV422p:
*/
      return 16;

/*
   case MP_COLORSPACE_YUV411:
   case MP_COLORSPACE_YUV411p:
   case MP_COLORSPACE_YUV420:
*/
   case MP_COLORSPACE_YUV420p:
      return 12;

/*
   case MP_COLORSPACE_GRAY:
      return 8;
*/
   }

   // Unknown colorspace. Something wrong...
   assert(false);
   return 0;
}

unsigned MpVideoBuf::getColospaceNumPlanes(ColorSpace colorspace)
{
   if (isPlanar(colorspace)) {
      // Current version support only three planes in planar mode.
      // We does not support neither YUV colorspaces with UV packed into one
      // plane, nor RGBA colorspace with four planes.
      return 3;
   } else {
      // Interleaved or grayscale
      return 1;
   }
}

unsigned MpVideoBuf::getColospaceUVWidthDivider(ColorSpace colorspace)
{
   switch(colorspace) {
   case MP_COLORSPACE_RGB24:
   case MP_COLORSPACE_RGB32:
   case MP_COLORSPACE_BGR24:
   case MP_COLORSPACE_BGR32:
   case MP_COLORSPACE_RGB555:
   case MP_COLORSPACE_RGB565:
   case MP_COLORSPACE_BGR555:
   case MP_COLORSPACE_BGR565:
/*
   case MP_COLORSPACE_GRAY:
   case MP_COLORSPACE_RGB24p:
   case MP_COLORSPACE_BGR24p:
*/
      return 1;

/*
   case MP_COLORSPACE_YUV:
   case MP_COLORSPACE_YUVp:
      return 1;
*/

/*
   case MP_COLORSPACE_YUV420:
   case MP_COLORSPACE_YUV422:
   case MP_COLORSPACE_YUV422p:
*/
   case MP_COLORSPACE_YUV420p:
      return 2;

/*
   case MP_COLORSPACE_YUV411:
   case MP_COLORSPACE_YUV411p:
      return 4;
*/
   }

   // Unknown colorspace. Something wrong...
   assert(false);
   return 1;
}

unsigned MpVideoBuf::getColospaceUVHeightDivider(ColorSpace colorspace)
{
   switch(colorspace) {
   case MP_COLORSPACE_RGB555:
   case MP_COLORSPACE_RGB565:
   case MP_COLORSPACE_BGR555:
   case MP_COLORSPACE_BGR565:
/*
   case MP_COLORSPACE_GRAY:
   case MP_COLORSPACE_RGB24p:
   case MP_COLORSPACE_BGR24p:
*/
   case MP_COLORSPACE_RGB24:
   case MP_COLORSPACE_RGB32:
   case MP_COLORSPACE_BGR24:
   case MP_COLORSPACE_BGR32:
      return 1;

/*
   case MP_COLORSPACE_YUV:
   case MP_COLORSPACE_YUVp:
   case MP_COLORSPACE_YUV422:
   case MP_COLORSPACE_YUV422p:
   case MP_COLORSPACE_YUV411:
   case MP_COLORSPACE_YUV411p:
      return 1;
*/

//   case MP_COLORSPACE_YUV420:
   case MP_COLORSPACE_YUV420p:
      return 2;
   }

   // Unknown colorspace. Something wrong...
   assert(false);
   return 1;
}

#ifdef DO_COLORSPACE_CONVERSION // [

int MpVideoBuf::getFFMpegColorspace(MpVideoBuf::ColorSpace colorspace)
{
   switch (colorspace)
   {
   case MP_COLORSPACE_RGB555:
      return PIX_FMT_RGB555;
   case MP_COLORSPACE_RGB565:
      return PIX_FMT_RGB565;
   case MP_COLORSPACE_BGR555:
      return PIX_FMT_BGR555;
   case MP_COLORSPACE_BGR565:
      return PIX_FMT_BGR565;
/*
   case MP_COLORSPACE_GRAY:
      return PIX_FMT_GRAY8;
   case MP_COLORSPACE_RGB24p:
   case MP_COLORSPACE_BGR24p:
*/
   case MP_COLORSPACE_RGB24:
      return PIX_FMT_RGB24;
   case MP_COLORSPACE_RGB32:
      return PIX_FMT_RGBA;
   case MP_COLORSPACE_BGR24:
      return PIX_FMT_BGR24;
   case MP_COLORSPACE_BGR32:
      return PIX_FMT_BGRA;

/*
   case MP_COLORSPACE_YUV:
   case MP_COLORSPACE_YUVp:
   case MP_COLORSPACE_YUV422:
   case MP_COLORSPACE_YUV422p:
   case MP_COLORSPACE_YUV411:
   case MP_COLORSPACE_YUV411p:
      return 1;
*/

//   case MP_COLORSPACE_YUV420:
   case MP_COLORSPACE_YUV420p:
      return PIX_FMT_YUV420P;
   }

   // Unknown colorspace. Something wrong...
   assert(false);
   return 0;
}

OsStatus MpVideoBuf::convertToColorSpace(MpVideoBuf::ColorSpace colorspace, char *pBuffer, int bufSize)
{
   OsStatus retcode = OS_FAILED;
   int srcPixFmt;
   int dstPixFmt;
   unsigned width;
   unsigned height;

   width = getFrameWidth();
   height = getFrameHeight();
   srcPixFmt = getFFMpegColorspace(mColorspace);
   dstPixFmt = getFFMpegColorspace(colorspace);

   if (avpicture_get_size(dstPixFmt, width, height) > bufSize)
   {
      return OS_FAILED;
   }

   // If colorspaces are equal we could simply copy frame data to output buffer.
   if (mColorspace == colorspace)
   {
      memcpy(pBuffer, getWriteDataPtr(), avpicture_get_size(srcPixFmt, width, height));
      retcode =  OS_SUCCESS;
   }
   else
   {
      AVPicture srcPicture;
      AVPicture dstPicture;

      // Setup srcPicture and dstPicture internal pointers
      avpicture_fill(&srcPicture, (uint8_t*)getWriteDataPtr(), srcPixFmt, width, height);
      avpicture_fill(&dstPicture, (uint8_t*)pBuffer, dstPixFmt, width, height);

      // Do convert
      if (img_convert(&dstPicture, dstPixFmt,
                      &srcPicture, srcPixFmt,
                      width, height) >= 0)
      {
         retcode =  OS_SUCCESS;
      }
   }

   return retcode;
}

OsStatus MpVideoBuf::convertFromColorSpace(MpVideoBuf::ColorSpace colorspace, char *pBuffer, int bufSize)
{
   OsStatus retcode = OS_FAILED;
   int srcPixFmt;
   int dstPixFmt;
   unsigned width;
   unsigned height;

   width = getFrameWidth();
   height = getFrameHeight();
   srcPixFmt = getFFMpegColorspace(colorspace);
   dstPixFmt = getFFMpegColorspace(mColorspace);

   if (  avpicture_get_size(srcPixFmt, width, height) > bufSize
      || avpicture_get_size(dstPixFmt, width, height) > mpData->getMaxDataSize())
   {
      return OS_FAILED;
   }

   // If colorspaces are equal we could simply copy frame data to output buffer.
   if (mColorspace == colorspace)
   {
      memcpy(getWriteDataPtr(), pBuffer, avpicture_get_size(srcPixFmt, width, height));
      retcode =  OS_SUCCESS;
   }
   else
   {
      AVPicture srcPicture;
      AVPicture dstPicture;

      // Setup srcPicture and dstPicture internal pointers
      avpicture_fill(&srcPicture, (uint8_t*)pBuffer, srcPixFmt, width, height);
      avpicture_fill(&dstPicture, (uint8_t*)getWriteDataPtr(), dstPixFmt, width, height);

      // Do convert
      if (img_convert(&dstPicture, dstPixFmt, &srcPicture, srcPixFmt, width, height) >= 0)
      {
         retcode =  OS_SUCCESS;
      }
   }

   return retcode;
}
#endif // DO_COLORSPACE_CONVERSION ]

/* ============================ MANIPULATORS ============================== */

void MpVideoBuf::setColorspace(ColorSpace colorspace)
{
   // Set colorspace
   mColorspace = colorspace;

   // Adjust number of planes according to given colorspace.
   mNumPlanes = getColospaceNumPlanes(colorspace);

   // Adjust pixel depth according to given colorspace.
   mPixelDepth = getColospacePixelDepth(colorspace);

   // Adjust U and V plane size dividers according to given colorspace.
   mUVWidthDivider = getColospaceUVWidthDivider(colorspace);
   mUVHeightDivider = getColospaceUVHeightDivider(colorspace);

   // Set line steps and pointers for planes.
   resetPlaneParameters();
}

void MpVideoBuf::resetPlaneParameters()
{
   // Set line step.
   unsigned step = getColorspaceStepMultiplier(mColorspace)*mWidth;
   for (unsigned i=0; i<mNumPlanes; i++) {
      mpPlaneParameters[i].mpStep = step;
   }

   // Set start of planes.
   mpPlaneParameters[0].mpBeginPointer = getWriteDataPtr();
   switch(mColorspace) {
   case MP_COLORSPACE_RGB555:
   case MP_COLORSPACE_RGB565:
   case MP_COLORSPACE_BGR555:
   case MP_COLORSPACE_BGR565:
/*
   case MP_COLORSPACE_YUV:
   case MP_COLORSPACE_GRAY:
   case MP_COLORSPACE_YUV422:
   case MP_COLORSPACE_YUV411:
   case MP_COLORSPACE_YUV420:
*/
   case MP_COLORSPACE_RGB24:
   case MP_COLORSPACE_RGB32:
   case MP_COLORSPACE_BGR24:
   case MP_COLORSPACE_BGR32:
      break;

/*
   case MP_COLORSPACE_RGB24p:
   case MP_COLORSPACE_BGR24p:
   case MP_COLORSPACE_YUVp:
      mpPlaneParameters[1].mpBeginPointer = mpPlaneParameters[0].mpBeginPointer
                                          + mpPlaneParameters[0].mpStep * mHeight;
      mpPlaneParameters[2].mpBeginPointer = mpPlaneParameters[1].mpBeginPointer
                                          + mpPlaneParameters[1].mpStep * mHeight;
      break;
*/

//   case MP_COLORSPACE_YUV422p:
   case MP_COLORSPACE_YUV420p:
      mpPlaneParameters[1].mpStep /= getUVWidthDivider();
      mpPlaneParameters[2].mpStep /= getUVWidthDivider();
      mpPlaneParameters[1].mpBeginPointer = mpPlaneParameters[0].mpBeginPointer
                                          + mpPlaneParameters[0].mpStep * mHeight;
      mpPlaneParameters[2].mpBeginPointer = mpPlaneParameters[1].mpBeginPointer
                                          + mpPlaneParameters[1].mpStep * mHeight / getUVHeightDivider();
      break;

/*
   case MP_COLORSPACE_YUV411p:
      // Ipse: I do not know how this is arranged...
      assert(false);
      break;
*/

   default:
      // Unknown colorspace. Something wrong...
      assert(false);
   }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

void MpVideoBuf::init()
{
#ifdef MPBUF_DEBUG
    osPrintf(">>> MpVideoBuf::init()\n");
#endif
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
