// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "OrbitListener.h"
#include "ParkedCallObject.h"
#include <tao/TaoMessage.h>
#include <tao/TaoString.h>
#include <cp/CallManager.h>
#include <net/Url.h>
#include <os/OsFS.h>
#include <os/OsDateTime.h>
#include "xmlparser/tinyxml.h"
#include "xmlparser/ExtractContent.h"
#include <utl/UtlHashMapIterator.h>
#include "os/OsMsg.h"
#include "os/OsEventMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define TAO_OFFER_PARAM_CALLID             0
#define TAO_OFFER_PARAM_ADDRESS            2
#define TAO_OFFER_PARAM_LOCAL_CONNECTION   6
#define TAO_OFFER_PARAM_NEW_CALLID        11
// This is the argument in a MULTICALL_META_TRANSFER_ENDED event that
// contains the callId of the call that was being transferred.
// (TA_OFFER_PARAM_CALLID is the callId of the new call created to
// represent the transfer operation.)  I don't know if this is truly fixed
// by the design, but it works consistently.
#define TAO_TRANSFER_ENDED_ORIGINAL_CALLID 12

#define DEFAULT_MOH_AUDIO                  "file://"SIPX_PARKMUSICDIR"/default.wav"

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OrbitListener::OrbitListener(CallManager* callManager) :
   TaoAdaptor("OrbitListener-%d")
{
   mpCallManager = callManager;

   // Assemble the full file name of the orbit file and initialize the reader.
   mOrbitFileReader.setFileName(SIPX_CONFDIR + OsPathBase::separator +
                                ORBIT_CONFIG_FILENAME);

   // Start the dummy listener, so that it will process messages (and its queue
   // will not fill up).
   mListener.start();
}


// Destructor
OrbitListener::~OrbitListener()
{
   // We should dispose of all the parked calls.  But what is the correct
   // method?  Should we just drop them, or try to transfer them back to
   // their parkers?
   // :TODO:  Not done yet because sipXpark never destroys its OrbitListener.
}


/* ============================ MANIPULATORS ============================== */

UtlBoolean OrbitListener::handleMessage(OsMsg& rMsg)
{
   OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - message type %d subtype %d",
                 rMsg.getMsgType(), rMsg.getMsgSubType());

   // React to telephony events
   if (rMsg.getMsgType() == OsMsg::TAO_MSG &&
       rMsg.getMsgSubType() == TaoMessage::EVENT)
   {
      TaoMessage* taoMessage = (TaoMessage*)&rMsg;

      TaoObjHandle taoEventId = taoMessage->getTaoObjHandle();
      UtlString argList(taoMessage->getArgList());
      TaoString arg(argList, TAOMESSAGE_DELIMITER);

      if (OsSysLog::willLog(FAC_PARK, PRI_DEBUG))
      {
         // Dump the contents of the Tao message.
         dumpTaoMessageArgs(taoEventId, arg) ;
      }
      UtlBoolean localConnection = atoi(arg[TAO_OFFER_PARAM_LOCAL_CONNECTION]);
      UtlString  callId = arg[TAO_OFFER_PARAM_CALLID] ;
      UtlString  address = arg[TAO_OFFER_PARAM_ADDRESS] ;

      switch (taoEventId) 
      {
         case PtEvent::CONNECTION_OFFERED:
         {
            OsSysLog::add(FAC_PARK, PRI_DEBUG,
                          "OrbitListener::handleMessage - Call offered: callId: '%s', address: '%s'\n", 
                          callId.data(), address.data());

            // Is this a retrieval call?
            if (isCallRetrievalInvite(callId.data(), address.data()))
            {
               // Accept the call.
               mpCallManager->acceptConnection(callId, address);
               mpCallManager->answerTerminalConnection(callId, address, "*");
               // We don't have to add a DTMF listener, as this call will
               // be turned into a transfer as soon as it is established.
            }
            else
            {
               // Validate the orbit and get the information about the orbit.
               UtlString orbit;
               UtlString audio;
               int timeout, keycode, capacity;
               if (validateOrbit(callId, address, orbit, audio,
                                 timeout, keycode, capacity) != OS_SUCCESS)
               {
                  // This should be a 404 error, but there is no provision for
                  // specifying that yet.
                  mpCallManager->rejectConnection(callId.data(),
                                                  address.data());
               }
               else
               {
                  // Check that the orbit is below the maximum number of calls
                  // allowed.
                  int calls_in_orbit = getNumCallsInOrbit(orbit);
                  if (calls_in_orbit >= capacity)
                  {
                     // rejectConnection() produces a 486 error on a ringing
                     // call.
                     mpCallManager->rejectConnection(callId.data(),
                                                     address.data());
                  }
                  else
                  {
                     // Accept the call.
                     mpCallManager->acceptConnection(callId, address);
                     mpCallManager->answerTerminalConnection(callId, address,
                                                             "*");

                     // Set up the data structures for this dialog.
                     setUpParkedCall(callId, address,
                                     orbit, audio, timeout, keycode, capacity,
                                     arg);
                  }
               }
            }
         }
      
         break;

         case PtEvent::CONNECTION_ESTABLISHED:
            if (localConnection) 
            {
               OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - Call established: callId '%s', address: '%s'",
                             callId.data(), address.data());
               
               // CallId at this point is either the callId of a normal call or the original callId
               // of a second leg of a transfer. 

               // Test if this is a call retrieval operation
               if (!isCallRetrievalInvite(callId.data(), address.data()))
               {
                  // If this is not a call retrieval call, then all the data
                  // structure setup was done by setUpParkedCall() when
                  // the call was offered.
               }
               else
               {
                  // Do the work for a call-retrieval call.
                  setUpRetrievalCall(callId, address);
               }
            }

            break;
            

         case PtEvent::CONNECTION_DISCONNECTED:
            if (!localConnection)
            {
               OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - Call disconnected: callId %s, address: %s",
                             callId.data(), address.data());

               // See if the callId is in our list and if the address matches.
               UtlVoidPtr* container = dynamic_cast <UtlVoidPtr *> (mCalls.findValue(&callId));
               if (container)
               {
                  ParkedCallObject* pDroppedCall = (ParkedCallObject *) container->getValue();
                  OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - found call object %p for %s\n",
                                pDroppedCall, callId.data());

                  if (pDroppedCall == NULL)
                  {
                     OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - No callId %s found in the active call list\n",
                                   callId.data());
                  }
                  else if (address.compareTo(pDroppedCall->getAddress()) == 0)
                  {
                     // Are there new CallIds attached to this call? Then clean them up too.
                     if (pDroppedCall->hasNewCallIds())
                     {
                        OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - Call has new call id list");
                        // Retrieve the list of new callIds from the call object and clean them up one by one
                        UtlSList* newCallIds = pDroppedCall->getNewCallIds();
                        UtlSListIterator iterator(*newCallIds);
                        UtlString* pCallId = NULL;
                       
                        while ((pCallId = dynamic_cast<UtlString*>(iterator())))
                        {
                           OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - cleaning up new callId '%s'", pCallId->data());
                           
                           UtlVoidPtr* newContainer = dynamic_cast <UtlVoidPtr *> (mCalls.findValue(pCallId));   
                           
                           if (newContainer)
                           {
                              ParkedCallObject* pNewCall = (ParkedCallObject*) newContainer->getValue();
                              if (pNewCall)
                              {
                                 mCalls.destroy(pCallId);
                                 OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage call objects in list : %d", mCalls.entries());                              
                                
                                 delete pNewCall;
                         
                                 // Don't tell the call manager to drop the call here, the call manager would not know
                                 // how to handle a drop() call with this callId.
                              }
                           }
                           else
                           {
                              OsSysLog::add(FAC_PARK, PRI_WARNING, "OrbitListener::handleMessage - Could not find call object for new call '%s'",
                                            pCallId->data());
                           }
                        }                     
                     }
                     // Remove the call from the pool and clean up the call. This does not destroy the pDrpopedCall,
                     // it destroys the UtlVoidPtr that points to it.
                     mCalls.destroy(&callId);
                     OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage call objects in list : %d", mCalls.entries());

                     OsSysLog::add(FAC_SIP, PRI_DEBUG, "OrbitListener::handleMessage remove call object %p from the list",
                                   pDroppedCall);
                     delete pDroppedCall;
   
                     // Drop the call
                     mpCallManager->drop(callId);
                  }
                  else
                  {
                     OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - Address mismatch passed in '%s' / stored '%s'",
                                   address.data(), pDroppedCall->getAddress().data());
                  }
               }
               else
               {
                  OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - No callId %s found in the active call list\n",
                                callId.data());                  
               }
            }

            break;
            

         case PtEvent::CONNECTION_FAILED:
         {
            OsSysLog::add(FAC_PARK, PRI_WARNING, "OrbitListener::handleMessage - Connection failed: %s\n", callId.data());
            
            ParkedCallObject* pDroppedCall = NULL;
            UtlVoidPtr* container = dynamic_cast <UtlVoidPtr *> (mCalls.findValue(&callId));
         
            if (container != NULL)
            {
               pDroppedCall = (ParkedCallObject *) container->getValue();
               
               if (pDroppedCall)
               {
                  UtlString pickupCallId = pDroppedCall->getPickupCallId();
                  // See if we have an executor attached to the call - drop that too if connection failed
                  if (pickupCallId.length() != 0)
                  {
                     UtlVoidPtr* pickupContainer = dynamic_cast <UtlVoidPtr *> (mCalls.findValue(&pickupCallId));   
                     
                     if (pickupContainer)
                     {
                        ParkedCallObject* pPickupCall = (ParkedCallObject*) pickupContainer->getValue();
                        if (pPickupCall)
                        {
                           OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - Found attached  pickup call %s - cleaning that up too", 
                                         pickupCallId.data()); 
                           mCalls.destroy(&pickupCallId);
                           
                           delete pPickupCall;
                   
                           mpCallManager->drop(pickupCallId);
                        }
                     }
                     else
                     {
                        OsSysLog::add(FAC_PARK, PRI_WARNING, "OrbitListener::handleMessage - Could not find call object for pickup call '%s'",
                                      pickupCallId.data());
                     }
                     // Are there new CallIds attached to this call? Then clean them up too.
                     if (pDroppedCall->hasNewCallIds())
                     {
                        OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - Call has new call id list");
                        // Retrieve the list of new callIds from the call object and clean them up one by one
                        UtlSList* newCallIds = pDroppedCall->getNewCallIds();
                        UtlSListIterator iterator(*newCallIds);
                        UtlString* pCallId = NULL;
                       
                        while ((pCallId = dynamic_cast<UtlString*>(iterator())))
                        {
                           OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - cleaning up new callId '%s'", pCallId->data());
                           
                           UtlVoidPtr* newContainer = dynamic_cast <UtlVoidPtr *> (mCalls.findValue(pCallId));   
                           
                           if (newContainer)
                           {
                              ParkedCallObject* pNewCall = (ParkedCallObject*) newContainer->getValue();
                              if (pNewCall)
                              {
                                 mCalls.destroy(pCallId);
                                 OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage call objects in list : %d", mCalls.entries());                              
                                 
                                 delete pNewCall;
                         
                                 // Don't tell the call manager to drop the call here, the call manager would not know
                                 // how to handle a drop() call with this callId.
                              }
                           }
                           else
                           {
                              OsSysLog::add(FAC_PARK, PRI_WARNING, "OrbitListener::handleMessage - Could not find call object for new call '%s'",
                                            pCallId->data());
                           }
                        }                     
                     }
                  }                 
                  // Remove the call from the pool and clean up the call
                  mCalls.destroy(&callId);

                  OsSysLog::add(FAC_SIP, PRI_DEBUG, "OrbitListener::handleMessage remove call object %p from the list due to failed connection",
                                pDroppedCall);
                                
                  delete pDroppedCall;
               }      
            }                                             
            
            mpCallManager->drop(callId);
         }
         break;


         case PtEvent::MULTICALL_META_TRANSFER_ENDED:
            // A transfer operation ended.  Re-enable processing of
            // transfer requests for this call.
            if (localConnection)
            {
               UtlString originalCallId =
                  arg[TAO_TRANSFER_ENDED_ORIGINAL_CALLID];
               OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - Transfer ended: callId %s, original callid: %s",
                             callId.data(), originalCallId.data());

               // See if the originalCallId is in our list and if the address matches.
               UtlVoidPtr* container =
                  dynamic_cast <UtlVoidPtr *> (mCalls.findValue(&originalCallId));
               if (container)
               {
                  ParkedCallObject* pCall = (ParkedCallObject *) container->getValue();
                  OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - found call object %p for %s\n",
                                pCall, callId.data());
                  if (pCall == NULL)
                  {
                     OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - No callId %s found in the active call list\n",
                                   originalCallId.data());
                  }
                  else
                  {
                     // Clear the transfer-in-progress indicator.
                     pCall->clearTransfer();
                  }
               }
               else
               {
                  OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - No callId %s found in the active call list\n",
                                originalCallId.data());                  
               }
            }

            break;
      }
   } else if (rMsg.getMsgType() == OsMsg::OS_EVENT &&
              rMsg.getMsgSubType() == OsEventMsg::NOTIFY)
   {
      // An event notification.

      // The userdata of the original OsQueuedEvent was copied by
      // OsQueuedEvent::signal() into the userdata of this OsEventMsg.
      // The userdata is the mSeqNo of the ParkedCallObject, plus an
      // "enum notifyCodes" value indicating what sort of event it was.
      int userData;
      OsEventMsg* pEventMsg = dynamic_cast <OsEventMsg*> (&rMsg);
      pEventMsg->getUserData(userData);
      int seqNo;
      enum ParkedCallObject::notifyCodes type;
      ParkedCallObject::splitUserData(userData, seqNo, type);
      ParkedCallObject* pParkedCallObject = findBySeqNo(seqNo);

      if (pParkedCallObject)
      {
         switch (type)
         {
         case ParkedCallObject::DTMF:
         {
            // Get the keycode.
            int keycode;
            pEventMsg->getEventData(keycode);
            keycode >>= 16;
            keycode &= 0xFFFF;

            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "OrbitListener::handleMessage DTMF "
                          "keycode %d, ParkedCallObject = %p",
                          keycode, pParkedCallObject);

            // :WORKAROUND:  Delay 1/2 second to allow the user time to let up on the
            // key.  This avoids a bug in Polycom ver 2.0 which is triggered when a key
            // is down and the other end puts the call on hold and takes it off.
            OsTask::delay(500);

            // Call the ParkedCallObject to process the keycode, which may
            // start a transfer..
            pParkedCallObject->keypress(keycode);
         }
         break;

         case ParkedCallObject::TIMEOUT:
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "OrbitListener::handleMessage TIMEOUT "
                          "ParkedCallObject = %p",
                          pParkedCallObject);

            // Call the ParkedCallObject to start the transfer.
            pParkedCallObject->startTransfer();
            break;
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_INFO,
                       "OrbitListener::handleMessage ParkedCallObject(seqNo = %d) "
                       "deleted while message in queue",
                       seqNo);
      }
   }

   return TRUE;
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


OsStatus OrbitListener::validateOrbit(const UtlString& callId,
                                      const UtlString& address,
                                      UtlString& orbit,
                                      UtlString& audio,
                                      int& timeout,
                                      int& keycode,
                                      int& capacity)
{
   // Retrieve the request URI
   SipDialog dialog;               
   mpCallManager->getSipDialog(callId,
                               address,
                               dialog);
   UtlString request;   
   dialog.getRemoteRequestUri(request);
   
   // Get orbit from request Uri.
   Url requestUri(request, TRUE);
   requestUri.getUserId(orbit);
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitListener::validateOrbit request URI '%s', orbit '%s'",
                 request.data(), orbit.data());

   audio = "";

   // See if this is a music-on-hold call or an orbit call

   if (orbit == "moh")
   {
      mOrbitFileReader.getMusicOnHoldFile(audio);
      timeout = OrbitData::NO_TIMEOUT;
      keycode = OrbitData::NO_KEYCODE;
      capacity = OrbitData::UNLIMITED_CAPACITY;
   }
   else
   {
      OrbitData* orbit_data = mOrbitFileReader.findInOrbitList(orbit);
      if (orbit_data)
      {
         audio = orbit_data->mAudio;
         timeout = orbit_data->mTimeout;
         keycode = orbit_data->mKeycode;
         capacity = orbit_data->mCapacity;
      }
   }

   if (audio != "")
   {
      return OS_SUCCESS;
   }
   else
   {
      // Check if this is for MOH.  If so, use the default audio file.
      if (orbit == "moh")
      {
         audio = DEFAULT_MOH_AUDIO;
         return OS_SUCCESS;
      }
      else
      {
         return OS_FAILED;
      }
   }
}


ParkedCallObject* OrbitListener::getOldestCallInOrbit(UtlString& orbit, UtlString& callId, UtlString& address)
{
   UtlVoidPtr* pContainer;
   ParkedCallObject* pCall;
   ParkedCallObject* pReturn = NULL;  
   UtlString *pKey;
   UtlString oldestKey("");
   UtlString oldestAddress("");
   OsTime oldest;
   OsTime parked;
   bool bRemovedInvalidObject = true;
   
   OsDateTime::getCurTime(oldest);
   
   // Loop while removing invalid entries. It seems the elements of the UtlHashMap become inaccessible
   // after removing an entry. In this case restart the iteration.
   while (bRemovedInvalidObject)
   {
      UtlHashMapIterator iterator(mCalls);      
      bRemovedInvalidObject = false;
      // Loop through the call list       
      while ((pKey = dynamic_cast<UtlString*>(iterator())))
      {
         pContainer = dynamic_cast<UtlVoidPtr*>(iterator.value());
         if (pContainer)
         {
            pCall = (ParkedCallObject*)pContainer->getValue();
            if (pCall)
            {
               OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::getOldestCallInOrbit - Entry for callId '%s', orbit '%s'",
                                                  pKey->data(), pCall->getOrbit().data());                   
               // Found a call, make sure that the orbit matches, that it's older than anything seen so far and 
               // that it is not a retrieval call.
               pCall->getTimeParked(parked);
               if (orbit.compareTo(pCall->getOrbit()) == 0 &&
                   parked <= oldest &&
                   !pCall->isPickupCall())
               {
                  SipDialog dialog;
                  // Check if the call manager knows about this callId, remove from list if not.
                  // Another way to check if this callId is valid is to test the returned callId in the SipDialog.
                  // Sometimes getSipDialog does not fail but returns with an empty callId.               
                  OsStatus ret = mpCallManager->getSipDialog(pKey->data(), pCall->getAddress(), dialog);
                  UtlString tCallId;
                  dialog.getCallId(tCallId);               
                  
                  if (ret == OS_SUCCESS && !tCallId.isNull())
                  {
                     // Copy 'new' oldest data 
                     oldest = parked;
                     oldestKey = pKey->data();
                     oldestAddress = pCall->getAddress();
                     pReturn = pCall;
                  
                     OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::getOldestCallInOrbit - Valid callId '%s', address '%s'",
                                   oldestKey.data(), oldestAddress.data());
                  }
                  else
                  {
                     OsSysLog::add(FAC_PARK, PRI_ERR, "OrbitListener::getOldestCallInOrbit - Unknown callId '%s', remove from list",
                                   pKey->data());
                     mCalls.destroy(pKey);
                     delete pCall;
                     bRemovedInvalidObject = true;
                     break;
                  }
               }
            }
            else
            {
               OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::getOldestCallInOrbit - No call object found for callId '%s'", 
                             pKey->data());                    
            }
         }
         else
         {
            OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::getOldestCallInOrbit - No container found for callId '%s'", 
                          pKey->data());                     
         }
      }
   }
   OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::getOldestCallInOrbit - Returning pCall %p, callId '%s', address '%s'",
                 pReturn, oldestKey.data(), oldestAddress.data());   
   callId = oldestKey;
   address = oldestAddress;
   return pReturn;
}


int OrbitListener::getNumCallsInOrbit(UtlString& orbit)
{
   UtlVoidPtr* pContainer;
   ParkedCallObject* pCall;
   UtlString *pKey;
   int callsInOrbit = 0;
   bool bRemovedInvalidObject = true;
   
   // Loop while removing invalid entries. It seems the elements of the UtlHashMap become inaccessible
   // after removing an entry. In this case restart the iteration.
   while (bRemovedInvalidObject)
   {
      UtlHashMapIterator iterator(mCalls);      
      bRemovedInvalidObject = false;
      // Loop through the call list       
      while ((pKey = dynamic_cast<UtlString*>(iterator())))
      {
         pContainer = dynamic_cast<UtlVoidPtr*>(iterator.value());
         if (pContainer)
         {
            pCall = (ParkedCallObject*)pContainer->getValue();
            if (pCall)
            {
               OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::getNumCallsInOrbit - Entry for callId '%s', orbit '%s'",
                                                  pKey->data(), pCall->getOrbit().data());                   
               // Found a call, make sure that the orbit matches and 
               // that it is not a retrieval call.
               if (orbit.compareTo(pCall->getOrbit()) == 0 &&
                   !pCall->isPickupCall())
               {
                  SipDialog dialog;
                  // Check if the call manager knows about this callId, remove from list if not.
                  // Another way to check if this callId is valid is to test the returned callId in the SipDialog.
                  // Sometimes getSipDialog does not fail but returns with an empty callId.               
                  OsStatus ret = mpCallManager->getSipDialog(pKey->data(), pCall->getAddress(), dialog);
                  UtlString tCallId;
                  dialog.getCallId(tCallId);               
                  
                  if (ret == OS_SUCCESS && !tCallId.isNull())
                  {
                     // Increment the count of calls in this orbit.
                     callsInOrbit++;
                     OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::getNumCallsInOrbit - Valid callId '%s', address '%s'",
                                   pKey->data(), pCall->getAddress().data());
                  }
                  else
                  {
                     OsSysLog::add(FAC_PARK, PRI_ERR, "OrbitListener::getNumCallsInOrbit - Unknown callId '%s', remove from list",
                                   pKey->data());
                     mCalls.destroy(pKey);
                     delete pCall;
                     bRemovedInvalidObject = true;
                     break;
                  }
               }
            }
            else
            {
               OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::getNumCallsInOrbit - No call object found for callId '%s'", 
                             pKey->data());                    
            }
         }
         else
         {
            OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::getNumCallsInOrbit - No container found for callId '%s'", 
                          pKey->data());                     
         }
      }
   }
   OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::getNumCallsInOrbit - Returning %d",
                 callsInOrbit);
   return callsInOrbit;
}


ParkedCallObject* OrbitListener::findBySeqNo(int seqNo)
{
   UtlString *pKey;
   UtlVoidPtr* pContainer;
   ParkedCallObject* pCall;
   ParkedCallObject* ret;
   UtlBoolean found;
   
   UtlHashMapIterator iterator(mCalls);      
   // Loop through the call list.
   ret = NULL;
   found = FALSE;
   while (!found &&
          (pKey = dynamic_cast<UtlString*>(iterator())))
   {
      pContainer = dynamic_cast<UtlVoidPtr*>(iterator.value());
      if (pContainer)
      {
         pCall = (ParkedCallObject*)pContainer->getValue();
         if (pCall)
         {
            // Found a call, check if the seqNo matches.
            if (seqNo == pCall->getSeqNo())
            {
               ret = pCall;
               found = TRUE;
            }
         }
      }
   }

   return ret;
}


bool OrbitListener::isCallRetrievalInvite(const char* callId,
                                          const char* address)
{
   bool bRet = false;
   
   // Retrieve the request URI
   SipDialog dialog;               
   mpCallManager->getSipDialog(callId,
                               address,
                               dialog);
   UtlString request;   
   dialog.getRemoteRequestUri(request);
                               
   OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::isCallPickupInvite remote request URI %s", request.data());
   
   Url requestURI(request, TRUE);
   UtlString urlParam;

   if (requestURI.getUrlParameter("operation", urlParam))
   {
      if (urlParam.compareTo("retrieve", UtlString::ignoreCase) == 0)
      {
         bRet = true;         
         OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::isCallPickupInvite found retrieve parameter");         
      }
   }
   else
   {
      OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::isCallPickupInvite no operation URL parameter");                  
   }
   return bRet;
}


void OrbitListener::dumpTaoMessageArgs(TaoObjHandle eventId, TaoString& args) 
{
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitListener::dumpTaoMessageArgs Tao event id: %d local: %s args: %d",
                 eventId, args[TAO_OFFER_PARAM_LOCAL_CONNECTION], args.getCnt()) ;
        
   int argc = args.getCnt();
   for(int argIndex = 0; argIndex < argc; argIndex++)
   {
      OsSysLog::add(FAC_PARK, PRI_DEBUG, "\targ[%d]=\"%s\"", argIndex, args[argIndex]);
   }
}


// Set up the data structures for a new call that isn't a call retrieval call.
// It may be a new call to be parked, or a new dialog that replaces a dialog
// that was parked.
void OrbitListener::setUpParkedCall(const UtlString& callId,
                                    const UtlString& address,
                                    const UtlString& orbit,
                                    const UtlString& audio,
                                    int timeout,
                                    int keycode,
                                    int capacity,
                                    const TaoString& arg)
{
   // Add a dummy DTMF listener to the call.
   // This listener doesn't process DTMF, but the
   // addToneListener() seems to be needed to
   // activate DTMF listening via enableDtmfEvent().
   mpCallManager->addToneListener(callId.data(), (int) &mListener);

   // Get a copy of the INVITE that we received or sent
   // to start this dialog.  Look for a Referred-By
   // header, which indicates that this is a parked
   // dialog and needs to have a timer set.  (If there
   // is no Referred-By, the caller may have gotten
   // here by direct dialing, or this may be an initial
   // leg in a consultative transfer.)
   SipMessage invite;
   UtlString parker;
   OsStatus s = mpCallManager->getInvite(callId.data(),
                                         address.data(),
                                         invite);
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitListener::setUpParkedCall getInvite returned %d",
                 s);
   if (s == OS_SUCCESS)
   {
      if (OsSysLog::willLog(FAC_PARK, PRI_DEBUG))
      {
         UtlString text;
         int length;
         invite.getBytes(&text, &length);
         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "OrbitListener::setUpParkedCall getInvite: INVITE is '%s'",
                       text.data());
      }
      invite.getReferredByField(parker);
      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                    "OrbitListener::setUpParkedCall parker = '%s'",
                    parker.data());
   }
   // If !isNull.parker(), then this dialog has been parked by
   // the user with URI 'parker'.

   // Look up callId in list of calls to see if this is
   // a replacement of a call we already have parked.
   UtlVoidPtr* container =
      dynamic_cast <UtlVoidPtr *> (mCalls.findValue(&callId));
                           
   ParkedCallObject* pThisCall =
      container == NULL ? NULL :
      (ParkedCallObject *) container->getValue();
   if (pThisCall == NULL)
   {
      //
      // This is a new call.
      //
      pThisCall = new ParkedCallObject(orbit, mpCallManager,
                                       callId, audio, false,
                                       this->getMessageQueue());
      pThisCall->setAddress(address);

      // Create a player and start to play out the file
      if (pThisCall->playAudio() == OS_SUCCESS)
      {
         // Put it in the list of parked calls
         mCalls.insertKeyAndValue(new UtlString(callId),
                                  new UtlVoidPtr(pThisCall));
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "OrbitListener::setUpParkedCall inserting ParkedCallObject for new call %p",
                       pThisCall);
         // Start the time-out timer if necessary.
         pThisCall->startEscapeTimer(parker, timeout, keycode);
      }
      else
      {
         // Drop the call
         mpCallManager->drop(callId);
         OsSysLog::add(FAC_PARK, PRI_WARNING,
                       "OrbitListener::setUpParkedCall - Dropping callId '%s' because playAudio() failed",
                       callId.data());
         delete pThisCall;
      }
   }
   else
   {
      //
      // Additional leg of transfer. This call replaces the previous call.
      //
      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                    "OrbitListener::setUpParkedCall - change the call address from %s to %s",
                    pThisCall->getAddress().data(), address.data());
      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                    "OrbitListener::setUpParkedCall - dump pThisCall: "
                    "address '%s', original address '%s', "
                    "pickup callId '%s'",
                    pThisCall->getAddress().data(),
                    pThisCall->getOriginalAddress().data(),
                    pThisCall->getPickupCallId().data());
      // Update the remote address of the dialog
      pThisCall->setAddress(address);
                     
      // Check if Tao message has enough parameters to contain a new call Id.
      if (arg.getCnt() > TAO_OFFER_PARAM_NEW_CALLID)
      {
         UtlString newCallId = arg[TAO_OFFER_PARAM_NEW_CALLID];
         UtlString audio;
                        
         // Attach the new callId to the original call object so we can clean it up when the first leg is 
         // torn down
         pThisCall->setNewCallId(newCallId.data());                        

         // Stop any timer attached to the old call, as
         // the old call's information is now obsolete.
         pThisCall->stopEscapeTimer();

         // Put the new call in the list for later retrieval.
         ParkedCallObject* pTransferredCall =
            new ParkedCallObject(orbit, mpCallManager, newCallId, audio,
                                 false, this->getMessageQueue());
         mCalls.insertKeyAndValue(new UtlString(newCallId),
                                  new UtlVoidPtr(pTransferredCall));
         pTransferredCall->setAddress(address);

         // :TODO:
         // Because the Call Manager can't handle
         // transferring a call that resulted from an
         // INVITE-with-Replaces, we do not start the
         // timeout timer here.  But once the Call
         // Manager is fixed, we should do so.
#if 0
         pTransferredCall->startEscapeTimer(parker, timeout,
                                            keycode);
#endif
                                                
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "OrbitListener::setUpParkedCall insert transfered call object %p "
                       "with callId '%s', address '%s' into the list",
                       pTransferredCall, newCallId.data(), address.data());
      }
      else
      {
         OsSysLog::add(FAC_PARK, PRI_ERR, "OrbitListener::setUpParkedCall Tao message does not contain new callId for call '%s'", 
                       callId.data());
      }
   }
}


// Do the work for a call-retrieval call.
void OrbitListener::setUpRetrievalCall(const UtlString& callId,
                                       const UtlString& address)
{
   // This is a call retrieval
   UtlString orbit, audio;
   int timeout, keycode, capacity;
                  
   // Get Orbit out of the requestUri and use that to find the oldest call.
   validateOrbit(callId, address, orbit, audio, timeout,
                 keycode, capacity);
                  
   UtlString originalCallId;
   UtlString originalAddress;                  
   OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage retrieving parked call from orbit '%s'", orbit.data()); 
                  
   // Find oldest call in call list, return that callId and orginal address in parameters.                 
   ParkedCallObject* pThisCall =
      getOldestCallInOrbit(orbit, originalCallId, originalAddress);

   if (pThisCall)
   {
      // Attach the callId of the executor to the call to be picked up, may need this for
      // clean up if the transfer fails.
      pThisCall->setPickupCallId(callId.data());
      OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - transferring original callId %s, address %s "
                    "to callId %s, address %s",
                    originalCallId.data(), originalAddress.data(), 
                    +  callId.data(), address.data());
                                                     
      mpCallManager->transfer(originalCallId, originalAddress,
                              callId, address);
      // Add this call to the park list so it can be cleaned up correctly, mark it as
      // a call retrieval,
      pThisCall = new ParkedCallObject(orbit, mpCallManager,
                                       callId, "", true,
                                       this->getMessageQueue());
      pThisCall->setAddress(address);                                             
      mCalls.insertKeyAndValue(new UtlString(callId), new UtlVoidPtr(pThisCall));
   }
   else
   {
      // Drop the call
      mpCallManager->drop(callId);
      OsSysLog::add(FAC_PARK, PRI_WARNING, "OrbitListener::handleMessage - Drop callId %s due to not finding a parked call",
                    callId.data());
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */


// Implementation of the dummy DTMF listener.

// This is used to provide a listener for CallManager::addToneListener.
// It does noting, but the addToneListener call seems to be necessary to
// make CallManager::enableDtmfEvent work.

// Constructor
DummyListener::DummyListener() :
   TaoAdaptor("DummyListener-%d")
{
}

// Copy constructor
DummyListener::DummyListener(const DummyListener& rDummyListener)
{
}

// Destructor
DummyListener::~DummyListener()
{
}

// Assignment operator
DummyListener& 
DummyListener::operator=(const DummyListener& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean DummyListener::handleMessage(OsMsg& rMsg)
{
   // Do nothing.
   return(TRUE);
}
