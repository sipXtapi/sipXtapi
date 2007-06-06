//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#define WATCH_FRAME_PROCESSING
#undef  WATCH_FRAME_PROCESSING

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpResourceMsg.h"
#include "mp/MpAudioResource.h"
#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#  ifdef RTL_AUDIO_ENABLED
#     include <SeScopeAudioBuffer.h>
#  endif
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpAudioResource::MpAudioResource(const UtlString& rName,
                                 int minInputs, int maxInputs,
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

#ifdef RTL_ENABLED
        RTL_BLOCK((UtlString)*this);
#endif

#ifdef WATCH_FRAME_PROCESSING /* [ */
   char      z[500];
   int       len;

   len = sprintf(z, "%s in(", (const char*)*this);
   for (i=0; i < mMaxInputs; i++)
   {
      if (mpInBufs[i].isValid())
      {
         len += sprintf(z+len, "%d,", mpInBufs[i].getBufferNumber());
      } else {
         len += sprintf(z+len, "-,");
      }
   }
   if (mMaxInputs > 0) len--;
   len += sprintf(z+len, ") -> unprocessed(");
#endif /* WATCH_FRAME_PROCESSING ] */

   // call doProcessFrame to do any "real" work
   res = doProcessFrame(mpInBufs, mpOutBufs,
                        mMaxInputs, mMaxOutputs, mIsEnabled,
                        mSamplesPerFrame, mSamplesPerSec);

#ifdef WATCH_FRAME_PROCESSING /* [ */
   for (i=0; i < mMaxInputs; i++)
   {
      if (mpInBufs[i].isValid())
      {
         len += sprintf(z+len, "%d,", mpInBufs[i].getBufferNumber());
      } else {
         len += sprintf(z+len, "-,");
      }
   }
   if (mMaxInputs > 0) len--;
   len += sprintf(z+len, ") -> out(");

   for (i=0; i < mMaxOutputs; i++)
   {
      if (mpOutBufs[i].isValid())
      {
         len += sprintf(z+len, "%d,", mpOutBufs[i].getBufferNumber());
      } else {
         len += sprintf(z+len, "-,");
      }
   }
   if (mMaxOutputs > 0) len--;
   len += sprintf(z+len, ")\n");
   z[len] = 0;
   osPrintf("%s", (int) z, 0,0,0,0,0);
#endif /* WATCH_FRAME_PROCESSING ] */

   // delete any input buffers that were not consumed by doProcessFrame()
   for (i=0; i < mMaxInputs; i++)
   {
      mpInBufs[i].release();
   }

#ifdef RTL_AUDIO_ENABLED
   int frameIndex = mpFlowGraph ? mpFlowGraph->numFramesProcessed() : 0;
#endif

   // pass the output buffers downstream
   for (i=0; i < mMaxOutputs; i++)
   {
#ifdef RTL_AUDIO_ENABLED
      // If there is a consumer of the output
      if(mpOutConns[i].pResource)
      {
         UtlString outputLabel(*this);
         outputLabel.append("_output_");
         outputLabel.append((char) i < 10 ? ('0' + i) : ('A' + i - 10));
         outputLabel.append('_');
         outputLabel.append(*mpOutConns[i].pResource);
         RTL_AUDIO_BUFFER(outputLabel, 
                          mSamplesPerSec, 
                          ((MpAudioBufPtr) mpOutBufs[i]), 
                          frameIndex);
      }
#endif
       pushBufferDownsream(i, mpOutBufs[i]);
       mpOutBufs[i].release();
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

// Handles an incoming flowgraph message for this media processing object.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpAudioResource::handleMessage(MpFlowGraphMsg& fgMsg)
{
   UtlBoolean msgHandled;

   msgHandled = TRUE;                       // assume we'll handle the msg
   switch (fgMsg.getMsg())
   {
   case MpFlowGraphMsg::RESOURCE_SET_SAMPLES_PER_FRAME:
      mSamplesPerFrame = fgMsg.getInt1();    // set the samples per frame
      break;
   case MpFlowGraphMsg::RESOURCE_SET_SAMPLES_PER_SEC:
      mSamplesPerSec = fgMsg.getInt1();      // set the samples per second
      break;
   default:
      msgHandled = MpResource::handleMessage(fgMsg);  // we didn't handle the msg
      break;                                         // pass it to the parent
   }

   return msgHandled;
}


// Handles an incoming resource message for this media processing object.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpAudioResource::handleMessage(MpResourceMsg& rMsg)
{
   UtlBoolean msgHandled = TRUE; // assume we'll handle the msg
   msgHandled = MpResource::handleMessage(rMsg);  // we didn't handle the msg
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

