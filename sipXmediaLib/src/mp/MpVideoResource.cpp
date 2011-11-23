//  
// Copyright (C) 2006-2011 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include "mp/MpVideoResource.h"
#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#  ifdef RTL_VIDEO_ENABLED
#     include <SeScopeVideoBuffer.h>
#  endif
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpVideoResource::MpVideoResource(const UtlString& rName,
                                 int minInputs, int maxInputs,
                                 int minOutputs, int maxOutputs)
:  MpResource(rName, minInputs, maxInputs, minOutputs, maxOutputs)
{
}

// Destructor
MpVideoResource::~MpVideoResource()
{
}

/* ============================ MANIPULATORS ============================== */

// Wrapper around doProcessFrame().
// Returns TRUE if successful, FALSE otherwise.
// This method prepares the input buffers before calling
// doProcessFrame() and distributes the output buffers to the
// appropriate downstream resources after doProcessFrame()
// returns.
UtlBoolean MpVideoResource::processFrame(void)
{
   int       i;
   UtlBoolean res;

#ifdef RTL_ENABLED
   UtlString str_fg(getFlowGraph()->getFlowgraphName());
   str_fg.append("_");
   str_fg.append(*this);
   RTL_BLOCK(str_fg);
#endif

   // call doProcessFrame to do any "real" work
   res = doProcessFrame(mpInBufs, mpOutBufs,
                        mMaxInputs, mMaxOutputs, mIsEnabled);

   // delete any input buffers that were not consumed by doProcessFrame()
   for (i=0; i < mMaxInputs; i++)
   {
      mpInBufs[i].release();
   }

#ifdef RTL_VIDEO_ENABLED
   int frameIndex = mpFlowGraph ? mpFlowGraph->numFramesProcessed() : 0;
#endif

   // pass the output buffers downstream
   for (i=0; i < mMaxOutputs; i++)
   {
#ifdef RTL_VIDEO_ENABLED
      // If there is a consumer of the output
      if(mpOutConns[i].pResource)
      {
         UtlString outputLabel(mpFlowGraph->getFlowgraphName());
         outputLabel.appendFormat("_%s_output_%d_",
                                  getName().data(), i);
         outputLabel.append(*mpOutConns[i].pResource);
         RTL_VIDEO_BUFFER(outputLabel, 
                          mpFlowGraph->getSamplesPerSec(), 
                          ((MpVideoBufPtr) mpOutBufs[i]), 
                          frameIndex);
      }
#endif
       pushBufferDownsream(i, mpOutBufs[i]);
       mpOutBufs[i].release();
   }

   return(res);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

