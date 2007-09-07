//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2007 Wirtualna Polska S.A.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// Author: Andrzej Ciarkowski <andrzejc AT wp-sa DOT pl>

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/video/MpVideoStreamParams.h"
#include "sdp/SdpCodec.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */

/* ============================= MANIPULATORS ============================= */

void MpVideoStreamParams::setFrameFormat(int frameFormat)
{
   int w, h;
   getFrameSize(frameFormat, w, h);
   setFrameSize(w, h);
}

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */


/* ////////////////////////////// PROTECTED /////////////////////////////// */


/* /////////////////////////////// PRIVATE //////////////////////////////// */


/* ============================== FUNCTIONS =============================== */

int MpVideoStreamParams::getFrameFormat(int width, int height)
{
   if (352 == width && 288 == height)
      return SDP_VIDEO_FORMAT_CIF;
   else if (176 == width && 144 == height)
      return SDP_VIDEO_FORMAT_QCIF;
   else if (128 == width && 96 == height)
      return SDP_VIDEO_FORMAT_SQCIF;
   else if (320 == width && 240 == height)
      return SDP_VIDEO_FORMAT_QVGA;
   else
   {
      assert(!"Unknown video frame format");
      return 0;
   }
}

void MpVideoStreamParams::getFrameSize(int frameFormat, int& width, int& height)
{
   switch (frameFormat)
   {
   case SDP_VIDEO_FORMAT_CIF:
      width = 352;
      height = 288;
      break;

   case SDP_VIDEO_FORMAT_QCIF:
      width = 176;
      height = 144;
      break;

   case SDP_VIDEO_FORMAT_SQCIF:
      width = 128;
      height = 96;
      break;

   case SDP_VIDEO_FORMAT_QVGA:
      width = 320;
      height = 240;
      break;

   default:
      assert(!"Unknown video frame format");
      width = 0;
      height = 0;
   }
}

