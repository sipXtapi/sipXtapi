//
// Copyright (C) 2008 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

// SYSTEM INCLUDES
#include <string.h>

// APPLICATION INCLUDES
#include "mp/MpVadBase.h"
#include "mp/MpVadSimple.h"

// DEFINES
#define ENERGY_SHIFT    8

// CONSTANTS
#define MIN_SPEECH_ENERGY_THRESHOLD 20000
#define MAX_SPEECH_ENERGY_THRESHOLD 70000

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char *MpVadSimple::name = "Simple";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpVadSimple::MpVadSimple()
: mLastFrameSize(0)
, mLastEnergy(0)
, mSamplesPerSecond(0)
{

}

OsStatus MpVadSimple::init(int samplesPerSec)
{
   mLastFrameSize = 0;
   mLastEnergy = 0;
   mSamplesPerSecond = samplesPerSec;

   mMinEnergy = MIN_SPEECH_ENERGY_THRESHOLD;

   return OS_SUCCESS;
}

MpVadSimple::~MpVadSimple()
{

}

void MpVadSimple::reset()
{
   mSamplesPerSecond = 0;
}

/* ============================ MANIPULATORS ============================== */

void MpVadSimple::setMinimumEnergy(int minEnergy)
{
   mMinEnergy = minEnergy;
}

int MpVadSimple::getEnergy() const
{
   if ((mSamplesPerSecond == 0) || (mLastFrameSize == 0))
      return 0;
   return (mLastEnergy/mLastFrameSize) << ENERGY_SHIFT;
}


MpSpeechType MpVadSimple::processFrame(uint32_t packetTimeStamp,
                                       const MpAudioSample* pBuf,
                                       unsigned inSamplesNum,
                                       const MpSpeechParams &speechParams,
                                       UtlBoolean calcEnergyOnly)
{
   int i;
   int energy = 0;

   if (mSamplesPerSecond == 0)
      return MP_SPEECH_UNKNOWN;

   mLastFrameSize = inSamplesNum;

   for (i = 0; i < mLastFrameSize; i++)
      energy += ((int)pBuf[i] * (int)pBuf[i]) >> ENERGY_SHIFT;

   mLastEnergy = energy;

   if (calcEnergyOnly)
   {
      return speechParams.mSpeechType;
   }

   if (energy > (mMinEnergy >> ENERGY_SHIFT))
      return MP_SPEECH_ACTIVE;

   return MP_SPEECH_SILENT;
}
