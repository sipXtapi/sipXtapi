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
#include <mp/MprNullAec.h>
#include <mp/MprAudioFrameBuffer.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprNullAec::MprNullAec(const UtlString& rName, 
                 int samplesPerFrame, 
                 int samplesPerSec,
                 MprAudioFrameBuffer& bufferResource)
:  MpAudioResource(rName, 0, 1, 0, 1,
                   samplesPerFrame, samplesPerSec),
                   mpOutputBufferResource(&bufferResource)
{
}

// Destructor
MprNullAec::~MprNullAec()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprNullAec::doProcessFrame(MpBufPtr inBufs[],
                                   MpBufPtr outBufs[],
                                   int inBufsSize,
                                   int outBufsSize,
                                   UtlBoolean isEnabled,
                                   int samplesPerFrame,
                                   int samplesPerSecond)
{
   if (!isEnabled)
   {
      // Straight pass through when disabled
      outBufs[0] = inBufs[0];
   }

   else if(inBufs[0].isValid())
   {
      // Get the last output frame
      //MpAudioBufPtr priorOutFrame;
      //mpOutputBufferResource->getFrame(0, priorOutFrame);
      //const MpAudioSample* priorSpeakerSamples = 
      //   priorOutFrame.isValid() ? priorOutFrame->getSamplesPtr() : NULL;
      // Note: if pure silence was output, the samples may be NULL

      // Next input frame from mic to remove echo from:
      //MpAudioSample* micSamples = ((MpAudioBufPtr)inBufs[0])->getSamplesWritePtr();
      //int numSamples = ((MpAudioBufPtr)inBufs[0])->getSamplesNumber();

      // Insert AEC algoithm here to remove echo from micSamples
      

      // The samples are cloned to avoid clobering samples if something 
      // else is referencing the input/mic buffer.
      outBufs[0] = inBufs[0];

   }

   return TRUE;
}

/* ============================ FUNCTIONS ================================= */

