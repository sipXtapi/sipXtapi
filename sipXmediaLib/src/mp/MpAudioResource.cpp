//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpAudioResource.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpAudioResource::MpAudioResource(const UtlString& rName, int minInputs, int maxInputs,
                       int minOutputs, int maxOutputs,
                       int samplesPerFrame, int samplesPerSec)
:  MpResource(rName, minInputs, maxInputs, minOutputs, maxOutputs),
   mSamplesPerFrame(samplesPerFrame),
   mSamplesPerSec(samplesPerSec)
{
}

// Destructor
MpAudioResource::~MpAudioResource()
{
}

/* ============================ MANIPULATORS ============================== */

// Wrapper around doProcessFrame().
// Returns TRUE if successful, FALSE otherwise.
// This method prepares the input buffers before calling
// doProcessFrame() and distributes the output buffers to the
// appropriate downstream resources after doProcessFrame()
// returns.
UtlBoolean MpAudioResource::processFrame(void)
{
   int       i;
   UtlBoolean res;

#define WATCH_FRAME_PROCESSING
#undef  WATCH_FRAME_PROCESSING
#ifdef WATCH_FRAME_PROCESSING /* [ */
   char      z[500];
   const char* pName;
   int       len;
#endif /* WATCH_FRAME_PROCESSING ] */

#ifdef WATCH_FRAME_PROCESSING /* [ */
   pName = mName;
   len = sprintf(z, "%s(", pName);
   for (i=0; i < mMaxInputs; i++)
   {
      if (mpInBufs[i] != NULL)
      {
         len += sprintf(z+len, "%d,", MpBuf_bufNum(mpInBufs[i]));
      } else {
         len += sprintf(z+len, "-,");
      }
   }
   if (mMaxInputs > 0) len--;
   len += sprintf(z+len, ")..(");
#endif /* WATCH_FRAME_PROCESSING ] */

   // call doProcessFrame to do any "real" work
   res = doProcessFrame(mpInBufs, mpOutBufs,
                        mMaxInputs, mMaxOutputs, mIsEnabled,
                        mSamplesPerFrame, mSamplesPerSec);

#ifdef WATCH_FRAME_PROCESSING /* [ */
   for (i=0; i < mMaxInputs; i++)
   {
      if (mpInBufs[i] != NULL)
      {
         len += sprintf(z+len, "%d,", MpBuf_bufNum(mpInBufs[i]));
      } else {
         len += sprintf(z+len, "-,");
      }
   }
   if (mMaxInputs > 0) len--;
   len += sprintf(z+len, ")..(");

   for (i=0; i < mMaxOutputs; i++)
   {
      if (mpOutBufs[i] != NULL)
      {
         len += sprintf(z+len, "%d,", MpBuf_bufNum(mpOutBufs[i]));
      } else {
         len += sprintf(z+len, "-,");
      }
   }
   if (mMaxOutputs > 0) len--;
   len += sprintf(z+len, ")\n");
   z[len] = 0;
   Zprintf("%s", (int) z, 0,0,0,0,0);
#endif /* WATCH_FRAME_PROCESSING ] */

   // delete any input buffers that were not consumed by doProcessFrame()
   for (i=0; i < mMaxInputs; i++)
   {
      if (mpInBufs[i] != NULL)
      {
         MpBuf_delRef(mpInBufs[i]);
         mpInBufs[i] = NULL;
      }
   }

   // pass the output buffers downstream
   for (i=0; i < mMaxOutputs; i++)
   {
      if (!setOutputBuffer(i, mpOutBufs[i])) MpBuf_delRef(mpOutBufs[i]);
      mpOutBufs[i] = NULL;
   }

   return res;
}

// Sets the number of samples expected per frame.
// Returns FALSE if the specified rate is not supported, TRUE otherwise.
UtlBoolean MpAudioResource::setSamplesPerFrame(int samplesPerFrame)
{
   MpFlowGraphMsg msg(MpFlowGraphMsg::RESOURCE_SET_SAMPLES_PER_FRAME, this,
                      NULL, NULL, samplesPerFrame);
   OsStatus       res;

   res = postMessage(msg);
   return (res == OS_SUCCESS);
}

// Sets the number of samples expected per second.
// Returns FALSE if the specified rate is not supported, TRUE otherwise.
UtlBoolean MpAudioResource::setSamplesPerSec(int samplesPerSec)
{
   MpFlowGraphMsg msg(MpFlowGraphMsg::RESOURCE_SET_SAMPLES_PER_SEC, this,
                      NULL, NULL, samplesPerSec);
   OsStatus       res;

   res = postMessage(msg);
   return (res == OS_SUCCESS);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Handles an incoming message for this media processing object.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpAudioResource::handleMessage(MpFlowGraphMsg& rMsg)
{
   UtlBoolean msgHandled;

   msgHandled = TRUE;                       // assume we'll handle the msg
   switch (rMsg.getMsg())
   {
   case MpFlowGraphMsg::RESOURCE_SET_SAMPLES_PER_FRAME:
      mSamplesPerFrame = rMsg.getInt1();    // set the samples per frame
      break;
   case MpFlowGraphMsg::RESOURCE_SET_SAMPLES_PER_SEC:
      mSamplesPerSec = rMsg.getInt1();      // set the samples per second
      break;
   default:
      msgHandled = MpResource::handleMessage(rMsg);  // we didn't handle the msg
      break;                                         // pass it to the parent
   }

   return msgHandled;
}

int MpAudioResource::getSamplesPerFrame()
{
   return mSamplesPerFrame;
}

int MpAudioResource::getSamplesPerSec()
{
   return mSamplesPerSec;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

