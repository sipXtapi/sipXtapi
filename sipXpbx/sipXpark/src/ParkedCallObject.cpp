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

int ParkedCallObject::sNextSeqNo = 0;
const int ParkedCallObject::sSeqNoIncrement = 2;
const int ParkedCallObject::sSeqNoMask = 0x3FFFFFFE;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ParkedCallObject::ParkedCallObject(const UtlString& orbit,
                                   CallManager* callManager, 
                                   const UtlString& callId,
                                   const UtlString& playFile,
                                   bool bPickup,
                                   OsMsgQ* listenerQ) :
   mSeqNo(sNextSeqNo),
   mpCallManager(callManager),
   mCallId(callId),
   mAddress(NULL),
   mpPlayer(NULL),
   mFile(playFile),
   mPickupCallId(NULL),
   mOrbit(orbit),
   mbPickup(bPickup),
   mTimeoutTimer(listenerQ, mSeqNo + TIMEOUT),
   // Create the OsQueuedEvent to handle DTMF events.
   // This would ordinarily be an allocated object, because
   // removeDtmfEvent will delete it asynchronously.
   // But we do not need to call removeDtmfEvent, as we let call
   // teardown remove the pointer to mDtmfEvent (without attempting to
   // delete it).  We know that call teardown happens first, becausse
   // we do not delete a ParkedCallObject before knowing that it is
   // torn down.
   mDtmfEvent(*listenerQ, mSeqNo + DTMF),
   mTransferInProgress(FALSE)
{
   OsDateTime::getCurTime(mParked);
   // Update sNextSeqNo.
   sNextSeqNo = (sNextSeqNo + sSeqNoIncrement) & sSeqNoMask;
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

   mpPlayer->setLoopCount(-1);    // Play forever.

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

// Activate the escape mechanisms, if the right conditions are present.
// One is the time-out timer, which if it expires, will transfer
// the call back to the user that parked it.
// The other is the escape keycode, which lets the user transfer the call back.
// Neither mechanism is activated if there is no parker URI to transfer back
// to.  Both need appropriate configuration items in the orbits.xml file
// to be activated.
void ParkedCallObject::startEscapeTimer(UtlString& parker,
                                        int timeout,
                                        int keycode)
{
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "ParkedCallObject::startEscapeTimer callId = '%s', "
                 "parker = '%s', timeout = %d, keycode = %d",
                 mCallId.data(), parker.data(), timeout, keycode);

   // First, check that there is a parker URI.  If not, none of these
   // mechanisms can function.
   if (parker.isNull())
   {
      return;
   }
   // Here, we can insert further validation, such as that the parker URI
   // is local.

   // Save the parker URI.
   mParker = parker;

   if (timeout != OrbitData::NO_TIMEOUT)
   {
      // Set the timeout timer.
      OsTime timeoutOsTime(timeout, 0);
      // Use a periodic timer, so if the transfer generated by one timeout
      // fails, we will try again later.
      mTimeoutTimer.periodicEvery(timeoutOsTime, timeoutOsTime);
   }
   if (keycode != OrbitData::NO_KEYCODE)
   {
      // Remember the keycode for escaping.
      mKeycode = keycode;
      // Register the DTMF listener.
      // The "interdigit timeout" time of 1 is just a guess.
      mpCallManager->enableDtmfEvent(mCallId.data(), 1,
                                     &mDtmfEvent, true);
   }
}

// Stop the parking escape mechanisms.
void ParkedCallObject::stopEscapeTimer()
{
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "ParkedCallObject::stopEscapeTimer callId = '%s'",
                 mCallId.data());
   mTimeoutTimer.stop();
   if (mKeycode != OrbitData::NO_KEYCODE)
   {
      // We can't use removeDtmfEvent() here, because it would try to 
      // free mDtmfEvent.
      mpCallManager->disableDtmfEvent(mCallId.data(), (int) &mDtmfEvent);
      mKeycode = OrbitData::NO_KEYCODE;
   }
}

// Do a blind transfer of this call to mParker.
void ParkedCallObject::startTransfer()
{
   if (!mTransferInProgress)
   {
      mTransferInProgress = TRUE;
      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                    "ParkedCallObject::startTransfer starting transfer "
                    "callId = '%s', parker = '%s'",
                    mCallId.data(), mParker.data());
      mpCallManager->transfer_blind(mCallId, mParker, NULL, NULL);
   }
   else
   {
      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                    "ParkedCallObject::startTransfer transfer already in "
                    "progress callId = '%s', parker = '%s'",
                    mCallId.data(), mParker.data());
   }
}

// Signal that a transfer attempt for a call has ended.
// The transfer may or may not be successful.  (If it was successful,
// one of the UAs will terminate this call soon.)  Re-enable starting
// transfers.
void ParkedCallObject::clearTransfer()
{
   mTransferInProgress = FALSE;
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "ParkedCallObject::clearTransfer transfer cleared "
                 "callId = '%s'",
                 mCallId.data());
}

// Process a DTMF keycode for this call.
void ParkedCallObject::keypress(int keycode)
{
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "ParkedCallObject::keypress callId = '%s', parker = '%s', keycode = %d",
                 mCallId.data(), mParker.data(), keycode);
   // Must test if the keypress is to cause a transfer.
   if (mKeycode != OrbitData::NO_KEYCODE &&
       keycode == mKeycode &&
       !mParker.isNull())
   {
      mpCallManager->transfer_blind(mCallId, mParker, NULL, NULL);
   }
}
