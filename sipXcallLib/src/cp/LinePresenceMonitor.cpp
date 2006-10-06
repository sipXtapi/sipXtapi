// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <utl/UtlSListIterator.h>
#include <net/SipRefreshManager.h>
#include <net/SipSubscribeClient.h>
#include <net/XmlRpcRequest.h>
#include <net/SipPresenceEvent.h>
#include <net/NetMd5Codec.h>
#include <cp/LinePresenceMonitor.h>
#include <cp/LinePresenceMonitorMsg.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define DEFAULT_REFRESH_INTERVAL      300
#define CONFIG_ETC_DIR                SIPX_CONFDIR

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
LinePresenceMonitor::LinePresenceMonitor(int userAgentPort,
                                         UtlString& domainName,
                                         UtlString& groupName,
                                         bool local,
                                         Url& remoteServer,
                                         Url& presenceServer)
   : OsServerTask("LinePresenceMonitor-%d"), mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   // Bind the SIP user agent to a port and start it up
   mpUserAgent = new SipUserAgent(userAgentPort, userAgentPort);
   mpUserAgent->start();
   
   mGroupName = groupName;
   mLocal = local;
   mDomainName = domainName;

   if (mLocal)
   {
      // Create a local Sip Dialog Monitor
      mpDialogMonitor = new SipDialogMonitor(mpUserAgent,
                                             domainName,
                                             userAgentPort,
                                             DEFAULT_REFRESH_INTERVAL,
                                             false);
      
      // Add itself to the dialog monitor for state change notification
      mpDialogMonitor->addStateChangeNotifier("Line_Presence_Monitor", this);

      // Verify that a presence server uri has been specified
      presenceServer.getHostAddress(mPresenceServer);
      if (mPresenceServer != NULL)
      {
         presenceServer.getIdentity(mPresenceServer);
      }
   }
   else
   {
      mRemoteServer = remoteServer;
   }

   // Create the SIP Subscribe Client for subscribing both dialog event and presence event
   mpRefreshMgr = new SipRefreshManager(*mpUserAgent, mDialogManager);
   mpRefreshMgr->start();
   
   mpSipSubscribeClient = new SipSubscribeClient(*mpUserAgent, mDialogManager, *mpRefreshMgr);
   mpSipSubscribeClient->start();
   
   UtlString localAddress;
   OsSocket::getHostIp(&localAddress);
   
   Url url(localAddress);
   url.setHostPort(userAgentPort);
   url.includeAngleBrackets();
   mContact = url.toString();    
}


// Destructor
LinePresenceMonitor::~LinePresenceMonitor()
{
   if (mpSipSubscribeClient)
   {
      mpSipSubscribeClient->endAllSubscriptions();
      delete mpSipSubscribeClient;
   }
   
   if (mpRefreshMgr)
   {
      delete mpRefreshMgr;
   }
   
   if (mpDialogMonitor)
   {
      // Remove itself to the dialog monitor
      mpDialogMonitor->removeStateChangeNotifier("Line_Presence_Monitor");

      delete mpDialogMonitor;
   }
   
   if (!mDialogSubscribeList.isEmpty())
   {
      mDialogSubscribeList.destroyAll();
   }

   if (!mPresenceSubscribeList.isEmpty())
   {
      mPresenceSubscribeList.destroyAll();
   }
   
   // Shut down the sipUserAgent
   mpUserAgent->shutdown(FALSE);

   while(!mpUserAgent->isShutdownDone())
   {
      ;
   }

   delete mpUserAgent;
}

/* ============================ MANIPULATORS ============================== */


// Assignment operator
LinePresenceMonitor&
LinePresenceMonitor::operator=(const LinePresenceMonitor& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;
}

/* ============================ ACCESSORS ================================= */
bool LinePresenceMonitor::setStatus(const Url& aor, const Status value)
{
   bool result = false;
   
   // We can only use userId to identify the line
   UtlString contact;
   aor.getUserId(contact);
   
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "LinePresenceMonitor::setStatus set the value %d for %s",
                 value, contact.data());
   
   // Set the dialog status
   UtlVoidPtr* container = dynamic_cast <UtlVoidPtr *> (mDialogSubscribeList.findValue(&contact));
   if (container != NULL)
   {
      LinePresenceBase* line = (LinePresenceBase *) container->getValue();
      // Set the state value in LinePresenceBase
      switch (value)
      {
      case StateChangeNotifier::ON_HOOK:
         if (!line->getState(LinePresenceBase::ON_HOOK))
         {
            line->updateState(LinePresenceBase::ON_HOOK, true);
            result = true;
         }
         
         break;
         
      case StateChangeNotifier::OFF_HOOK:
         if (line->getState(LinePresenceBase::ON_HOOK))
         {
            line->updateState(LinePresenceBase::ON_HOOK, false);
            result = true;
         }
         
         break;

      case StateChangeNotifier::RINGING:
         if (line->getState(LinePresenceBase::ON_HOOK))
         {
            line->updateState(LinePresenceBase::ON_HOOK, false);
            result = true;
         }
         
         break;         
      }
   }

   // Set the presence status      
   container = dynamic_cast <UtlVoidPtr *> (mPresenceSubscribeList.findValue(&contact));
   if (container != NULL)
   {
      LinePresenceBase* line = (LinePresenceBase *) container->getValue();
      // Set the state value in LinePresenceBase
      switch (value)
      {
      case StateChangeNotifier::SIGN_IN:
         if (!line->getState(LinePresenceBase::SIGNED_IN))
         {
            line->updateState(LinePresenceBase::SIGNED_IN, true);
            result = true;
         }
         
         break;
         
      case StateChangeNotifier::SIGN_OUT:
         if (line->getState(LinePresenceBase::SIGNED_IN))
         {
            line->updateState(LinePresenceBase::SIGNED_IN, false);
            result = false;
         }
         
         break;
      }
   }

   return result;
}

OsStatus LinePresenceMonitor::subscribeDialog(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;
   mLock.acquire();

   LinePresenceMonitorMsg subscribeDialogMsg(LinePresenceMonitorMsg::SUBSCRIBE_DIALOG, line);
   postMessage(subscribeDialogMsg);

   mLock.release();   
   return result;
}

OsStatus LinePresenceMonitor::unsubscribeDialog(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;
   mLock.acquire();

   LinePresenceMonitorMsg unsubscribeDialogMsg(LinePresenceMonitorMsg::UNSUBSCRIBE_DIALOG, line);
   postMessage(unsubscribeDialogMsg);

   mLock.release();   
   return result;
}

OsStatus LinePresenceMonitor::subscribeDialog(UtlSList& list)
{
   OsStatus result = OS_FAILED;
   
   UtlSListIterator iterator(list);
   LinePresenceBase* line;
   while ((line = dynamic_cast <LinePresenceBase *> (iterator())) != NULL)
   {
      subscribeDialog(line);
   }

   return result;
}

OsStatus LinePresenceMonitor::unsubscribeDialog(UtlSList& list)
{
   OsStatus result = OS_FAILED;
   
   UtlSListIterator iterator(list);
   LinePresenceBase* line;
   while ((line = dynamic_cast <LinePresenceBase *> (iterator())) != NULL)
   {
      unsubscribeDialog(line);
   }

   return result;
}

OsStatus LinePresenceMonitor::subscribePresence(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;
   mLock.acquire();

   LinePresenceMonitorMsg subscribePresenceMsg(LinePresenceMonitorMsg::SUBSCRIBE_PRESENCE, line);
   postMessage(subscribePresenceMsg);

   mLock.release();   
   return result;
}

OsStatus LinePresenceMonitor::unsubscribePresence(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;
   mLock.acquire();

   LinePresenceMonitorMsg unsubscribePresenceMsg(LinePresenceMonitorMsg::UNSUBSCRIBE_PRESENCE, line);
   postMessage(unsubscribePresenceMsg);

   mLock.release();   
   return result;
}

OsStatus LinePresenceMonitor::subscribePresence(UtlSList& list)
{
   OsStatus result = OS_FAILED;
   
   UtlSListIterator iterator(list);
   LinePresenceBase* line;
   while ((line = dynamic_cast <LinePresenceBase *> (iterator())) != NULL)
   {
      subscribePresence(line);
   }

   return result;
}

OsStatus LinePresenceMonitor::unsubscribePresence(UtlSList& list)
{
   OsStatus result = OS_FAILED;

   UtlSListIterator iterator(list);
   LinePresenceBase* line;
   while ((line = dynamic_cast <LinePresenceBase *> (iterator())) != NULL)
   {
      unsubscribePresence(line);
   }

   return result;
}


void LinePresenceMonitor::subscriptionStateCallback(SipSubscribeClient::SubscriptionState newState,
                                                    const char* earlyDialogHandle,
                                                    const char* dialogHandle,
                                                    void* applicationData,
                                                    int responseCode,
                                                    const char* responseText,
                                                    long expiration,
                                                    const SipMessage* subscribeResponse)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "LinePresenceMonitor::subscriptionStateCallback is called with responseCode = %d (%s)",
                 responseCode, responseText); 
}                                            


void LinePresenceMonitor::notifyEventCallback(const char* earlyDialogHandle,
                                              const char* dialogHandle,
                                              void* applicationData,
                                              const SipMessage* notifyRequest)
{
   // Receive the notification and process the message
   LinePresenceMonitor* pThis = (LinePresenceMonitor *) applicationData;
   
   pThis->handleNotifyMessage(notifyRequest);
}


void LinePresenceMonitor::handleNotifyMessage(const SipMessage* notifyMessage)
{
   Url fromUrl;
   notifyMessage->getFromUrl(fromUrl);
   UtlString contact;

   fromUrl.getUserId(contact);

   contact += mPresenceServer;

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "LinePresenceMonitor::handleNotifyMessage receiving a notify message from %s",
                 contact.data()); 
   
   const HttpBody* notifyBody = notifyMessage->getBody();
   
   if (notifyBody)
   {
      UtlString messageContent;
      int bodyLength;
      
      notifyBody->getBytes(&messageContent, &bodyLength);
      
      // Parse the content and store it in a SipPresenceEvent object
      SipPresenceEvent* sipPresenceEvent = new SipPresenceEvent(contact, messageContent);
      
      UtlString id;
      NetMd5Codec::encode(contact, id);
      Tuple* tuple = sipPresenceEvent->getTuple(id);
      
      if (tuple != NULL)
      {
         UtlString status;
         tuple->getStatus(status);
      
         Url contactUrl(contact);
         if (status.compareTo(STATUS_CLOSED) == 0)
         {
            setStatus(contactUrl, StateChangeNotifier::SIGN_OUT);
         }
         else
         {     
            setStatus(contactUrl, StateChangeNotifier::SIGN_IN);
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "LinePresenceMonitor::handleNotifyMessage unable to find matching tuple for: %s",
                       contact.data()); 
      }
      
      delete sipPresenceEvent;
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "LinePresenceMonitor::handleNotifyMessage receiving an empty notify body from %s",
                    contact.data()); 
   }
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
UtlBoolean LinePresenceMonitor::handleMessage(OsMsg& rMessage)
{
   LinePresenceMonitorMsg*    pMessage;
   LinePresenceBase*          pLine;

   if (rMessage.getMsgType() == OsMsg::USER_START)
   {
      switch (rMessage.getMsgSubType())
      {
         case LinePresenceMonitorMsg::SUBSCRIBE_DIALOG:
            pMessage = (LinePresenceMonitorMsg*)&rMessage;
            pLine     = pMessage->getLine();

            subscribeDialogMessage(pLine);
            break;

         case LinePresenceMonitorMsg::UNSUBSCRIBE_DIALOG:
            pMessage = (LinePresenceMonitorMsg*)&rMessage;
            pLine     = pMessage->getLine();

            unsubscribeDialogMessage(pLine);
            break;

         case LinePresenceMonitorMsg::SUBSCRIBE_PRESENCE:
            pMessage = (LinePresenceMonitorMsg*)&rMessage;
            pLine     = pMessage->getLine();

            subscribePresenceMessage(pLine);
            break;

         case LinePresenceMonitorMsg::UNSUBSCRIBE_PRESENCE:
            pMessage = (LinePresenceMonitorMsg*)&rMessage;
            pLine     = pMessage->getLine();

            unsubscribePresenceMessage(pLine);
            break;

         default:
            // Bad message
            OsSysLog::add(FAC_SIP, PRI_ERR, "LinePresenceMonitor::handleMessage - Received bad message");
            
            break;
      }

      return true;
   }
   else
   {
      // Otherwise, pass the message to the base for processing.
      return false;
   }
}

OsStatus LinePresenceMonitor::subscribeDialogMessage(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;

   Url* lineUrl = line->getUri();
   
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "LinePresenceMonitor::subscribeDialogMessage subscribing dialog for line %s",
                 lineUrl->toString().data()); 

   if (mLocal)
   {
      if(mpDialogMonitor->addExtension(mGroupName, *lineUrl))
      {
         result = OS_SUCCESS;
      }
      else
      {
         result = OS_FAILED;
      }
   }
   else
   {
      // Use XML-RPC to communicate with the sipX dialog monitor
      XmlRpcRequest request(mRemoteServer, "addExtension");
      
      request.addParam(&mGroupName);
      UtlString contact = lineUrl->toString();
      request.addParam(&contact);

      XmlRpcResponse response;
      if (request.execute(response))
      {
         result = OS_SUCCESS;
      }
      else
      {
         result = OS_FAILED;
      }      
   }
   
   // Insert the line to the Subscribe Map
   UtlString contactId;
   lineUrl->getUserId(contactId);
   mDialogSubscribeList.insertKeyAndValue(new UtlString(contactId),
                                          new UtlVoidPtr(line));

   return result;
}

OsStatus LinePresenceMonitor::unsubscribeDialogMessage(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;

   Url* lineUrl = line->getUri();
   
   if (lineUrl == NULL)
   {
      return OS_FAILED;
   }

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "LinePresenceMonitor::unsubscribeDialogMessage unsubscribing dialog for line %s",
                 lineUrl->toString().data());
                  
   if (mLocal)
   {
      if (mpDialogMonitor->removeExtension(mGroupName, *lineUrl))
      {
         result = OS_SUCCESS;
      }
      else
      {
         result = OS_FAILED;
      }
   }
   else
   {
      // Use XML-RPC to communicate with the sipX dialog monitor
      XmlRpcRequest request(mRemoteServer, "removeExtension");
      
      request.addParam(&mGroupName);
      UtlString contact = lineUrl->toString();
      request.addParam(&contact);

      XmlRpcResponse response;
      if (request.execute(response))
      {
         result = OS_SUCCESS;
      }
      else
      {
         result = OS_FAILED;
      }      
   }
         
   UtlString contact;
   lineUrl->getUserId(contact);
   mDialogSubscribeList.destroy(&contact);
   
   return result;
}

OsStatus LinePresenceMonitor::subscribePresenceMessage(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;

   Url* lineUrl = line->getUri();
   
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "LinePresenceMonitor::subscribePresenceMessage subscribing presence for line %s",
                 lineUrl->toString().data()); 
   
   // Send out the SUBSCRIBE to the presence server
   UtlString contactId, resourceId;
   lineUrl->getUserId(contactId);
   if (!mPresenceServer.isNull())
   {
      resourceId = contactId + mPresenceServer;
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "LinePresenceMonitor::subscribePresenceMessage Sending out the SUBSCRIBE to contact %s",
                    resourceId.data());
                    
      // Delay for 2 seconds because proxy might not be ready yet
      OsTask::delay(2000);   
      
      UtlString toUrl;
      lineUrl->toString(toUrl);
         
      UtlString fromUri = "linePresenceMonitor@" + mDomainName;
      UtlString dialogHandle;
               
      UtlBoolean status = mpSipSubscribeClient->addSubscription(resourceId.data(),
                                                                PRESENCE_EVENT_TYPE,
                                                                PRESENCE_EVENT_CONTENT_TYPE,
                                                                fromUri.data(),
                                                                toUrl.data(),
                                                                mContact.data(),
                                                                DEFAULT_REFRESH_INTERVAL,
                                                                (void *) this,
                                                                LinePresenceMonitor::subscriptionStateCallback,
                                                                LinePresenceMonitor::notifyEventCallback,
                                                                dialogHandle);
                  
      if (!status)
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "LinePresenceMonitor::subscribePresenceMessage Subscription failed to contact %s.",
                       resourceId.data());
      }
      else
      {
         mDialogHandleList.insertKeyAndValue(new UtlString(contactId), new UtlString(dialogHandle));
      }
   }

   // Insert the line to the Subscribe Map
   mPresenceSubscribeList.insertKeyAndValue(new UtlString(contactId),
                                            new UtlVoidPtr(line));

   return result;
}

OsStatus LinePresenceMonitor::unsubscribePresenceMessage(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;

   Url* lineUrl = line->getUri();
   
   if (lineUrl == NULL)
   {
      return OS_FAILED;
   }

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "LinePresenceMonitor::unsubscribePresenceMessage unsubscribing presence for line %s",
                 lineUrl->toString().data());
                  
   // Remove the line from the Subscribe Map
   UtlString contact;
   lineUrl->getUserId(contact);
   
   if (!mPresenceServer.isNull())
   {
      UtlBoolean status = true;

      UtlString* dialogHandle = dynamic_cast <UtlString *> (mDialogHandleList.findValue(&contact));
      if (dialogHandle != NULL)
      {
         UtlBoolean status = mpSipSubscribeClient->endSubscription(dialogHandle->data());
                  
         if (!status)
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "LinePresenceMonitor::unsubscribePresenceMessage Unsubscription failed for %s.",
                          contact.data());
         }
      }
      
      mDialogHandleList.destroy(&contact);
   }
   
   mPresenceSubscribeList.destroy(&contact);
   return result;
}

/* ============================ FUNCTIONS ================================= */

