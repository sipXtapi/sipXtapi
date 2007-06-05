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


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <mp/MpBuf.h>
#include <mp/MprBridge.h>
#include <mp/MpMisc.h>
#ifdef RTL_ENABLED
#  include <mp/MpFlowGraphBase.h>
#  include <rtl_macro.h>
#  ifdef RTL_AUDIO_ENABLED
#     include <SeScopeAudioBuffer.h>
#  endif
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
#define TEST_PRINT_CONTRIBUTORS
//#undef  TEST_PRINT_CONTRIBUTORS


#ifdef TEST_PRINT_CONTRIBUTORS
class MpContributorVector
{
public:
   MpContributorVector(int maxContributors = 0)
   {
      mMaxContributors = maxContributors;
      if (mMaxContributors > 0)
      {
         mpContributorVector = new int[mMaxContributors];
         zero();
      }
      else
      {
         mMaxContributors = 0;
         mpContributorVector = NULL;
      }
   };

   ~MpContributorVector()
   {
      if (mpContributorVector)
      {
         delete mpContributorVector;
         mpContributorVector = NULL;
      }
   };

   void zero()
   {
      if (mpContributorVector && mMaxContributors > 0)
      {
         memset(mpContributorVector, 0, sizeof(int) * mMaxContributors);
      }
   };

   void set(int contributorIndex, int mixWeight)
   {
      if (mpContributorVector && 
          contributorIndex >= 0 && 
          contributorIndex < mMaxContributors)
      {
         mpContributorVector[contributorIndex] = mixWeight;
      }
   };

   int get(int contributorIndex) const
   {
      int retValue;
      if (mpContributorVector && 
          contributorIndex >= 0 && 
          contributorIndex < mMaxContributors)
      {
         retValue = mpContributorVector[contributorIndex];
      }
      else
      {
         retValue = 0;
      }
      return retValue;
   };

   UtlBoolean differs(const MpContributorVector& otherVector) const
   {
      assert(otherVector.mMaxContributors == mMaxContributors);
      UtlBoolean isDiff = FALSE;
      for (int i = 0; i < mMaxContributors; i++)
      {
         if (mpContributorVector[i] != otherVector.mpContributorVector[i])
         {
            isDiff = TRUE;
            break;
         }
      }
      return isDiff;
   }

   UtlBoolean operator==(const MpContributorVector& otherVector) const
   {
      return !differs(otherVector);
   }

   UtlBoolean operator!=(const MpContributorVector& otherVector) const
   {
      return differs(otherVector);
   }

   MpContributorVector& operator=(const MpContributorVector& source)
   {
      assert(source.mMaxContributors == mMaxContributors);
      memcpy(mpContributorVector, 
             source.mpContributorVector,
             sizeof(int) * mMaxContributors);
      return *this;
   }

private:
   int mMaxContributors;
   int* mpContributorVector;

   MpContributorVector(const MpContributorVector&);

};
#endif

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprBridge::MprBridge(const UtlString& rName,
                     int maxInOutputs,
                     int samplesPerFrame, 
                     int samplesPerSec)
:  MpAudioResource(rName, 
                   1, maxInOutputs, 
                   1, maxInOutputs, 
                   samplesPerFrame, 
                   samplesPerSec)
{
   handleDisable();

#ifdef TEST_PRINT_CONTRIBUTORS
   mpMixContributors = new MpContributorVector(maxInOutputs);
   mpLastOutputContributors = new MpContributorVector*[maxInOutputs];
   for (int i = 0; i < maxInOutputs; i++)
   {
      mpLastOutputContributors[i] = new MpContributorVector(maxInOutputs);
   }
#endif
}

// Destructor
MprBridge::~MprBridge()
{

#ifdef TEST_PRINT_CONTRIBUTORS
   delete mpMixContributors;
   mpMixContributors = NULL;
   for (int i = 0; i < mMaxOutputs; i++)
   {
      delete mpLastOutputContributors[i];
      mpLastOutputContributors[i] = NULL;
   }
   delete[] mpLastOutputContributors;
   mpLastOutputContributors = NULL;
#endif

}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MprBridge::doMix(MpAudioBufPtr inBufs[], int inBufsSize,
                            MpAudioBufPtr &out, int samplesPerFrame,
                            MpContributorVector& contributors)
{
    int inputsConnected;  // Number of connected ports
    int inputsValid;      // Number of ports with available data
    int inputsActive;     // Number of ports with active speech
    int lastConnected;    // Port number of last connected port
    int lastValid;        // Port number of last port with available data
    int lastActive;       // Port number of last port with active speech

    // Count contributing inputs
    inputsConnected = 0;
    inputsValid = 0;
    inputsActive = 0;
    lastConnected = -1;
    lastValid = -1;
    lastActive = -1;
    for (int inIdx=0; inIdx < inBufsSize; inIdx++) {
       if (isPortActive(inIdx))
       {
          inputsConnected++;
          lastConnected = inIdx;
          if (inBufs[inIdx].isValid())
          {
#ifdef TEST_PRINT_CONTRIBUTORS
             contributors.set(inIdx, 1);
#endif
             inputsValid++;
             lastValid = inIdx;
             if (inBufs[inIdx]->isActiveAudio())
             {
                inputsActive++;
                lastActive = inIdx;
             }
          }
          else
          {
#ifdef TEST_PRINT_CONTRIBUTORS
             contributors.set(inIdx, 0);
#endif
          }
       }
       else
       {
#ifdef TEST_PRINT_CONTRIBUTORS
          contributors.set(inIdx, 0);
#endif
       }
    }

    // If there is only one input we could skip all processing and copy it
    // to output. Special case for single input is needed because other case
    // make decision make its choice depending on voice activity, which lead
    // to unwanted silence insertion. Someday this function will get smarter...
    if (inputsValid == 1)
    {
       out = inBufs[lastValid];
    }
    else if (inputsActive == 1)
    {
       // If only one active input then just return it
       out = inBufs[lastActive];
    }
    else if (inputsActive > 1)
    {
        // Compute a logarithmic scale factor to renormalize (approximately)
        int scale = 0;
        while (inputsActive > 1)
        {
            scale++;
            inputsActive = inputsActive >> 1;
        }

        // Get new buffer for mixed output
        out = MpMisc.RawAudioPool->getBuffer();
        if (!out.isValid())
            return FALSE;
        out->setSamplesNumber(samplesPerFrame);

        // Fill output buffer with silence
        MpAudioSample* outstart = out->getSamples();
        memset((char *) outstart, 0, samplesPerFrame * sizeof(MpAudioSample));

        // Mix them all
        for (int inIdx=0; inIdx < inBufsSize; inIdx++)
        {
            if (isPortActive(inIdx))
            {
                MpAudioSample* output = outstart;
                // Mix only non-silent audio
                if (inBufs[inIdx].isValid() && inBufs[inIdx]->isActiveAudio())
                { 
                    MpAudioSample* input = inBufs[inIdx]->getSamples();
                    int n = min(inBufs[inIdx]->getSamplesNumber(), samplesPerFrame);
                    for (int i=0; i<n; i++)
                        *output++ += (*input++) >> scale;
                }
            }
        }
    }

    return TRUE;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

//Check whether this port is connected to both input and output
UtlBoolean MprBridge::isPortActive(int portIdx)
{
   return (isInputConnected(portIdx) && isOutputConnected(portIdx));
}

UtlBoolean MprBridge::doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond)
{
   MpAudioBufPtr in;
   UtlBoolean ret = FALSE;

#ifdef RTL_AUDIO_ENABLED
   int frameIndex = mpFlowGraph ? mpFlowGraph->numFramesProcessed() : 0;
   RTL_AUDIO_BUFFER("bridge_input_0", samplesPerSecond, ((MpAudioBufPtr)inBufs[0]), frameIndex);
   RTL_AUDIO_BUFFER("bridge_input_1", samplesPerSecond, ((MpAudioBufPtr)inBufs[1]), frameIndex);
   RTL_AUDIO_BUFFER("bridge_input_2", samplesPerSecond, ((MpAudioBufPtr)inBufs[2]), frameIndex);
#endif

   if (outBufsSize == 0)
      return FALSE;

   if (inBufsSize == 0)
      return FALSE;

   // We want correct in/out pairs
   if (inBufsSize != outBufsSize)
      return FALSE;

   MpAudioBufPtr* inAudioBufs = new MpAudioBufPtr[inBufsSize];
   for (int i=0; i<inBufsSize; i++) {
       inAudioBufs[i].swap(inBufs[i]);
   }

   // If disabled, mix all remote inputs onto local speaker, and copy
   // our local microphone to all remote outputs.
   if (!isEnabled)
   {
       printf("Bridge disabled\n");
      // Move local mic data to all remote parties
      in.swap(inAudioBufs[0]);
      for (int outIdx=1; outIdx < outBufsSize; outIdx++) {
         if (isPortActive(outIdx)) {
            outBufs[outIdx] = in;
         }
      }

      // Copy mixed remote inputs to local speaker and exit
      MpAudioBufPtr out;

#ifdef TEST_PRINT_CONTRIBUTORS
      mpMixContributors->zero();
#endif

      ret = doMix(inAudioBufs, inBufsSize, out, samplesPerFrame, *mpMixContributors);
 
#ifdef TEST_PRINT_CONTRIBUTORS
      // Keep track of the sources mixed for this output
      if (*mpLastOutputContributors[0] != *mpMixContributors)
      {
         int contribIndex;
         printf("Bridge output: %d vector changed: %d", 
                0, mpMixContributors->get(0));
         for (contribIndex = 1; contribIndex < inBufsSize; contribIndex++)
         {
            printf(", %d", mpMixContributors->get(contribIndex));
         }
         printf("\n");
         *(mpLastOutputContributors[0]) = *mpMixContributors;
      }
#endif

      outBufs[0] = out;
   } 
   else
   {
      MpAudioBufPtr temp;

      // Enabled.  Mix together inputs onto outputs, with the requirement
      // that no output receive its own input.
      for (int outIdx=0; outIdx < outBufsSize; outIdx++) {
         // Skip unconnected outputs
         if (!isPortActive(outIdx))
         {
             continue;
         }

         // Exclude current input from mixing
         temp.swap(inAudioBufs[outIdx]);

         // Mix all inputs except outIdx together and put to the output
         MpAudioBufPtr out;

#ifdef TEST_PRINT_CONTRIBUTORS
         mpMixContributors->zero();
#endif

         ret = doMix(inAudioBufs, inBufsSize, out, samplesPerFrame, *mpMixContributors);

#ifdef TEST_PRINT_CONTRIBUTORS
         // Keep track of the sources mixed for this output
         if (*mpLastOutputContributors[outIdx] != *mpMixContributors)
         {
            int contribIndex;
            printf("Bridge output: %d vector change: %d", 
               outIdx, mpMixContributors->get(0));
            for (contribIndex = 1; contribIndex < inBufsSize; contribIndex++)
            {
               printf(", %d", mpMixContributors->get(contribIndex));
            }
            printf("\n");

            *mpLastOutputContributors[outIdx] = *mpMixContributors;
         }
#endif

         outBufs[outIdx] = out;

         // Return current input to input buffers vector
         temp.swap(inAudioBufs[outIdx]);
      }
   }

#ifdef RTL_AUDIO_ENABLED
   RTL_AUDIO_BUFFER("bridge_output_0", samplesPerSecond, ((MpAudioBufPtr)outBufs[0]), frameIndex);
#endif

   // Cleanup temporary buffers
   delete[] inAudioBufs;

   return ret;
}

/* ============================ FUNCTIONS ================================= */

