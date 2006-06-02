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
ParkedCallObject::ParkedCallObject(UtlString& orbit, CallManager* callManager, 
                                   UtlString callId, UtlString playFile, bool bPickup)
{
   mOrbit = orbit;
   mpCallManager = callManager;
   mCallId = callId;
   mbPickup = bPickup;
   mAddress = NULL;
   
   mpPlayer = NULL;
   mFile = playFile;
   mPickupCallId = NULL;
   
   OsDateTime::getCurTime(mParked);
}


ParkedCallObject::~ParkedCallObject()
{
   if (mpPlayer)
      mpCallManager->destroyPlayer(MpPlayer::STREAM_PLAYER, mCallId, mpPlayer);

   mNewCallIds.destroyAll();   
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
      return OS_FAILED;
   }

   if (mpPlayer->prefetch(TRUE) != OS_SUCCESS)
   {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ParkedCallObject::playAudio - CallId %s: Failed to prefetch player", mCallId.data());
      return OS_FAILED;
   }

   if (mpPlayer->play(FALSE) != OS_SUCCESS)
   {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ParkedCallObject::playAudio - CallId %s: Failed to play", mCallId.data());
      return OS_FAILED;
   }
   OsSysLog::add(FAC_ACD, PRI_DEBUG, "ParkedCallObject::playAudio - Successful");

   return result;
}

UtlString ParkedCallObject::getPickupCallId()
{
   return mPickupCallId;
}

void ParkedCallObject::setPickupCallId(const char* callId)
{
   mPickupCallId = callId;
}

UtlSList* ParkedCallObject::getNewCallIds()
{
   return &mNewCallIds;
}

void ParkedCallObject::setNewCallId(const char* callId)
{
   UtlString *pEntry = new UtlString(callId);
   
   mNewCallIds.insert(pEntry);
}

UtlString ParkedCallObject::getOrbit()
{
   return mOrbit;
}
   
void ParkedCallObject::getTimeParked(OsTime& parked)
{
   parked = mParked;
}

bool ParkedCallObject::isPickupCall()
{
   return mbPickup;
}

void ParkedCallObject::setOriginalAddress(UtlString& address)
{
   mOriginalAddress = address;
}

UtlString ParkedCallObject::getOriginalAddress()
{
   return mOriginalAddress;
}

bool ParkedCallObject::hasNewCallIds()
{
   return !mNewCallIds.isEmpty();
}