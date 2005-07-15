// 
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
                                   int refreshTimeout)
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
   
   // Create the SIP Subscribe Client
   mpRefreshMgr = new SipRefreshManager(*mpUserAgent, mDialogManager); // Component for refreshing the subscription
   mpRefreshMgr->start();

   mpSipSubscribeClient = new SipSubscribeClient(*mpUserAgent, mDialogManager, *mpRefreshMgr);
   mpSipSubscribeClient->start();  

   // Create the SIP Subscribe Server
   mpSubscriptionMgr = new SipSubscriptionMgr(mDialogMgr); // Component for holding the subscription data

   mpSubscribeServer = new SipSubscribeServer(*mpUserAgent, mSipPublishContentMgr,
                                              *mpSubscriptionMgr, mPolicyHolder);
   mpSubscribeServer->enableEventType(DIALOG_EVENT_TYPE);
   mpSubscribeServer->start();
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
   SipResourceList* pList = (SipResourceList *)mMonitoredLists.findValue(&groupName);
   if (pList == NULL)
   {
      UtlString* pListName = new UtlString(groupName);
      pList = new SipResourceList((UtlBoolean)TRUE, pListName->data(), DIALOG_EVENT_TYPE);
      
      mMonitoredLists.insertKeyAndValue(pListName, pList);
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::addExtension insert listName %s and object %p to the resource list",
                    groupName.data(), pList);   
   }

   // Check whether the contact has already being added to the group
   UtlString resourceId;
   contactUrl.getIdentity(resourceId);
   Resource* pResource = pList->getResource(resourceId);
   if (pResource == NULL)
   {
      pResource = new Resource(resourceId);
      
      UtlString userName;
      contactUrl.getDisplayName(userName);
      pResource->setName(userName);
      
      UtlString id;
      NetMd5Codec::encode(resourceId, id);
      pResource->setInstance(id, STATE_PENDIND);
      pList->insertResource(pResource);
      
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

   pList->buildBody();
   
   mLock.release();
   return result;
}

bool SipDialogMonitor::removeExtension(UtlString& groupName, Url& contactUrl)
{
   bool result = false;
   mLock.acquire();
   // Check whether the group has existed or not. If not, return false.
   SipResourceList* pList = (SipResourceList *)mMonitoredLists.findValue(&groupName);
   if (pList == NULL)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::removeExtension group %s does not exist",
                    groupName.data());   
   }
   else
   {
      // Check whether the contact has existed or not
      UtlString resourceId;
      contactUrl.getIdentity(resourceId);
      Resource* pResource = pList->getResource(resourceId);
      if (pResource)
      {
         pResource = pList->removeResource(pResource);
         delete pResource;
         
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

void SipDialogMonitor::addDialogEvent(UtlString* contact, SipDialogEvent* dialogEvent)
{
   int numOldContents;
   HttpBody* oldContent[1];           
   
   if (mDialogEventList.find(contact) == NULL)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::addDialogEvent adding the dialogEvent %p for contact %s",
                    dialogEvent, contact->data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::addDialogEvent dialogEvent %p for contact %s already exists, just update the content.",
                    dialogEvent, contact->data());
                    
      // Get the object from the dialog event list
      UtlContainable* foundValue;
      mDialogEventList.removeKeyAndValue(contact, foundValue);
      SipDialogEvent* pOldDialogEvent = (SipDialogEvent *) foundValue;

      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::addDialogEvent remove the dialogEvent %p for contact %s",
                    pOldDialogEvent, contact->data()); 

      // Unpublish the old content to the subscribe server
      if (!mSipPublishContentMgr.unpublish(contact->data(), DIALOG_EVENT_TYPE, DIALOG_EVENT_TYPE, 1, numOldContents, oldContent))
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "SipDialogMonitor::addDialogEvent DialogEvent for contact %s was not successfully unpublished to the subscribe server",
                       contact->data());
      }
      
      if (pOldDialogEvent)
      {
         delete pOldDialogEvent;
      }
   }
         
   // Modify the entity
   dialogEvent->setEntity(contact->data());   
   dialogEvent->buildBody();
   
   // Insert it into the dialog event list
   mDialogEventList.insertKeyAndValue(contact, dialogEvent);
   
                    
   // Publish the content to the subscribe server
   if (!mSipPublishContentMgr.publish(contact->data(), DIALOG_EVENT_TYPE, DIALOG_EVENT_TYPE, 1, (HttpBody**)&dialogEvent, 1, numOldContents, oldContent))
   {
      UtlString dialogContent;
      int length;
      
      dialogEvent->getBytes(&dialogContent, &length);
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipDialogMonitor::addDialogEvent DialogEvent %s\n was not successfully published to the subscribe server",
                    dialogContent.data());
   }

   // Publish the content to the resource list
   publishContent(*contact, dialogEvent);
   
   // Notify the state change
   notifyStateChange(*contact, dialogEvent);
}


void SipDialogMonitor::publishContent(UtlString& contact, SipDialogEvent* dialogEvent)
{

   // Loop through all the resource lists
   UtlHashMapIterator iterator(mMonitoredLists);
   UtlString* pListUri;
   SipResourceList* pList;
   Resource* pResource;
   UtlString id, state;
   while (pListUri = (UtlString *) iterator())
   {
      pList = (SipResourceList *) mMonitoredLists.findValue(pListUri);
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::publishContent listUri %s list %p",
                    pListUri->data(), pList); 

      // Search for the contact in this list
      pResource = pList->getResource(contact);
      if (pResource)
      {
         pResource->getInstance(id, state);
         
         if (dialogEvent->isEmpty())
         {
            pResource->setInstance(id, STATE_TERMINATED);
         }
         else
         {
            Dialog* pDialog = dialogEvent->getFirstDialog();
            
            UtlString state, event, code;
            pDialog->getState(state, event, code);
            
            if (state.compareTo(STATE_TERMINATED) == 0)
            {
               pResource->setInstance(id, STATE_TERMINATED);
            }
            else
            {     
               pResource->setInstance(id, STATE_ACTIVE);
            }
         }
         
         pList->buildBody();
      }
      else
      {
         UtlString name;
         pList->getListUri(name);
         OsSysLog::add(FAC_SIP, PRI_ERR, "SipDialogMonitor::publishContent contact %s is not in the resource list %s",
                       contact.data(), name.data());
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
      SipDialogEvent* pSipDialogEvent = new SipDialogEvent(messageContent);
      
      // Add the SipDialogEvent object to the hash table
      UtlString* pContact = new UtlString(contact);
      addDialogEvent(pContact, pSipDialogEvent);
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

   // Loop through the notifier list
   UtlHashMapIterator iterator(mStateChangeNotifiers);
   UtlString* pListUri;
   StateChangeNotifier* notifier;
   Url contactUrl(contact);
   mLock.acquire();
   while (pListUri = (UtlString *) iterator())
   {
      notifier = (StateChangeNotifier *) mStateChangeNotifiers.findValue(pListUri);

      if (dialogEvent->isEmpty())
      {
         notifier->setStatus(contactUrl, StateChangeNotifier::ON_HOOK);
      }
      else
      {
         Dialog* pDialog = dialogEvent->getFirstDialog();
            
         UtlString state, event, code;
         pDialog->getState(state, event, code);
            
         if (state.compareTo(STATE_CONFIRMED) == 0)
         {
            notifier->setStatus(contactUrl, StateChangeNotifier::OFF_HOOK);
         }
         else
         {     
            if (state.compareTo(STATE_TERMINATED) == 0)
            {
               notifier->setStatus(contactUrl, StateChangeNotifier::ON_HOOK);
            }
            else
            {
               notifier->setStatus(contactUrl, StateChangeNotifier::RINGING);
            }
         }
      }
   }
   mLock.release();
}

