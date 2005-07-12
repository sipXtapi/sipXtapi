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
                                   UtlString& hostAddress,
                                   int refreshTimeout)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   mpUserAgent = userAgent;
   mDomainName = domainName;
   mHostAddress = hostAddress;
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
   mpSubscribeServer->enableEventType(RESOURCE_LIST_CONTENT_TYPE);
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
}


void SipDialogMonitor::addExtension(UtlString& groupName, UtlString& contact)
{
   // Check whether the group has already existed. If not, create one.
   SipResourceList* pList = (SipResourceList *)mMonitoredLists.findValue(&groupName);
   if (pList == NULL)
   {
      UtlString* pListName = new UtlString(groupName);
      pList = new SipResourceList((UtlBoolean)TRUE, pListName->data(), DIALOG_EVENT_TYPE);
      
      mMonitoredLists.insertKeyAndValue(pListName, pList);
   }

   // Check whether the contact has already being added to the group
   Resource* pResource = pList->getResource(contact);
   if (pResource == NULL)
   {
      pResource = new Resource(contact);
      
      Url contactUrl(contact);
      UtlString userName;
      contactUrl.getUserId(userName);
      pResource->setName(userName);
      
      UtlString id;
      NetMd5Codec::encode(contact, id);
      pResource->setInstance(id, STATE_PENDIND);
      pList->insertResource(pResource);
      
      // Send out the SUBSCRIBE
      OsSysLog::add(FAC_LOG, PRI_DEBUG, "SipDialogMonitor::addExtension Sending out the SUBSCRIBE to contact %s", contact.data());

      UtlString toUrl;
      contactUrl.toString(toUrl);
      
      UtlString fromUri = "dialogMonitor@" + mDomainName;
      UtlString earlyDialogHandle;
      
      printf("to = %s\n", toUrl.data());
      printf("from = %s\n", fromUri.data());
      printf("contact = %s\n", mHostAddress.data());
      
      UtlBoolean result = mpSipSubscribeClient->addSubscription(contact.data(),
                                                                DIALOG_EVENT_TYPE,
                                                                fromUri.data(),
                                                                toUrl.data(),
                                                                mHostAddress.data(),
                                                                mRefreshTimeout,
                                                                (void *) this,
                                                                SipDialogMonitor::subscriptionStateCallback,
                                                                SipDialogMonitor::notifyEventCallback,
                                                                earlyDialogHandle);
               
      if (!result)
      {
         OsSysLog::add(FAC_LOG, PRI_ERR, "SipDialogMonitor::addExtension Subscription failed to contact %s.", contact.data());
      }
   }

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::addExtension insert listName %s and object %p to the resource list",
                 groupName.data(), pList);
   
   pList->buildBody();
}


void SipDialogMonitor::addDialogEvent(UtlString* contact, SipDialogEvent* dialogEvent)
{
   
   if (mSubscribeList.find(contact) == NULL)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::addDialogEvent adding the dialogEvent %p for contact %s",
                    dialogEvent, contact->data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::addDialogEvent dialogEvent %p for contact %s already exists, just update the content.",
                    dialogEvent, contact->data());
                    
      // Get the object from the subscribe list
      UtlContainable* foundValue;
      mSubscribeList.removeKeyAndValue(contact, foundValue);
      SipDialogEvent* pOldDialogEvent = (SipDialogEvent *) foundValue;

      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::addDialogEvent remove the dialogEvent %p for contact %s",
                    pOldDialogEvent, contact->data()); 

      if (pOldDialogEvent)
      {
         delete pOldDialogEvent;
      }
   }
         
   // Modify the entity
   dialogEvent->setEntity(contact->data());   
   dialogEvent->buildBody();
   
   // Insert it into the subscribe list
   mSubscribeList.insertKeyAndValue(contact, dialogEvent);
   
   // Publish the content
   publishContent(*contact, dialogEvent);
}


void SipDialogMonitor::publishContent(UtlString& contact, SipDialogEvent* dialogEvent)
{

   // Loop through all the resource lists
   UtlHashMapIterator iterator(mMonitoredLists);
   UtlString* pListUri;
   SipResourceList* pList;
   SipDialogEvent* pDialogEvent;
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


#if 0
   int numOldContents;
   HttpBody* oldContent[1];

      // Unpublish the old content to the subscribe server
      if (!mpSipPublishContentMgr->unpublish(contact.data(), DIALOG_EVENT_TYPE, DIALOG_EVENT_TYPE, 1, numOldContents, oldContent))
      {
         dialogEvent->getBytes(&dialogContent, &length);
         OsSysLog::add(FAC_SIP, PRI_ERR, "SipDialogMonitor::updateDialogEvent DialogEvent %s\n was not successfully unpublished to the subscribe server",
                       dialogContent.data());
      }

      // Publish the content to the subscribe server
      if (!mpSipPublishContentMgr->publish(contact->data(), DIALOG_EVENT_TYPE, DIALOG_EVENT_TYPE, 1, (HttpBody**)&dialogEvent, 1, numOldContents, oldContent))
      {
         UtlString dialogContent;
         int length;
         
         dialogEvent->getBytes(&dialogContent, &length);
         OsSysLog::add(FAC_SIP, PRI_ERR, "SipDialogMonitor::addDialogEvent DialogEvent %s\n was not successfully published to the subscribe server",
                       dialogContent.data());
      }
   }
#endif
}

void
SipDialogMonitor::subscriptionStateCallback(SipSubscribeClient::SubscriptionState newState,
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


void
SipDialogMonitor::notifyEventCallback(const char* earlyDialogHandle,
                                      const char* dialogHandle,
                                      void* applicationData,
                                      const SipMessage* notifyRequest)
{
   // Receive the notification and process the message
   SipDialogMonitor* pThis = (SipDialogMonitor *) applicationData;
   
   pThis->handleNotifyMessage(notifyRequest);
}


void
SipDialogMonitor::handleNotifyMessage(const SipMessage* notifyMessage)
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
 