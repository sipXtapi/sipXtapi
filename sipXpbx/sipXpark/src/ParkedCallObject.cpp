// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "ParkedCallObject.h"
#include <net/Url.h>
#include <os/OsFS.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ParkedCallObject::ParkedCallObject(CallManager* callManager, UtlString callId, UtlString playFile)
{
   mpCallManager = callManager;
   mCallId = callId;
   mAddress = NULL;
   
   mpPlayer = NULL;
   mFile = playFile;
}


ParkedCallObject::~ParkedCallObject()
{
   mpCallManager = NULL;
}
   

void ParkedCallObject::setAddress(UtlString address)
{
   mAddress = address;
}


UtlString ParkedCallObject::getAddress()
{
   return mAddress;
}


OsStatus ParkedCallObject::playAudio()
{
   OsStatus result = OS_SUCCESS;

   OsSysLog::add(FAC_ACD, PRI_DEBUG, "CallId %s is requesting to play the wavefile", mCallId.data());

   // Create an audio player and queue up the audio to be played.
   mpCallManager->createPlayer(MpPlayer::STREAM_PLAYER, mCallId, mFile.data(), STREAM_SOUND_REMOTE | STREAM_FORMAT_WAV, &mpPlayer) ;

   if (mpPlayer == NULL)
   {
      OsSysLog::add(FAC_ACD, PRI_ERR, "CallId %s: Failed to create player", mCallId.data());
      return OS_FAILED;
   }

   mpPlayer->setLoopCount(-1);    // Play for ever.

   if (mpPlayer->realize(TRUE) != OS_SUCCESS)
   {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ParkedCallObject::playAudio - CallId %s: Failed to realize player", mCallId.data());
      cleanUp();
      return OS_FAILED;
   }

   if (mpPlayer->prefetch(TRUE) != OS_SUCCESS)
   {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ParkedCallObject::playAudio - CallId %s: Failed to prefetch player", mCallId.data());
      cleanUp();
      return OS_FAILED;
   }

   if (mpPlayer->play(FALSE) != OS_SUCCESS)
   {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ParkedCallObject::playAudio - CallId %s: Failed to play", mCallId.data());
      cleanUp();
      return OS_FAILED;
   }

   return result;
}


void ParkedCallObject::cleanUp()
{
   if (mpPlayer)
      mpCallManager->destroyPlayer(MpPlayer::STREAM_PLAYER, mCallId, mpPlayer);

   mpPlayer = NULL;
}
