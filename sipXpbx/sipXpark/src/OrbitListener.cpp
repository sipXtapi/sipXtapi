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
#include <utl/UtlHashMapIterator.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define TAO_OFFER_PARAM_CALLID             0
#define TAO_OFFER_PARAM_ADDRESS            2
#define TAO_OFFER_PARAM_LOCAL_CONNECTION   6
#define TAO_OFFER_PARAM_NEW_CALLID        11

#define DEFAULT_MOH_AUDIO                  "file://"SIPX_PARKMUSICDIR"/default.wav"

//#define DEBUGGING 1

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OrbitListener::OrbitListener(CallManager* callManager)
{
   mpCallManager = callManager;
}


//Destructor
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

#ifdef DEBUGGING
      dumpTaoMessageArgs(taoEventId, arg) ;
#endif        
      UtlBoolean localConnection = atoi(arg[TAO_OFFER_PARAM_LOCAL_CONNECTION]);
      UtlString  callId = arg[TAO_OFFER_PARAM_CALLID] ;
      UtlString  address = arg[TAO_OFFER_PARAM_ADDRESS] ;

      switch (taoEventId) 
      {
         case PtEvent::CONNECTION_OFFERED:
            OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - Call arrived: callId: '%s', address: '%s'\n", 
                          SIPX_SAFENULL(callId.data()), SIPX_SAFENULL(address.data()));

            mpCallManager->acceptConnection(callId, address);
            mpCallManager->answerTerminalConnection(callId, address, "*");

            break;


         case PtEvent::CONNECTION_ESTABLISHED:
            if (localConnection) 
            {
               OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - Call connected: callId '%s', address: '%s'",
                             SIPX_SAFENULL(callId.data()), SIPX_SAFENULL(address.data()));
               
               ParkedCallObject* pThisCall = NULL;
               // CallId at this point is either the callId of a normal call or the original callId
               // of a second leg of a transfer. 

               // Test if this is a call retrieval operation
               if (!isCallRetrievalInvite(callId.data(), address.data()))
               {
                  // Not a call retrieval, this is a call to be parked. Look up callId in list of calls
                  // to see if this is an replacement of a call we already have parked.
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
                     UtlString audio;
                     if (validateOrbitRequest(callId, address, audio) == OS_SUCCESS)
                     {
                        UtlString orbit;
                        getOrbit(callId, address, orbit);
                        
                        pThisCall = new ParkedCallObject(orbit, mpCallManager, callId, audio);
                        pThisCall->setAddress(address);
   
                        // Create a player and start to play out the file
                        if (pThisCall->playAudio() == OS_SUCCESS)
                        {
                           // Put it in a list
                           mCalls.insertKeyAndValue(new UtlString(callId), new UtlVoidPtr(pThisCall));
                           OsSysLog::add(FAC_SIP, PRI_DEBUG, "OrbitListener::handleMessage insert call object %p to the list",
                                         pThisCall);
                        }
                        else
                        {
                           // Drop the call
                           mpCallManager->drop(callId);
                           OsSysLog::add(FAC_ACD, PRI_WARNING, "OrbitListener::handleMessage - Drop callId %s due to failure of playing audio",
                                         SIPX_SAFENULL(callId.data()));
   
                           delete pThisCall;
                        }
                     }
                     else
                     {
                        // Do not accept the call to the requested orbit address.
                        mpCallManager->drop(callId);
                        OsSysLog::add(FAC_ACD, PRI_WARNING, "OrbitListener::handleMessage - Drop callId %s due to invalid orbit address",
                                      SIPX_SAFENULL(callId.data()));
                     }
                  }
                  else
                  {
                     //
                     // Additional leg of transfer. This call replaces the previous call.
                     //
                     OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - change the call address from %s to %s",
                                   SIPX_SAFENULL(pThisCall->getAddress().data()), SIPX_SAFENULL(address.data()));
                     OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - dump pThisCall:\n"
                                                       "address '%s', original address '%s',\n"
                                                       "pickup callId '%s'",
                                                       SIPX_SAFENULL(pThisCall->getAddress().data()), SIPX_SAFENULL(pThisCall->getOriginalAddress().data()),
                                                       SIPX_SAFENULL(pThisCall->getPickupCallId().data()));
                     // Update the remote address of the dialog
                     pThisCall->setAddress(address);
                     
                     UtlString orbit;
                     getOrbit(callId, address, orbit);
                     
                     // Check if Tao message has enough parameteres to contain new call Id.
                     if (arg.getCnt() > TAO_OFFER_PARAM_NEW_CALLID)
                     {
                        UtlString newCallId = arg[TAO_OFFER_PARAM_NEW_CALLID];
                        UtlString audio;
                        
                        // Attach the new callId to the original call object so we can clean it up when the first leg is 
                        // torn down
                        pThisCall->setNewCallId(newCallId.data());                        

                        // Put the new call in the list for later retrieval.                        
                        ParkedCallObject* pTransferredCall = new ParkedCallObject(orbit, mpCallManager, newCallId, audio);                        
                        mCalls.insertKeyAndValue(new UtlString(newCallId), new UtlVoidPtr(pTransferredCall));
                        pTransferredCall->setAddress(address);
                                                
                        OsSysLog::add(FAC_SIP, PRI_DEBUG, "OrbitListener::handleMessage insert transfered call object %p "
                                      "with callId '%s', address '%s' into the list",
                                      SIPX_SAFENULL(pTransferredCall), SIPX_SAFENULL(newCallId.data()), SIPX_SAFENULL(address.data()));
                     }
                     else
                     {
                        OsSysLog::add(FAC_ACD, PRI_ERR, "OrbitListener::handleMessage Tao message does not contain new callId for call '%s'", 
                                      SIPX_SAFENULL(callId.data()));
                     }
                  }
               }
               else
               {
                  // This is a call retrieval
                  UtlString orbit;
                  
                  // Get Orbit out of the requestUri and use that to find the oldest call.
                  getOrbit(callId, address, orbit);
                  
                  UtlString originalCallId;
                  UtlString originalAddress;                  
                  OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage retrieving parked call from orbit '%s'", SIPX_SAFENULL(orbit.data())); 
                  
                  // Find oldest call in call list, return that callId and orginal address in parameters.                 
                  pThisCall = getOldestCallInOrbit(orbit, originalCallId, originalAddress);

                  if (pThisCall)
                  {
                     // Attach the callId of the executor to the call to be picked up, may need this for
                     // clean up if the transfer fails.
                     pThisCall->setPickupCallId(callId.data());
                     OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - transferring original callId %s, address %s "
                                                       "to callId %s, address %s",
                                                        SIPX_SAFENULL(originalCallId.data()), SIPX_SAFENULL(originalAddress.data()), 
                                                        SIPX_SAFENULL(callId.data()),SIPX_SAFENULL(address.data()));
                                                     
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
                     OsSysLog::add(FAC_ACD, PRI_WARNING, "OrbitListener::handleMessage - Drop callId %s due to not finding a parked call",
                                   SIPX_SAFENULL(callId.data()));
                  }
               }
            }

            break;
            

         case PtEvent::CONNECTION_DISCONNECTED:
            if (!localConnection)
            {
               OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - Call Dropped: callId %s, address: %s",
                             SIPX_SAFENULL(callId.data()), SIPX_SAFENULL(address.data()));

               // See if the callId is in our list and if the address matches.
               UtlVoidPtr* container = dynamic_cast <UtlVoidPtr *> (mCalls.findValue(&callId));
               if (container)
               {
                  ParkedCallObject* pDroppedCall = (ParkedCallObject *) container->getValue();
                  OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - found call object %p for %s\n",
                                pDroppedCall, SIPX_SAFENULL(callId.data()));

                  if (pDroppedCall == NULL)
                  {
                     OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - No callId %s found in the active call list\n",
                                   SIPX_SAFENULL(callId.data()));
                  }
                  else if (address.compareTo(pDroppedCall->getAddress()) == 0)
                  {
                     // Are there new CallIds attached to this call? Then clean them up too.
                     if (pDroppedCall->hasNewCallIds())
                     {
                        OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - Call has new call id list");
                        // Retrieve the list of new callIds from the call object and clean them up one by one
                        UtlSList* newCallIds = pDroppedCall->getNewCallIds();
                        UtlSListIterator iterator(*newCallIds);
                        UtlString* pCallId = NULL;
                       
                        while ((pCallId = dynamic_cast<UtlString*>(iterator())))
                        {
                           OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - cleaning up new callId '%s'", SIPX_SAFENULL(pCallId->data()));
                           
                           UtlVoidPtr* newContainer = dynamic_cast <UtlVoidPtr *> (mCalls.findValue(pCallId));   
                           
                           if (newContainer)
                           {
                              ParkedCallObject* pNewCall = (ParkedCallObject*) newContainer->getValue();
                              if (pNewCall)
                              {
                                 mCalls.destroy(pCallId);
                                 OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage call objects in list : %d", mCalls.entries());                              
                                
                                 delete pNewCall;
                         
                                 // Don't tell the call manager to drop the call here, the call manager would not know
                                 // how to handle a drop() call with this callId.
                              }
                           }
                           else
                           {
                              OsSysLog::add(FAC_ACD, PRI_WARNING, "OrbitListener::handleMessage - Could not find call object for new call '%s'",
                                            SIPX_SAFENULL(pCallId->data()));
                           }
                        }                     
                     }
                     // Remove the call from the pool and clean up the call. This does not destroy the pDrpopedCall,
                     // it destroys the UtlVoidPtr that points to it.
                     mCalls.destroy(&callId);
                     OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage call objects in list : %d", mCalls.entries());

                     OsSysLog::add(FAC_SIP, PRI_DEBUG, "OrbitListener::handleMessage remove call object %p from the list",
                                   pDroppedCall);
                     delete pDroppedCall;
   
                     // Drop the call
                     mpCallManager->drop(callId);
                  }
                  else
                  {
                     OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - Address mismatch passed in '%s' / stored '%s'",
                                   SIPX_SAFENULL(address.data()), SIPX_SAFENULL(pDroppedCall->getAddress().data()));
                  }
               }
               else
               {
                  OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - No callId %s found in the active call list\n",
                                SIPX_SAFENULL(callId.data()));                  
               }
            }

            break;
            

         case PtEvent::CONNECTION_FAILED:
            OsSysLog::add(FAC_ACD, PRI_WARNING, "OrbitListener::handleMessage - Failed connection: %s\n", SIPX_SAFENULL(callId.data()));
            
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
                           OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - Found attached  pickup call %s - cleaning that up too", 
                                         SIPX_SAFENULL(pickupCallId.data())); 
                           mCalls.destroy(&pickupCallId);
                           
                           delete pPickupCall;
                   
                           mpCallManager->drop(pickupCallId);
                        }
                     }
                     else
                     {
                        OsSysLog::add(FAC_ACD, PRI_WARNING, "OrbitListener::handleMessage - Could not find call object for pickup call '%s'",
                                      SIPX_SAFENULL(pickupCallId.data()));
                     }
                     // Are there new CallIds attached to this call? Then clean them up too.
                     if (pDroppedCall->hasNewCallIds())
                     {
                        OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - Call has new call id list");
                        // Retrieve the list of new callIds from the call object and clean them up one by one
                        UtlSList* newCallIds = pDroppedCall->getNewCallIds();
                        UtlSListIterator iterator(*newCallIds);
                        UtlString* pCallId = NULL;
                       
                        while ((pCallId = dynamic_cast<UtlString*>(iterator())))
                        {
                           OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - cleaning up new callId '%s'", SIPX_SAFENULL(pCallId->data()));
                           
                           UtlVoidPtr* newContainer = dynamic_cast <UtlVoidPtr *> (mCalls.findValue(pCallId));   
                           
                           if (newContainer)
                           {
                              ParkedCallObject* pNewCall = (ParkedCallObject*) newContainer->getValue();
                              if (pNewCall)
                              {
                                 mCalls.destroy(pCallId);
                                 OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage call objects in list : %d", mCalls.entries());                              
                                 
                                 delete pNewCall;
                         
                                 // Don't tell the call manager to drop the call here, the call manager would not know
                                 // how to handle a drop() call with this callId.
                              }
                           }
                           else
                           {
                              OsSysLog::add(FAC_ACD, PRI_WARNING, "OrbitListener::handleMessage - Could not find call object for new call '%s'",
                                            SIPX_SAFENULL(pCallId->data()));
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


unsigned int OrbitListener::validateOrbitRequest(UtlString& callId, UtlString& address, UtlString& audio)
{
   OsPath workingDirectory;
   UtlString fromField, userId;

   mpCallManager->getFromField(callId.data(), address.data(), fromField);
   Url orbitUrl(fromField);
   orbitUrl.getUserId(userId);
   
   OsSysLog::add(FAC_ACD, PRI_DEBUG,"OrbitListener::validateOrbitRequest fromField %s, userId %s", 
                 SIPX_SAFENULL(fromField.data()), SIPX_SAFENULL(userId.data())); 

   audio = "";

   //
   // Now scan the orbit.xml file looking for a matching orbit entry
   //

   // First try and find the orbit.xml file, either in the SIPX_CONFDIR or the CWD.
   if (OsFileSystem::exists(SIPX_CONFDIR))
   {
      workingDirectory = SIPX_CONFDIR;
      OsPath path(workingDirectory);
      path.getNativePath(workingDirectory);

   } else
   {
      OsPath path;
      OsFileSystem::getWorkingDirectory(path);
      path.getNativePath(workingDirectory);
   }

   UtlString fileName = workingDirectory + OsPathBase::separator + ORBIT_CONFIG_FILE;

   TiXmlDocument doc(fileName);

   doc.LoadFile();
   if (!doc.Error())
   {
      TiXmlNode* rootNode = doc.FirstChild("orbits");
      if (rootNode != NULL)
      {
         // See if this is a music-on-hold call or an orbit call
         if (userId == "moh")
         {
            TiXmlNode *groupNode = rootNode->FirstChild("music-on-hold");
            if (groupNode != NULL)
            {
               TiXmlNode* audioNode = groupNode->FirstChild("background-audio");
               if ((audioNode != NULL)
                && (audioNode->FirstChild() != NULL))
               {
                  audio = (audioNode->FirstChild())->Value();
               }
            }
         }
         else
         {
            // Search each <orbit> group
            for (TiXmlNode *groupNode = rootNode->FirstChild("orbit");
                 groupNode;
                 groupNode = groupNode->NextSibling("orbit"))
            {
               // See if the <extension> matches
               TiXmlNode* orbitNode = groupNode->FirstChild("extension");
               if (orbitNode != NULL)
               {
                  if ((orbitNode->FirstChild() != NULL)
                   && ((orbitNode->FirstChild())->Value() == userId))
                  {
                     TiXmlNode* audioNode = groupNode->FirstChild("background-audio");
                     if ((audioNode != NULL)
                      && (audioNode->FirstChild() != NULL))
                     {
                        audio = (audioNode->FirstChild())->Value();
                     }
                     break;
                  }
               }
            }
         }
      }
   }
   else
   {
      OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::validateOrbitRequest - Unable to open orbit config file: %s", SIPX_SAFENULL(fileName.data()));
   }

   if (audio != "")
   {
      return OS_SUCCESS;
   }
   else
   {
      // Check if this is for MOH.  If so, use the default audio file.
      if (userId == "moh")
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


void OrbitListener::getOrbit(UtlString& callId, UtlString& address, UtlString &orbit)
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
   OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::getOrbit - orbit '%s' from '%s'", SIPX_SAFENULL(orbit.data()), SIPX_SAFENULL(request.data()));
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
               OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::getOldestCallInOrbit - Entry for callId '%s', orbit '%s'",
                                                  SIPX_SAFENULL(pKey->data()), SIPX_SAFENULL(pCall->getOrbit().data()));                   
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
                  
                     OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::getOldestCallInOrbit - Valid callId '%s', address '%s'",
                                   SIPX_SAFENULL(oldestKey.data()), SIPX_SAFENULL(oldestAddress.data()));
                  }
                  else
                  {
                     OsSysLog::add(FAC_ACD, PRI_ERR, "OrbitListener::getOldestCallInOrbit - Unknown callId '%s', remove from list",
                                   SIPX_SAFENULL(pKey->data()));
                     mCalls.destroy(pKey);
                     delete pCall;
                     bRemovedInvalidObject = true;
                     break;
                  }
               }
            }
            else
            {
               OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::getOldestCallInOrbit - No call object found for callId '%s'", 
                             SIPX_SAFENULL(pKey->data()));                    
            }
         }
         else
         {
            OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::getOldestCallInOrbit - No container found for callId '%s'", 
                          SIPX_SAFENULL(pKey->data()));                     
         }
      }
   }
   OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::getOldestCallInOrbit - Returning pCall %p, callId '%s', address '%s'",
                 pReturn, SIPX_SAFENULL(oldestKey.data()), SIPX_SAFENULL(oldestAddress.data()));   
   callId = oldestKey;
   address = oldestAddress;
   return pReturn;
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
                               
   OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::isCallPickupInvite remote request URI %s", SIPX_SAFENULL(request.data()));
   
   Url requestURI(request, TRUE);
   UtlString urlParam;

   if (requestURI.getUrlParameter("operation", urlParam))
   {
      if (urlParam.compareTo("retrieve", UtlString::ignoreCase) == 0)
      {
         bRet = true;         
         OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::isCallPickupInvite found retrieve parameter");         
      }
   }
   else
   {
      OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::isCallPickupInvite no operation URL parameter");                  
   }
   return bRet;
}


void OrbitListener::dumpTaoMessageArgs(unsigned char eventId, TaoString& args) 
{
   OsSysLog::add(FAC_ACD, PRI_DEBUG,"===>\nMessage type: %d args: %d", eventId, args.getCnt()) ;
        
   int argc = args.getCnt();
   for(int argIndex = 0; argIndex < argc; argIndex++)
   {
      OsSysLog::add(FAC_ACD, PRI_DEBUG, "\targ[%d]=\"%s\"\n", argIndex, SIPX_SAFENULL(args[argIndex]));
   }
}




/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

