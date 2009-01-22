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
// APPLICATION INCLUDES
#include "mp/MpAgcBase.h"
#include "mp/MpAgcSimple.h"
#include "mp/MpDspUtils.h"
#include <os/OsSysLog.h>

static const float filteringCoeff = 0.001f;

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlString MpAgcSimple::name = "AGC Simple";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
MpAgcSimple::MpAgcSimple()
: mAdaptation(INT16_MAX)
, mAmplitude(INT16_MAX)
, mWasClipping(FALSE)
{

}

OsStatus MpAgcSimple::init(int samplerate)
{
   return OS_SUCCESS;
}

MpAgcSimple::~MpAgcSimple()
{

}

void MpAgcSimple::reset()
{
   mAdaptation = INT16_MAX;
   mAmplitude = INT16_MAX;
   mWasClipping = FALSE;
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpAgcSimple::processFrame(const MpAudioSample* data, int count)
{
   int localAmp = MpDspUtils::maxAbs(data, count);
   mAdaptation = mAdaptation * (1-filteringCoeff) + (float)localAmp * filteringCoeff;
   if (localAmp > 1.05f * mAdaptation)
      mAdaptation = (float)localAmp;

   mAmplitude = MPF_SATURATE16((MpAudioSample)mAdaptation);
   mWasClipping = (localAmp >= INT16_MAX);

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

OsStatus MpAgcSimple::getAmplitude(MpAudioSample& amp, UtlBoolean &isClipped)
{
   amp = mAmplitude;
   isClipped = mWasClipping;

   return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
