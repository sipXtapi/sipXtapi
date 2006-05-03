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
#include <os/OsFS.h>
#include <os/OsSysLog.h>
#include <utl/UtlHashMapIterator.h>
#include <net/SipResourceList.h>
#include <net/NetMd5Codec.h>
#include <net/SipMessage.h>
#include <net/SipDialogMonitor.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// STATIC VARIABLE INITIALIZATIONS

// Constructor
SipDialogMonitor::SipDialogMonitor(SipUserAgent* userAgent,
                                   UtlString& domainName,
                                   int hostPort,
                                   int refreshTimeout,
                                   bool toBePublished)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   mpUserAgent = userAgent;
   mDomainName = domainName;

   UtlString localAddress;
   OsSocket::getHostIp(&localAddress);
   
   Url url(localAddress);
   url.setHostPort(hostPort);
   mContact = url.toString();
   mRefreshTimeout = refreshTimeout;
   mToBePublished = toBePublished;
   
   // Create the SIP Subscribe Client
   mpRefreshMgr = new SipRefreshManager(*mpUserAgent, mDialogManager); // Component for refreshing the subscription
   mpRefreshMgr->start();

   mpSipSubscribeClient = new SipSubscribeClient(*mpUserAgent, mDialogManager, *mpRefreshMgr);
   mpSipSubscribeClient->start();  

   if (mToBePublished)
   {
      // Create the SIP Subscribe Server
      mpSubscriptionMgr = new SipSubscriptionMgr(mDialogMgr); // Component for holding the subscription data

      mpSubscribeServer = new SipSubscribeServer(*mpUserAgent, mSipPublishContentMgr,
                                              *mpSubscriptionMgr, mPolicyHolder);
      mpSubscribeServer->enableEventType(DIALOG_EVENT_TYPE);
      mpSubscribeServer->start();
   }
}

// Destructor
SipDialogMonitor::~SipDialogMonitor()
{
   if (mpRefreshMgr)
   {
      delete mpRefreshMgr;
   }
   
   if (mpSipSubscribeClient)
   {
      delete mpSipSubscribeClient;
   }
   
   if (mpSubscriptionMgr)
   {
      delete mpSubscriptionMgr;
   }
   
   if (mpSubscribeServer)
   {
      delete mpSubscribeServer;
   }
   
   if (!mMonitoredLists.isEmpty())
   {
      mMonitoredLists.destroyAll();
   }

   if (!mDialogEventList.isEmpty())
   {
      mDialogEventList.destroyAll();
   }

   if (!mStateChangeNotifiers.isEmpty())
   {
      mStateChangeNotifiers.destroyAll();
   }   
}


bool SipDialogMonitor::addExtension(UtlString& groupName, Url& contactUrl)
{
   bool result = false;
   mLock.acquire();
   
   // Check whether the group has already existed. If not, create one.
   SipResourceList* list = dynamic_cast <SipResourceList *> (mMonitoredLists.findValue(&groupName));
   if (list == NULL)
   {
      UtlString* listName = new UtlString(groupName);
      list = new SipResourceList((UtlBoolean)TRUE, listName->data(), DIALOG_EVENT_TYPE);
      
      mMonitoredLists.insertKeyAndValue(listName, list);
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::addExtension insert listName %s and object %p to the resource list",
                    groupName.data(), list);   
   }

   // Check whether the contact has already being added to the group
   UtlString resourceId;
   contactUrl.getIdentity(resourceId);
   Resource* resource = list->getResource(resourceId);
   if (resource == NULL)
   {
      resource = new Resource(resourceId);
      
      UtlString userName;
      contactUrl.getDisplayName(userName);
      resource->setName(userName);
      
      UtlString id;
      NetMd5Codec::encode(resourceId, id);
      resource->setInstance(id, STATE_PENDIND);
      list->insertResource(resource);
      
      // Send out the SUBSCRIBE
      OsSysLog::add(FAC_LOG, PRI_DEBUG,
                    "SipDialogMonitor::addExtension Sending out the SUBSCRIBE to contact %s",
                    resourceId.data());

      UtlString toUrl;
      contactUrl.toString(toUrl);
      
      UtlString fromUri = "dialogMonitor@" + mDomainName;
      UtlString earlyDialogHandle;
            
      UtlBoolean status = mpSipSubscribeClient->addSubscription(resourceId.data(),
                                                                DIALOG_EVENT_TYPE,
                                                                NULL,
                                                                fromUri.data(),
                                                                toUrl.data(),
                                                                mContact.data(),
                                                                mRefreshTimeout,
                                                                (void *) this,
                                                                SipDialogMonitor::subscriptionStateCallback,
                                                                SipDialogMonitor::notifyEventCallback,
                                                                earlyDialogHandle);
               
      if (!status)
      {
         result = false;
         OsSysLog::add(FAC_LOG, PRI_ERR,
                       "SipDialogMonitor::addExtension Subscription failed to contact %s.",
                       resourceId.data());
      }
      else
      {
         result = true;
      }
   }
   else
   {
      OsSysLog::add(FAC_LOG, PRI_WARNING,
                    "SipDialogMonitor::addExtension contact %s already exists.",
                    resourceId.data());
   }

   list->buildBody();
   
   mLock.release();
   return result;
}

bool SipDialogMonitor::removeExtension(UtlString& groupName, Url& contactUrl)
{
   bool result = false;
   mLock.acquire();
   // Check whether the group has existed or not. If not, return false.
   SipResourceList* list = dynamic_cast <SipResourceList *> (mMonitoredLists.findValue(&groupName));
   if (list == NULL)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::removeExtension group %s does not exist",
                    groupName.data());   
   }
   else
   {
      // Check whether the contact has existed or not
      UtlString resourceId;
      contactUrl.getIdentity(resourceId);
      Resource* resource = list->getResource(resourceId);
      if (resource)
      {
         resource = list->removeResource(resource);
         delete resource;
         
         result = true;
      }
      else
      {
         OsSysLog::add(FAC_LOG, PRI_WARNING,
                       "SipDialogMonitor::removeExtension subscription for contact %s does not exists.",
                       resourceId.data());
      }
   }

   mLock.release();   
   return result;   
}

void SipDialogMonitor::addDialogEvent(UtlString& contact, SipDialogEvent* dialogEvent)
{
   if (mDialogEventList.find(&contact) == NULL)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::addDialogEvent adding the dialogEvent %p for contact %s",
                    dialogEvent, contact.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::addDialogEvent dialogEvent %p for contact %s already exists, just update the content.",
                    dialogEvent, contact.data());
                    
      // Get the object from the dialog event list
      UtlContainable* oldKey;
      UtlContainable* foundValue;
      oldKey = mDialogEventList.removeKeyAndValue(&contact, foundValue);
      delete oldKey;
      SipDialogEvent* oldDialogEvent = dynamic_cast <SipDialogEvent *> (foundValue);

      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::addDialogEvent remove the dialogEvent %p for contact %s",
                    oldDialogEvent, contact.data()); 

      if (oldDialogEvent)
      {
         delete oldDialogEvent;
      }
   }
         
   // Modify the entity
   dialogEvent->setEntity(contact.data());   
   dialogEvent->buildBody();
   
   // Insert it into the dialog event list
   mDialogEventList.insertKeyAndValue(new UtlString(contact), dialogEvent);
   
   if (mToBePublished)
   {
      // Publish the content to the resource list
      publishContent(contact, dialogEvent);
   }
   
   // Notify the state change
   notifyStateChange(contact, dialogEvent);
}


void SipDialogMonitor::publishContent(UtlString& contact, SipDialogEvent* dialogEvent)
{
   bool contentChanged;
   
   // Loop through all the resource lists
   UtlHashMapIterator iterator(mMonitoredLists);
   UtlString* listUri;
   SipResourceList* list;
   Resource* resource;
   UtlString id, state;
   while (listUri = dynamic_cast <UtlString *> (iterator()))
   {
      contentChanged = false;
      
      list = dynamic_cast <SipResourceList *> (mMonitoredLists.findValue(listUri));
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::publishContent listUri %s list %p",
                    listUri->data(), list); 

      // Search for the contact in this list
      resource = list->getResource(contact);
      if (resource)
      {
         resource->getInstance(id, state);
         
         if (dialogEvent->isEmpty())
         {
            resource->setInstance(id, STATE_TERMINATED);
         }
         else
         {
            Dialog* dialog = dialogEvent->getFirstDialog();
            
            UtlString state, event, code;
            dialog->getState(state, event, code);
            
            if (state.compareTo(STATE_TERMINATED) == 0)
            {
               resource->setInstance(id, STATE_TERMINATED);
            }
            else
            {     
               resource->setInstance(id, STATE_ACTIVE);
            }
         }
         
         list->buildBody();
         contentChanged = true;
      }
      
      if (contentChanged)
      {
         int numOldContents;
         HttpBody* oldContent[1];           
   
         // Publish the content to the subscribe server
         if (!mSipPublishContentMgr.publish(listUri->data(), DIALOG_EVENT_TYPE, DIALOG_EVENT_TYPE, 1, (HttpBody**)&list, 1, numOldContents, oldContent))
         {
            UtlString dialogContent;
            int length;
            
            list->getBytes(&dialogContent, &length);
            OsSysLog::add(FAC_SIP, PRI_ERR, "SipDialogMonitor::publishContent DialogEvent %s\n was not successfully published to the subscribe server",
                          dialogContent.data());
         }
      }
   }
}

void SipDialogMonitor::subscriptionStateCallback(SipSubscribeClient::SubscriptionState newState,
                                                 const char* earlyDialogHandle,
                                                 const char* dialogHandle,
                                                 void* applicationData,
                                                 int responseCode,
                                                 const char* responseText,
                                                 long expiration,
                                                 const SipMessage* subscribeResponse)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::subscriptionStateCallback is called with responseCode = %d (%s)",
                 responseCode, responseText); 
}                                            


void SipDialogMonitor::notifyEventCallback(const char* earlyDialogHandle,
                                           const char* dialogHandle,
                                           void* applicationData,
                                           const SipMessage* notifyRequest)
{
   // Receive the notification and process the message
   SipDialogMonitor* pThis = (SipDialogMonitor *) applicationData;
   
   pThis->handleNotifyMessage(notifyRequest);
}


void SipDialogMonitor::handleNotifyMessage(const SipMessage* notifyMessage)
{
   Url fromUrl;
   notifyMessage->getFromUrl(fromUrl);
   UtlString contact;
   fromUrl.getIdentity(contact);
   
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::handleNotifyMessage receiving a notify message from %s",
                 contact.data()); 
   
   const HttpBody* notifyBody = notifyMessage->getBody();
   
   if (notifyBody)
   {
      UtlString messageContent;
      int bodyLength;
      
      notifyBody->getBytes(&messageContent, &bodyLength);
      
      // Parse the content and store it in a SipDialogEvent object
      SipDialogEvent* sipDialogEvent = new SipDialogEvent(messageContent);
      
      // Add the SipDialogEvent object to the hash table
      addDialogEvent(contact, sipDialogEvent);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::handleNotifyMessage receiving an empty notify body from %s",
                    contact.data()); 
   }
}

void SipDialogMonitor::addStateChangeNotifier(const char* fileUrl, StateChangeNotifier* notifier)
{
   mLock.acquire();
   UtlString* name = new UtlString(fileUrl);
   UtlVoidPtr* value = new UtlVoidPtr(notifier);
   mStateChangeNotifiers.insertKeyAndValue(name, value);
   mLock.release();
}

void SipDialogMonitor::removeStateChangeNotifier(const char* fileUrl)
{
   mLock.acquire();
   UtlString name(fileUrl);
   mStateChangeNotifiers.destroy(&name);
   mLock.release();
}

void SipDialogMonitor::notifyStateChange(UtlString& contact, SipDialogEvent* dialogEvent)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::notifyStateChange contact = %s",
                 contact.data());

   // Loop through the notifier list
   UtlHashMapIterator iterator(mStateChangeNotifiers);
   UtlString* listUri;
   UtlVoidPtr* container;
   StateChangeNotifier* notifier;
   Url contactUrl(contact);
   mLock.acquire();
   while (listUri = dynamic_cast <UtlString *> (iterator()))
   {
      container = dynamic_cast <UtlVoidPtr *> (mStateChangeNotifiers.findValue(listUri));
      notifier = (StateChangeNotifier *) container->getValue();

      if (dialogEvent->isEmpty())
      {
         notifier->setStatus(contactUrl, StateChangeNotifier::ON_HOOK);
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::notifyStateChange dialog is empty, setting state to on hook");
      }
      else
      {
         Dialog* dialog = dialogEvent->getFirstDialog();
            
         UtlString state, event, code;
         dialog->getState(state, event, code);
            
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::notifyStateChange dialog state = %s",
                       state.data());
         if (state.compareTo(STATE_CONFIRMED) == 0)
         {
            notifier->setStatus(contactUrl, StateChangeNotifier::OFF_HOOK);
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::notifyStateChange setting state to off hook");
         }
         else
         {     
            if (state.compareTo(STATE_TERMINATED) == 0)
            {
               notifier->setStatus(contactUrl, StateChangeNotifier::ON_HOOK);
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::notifyStateChange setting state to on hook");
            }
            else
            {
               notifier->setStatus(contactUrl, StateChangeNotifier::RINGING);
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::notifyStateChange setting state to ringing");
            }
         }
      }
   }
   mLock.release();
}

