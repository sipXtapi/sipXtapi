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

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char *MpVadSimple::name = "Simple";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

OsStatus MpVadSimple::init(int samplesPerSec)
{
   return OS_SUCCESS;
}

MpVadSimple::~MpVadSimple()
{

}

/* ============================ MANIPULATORS ============================== */

MpSpeechType MpVadSimple::processFrame(uint32_t packetTimeStamp,
                                       const MpAudioSample* pBuf,
                                       unsigned inSamplesNum)
{
   return MP_SPEECH_UNKNOWN;
}
