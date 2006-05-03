//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <cp/DialogEventPublisher.h>
#include <tao/TaoMessage.h>
#include <tao/TaoString.h>
#include <cp/CallManager.h>
#include <net/SipDialog.h>
#include <net/SipPublishContentMgr.h>
#include <os/OsFS.h>
#include <os/OsDateTime.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define TAO_OFFER_PARAM_CALLID             0
#define TAO_OFFER_PARAM_ADDRESS            2
#define TAO_OFFER_PARAM_LOCAL_CONNECTION   6
#define STATE "full"

//#define DEBUGGING 1

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
DialogEventPublisher::DialogEventPublisher(CallManager* callManager,
                                           SipPublishContentMgr* contentMgr)
{
   mpCallManager = callManager;
   mpSipPublishContentMgr = contentMgr;
   mDialogId = 0;
}


//Destructor
DialogEventPublisher::~DialogEventPublisher()
{
}


/* ============================ MANIPULATORS ============================== */

UtlBoolean DialogEventPublisher::handleMessage(OsMsg& rMsg)
{
   SipDialog sipDialog;
   UtlString sipDialogContent;
   Url requestUrl;
   UtlString entity;
   UtlString* pEntity;
   char dialogId[10];
   SipDialogEvent* pThisCall;
   Dialog* pDialog;
   UtlString localTag, remoteTag;
   Url localIdentity, remoteIdentity;
   Url localTarget, remoteTarget;
   UtlString identity, displayName;
   OsTime receivedTime;
   int numOldContents;
   HttpBody* oldContent[1];

   int length;
   UtlString dialogEvent;
   
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

            mpCallManager->getSipDialog(callId, address, sipDialog);
#ifdef DEBUGGING            
            sipDialog.toString(sipDialogContent);
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher:: sipDialog = %s", 
                          sipDialogContent.data());
#endif            
            sipDialog.getRemoteRequestUri(entity);
            
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher:: Call arrived: callId %s address %s requestUrl %s", 
                          callId.data(), address.data(), entity.data());

            if (entity.isNull())
            {
               OsSysLog::add(FAC_SIP, PRI_WARNING, "DialogEventPublisher:: Call arrived: callId %s address %s without requestUrl", 
                             callId.data(), address.data());
               break;
            }
            else
            {
               requestUrl = Url(entity);
               requestUrl.getIdentity(entity);
            }
               
            // Create a dialog event if has not been created yet
            pThisCall = (SipDialogEvent *) mCalls.findValue(&entity);
            if (pThisCall == NULL)
            {
               pEntity = new UtlString(entity);
               pThisCall = new SipDialogEvent(STATE, entity);
               mCalls.insertKeyAndValue(pEntity, pThisCall);
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher:: insert DialogEvent object %p to the list",
                             pThisCall);
            }

            // Create the dialog element
            sipDialog.getLocalField(localIdentity);
            localIdentity.getFieldParameter("tag", localTag);
   
            sipDialog.getRemoteField(remoteIdentity);
            remoteIdentity.getFieldParameter("tag", remoteTag);
               
            sprintf(dialogId, "%ld", mDialogId);
            mDialogId++;

            pDialog = new Dialog(dialogId, callId, localTag, remoteTag, "recipient");
            pDialog->setState(STATE_EARLY, NULL, NULL);
   
            localIdentity.getIdentity(identity);
            localIdentity.getDisplayName(displayName);
            pDialog->setLocalIdentity(identity, displayName);
   
            remoteIdentity.getIdentity(identity);
            remoteIdentity.getDisplayName(displayName);
            pDialog->setRemoteIdentity(identity, displayName);
   
            sipDialog.getLocalContact(localTarget);
            pDialog->setLocalTarget(localTarget.toString());
   
            sipDialog.getRemoteContact(remoteTarget);
            pDialog->setRemoteTarget(remoteTarget.toString());
               
            pDialog->setDuration(OsDateTime::getSecsSinceEpoch());
   
            pThisCall->insertDialog(pDialog);
   
            // Insert it into the active call list
            pThisCall->buildBody();

            // Send the content to the subscribe server
            if (!mpSipPublishContentMgr->publish(entity.data(), DIALOG_EVENT_TYPE, DIALOG_EVENT_TYPE, 1, (HttpBody**)&pThisCall, 1, numOldContents, oldContent))
            {
               pThisCall->getBytes(&dialogEvent, &length);
               OsSysLog::add(FAC_SIP, PRI_ERR, "DialogEventPublisher:: Call arrived - DialogEvent %s\n was not successfully published to the subscribe server",
                             dialogEvent.data());
            }
                        
            break;

         case PtEvent::CONNECTION_ESTABLISHED:
            if (localConnection) 
            {
               mpCallManager->getSipDialog(callId, address, sipDialog);
#ifdef DEBUGGING            
               sipDialog.toString(sipDialogContent);
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher:: sipDialog = %s", 
                             sipDialogContent.data());
#endif            
               sipDialog.getRemoteRequestUri(entity);

               OsSysLog::add(FAC_SIP, PRI_DEBUG, "Call connected: callId %s address %s with request %s",
                             callId.data(), address.data(), entity.data());

               if (entity.isNull())
               {
                  OsSysLog::add(FAC_SIP, PRI_WARNING, "DialogEventPublisher:: Call connected: callId %s address %s without requestUrl", 
                                callId.data(), address.data());
                  break;
               }
               else
               {
                  requestUrl = Url(entity);
                  requestUrl.getIdentity(entity);
               }
               
               pThisCall = (SipDialogEvent *) mCalls.findValue(&entity);
               if (pThisCall == NULL)
               {
                  pEntity = new UtlString(entity);
                  pThisCall = new SipDialogEvent(STATE, entity);
   
                  // Insert it into the active call list
                  mCalls.insertKeyAndValue(pEntity, pThisCall);
               }
               
               // Get the new callId because it might be changed
               sipDialog.getCallId(callId);

               pDialog = pThisCall->getDialog(callId);
               // Update the dialog content if exist
               if (pDialog)
               {
                  sipDialog.getLocalField(localIdentity);
                  localIdentity.getFieldParameter("tag", localTag);
   
                  sipDialog.getRemoteField(remoteIdentity);
                  remoteIdentity.getFieldParameter("tag", remoteTag);
               
                  pDialog->setTags(localTag, remoteTag);
   
                  sipDialog.getLocalContact(localTarget);
                  pDialog->setLocalTarget(localTarget.toString());
   
                  sipDialog.getRemoteContact(remoteTarget);
                  pDialog->setRemoteTarget(remoteTarget.toString());
   
                  pDialog->setState(STATE_CONFIRMED, NULL, NULL);
               }
               else
               {
                  // Create a new dialog element
                  sipDialog.getLocalField(localIdentity);
                  localIdentity.getFieldParameter("tag", localTag);
   
                  sipDialog.getRemoteField(remoteIdentity);
                  remoteIdentity.getFieldParameter("tag", remoteTag);
               
                  sprintf(dialogId, "%ld", mDialogId);
                  mDialogId++;
   
                  pDialog = new Dialog(dialogId, callId, localTag, remoteTag, "recipient");
                  pDialog->setState(STATE_CONFIRMED, NULL, NULL);
   
                  localIdentity.getIdentity(identity);
                  localIdentity.getDisplayName(displayName);
                  pDialog->setLocalIdentity(identity, displayName);
   
                  remoteIdentity.getIdentity(identity);
                  remoteIdentity.getDisplayName(displayName);
                  pDialog->setRemoteIdentity(identity, displayName);
   
                  sipDialog.getLocalContact(localTarget);
                  pDialog->setLocalTarget(localTarget.toString());
   
                  sipDialog.getRemoteContact(remoteTarget);
                  pDialog->setRemoteTarget(remoteTarget.toString());
   
                  pDialog->setDuration(OsDateTime::getSecsSinceEpoch());
   
                  pThisCall->insertDialog(pDialog);
               }
                   
               pThisCall->buildBody();

               // Publish the content to the subscribe server
               if (!mpSipPublishContentMgr->publish(entity.data(), DIALOG_EVENT_TYPE, DIALOG_EVENT_TYPE, 1, (HttpBody**)&pThisCall, 1, numOldContents, oldContent))
               {
                  pThisCall->getBytes(&dialogEvent, &length);
                  OsSysLog::add(FAC_SIP, PRI_ERR, "DialogEventPublisher:: Call connected - DialogEvent %s\n was not successfully published to the subscribe server",
                                dialogEvent.data());
               }
            }

            break;
            
         case PtEvent::CONNECTION_DISCONNECTED:
            if (!localConnection) 
            {
               mpCallManager->getSipDialog(callId, address, sipDialog);
#ifdef DEBUGGING            
               sipDialog.toString(sipDialogContent);
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher:: sipDialog = %s", 
                             sipDialogContent.data());
#endif            
               sipDialog.getLocalContact(requestUrl);
               requestUrl.getIdentity(entity);

               OsSysLog::add(FAC_SIP, PRI_DEBUG, "Call dropped: %s address %s with entity %s",
                             callId.data(), address.data(), entity.data());

               if (entity.isNull())
               {
                  OsSysLog::add(FAC_SIP, PRI_WARNING, "DialogEventPublisher:: Call dropped: callId %s address %s without requestUrl", 
                                callId.data(), address.data());
                  break;
               }
               
               // Get the new callId because it might be changed
               sipDialog.getCallId(callId);

               // Remove the call from the pool and clean up the call
               pThisCall = (SipDialogEvent *) mCalls.findValue(&entity);
               if (pThisCall)
               {
                  pDialog = pThisCall->getDialog(callId);
                  if (pDialog)
                  {
                     pDialog->setState(STATE_TERMINATED, NULL, NULL);
                   
                     pThisCall->buildBody();

                     // Publish the content to the subscribe server
                     if (!mpSipPublishContentMgr->publish(entity.data(), DIALOG_EVENT_TYPE, DIALOG_EVENT_TYPE, 1, (HttpBody**)&pThisCall, 1, numOldContents, oldContent))
                     {
                        pThisCall->getBytes(&dialogEvent, &length);
                        OsSysLog::add(FAC_SIP, PRI_ERR, "DialogEventPublisher:: Call dropped - DialogEvent %s\n was not successfully published to the subscribe server",
                                      dialogEvent.data());
                     }
                       
                     // Remove the dialog from the dialog event package
                     pDialog = pThisCall->removeDialog(pDialog);
                     delete pDialog;
                  }
                  
                  if (pThisCall->isEmpty())
                  {
                     // Unpublisher the content from the subscribe server
                     if (!mpSipPublishContentMgr->unpublish(entity.data(), DIALOG_EVENT_TYPE, DIALOG_EVENT_TYPE, 1, numOldContents, oldContent))
                     {
                        pThisCall->getBytes(&dialogEvent, &length);
                        OsSysLog::add(FAC_SIP, PRI_ERR, "DialogEventPublisher:: Call dropped - DialogEvent %s\n was not successfully unpublished to the subscribe server",
                                      dialogEvent.data());
                     }
                     
                     UtlContainable *foundValue;
                     mCalls.removeKeyAndValue(pEntity, foundValue);
                     if (foundValue)
                     {
                        OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher:: remove DialogEvent object %p from the list",
                                      pThisCall);
                        pThisCall = (SipDialogEvent *) foundValue;
                        delete pThisCall;
                        delete pEntity;
                     }
                  }
               }
               else
               {
                  OsSysLog::add(FAC_SIP, PRI_ERR, "DialogEventPublisher:: Call dropped - no entity %s founded in the active call list",
                                entity.data());
               }
            }

            break;
            
         case PtEvent::CONNECTION_FAILED:
            OsSysLog::add(FAC_SIP, PRI_WARNING, "Connection failed on call: %s", callId.data());

            break;
      }
   }
   return(TRUE);
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


void DialogEventPublisher::dumpTaoMessageArgs(unsigned char eventId, TaoString& args) 
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "===>Message type: %d args:\n", eventId) ;
        
   int argc = args.getCnt();
   for(int argIndex = 0; argIndex < argc; argIndex++)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "\targ[%d]=\"%s\"", argIndex, args[argIndex]);
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

