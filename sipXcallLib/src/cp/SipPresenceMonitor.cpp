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
#include <os/OsConfigDb.h>
#include <utl/UtlHashMapIterator.h>
#include <net/SipResourceList.h>
#include <net/NetMd5Codec.h>
#include <net/SipMessage.h>
#include <cp/SipPresenceMonitor.h>
#include <mi/CpMediaInterfaceFactoryFactory.h>
#include <mp/MpMediaTask.h>
#include <mp/NetInTask.h>
#ifdef INCLUDE_RTCP
#include <rtcp/RTCManager.h>
#endif // INCLUDE_RTCP

// DEFINES
#define RTP_START_PORT          12000    // Starting RTP port

#define CODEC_G711_PCMU         "258"   // ID for PCMU
#define CODEC_G711_PCMA         "257"   // ID for PCMA
#define CODEC_DTMF_RFC2833      "128"   // ID for RFC2833 DMTF 

#define MAX_CONNECTIONS         200     // Max number of sim. conns
#define MP_SAMPLE_RATE          8000    // Sample rate (don't change)
#define MP_SAMPLES_PER_FRAME    80      // Frames per second (don't change)

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// STATIC VARIABLE INITIALIZATIONS

// Constructor
SipPresenceMonitor::SipPresenceMonitor(SipUserAgent* userAgent,
                                       UtlString& domainName,
                                       int hostPort,
                                       OsConfigDb* configFile,
                                       bool toBePublished)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   mpUserAgent = userAgent;
   mDomainName = domainName;
   mToBePublished = toBePublished;
   
   char buffer[80];
   sprintf(buffer, "@%s:%d", mDomainName.data(), hostPort);
   mHostAndPort = UtlString(buffer);

   UtlString localAddress;
   OsSocket::getHostIp(&localAddress);

   // Enable PCMU, PCMA, Tones/RFC2833 codecs
   SdpCodec::SdpCodecTypes codecs[3];
    
   codecs[0] = SdpCodecFactory::getCodecType(CODEC_G711_PCMU) ;
   codecs[1] = SdpCodecFactory::getCodecType(CODEC_G711_PCMA) ;
   codecs[2] = SdpCodecFactory::getCodecType(CODEC_DTMF_RFC2833) ;

   mCodecFactory.buildSdpCodecFactory(3, codecs);

   // Initialize and start up the media subsystem
   OsConfigDb dummyDb;
   mpStartUp(MP_SAMPLE_RATE, MP_SAMPLES_PER_FRAME, 6 * MAX_CONNECTIONS, &dummyDb);
   MpMediaTask::getMediaTask(MAX_CONNECTIONS);
#ifdef INCLUDE_RTCP
   CRTCManager::getRTCPControl();
#endif //INCLUDE_RTCP
   mpStartTasks();

   // Instantiate the call processing subsystem
   mpCallManager = new CallManager(FALSE,
                                   NULL,
                                   TRUE,                              // early media in 180 ringing
                                   &mCodecFactory,
                                   RTP_START_PORT,                    // rtp start
                                   RTP_START_PORT + (2*MAX_CONNECTIONS), // rtp end
                                   localAddress,
                                   localAddress,
                                   mpUserAgent, 
                                   0,                                 // sipSessionReinviteTimer
                                   NULL,                              // mgcpStackTask
                                   NULL,                              // defaultCallExtension
                                   Connection::RING,                  // availableBehavior
                                   NULL,                              // unconditionalForwardUrl
                                   -1,                                // forwardOnNoAnswerSeconds
                                   NULL,                              // forwardOnNoAnswerUrl
                                   Connection::BUSY,                  // busyBehavior
                                   NULL,                              // sipForwardOnBusyUrl
                                   NULL,                              // speedNums
                                   CallManager::SIP_CALL,             // phonesetOutgoingCallProtocol
                                   4,                                 // numDialPlanDigits
                                   CallManager::NEAR_END_HOLD,        // holdType
                                   5000,                              // offeringDelay
                                   "",                                // pLocal
                                   CP_MAXIMUM_RINGING_EXPIRE_SECONDS, // inviteExpiresSeconds
                                   QOS_LAYER3_LOW_DELAY_IP_TOS,       // expeditedIpTos
                                   MAX_CONNECTIONS,                   // maxCalls
                                   sipXmediaFactoryFactory(NULL));    // CpMediaInterfaceFactory

   mpDialInServer = new PresenceDialInServer(mpCallManager, configFile);    
   mpCallManager->addTaoListener(mpDialInServer);
   mpDialInServer->start();

   // Startup the call processing system
   mpCallManager->start();
      
   // Add itself to the presence dial-in server for state change notification
   mpDialInServer->addStateChangeNotifier("Presence_Dial_In_Server", this);

   if (mToBePublished)
   {
      // Create the SIP Subscribe Server
      mpSubscriptionMgr = new SipSubscriptionMgr(mDialogMgr); // Component for holding the subscription data
   
      mpSubscribeServer = new SipSubscribeServer(*mpUserAgent, mSipPublishContentMgr,
                                                 *mpSubscriptionMgr, mPolicyHolder);
      mpSubscribeServer->enableEventType(PRESENCE_EVENT_TYPE);
      mpSubscribeServer->start();
   }
}

// Destructor
SipPresenceMonitor::~SipPresenceMonitor()
{
   // Remove itself from the presence dial-in server
   mpDialInServer->removeStateChangeNotifier("Presence_Dial_In_Server");

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

   if (!mPresenceEventList.isEmpty())
   {
      mPresenceEventList.destroyAll();
   }

   if (!mStateChangeNotifiers.isEmpty())
   {
      mStateChangeNotifiers.destroyAll();
   }   
}


bool SipPresenceMonitor::addExtension(UtlString& groupName, Url& contactUrl)
{
   bool result = false;
   mLock.acquire();
   
   // Check whether the group has already existed. If not, create one.
   SipResourceList* list = dynamic_cast <SipResourceList *> (mMonitoredLists.findValue(&groupName));
   if (list == NULL)
   {
      UtlString* listName = new UtlString(groupName);
      list = new SipResourceList((UtlBoolean)TRUE, listName->data(), PRESENCE_EVENT_TYPE);
      
      mMonitoredLists.insertKeyAndValue(listName, list);
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceMonitor::addExtension insert listName %s and object %p to the resource list",
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
      
      result = true;
   }
   else
   {
      OsSysLog::add(FAC_LOG, PRI_WARNING,
                    "SipPresenceMonitor::addExtension contact %s already exists.",
                    resourceId.data());
   }

   list->buildBody();
   
   mLock.release();
   return result;
}

bool SipPresenceMonitor::removeExtension(UtlString& groupName, Url& contactUrl)
{
   bool result = false;
   mLock.acquire();
   // Check whether the group has existed or not. If not, return false.
   SipResourceList* list = dynamic_cast <SipResourceList *> (mMonitoredLists.findValue(&groupName));
   if (list == NULL)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceMonitor::removeExtension group %s does not exist",
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
                       "SipPresenceMonitor::removeExtension subscription for contact %s does not exists.",
                       resourceId.data());
      }
   }

   mLock.release();   
   return result;   
}

bool SipPresenceMonitor::addPresenceEvent(UtlString& contact, SipPresenceEvent* presenceEvent)
{
   bool requiredPublish = false;
   
   if (mPresenceEventList.find(&contact) == NULL)
   {
      requiredPublish = true;
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceMonitor::addPresenceEvent adding the presenceEvent %p for contact %s",
                    presenceEvent, contact.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceMonitor::addPresenceEvent presenceEvent %p for contact %s already exists, just update the content.",
                    presenceEvent, contact.data());
                    
      // Get the object from the presence event list
      UtlContainable* oldKey;
      UtlContainable* foundValue;
      foundValue = mPresenceEventList.findValue(&contact);
      SipPresenceEvent* oldPresenceEvent = dynamic_cast <SipPresenceEvent *> (foundValue);
      UtlString oldStatus, status;
      UtlString id;
      NetMd5Codec::encode(contact, id);
      oldPresenceEvent->getTuple(id)->getStatus(oldStatus);
      presenceEvent->getTuple(id)->getStatus(status);
      
      if (status.compareTo(oldStatus) != 0)
      {
         requiredPublish = true;
         oldKey = mPresenceEventList.removeKeyAndValue(&contact, foundValue);
         delete oldKey;

         OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceMonitor::addPresenceEvent remove the presenceEvent %p for contact %s",
                       oldPresenceEvent, contact.data()); 

         int numOldContents;
         HttpBody* oldContent[1];           
         
         // Unpublish the content to the subscribe server
         if (!mSipPublishContentMgr.unpublish(contact.data(), PRESENCE_EVENT_TYPE, PRESENCE_EVENT_TYPE, 1, numOldContents, oldContent))
         {
            UtlString presenceContent;
            int length;
                 
            oldPresenceEvent->getBytes(&presenceContent, &length);
            OsSysLog::add(FAC_SIP, PRI_ERR, "SipPresenceMonitor::publishContent PresenceEvent %s\n was not successfully unpublished from the subscribe server",
                          presenceContent.data());
         }

         if (oldPresenceEvent)
         {
            delete oldPresenceEvent;
         }
      }
      else
      {
         delete presenceEvent;
      }
   }

   if (requiredPublish)
   {         
      // Insert it into the presence event list
      presenceEvent->buildBody();
      mPresenceEventList.insertKeyAndValue(new UtlString(contact), presenceEvent);

      if (mToBePublished)
      { 
         // Publish the content to the resource list
         publishContent(contact, presenceEvent);
      }
      
      // Notify the state change
      notifyStateChange(contact, presenceEvent);
   }
   
   return requiredPublish;
}


void SipPresenceMonitor::publishContent(UtlString& contact, SipPresenceEvent* presenceEvent)
{
#ifdef SUPPORT_RESOURCE_LIST
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
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceMonitor::publishContent listUri %s list %p",
                    listUri->data(), list); 

      // Search for the contact in this list
      resource = list->getResource(contact);
      if (resource)
      {
         resource->getInstance(id, state);
         
         if (presenceEvent->isEmpty())
         {
            resource->setInstance(id, STATE_TERMINATED);
         }
         else
         {
            UtlString id;
            NetMd5Codec::encode(contact, id);
            Tuple* tuple = presenceEvent->getTuple(id);
            
            UtlString status;
            tuple->getStatus(status);
            
            if (status.compareTo(STATUS_CLOSE) == 0)
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
         if (!mSipPublishContentMgr.publish(listUri->data(), PRESENCE_EVENT_TYPE, PRESENCE_EVENT_TYPE, 1, (HttpBody**)&list, 1, numOldContents, oldContent))
         {
            UtlString presenceContent;
            int length;
            
            list->getBytes(&presenceContent, &length);
            OsSysLog::add(FAC_SIP, PRI_ERR, "SipPresenceMonitor::publishContent PresenceEvent %s\n was not successfully published to the subscribe server",
                          presenceContent.data());
         }
      }      
   }
#endif

   int numOldContents;
   HttpBody* oldContent[1];           
   
   // Publish the content to the subscribe server
   if (!mSipPublishContentMgr.publish(contact.data(), PRESENCE_EVENT_TYPE, PRESENCE_EVENT_TYPE, 1, (HttpBody**)&presenceEvent, 1, numOldContents, oldContent))
   {
      UtlString presenceContent;
      int length;
           
      presenceEvent->getBytes(&presenceContent, &length);
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipPresenceMonitor::publishContent PresenceEvent %s\n was not successfully published to the subscribe server",
                    presenceContent.data());
   }
}


void SipPresenceMonitor::addStateChangeNotifier(const char* fileUrl, StateChangeNotifier* notifier)
{
   mLock.acquire();
   UtlString* name = new UtlString(fileUrl);
   UtlVoidPtr* value = new UtlVoidPtr(notifier);
   mStateChangeNotifiers.insertKeyAndValue(name, value);
   mLock.release();
}

void SipPresenceMonitor::removeStateChangeNotifier(const char* fileUrl)
{
   mLock.acquire();
   UtlString name(fileUrl);
   mStateChangeNotifiers.destroy(&name);
   mLock.release();
}

void SipPresenceMonitor::notifyStateChange(UtlString& contact, SipPresenceEvent* presenceEvent)
{

   // Loop through the notifier list
   UtlHashMapIterator iterator(mStateChangeNotifiers);
   UtlString* listUri;
   StateChangeNotifier* notifier;
   Url contactUrl(contact);
   mLock.acquire();
   while (listUri = dynamic_cast <UtlString *> (iterator()))
   {
      notifier = dynamic_cast <StateChangeNotifier *> (mStateChangeNotifiers.findValue(listUri));

      if (presenceEvent->isEmpty())
      {
         notifier->setStatus(contactUrl, StateChangeNotifier::AWAY);
      }
      else
      {
         UtlString id;
         NetMd5Codec::encode(contact, id);
         Tuple* tuple = presenceEvent->getTuple(id);
            
         UtlString status;
         tuple->getStatus(status);
            
         if (status.compareTo(STATUS_CLOSE) == 0)
         {
            notifier->setStatus(contactUrl, StateChangeNotifier::AWAY);
         }
         else
         {     
            notifier->setStatus(contactUrl, StateChangeNotifier::PRESENT);
         }
      }
   }
   mLock.release();
}

bool SipPresenceMonitor::setStatus(const Url& aor, const Status value)
{
   bool result = false;
   
   UtlString contact;
   aor.getUserId(contact);
   contact += mHostAndPort;
   
   // Create a presence event package and store it in the publisher
   SipPresenceEvent* sipPresenceEvent = new SipPresenceEvent(contact);
      
   UtlString id;
   NetMd5Codec::encode(contact, id);
   
   Tuple* tuple = new Tuple(id.data());
   
   if (value == StateChangeNotifier::PRESENT)
   {
      tuple->setStatus(STATUS_OPEN);
      tuple->setContact(contact, 1.0);
   }
   else
   {
      if (value == StateChangeNotifier::AWAY)
      {
         tuple->setStatus(STATUS_CLOSE);
         tuple->setContact(contact, 1.0);
      }
   }

   sipPresenceEvent->insertTuple(tuple); 
   
   // Add the SipPresenceEvent object to the subscribe list
   result = addPresenceEvent(contact, sipPresenceEvent);
   
   return result;
} 
