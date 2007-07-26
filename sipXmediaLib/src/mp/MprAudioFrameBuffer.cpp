//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <mp/MpBuf.h>
#include <mp/MprAudioFrameBuffer.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprAudioFrameBuffer::MprAudioFrameBuffer(const UtlString& rName, 
                 int samplesPerFrame, 
                 int samplesPerSec,
                 int numFramesOfHistory)
:  MpAudioResource(rName, 0, 1, 0, 1,
                   samplesPerFrame, samplesPerSec),
                   mFrameCount(0),
                   mNumBufferFrames(numFramesOfHistory),
                   mpBufferedFrameArray(NULL)

{
   mpBufferedFrameArray = new MpBufPtr[mNumBufferFrames];
}

// Destructor
MprAudioFrameBuffer::~MprAudioFrameBuffer()
{
   if(mpBufferedFrameArray)
   {
      delete[] mpBufferedFrameArray;
      mpBufferedFrameArray = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprAudioFrameBuffer::doProcessFrame(MpBufPtr inBufs[],
                                               MpBufPtr outBufs[],
                                               int inBufsSize,
                                               int outBufsSize,
                                               UtlBoolean isEnabled,
                                               int samplesPerFrame,
                                               int samplesPerSecond)
{
   mFrameCount++;

   if (!isEnabled)
   {
   }


   else
   {
      if(inBufs[0].isValid())
      {
         // Valid buffer, keep a reference
         mpBufferedFrameArray[mFrameCount % mNumBufferFrames] = inBufs[0];
      }
      else
      {
         // invalid buffer, clear the reference
         mpBufferedFrameArray[mFrameCount % mNumBufferFrames].release();
      }
   }


   if(inBufs[0].isValid())
   {
      outBufs[0] = inBufs[0];
   }
   return TRUE;
}

OsStatus MprAudioFrameBuffer::getFrame(int pastFramesIndex, MpBufPtr& frameBuffer)
{
   OsStatus status;
   assert(pastFramesIndex >= 0);
   assert(pastFramesIndex < mNumBufferFrames);
   if(pastFramesIndex >= 0 &&
      pastFramesIndex < mNumBufferFrames)
   {
      frameBuffer = mpBufferedFrameArray[(mFrameCount + pastFramesIndex) % mNumBufferFrames];
      status = OS_SUCCESS;
   }
   else
   {
      status = OS_NOT_FOUND;
   }
   return(status);
}

/* ============================ FUNCTIONS ================================= */

