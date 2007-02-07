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
// #include "os/OsDefs.h"
// #include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MprBridge.h"
#include "mp/MpMisc.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprBridge::MprBridge(const UtlString& rName,
                           int samplesPerFrame, int samplesPerSec)
:  MpAudioResource(rName, 1, MAX_BRIDGE_PORTS, 1,
                   MAX_BRIDGE_PORTS, samplesPerFrame, samplesPerSec),
   mPortLock(OsBSem::Q_FIFO, OsBSem::FULL)
{
   int i;

   mpConnectionIDs[0] = (1<<30);
   for (i=1; i<MAX_BRIDGE_PORTS; i++) {
      mpConnectionIDs[i] = -1;
   }
}

// Destructor
MprBridge::~MprBridge()
{
}

/* ============================ MANIPULATORS ============================== */

// Attach a connection container to an available port.
int MprBridge::connectPort(const MpConnectionID connID)
{
   int port = findFreePort();

   if (port > -1) {
      assert(-2 == mpConnectionIDs[port]);
      mpConnectionIDs[port] = connID;
   }
   return port;
}


// Disconnect a connection container from its port.
OsStatus MprBridge::disconnectPort(const MpConnectionID connID)
{
   int i;

   for (i=1; i<MAX_BRIDGE_PORTS; i++)
   {
      if (connID == mpConnectionIDs[i]) {
         // Found the port, mark it free
         mpConnectionIDs[i] = -1;
         return OS_SUCCESS;
      }
   }
   return OS_NOT_FOUND;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MprBridge::doMix(MpAudioBufPtr inBufs[], int inBufsSize,
                            MpAudioBufPtr &out, int samplesPerFrame) const
{
    // First, count how many contributing inputs
    int inputs = 0;
    int lastActive = -1;
    for (int inIdx=0; inIdx < inBufsSize; inIdx++) {
        if (  isPortActive(inIdx)
           && inBufs[inIdx].isValid()
           && inBufs[inIdx]->isActiveAudio())
        {
                inputs++;
                lastActive = inIdx;
        }
    }

    if (inputs == 1) {
       // If only one active input then just return it
       out = inBufs[lastActive];
    }
    else if (inputs > 1) {
        // Compute a logarithmic scale factor to renormalize (approximately)
        int scale = 0;
        while (inputs > 1) {
            scale++;
            inputs = inputs >> 1;
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
        for (int inIdx=0; inIdx < inBufsSize; inIdx++) {
            if (isPortActive(inIdx)) {
                MpAudioSample* output = outstart;
                // Mix only non-silent audio
                if(inBufs[inIdx].isValid() && inBufs[inIdx]->isActiveAudio()) { 
                    MpAudioSample* input = inBufs[inIdx]->getSamples();
                    int n = min(inBufs[inIdx]->getSamplesNumber(), samplesPerFrame);
                    for (int i=0; i<n; i++)
                        *output++ += (*input++) >> scale;
                }
            }
        }
    } else {
       // Ipse: Disabled CN output. No input - no output.

       // Local output==comfort noise if all remote inputs are disabled or silent
//       out = MpMisc.comfortNoise;
    }
    return TRUE;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

//Find and return the index to an unused port pair
int MprBridge::findFreePort(void)
{
   int i;
   int found = -1;

   mPortLock.acquire();
   for (i=1; i<MAX_BRIDGE_PORTS; i++)
   {
      if (-1 == mpConnectionIDs[i]) {
         // Found a free port, we can put a new connection here.
         mpConnectionIDs[i] = -2;
         found = i;
         i = MAX_BRIDGE_PORTS;
      }
   }
   mPortLock.release();
   return found;
}

//Check whether this port is connected to both input and output
UtlBoolean MprBridge::isPortActive(int portIdx) const
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
   MpAudioBufPtr* inAudioBufs;

   if (outBufsSize == 0)
      return FALSE;

   if (inBufsSize == 0)
      return FALSE;

   // We want correct in/out pairs
   if (inBufsSize != outBufsSize)
      return FALSE;

   inAudioBufs = new MpAudioBufPtr[inBufsSize];
   for (int i=0; i<inBufsSize; i++) {
       inAudioBufs[i].swap(inBufs[i]);
   }

   // If disabled, mix all remote inputs onto local speaker, and copy
   // our local microphone to all remote outputs.
   if (!isEnabled)
   {
      // Move local mic data to all remote parties
      in.swap(inAudioBufs[0]);
      for (int outIdx=1; outIdx < outBufsSize; outIdx++) {
         if (isPortActive(outIdx)) {
            outBufs[outIdx] = in;
         }
      }

      // Copy mixed remote inputs to local speaker and exit
      MpAudioBufPtr out;
      ret = doMix(inAudioBufs, inBufsSize, out, samplesPerFrame);
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
            continue;

         // Exclude current input from mixing
         temp.swap(inAudioBufs[outIdx]);

         // Mix all inputs except outIdx together and put to the output
         MpAudioBufPtr out;
         ret = doMix(inAudioBufs, inBufsSize, out, samplesPerFrame);
         outBufs[outIdx] = out;

         // Return current input to input buffers vector
         temp.swap(inAudioBufs[outIdx]);
      }
   }

   // Cleanup temporary buffers
   delete[] inAudioBufs;

   return ret;
}

/* ============================ FUNCTIONS ================================= */

