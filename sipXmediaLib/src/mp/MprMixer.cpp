//  
// Copyright (C) 2006 SIPez LLC. 
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


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MprMixer.h"

#ifndef ABS
#define ABS(x) (max((x), -(x)))
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprMixer::MprMixer(const UtlString& rName, int numWeights,
                   int samplesPerFrame, int samplesPerSec)
:  MpAudioResource(rName, 1, numWeights, 1, 1, samplesPerFrame, samplesPerSec),
   mScale(0)
{
   int i;

   mNumWeights = max(0, min(numWeights, MAX_MIXER_INPUTS));
   for (i=0; i<numWeights; i++)
      mWeights[i] = 0;
}

// Destructor
MprMixer::~MprMixer()
{
   // no work to do
}

/* ============================ MANIPULATORS ============================== */

// Sets the weighting factors for the first "numWeights" inputs.
// For now, this method always returns TRUE.
UtlBoolean MprMixer::setWeights(int *newWeights, int numWeights)
{
   int            i;
   MpFlowGraphMsg msg(SET_WEIGHTS, this, NULL, NULL, numWeights);
   OsStatus       res;
   int*           weights;
                                               
   weights = new int[numWeights];  // allocate storage for the weights here.
   for (i=0; i < numWeights; i++)  //  the storage will be freed by
      weights[i] = newWeights[i];  //  handleMessage()

   msg.setPtr1(weights);

   res = postMessage(msg);
   return (res == OS_SUCCESS);
}

// Sets the weighting factor for the "weightIndex" input.
// For now, this method always returns TRUE.
UtlBoolean MprMixer::setWeight(int newWeight, int weightIndex)
{
   MpFlowGraphMsg msg(SET_WEIGHT, this, NULL, NULL,
                      newWeight, weightIndex);
   OsStatus       res;

   res = postMessage(msg);
   return (res == OS_SUCCESS);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprMixer::doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond)
{
   MpAudioBufPtr  out;
   MpAudioSample* outstart;

   if (outBufsSize != 1)
       return FALSE;

   // Don't waste the time if output is not connected
   if (!isOutputConnected(0))
       return TRUE;

   if (!isEnabled || (inBufsSize == 0)) {
      // Disabled, return first input

      outBufs[0].swap(inBufs[0]);
      return TRUE;
   }

   if (mScale == 0) {
      // scale factors are all zero, mute output
      return TRUE;       
   }

   if (mScale == 1) {
      // must be only one weight != 0, and it is == 1

      for (int i=0; i < inBufsSize; i++)
      {
         if (mWeights[i] != 0)
         {
            outBufs[0].swap(inBufs[i]);
            return TRUE;
         }
      }
      return TRUE; // even if we did not find it (mNWeights > inBufsSize)
   }

   // Get new buffer for our output
   out = MpMisc.RawAudioPool->getBuffer();
   if (!out.isValid())
      return FALSE;
   out->setSamplesNumber(samplesPerFrame);
   out->setSpeechType(MpAudioBuf::MP_SPEECH_UNKNOWN);

   // Fill buffer with silence
   outstart = out->getSamplesWritePtr();
   memset((char *) outstart, 0, samplesPerFrame * sizeof(MpAudioSample));

   for (int i=0; i < inBufsSize; i++)
   {
      int wgt = mWeights[i];
      if ((inBufs[i].isValid()) && (wgt != 0))
      {
         MpAudioBufPtr  in = inBufs[i];
         MpAudioSample* output = outstart;
         const MpAudioSample* input = in->getSamplesPtr();
         int n = min(in->getSamplesNumber(), samplesPerFrame);
         if (wgt == 1)
         {
            for (int j=0; j<n; j++)
                *output++ += (*input++) / mScale;
         }
         else
         {
            for (int j=0; j<n; j++)
                *output++ += (*input++ * wgt) / mScale;
         }
      }
   }

   // push it downstream
   outBufs[0].swap(out);

   return TRUE;
}

// Handle messages for this resource.
UtlBoolean MprMixer::handleMessage(MpFlowGraphMsg& rMsg)
{
   UtlBoolean boolRes;
   int       msgType;
   int*      weights;

   msgType = rMsg.getMsg();
   switch (msgType)
   {
   case SET_WEIGHT:
      return handleSetWeight(rMsg.getInt1(), rMsg.getInt2());
      break;
   case SET_WEIGHTS:
      weights = (int*) rMsg.getPtr1();
      boolRes = handleSetWeights(weights, rMsg.getInt1());
      delete[] weights; // delete storage allocated in the setWeights() method
      return boolRes;
      break;
   default:
      return MpAudioResource::handleMessage(rMsg);
      break;
   }
}

// Handle the SET_WEIGHT message.
UtlBoolean MprMixer::handleSetWeight(int newWeight, int weightIndex)
{
   if (weightIndex < mNumWeights)
   {
      mScale -= ABS(mWeights[weightIndex]);
      mScale += ABS(newWeight);
      mWeights[weightIndex] = newWeight;
   }

   return TRUE;
}

// Handle the SET_WEIGHTS message.
UtlBoolean MprMixer::handleSetWeights(int *newWeights, int numWeights)
{
   int i;
   int wgt;

   mNumWeights = max(0, min(numWeights, MAX_MIXER_INPUTS));
   mScale = 0;
   for (i=0; i < numWeights; i++)
   {
      mWeights[i] = wgt = newWeights[i];
      mScale += ABS(wgt);
   }
   
   return TRUE;
}

/* ============================ FUNCTIONS ================================= */

