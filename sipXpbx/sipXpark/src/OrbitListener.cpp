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

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define TAO_OFFER_PARAM_CALLID             0
#define TAO_OFFER_PARAM_ADDRESS            2
#define TAO_OFFER_PARAM_LOCAL_CONNECTION   6
#define TAO_OFFER_PARAM_NEW_CALLID        11

#define DEFAULT_MOH_AUDIO                  "file://"SIPX_PARKMUSICDIR"/default.wav"

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OrbitListener::OrbitListener(CallManager* callManager)
{
   mpCallManager = callManager;

   // Assemble the full file name of the orbit file and initialize the reader.
   mOrbitFileReader.setFileName(SIPX_CONFDIR + OsPathBase::separator +
                                ORBIT_CONFIG_FILENAME);
}


// Destructor
OrbitListener::~OrbitListener()
{
}


/* ============================ MANIPULATORS ============================== */

UtlBoolean OrbitListener::handleMessage(OsMsg& rMsg)
{
   // React to telephony events
   if(rMsg.getMsgSubType()== TaoMessage::EVENT)
   {
      TaoMessage* taoMessage = (TaoMessage*)&rMsg;

      int taoEventId = taoMessage->getTaoObjHandle();
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
            OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - Call offered: callId: '%s', address: '%s'\n", 
                          callId.data(), address.data());

            // Don't attempt to validate the incoming address, as we don't
            // seem to be able to retrieve it from the CallManager at this
            // point.
            mpCallManager->acceptConnection(callId, address);
            mpCallManager->answerTerminalConnection(callId, address, "*");

            break;

         case PtEvent::CONNECTION_ESTABLISHED:
            if (localConnection) 
            {
               OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - Call established: callId '%s', address: '%s'",
                             callId.data(), address.data());
               
               ParkedCallObject* pThisCall = NULL;
               // CallId at this point is either the callId of a normal call or the original callId
               // of a second leg of a transfer. 

               // Test if this is a call retrieval operation
               if (!isCallRetrievalInvite(callId.data(), address.data()))
               {
                  // Not a call retrieval, this is a call to be
                  // parked. 

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
                                "OrbitListener::handleMessage getInvite returned %d",
                                s);
                  if (s == OS_SUCCESS)
                  {
                     if (OsSysLog::willLog(FAC_PARK, PRI_DEBUG))
                     {
                        UtlString text;
                        int length;
                        invite.getBytes(&text, &length);
                        OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                      "OrbitListener::handleMessage getInvite: INVITE is '%s'",
                                      text.data());
                     }
                     invite.getReferredByField(parker);
                     OsSysLog::add(FAC_PARK, PRI_DEBUG,
                                   "OrbitListener::handleMessage parker = '%s'",
                                   parker.data());
                  }
                  // If !isNull.parker(), then this dialog has been parked by
                  // the user with URI 'parker'.

                  // Look up callId in list of calls to see if this is
                  // a replacement of a call we already have parked.
                  UtlVoidPtr* container = dynamic_cast <UtlVoidPtr *> (mCalls.findValue(&callId));
                           
                  if (container != NULL)
                  {
                     pThisCall = (ParkedCallObject *) container->getValue();
                  }                                 
                  if (pThisCall == NULL)
                  {
                     //
                     // New call, validate.
                     //
                     UtlString orbit;
                     UtlString audio;
                     int timeout;
                     if (validateOrbit(callId, address, orbit, audio, timeout) == OS_SUCCESS)
                     {
                        pThisCall = new ParkedCallObject(orbit, mpCallManager,
                                                         callId, audio);
                        pThisCall->setAddress(address);
   
                        // Create a player and start to play out the file
                        if (pThisCall->playAudio() == OS_SUCCESS)
                        {
                           // Put it in the list of parked calls
                           mCalls.insertKeyAndValue(new UtlString(callId), new UtlVoidPtr(pThisCall));
                           OsSysLog::add(FAC_SIP, PRI_DEBUG, "OrbitListener::handleMessage insert call object %p to the list",
                                         pThisCall);
                           // Start the timer if necessary.
                           pThisCall->startEscapeTimer(parker, timeout);
                        }
                        else
                        {
                           // Drop the call
                           mpCallManager->drop(callId);
                           OsSysLog::add(FAC_PARK, PRI_WARNING, "OrbitListener::handleMessage - Drop callId %s due to failure of playing audio",
                                         callId.data());
   
                           delete pThisCall;
                        }
                     }
                     else
                     {
                        // Do not accept the call to the requested orbit address.
                        mpCallManager->drop(callId);
                        OsSysLog::add(FAC_PARK, PRI_WARNING, "OrbitListener::handleMessage - Drop callId %s due to invalid orbit address",
                                      callId.data());
                     }
                  }
                  else
                  {
                     //
                     // Additional leg of transfer. This call replaces the previous call.
                     //
                     OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - change the call address from %s to %s",
                                   pThisCall->getAddress().data(), address.data());
                     OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage - dump pThisCall:\n"
                                                       "address '%s', original address '%s',\n"
                                                       "pickup callId '%s'",
                                                       pThisCall->getAddress().data(), pThisCall->getOriginalAddress().data(),
                                                       pThisCall->getPickupCallId().data());
                     // Update the remote address of the dialog
                     pThisCall->setAddress(address);
                     
                     UtlString orbit;
                     UtlString audio;
                     int timeout;
                     validateOrbit(callId, address, orbit, audio, timeout);
                     
                     // Check if Tao message has enough parameters to contain new call Id.
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
                        ParkedCallObject* pTransferredCall = new ParkedCallObject(orbit, mpCallManager, newCallId, audio);                        
                        mCalls.insertKeyAndValue(new UtlString(newCallId), new UtlVoidPtr(pTransferredCall));
                        pTransferredCall->setAddress(address);

                        // :TODO:
                        // Because the Call Manager can't handle transferring a call resulting from
                        // an INVITE-with-Replaces, we do not start the timeout timer here.
                        // But once the Call Manager is fixed, we should do so.
                        #if 0
                        pTransferredCall->startEscapeTimer(parker, timeout);
                        #endif
                                                
                        OsSysLog::add(FAC_SIP, PRI_DEBUG, "OrbitListener::handleMessage insert transfered call object %p "
                                      "with callId '%s', address '%s' into the list",
                                      pTransferredCall, newCallId.data(), address.data());
                     }
                     else
                     {
                        OsSysLog::add(FAC_PARK, PRI_ERR, "OrbitListener::handleMessage Tao message does not contain new callId for call '%s'", 
                                      callId.data());
                     }
                  }
               }
               else
               {
                  // This is a call retrieval
                  UtlString orbit;
                  UtlString audio;
                  int timeout;
                  
                  // Get Orbit out of the requestUri and use that to find the oldest call.
                  validateOrbit(callId, address, orbit, audio, timeout);
                  
                  UtlString originalCallId;
                  UtlString originalAddress;                  
                  OsSysLog::add(FAC_PARK, PRI_DEBUG, "OrbitListener::handleMessage retrieving parked call from orbit '%s'", orbit.data()); 
                  
                  // Find oldest call in call list, return that callId and orginal address in parameters.                 
                  pThisCall = getOldestCallInOrbit(orbit, originalCallId, originalAddress);

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
                     pThisCall = new ParkedCallObject(orbit, mpCallManager, callId, "", true);
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
            break;
      }
   }
   return(TRUE);
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


OsStatus OrbitListener::validateOrbit(UtlString& callId,
                                      UtlString& address,
                                      UtlString& orbit,
                                      UtlString& audio,
                                      int& timeout)
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
   }
   else
   {
      OrbitData* orbit_data = mOrbitFileReader.findInOrbitList(orbit);
      if (orbit_data)
      {
         audio = orbit_data->mAudio;
         timeout = orbit_data->mTimeout;
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


int OrbitListener::getNumCallsInOrbit(UtlString& orbit, UtlString& callId, UtlString& address)
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


void OrbitListener::dumpTaoMessageArgs(unsigned char eventId, TaoString& args) 
{
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitListener::dumpTaoMessageArgs Message type: %d local: %s args: %d",
                 eventId, args[TAO_OFFER_PARAM_LOCAL_CONNECTION], args.getCnt()) ;
        
   int argc = args.getCnt();
   for(int argIndex = 0; argIndex < argc; argIndex++)
   {
      OsSysLog::add(FAC_PARK, PRI_DEBUG, "\targ[%d]=\"%s\"", argIndex, args[argIndex]);
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
