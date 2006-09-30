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
#include "OrbitFileReader.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ParkedCallObject::ParkedCallObject(UtlString& orbit, CallManager* callManager, 
                                   UtlString callId, UtlString playFile, bool bPickup) :
   mTimeoutNotification(this),
   mTimeoutTimer(mTimeoutNotification)
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
   // Stop any timeout timer.
   stopEscapeTimer();

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

   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "CallId %s is requesting to play the audio file",
                 mCallId.data());

   // Create an audio player and queue up the audio to be played.
   mpCallManager->createPlayer(MpPlayer::STREAM_PLAYER, mCallId, mFile.data(), STREAM_SOUND_REMOTE | STREAM_FORMAT_WAV, &mpPlayer) ;

   if (mpPlayer == NULL)
   {
      OsSysLog::add(FAC_PARK, PRI_ERR, "CallId %s: Failed to create player", mCallId.data());
      return OS_FAILED;
   }

   mpPlayer->setLoopCount(-1);    // Play for ever.

   if (mpPlayer->realize(TRUE) != OS_SUCCESS)
   {
      OsSysLog::add(FAC_PARK, PRI_ERR, "ParkedCallObject::playAudio - CallId %s: Failed to realize player", mCallId.data());
      return OS_FAILED;
   }

   if (mpPlayer->prefetch(TRUE) != OS_SUCCESS)
   {
      OsSysLog::add(FAC_PARK, PRI_ERR, "ParkedCallObject::playAudio - CallId %s: Failed to prefetch player", mCallId.data());
      return OS_FAILED;
   }

   if (mpPlayer->play(FALSE) != OS_SUCCESS)
   {
      OsSysLog::add(FAC_PARK, PRI_ERR, "ParkedCallObject::playAudio - CallId %s: Failed to play", mCallId.data());
      return OS_FAILED;
   }
   OsSysLog::add(FAC_PARK, PRI_DEBUG, "ParkedCallObject::playAudio - Successful");

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

// Consider starting the time-out timer, which if it expires, will transfer
// the call back to the user that parked it.
void ParkedCallObject::startEscapeTimer(UtlString& parker,
                                        int timeout)
{
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "ParkedCallObject::startEscapeTimer callId = '%s', parker = '%s', timeout = %d",
                 mCallId.data(), parker.data(), timeout);

   // First, validate the parker URI and timeout value.
   if (parker.isNull())
   {
      return;
   }
   if (timeout == OrbitData::NO_TIMEOUT)
   {
      return;
   }
   // Here, we can insert further validation, such as that the parker URI
   // is local.

   // At this point, we have decided to activate the timeout for this call.

   // Save the parker URI.
   mParker = parker;
   OsTime timeoutOsTime(timeout, 0);
   mTimeoutTimer.oneshotAfter(timeoutOsTime);
}

// Consider starting the time-out timer, which if it expires, will transfer
// the call back to the user that parked it.
void ParkedCallObject::stopEscapeTimer()
{
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "ParkedCallObject::stopEscapeTimer callId = '%s'",
                 mCallId.data());
}

// Do a blind transfer of this call to mParker.
void ParkedCallObject::timeout()
{
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "ParkedCallObject::timeout callId = '%s', parker = '%s'",
                 mCallId.data(), mParker.data());
   mpCallManager->transfer_blind(mCallId, mParker, NULL, NULL);
}

// Constructor for ParkedCallTimeoutNotification.
ParkedCallTimeoutNotification::ParkedCallTimeoutNotification(ParkedCallObject* const parkedCall) :
   mpParkedCall(parkedCall)
{
}

// Destructor for ParkedCallTimeoutNotification.
ParkedCallTimeoutNotification::~ParkedCallTimeoutNotification()
{
}

// Signal method.
OsStatus ParkedCallTimeoutNotification::signal(const int eventData)
{
   // Call the ParkedCallObject to do the transfer.
   mpParkedCall->timeout();
   return OS_SUCCESS;
}
