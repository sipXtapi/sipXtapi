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
                                 int minOutputs, int maxOutputs)
:  MpResource(rName, minInputs, maxInputs, minOutputs, maxOutputs)
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
   UtlString str_fg(getFlowGraph()->getFlowgraphName());
   str_fg.append("_");
   str_fg.append(*this);
   RTL_BLOCK(str_fg);
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
                        mpFlowGraph->getSamplesPerFrame(),
                        mpFlowGraph->getSamplesPerSec());

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
         UtlString outputLabel(mpFlowGraph->getFlowgraphName());
         outputLabel.appendFormat("_%s_output_%d_",
                                  getName().data(), i);
         outputLabel.append(*mpOutConns[i].pResource);
         RTL_AUDIO_BUFFER(outputLabel, 
                          mpFlowGraph->getSamplesPerSec(), 
                          ((MpAudioBufPtr) mpOutBufs[i]), 
                          frameIndex);
      }
#endif
       pushBufferDownsream(i, mpOutBufs[i]);
       mpOutBufs[i].release();
   }

   return res;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

