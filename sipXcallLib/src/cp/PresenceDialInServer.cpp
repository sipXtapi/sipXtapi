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
#include <cp/PresenceDialInServer.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlVoidPtr.h>
#include <tao/TaoMessage.h>
#include <tao/TaoString.h>
#include <cp/CallManager.h>
#include <net/SipDialog.h>
#include <os/OsFS.h>
#include <xmlparser/tinyxml.h>
#include <os/OsDateTime.h>
#include <os/OsTimer.h>
#include <os/OsEventMsg.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define TAO_OFFER_PARAM_CALLID             0
#define TAO_OFFER_PARAM_ADDRESS            2
#define TAO_OFFER_PARAM_LOCAL_CONNECTION   6

#define SECONDS_DELAY 1

//#define DEBUGGING 1

// STATIC VARIABLE INITIALIZATIONS

// Private class to contain an incoming call for given callId
class CallContainer : public UtlString
{
public:
    CallContainer(UtlString& callId, UtlString& address, OsMsgQ* incomingQ);

    virtual ~CallContainer();

    // parent UtlString contains the resourceId and eventTypeKey
    UtlString mCallId;
    UtlString mAddress;
    OsQueuedEvent* mQueuedEvent;
    OsTimer* mTimer;

private:
    //! DISALLOWED accendental copying
    CallContainer(const CallContainer& rCallContainer);
    CallContainer& operator=(const CallContainer& rhs);

};

CallContainer::CallContainer(UtlString& callId, UtlString& address, OsMsgQ* incomingQ)
{
   mCallId = callId;
   mAddress = address;
   
   mQueuedEvent = new OsQueuedEvent(*incomingQ, (int)this);
   mTimer = new OsTimer(*mQueuedEvent);
}

CallContainer::~CallContainer()
{
   if (mQueuedEvent)
   {
      delete mQueuedEvent;
   }
   
   if (mTimer)
   {
      delete mTimer;
   }
}

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PresenceDialInServer::PresenceDialInServer(CallManager* callManager,
                                           UtlString& configFile)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   mpCallManager = callManager;
   parseConfig(configFile);

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: configuration for PresenceDialIn:"); 
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: signInFeatureCode = %s", mSignInFC.data()); 
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: signOutFeatureCode = %s", mSignOutFC.data()); 
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: signInConfirmationAudio = %s", mSignInConfirmationAudio.data()); 
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: signOutConfirmationAudio = %s", mSignOutConfirmationAudio.data()); 
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: errorAudio = %s", mErrorAudio.data());
   
   mpIncomingQ = getMessageQueue();   
}


//Destructor
PresenceDialInServer::~PresenceDialInServer()
{
   if (!mStateChangeNotifiers.isEmpty())
   {
      mStateChangeNotifiers.destroyAll();
   }  
}


/* ============================ MANIPULATORS ============================== */

UtlBoolean PresenceDialInServer::handleMessage(OsMsg& rMsg)
{
   SipDialog sipDialog;
   UtlString sipDialogContent;
   Url requestUrl;
   UtlString entity;
   UtlString featureCode;
   Url contactUrl;
   UtlString contact;
   
    int msgType = rMsg.getMsgType();
    int msgSubType = rMsg.getMsgSubType();

   // React to telephony events
   if(msgSubType == TaoMessage::EVENT)
   {
      TaoMessage* taoMessage = dynamic_cast <TaoMessage*> (&rMsg);

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

            mpCallManager->acceptConnection(callId, address);
            mpCallManager->answerTerminalConnection(callId, address, "*");

            mpCallManager->getSipDialog(callId, address, sipDialog);
#ifdef DEBUGGING            
            sipDialog.toString(sipDialogContent);
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: sipDialog = %s", 
                          sipDialogContent.data());
#endif            
            sipDialog.getRemoteRequestUri(entity);
            
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: Call arrived: callId %s address %s requestUrl %s", 
                          callId.data(), address.data(), entity.data());

            if (entity.isNull())
            {
               OsSysLog::add(FAC_SIP, PRI_WARNING, "PresenceDialInServer:: Call arrived: callId %s address %s without requestUrl", 
                             callId.data(), address.data());
            }

            break;

         case PtEvent::CONNECTION_ESTABLISHED:
            if (localConnection) 
            {
               mpCallManager->getSipDialog(callId, address, sipDialog);
#ifdef DEBUGGING            
               sipDialog.toString(sipDialogContent);
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: sipDialog = %s", 
                             sipDialogContent.data());
#endif            
               sipDialog.getRemoteRequestUri(entity);

               OsSysLog::add(FAC_SIP, PRI_DEBUG, "Call connected: callId %s address %s with request %s",
                             callId.data(), address.data(), entity.data());

               if (entity.isNull())
               {
                  OsSysLog::add(FAC_SIP, PRI_WARNING, "PresenceDialInServer:: Call connected: callId %s address %s without requestUrl", 
                                callId.data(), address.data());
               }
               else
               {
                  // Create a CallContainer object and insert it into the call list
                  CallContainer* thisCall = new CallContainer(callId, address, mpIncomingQ);
                  mCalls.insertKeyAndValue(new UtlString(callId), thisCall);
                  
                  // Get the feature code from the request url
                  requestUrl = Url(entity);
                  requestUrl.getUserId(featureCode);
                  contactUrl = Url(address);
                  contactUrl.getIdentity(contact);
                  OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: contact %s request for %s",
                                contact.data(), featureCode.data());
                  
                  if (featureCode.compareTo(mSignInFC) == 0)
                  {
                     if (notifyStateChange(contact, true))
                     {
                        mpCallManager->audioPlay(callId, mSignInConfirmationAudio, FALSE, FALSE, TRUE);
                     }
                     else
                     {
                        mpCallManager->audioPlay(callId, mErrorAudio, FALSE, FALSE, TRUE);
                        OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: contact %s has already signed in",
                                      contact.data());
                     }                        
                  }
                  
                  if (featureCode.compareTo(mSignOutFC) == 0)
                  {
                     if (notifyStateChange(contact, false))
                     {
                        mpCallManager->audioPlay(callId, mSignOutConfirmationAudio, FALSE, FALSE, TRUE);
                     }
                     else
                     {
                        mpCallManager->audioPlay(callId, mErrorAudio, FALSE, FALSE, TRUE);
                        OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: contact %s has already signed out",
                                      contact.data());
                     }
                  }
                  
                  OsTime timerTime(SECONDS_DELAY, 0);
                  thisCall->mTimer->oneshotAfter(timerTime);                                                            
               }
            }

            break;

         case PtEvent::CONNECTION_DISCONNECTED:
            if (!localConnection) 
            {
               mpCallManager->getSipDialog(callId, address, sipDialog);
#ifdef DEBUGGING            
               sipDialog.toString(sipDialogContent);
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "PresenceDialInServer:: sipDialog = %s", 
                             sipDialogContent.data());
#endif            
               sipDialog.getLocalContact(requestUrl);
               requestUrl.getIdentity(entity);

               OsSysLog::add(FAC_SIP, PRI_DEBUG, "Call dropped: %s address %s with entity %s",
                             callId.data(), address.data(), entity.data());

               if (entity.isNull())
               {
                  OsSysLog::add(FAC_SIP, PRI_WARNING, "PresenceDialInServer:: Call dropped: callId %s address %s without requestUrl", 
                                callId.data(), address.data());
               }
               
               // Remove the CallContainer object from the call list
               mCalls.destroy(&callId);
            }

            break;
            
         case PtEvent::CONNECTION_FAILED:
            OsSysLog::add(FAC_SIP, PRI_WARNING, "Connection failed on call: %s", callId.data());

            break;
      }
   }
   else
   {
       if(msgType == OsMsg::OS_EVENT && msgSubType == OsEventMsg::NOTIFY)
       {
         CallContainer* thisCall = NULL;
         ((OsEventMsg&)rMsg).getUserData((int&)thisCall);
         mpCallManager->dropConnection(thisCall->mCallId, thisCall->mAddress);
       }
   }
      
   return(TRUE);
}


/* ============================ ACCESSORS ================================= */
void PresenceDialInServer::addStateChangeNotifier(const char* fileUrl, StateChangeNotifier* notifier)
{
   mLock.acquire();
   UtlString* name = new UtlString(fileUrl);
   UtlVoidPtr* value = new UtlVoidPtr(notifier);
   mStateChangeNotifiers.insertKeyAndValue(name, value);
   mLock.release();
}

void PresenceDialInServer::removeStateChangeNotifier(const char* fileUrl)
{
   mLock.acquire();
   UtlString name(fileUrl);
   mStateChangeNotifiers.destroy(&name);
   mLock.release();
}
 
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


void PresenceDialInServer::dumpTaoMessageArgs(unsigned char eventId, TaoString& args) 
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "===>Message type: %d args:\n", eventId) ;
        
   int argc = args.getCnt();
   for(int argIndex = 0; argIndex < argc; argIndex++)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "\targ[%d]=\"%s\"", argIndex, args[argIndex]);
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */
bool PresenceDialInServer::notifyStateChange(UtlString& contact, bool signIn)
{
   bool result = false;
   
   // Loop through the notifier list
   UtlHashMapIterator iterator(mStateChangeNotifiers);
   UtlString* listUri;
   StateChangeNotifier* notifier;
   UtlVoidPtr* container;
   Url contactUrl(contact);
   mLock.acquire();
   while (listUri = dynamic_cast <UtlString *> (iterator()))
   {
      container = dynamic_cast <UtlVoidPtr *> (mStateChangeNotifiers.findValue(listUri));
      notifier = (StateChangeNotifier *) container->getValue();

      if (signIn)
      {
         result = notifier->setStatus(contactUrl, StateChangeNotifier::PRESENT);
      }
      else
      {
         result = notifier->setStatus(contactUrl, StateChangeNotifier::AWAY);
      }
   }
   mLock.release();
   
   return result;
}

void PresenceDialInServer::parseConfig(UtlString& configFile)
{
   // Parse the XML formated confgiuration file
   TiXmlDocument doc(configFile);      
   if (doc.LoadFile())
   {
      TiXmlNode* rootNode = doc.FirstChild ("PresenceDialIn");
      TiXmlNode* paramNode;      
      if (rootNode != NULL)
      {
         paramNode = rootNode->FirstChild("signInFeatureCode");
         if (paramNode)
         {
            mSignInFC = paramNode->FirstChild()->Value();
         }
         
         paramNode = rootNode->FirstChild("signOutFeatureCode");
         if (paramNode)
         {
            mSignOutFC = paramNode->FirstChild()->Value();
         }
               
         paramNode = rootNode->FirstChild("signInConfirmationAudio");
         if (paramNode)
         {
            mSignInConfirmationAudio = paramNode->FirstChild()->Value();
         }
         
         paramNode = rootNode->FirstChild("signOutConfirmationAudio");
         if (paramNode)
         {
             mSignOutConfirmationAudio = paramNode->FirstChild()->Value();
         }

         paramNode = rootNode->FirstChild("errorAudio");
         if (paramNode)
         {
            mErrorAudio = paramNode->FirstChild()->Value();
         }
      }
   }   
}

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

