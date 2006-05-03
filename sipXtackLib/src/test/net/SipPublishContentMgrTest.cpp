//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <net/SipPublishContentMgr.h>
#include <net/HttpBody.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/SipMessage.h>

static void* mAppData = NULL;
static UtlString mResourceId;
static UtlString mEventTypeKey;
static UtlString mEventType;
static UtlBoolean mIsDefaultContent = TRUE;

void static contentChangeCallback(void* applicationData,
                                 const char* resourceId,
                                 const char* eventTypeKey,
                                 const char* eventType,
                                 UtlBoolean isDefaultContent)
{
   mAppData = applicationData;
   mResourceId = resourceId;
   mEventTypeKey = eventTypeKey;
   mEventType = eventType;
   mIsDefaultContent = isDefaultContent;
}

/**
 * Unit test for SipPublishContentMgr
 */
class SipPublishContentMgrTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipPublishContentMgrTest);
   CPPUNIT_TEST(testDefaultPublishContent);
   CPPUNIT_TEST(testPublishContent);
   CPPUNIT_TEST(testGetContent);
   CPPUNIT_TEST(testContentChangeObserver);
   CPPUNIT_TEST_SUITE_END();

public:

   void testDefaultPublishContent()
      {
         const char *content = 
            "<?xml version=\"1.0\"?>\n"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" version=\"0\" state=\"full\" entity=\"moh@panther.pingtel.com:5120\">\n"
            "<dialog id=\"1\" call-id=\"call-1116603513-270@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>moh@panther.pingtel.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "<dialog id=\"2\" call-id=\"call-1116603513-890@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>moh@panther.pingtel.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "<dialog id=\"3\" call-id=\"call-2226603513-890@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>moh@panther.pingtel.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "</dialog-info>\n"
            ;
       
         SipPublishContentMgr publisher;

         int bodyLength = strlen(content);
         HttpBody *body = new HttpBody(content, bodyLength, "text/xml");

         int numOldContents;
         HttpBody *oldContents[2];

         publisher.publishDefault("dialog", "dialog", 1, &body,
                                  1, numOldContents, oldContents);

         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of contents should be zero",
                                      0, numOldContents);

         publisher.unpublishDefault("dialog", "dialog",
                                    1, numOldContents, oldContents);

         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of contents are not the same",
                                      1, numOldContents);
         
         CPPUNIT_ASSERT_MESSAGE("bad body pointer", body == oldContents[0]);
      }

   void testPublishContent()
      {
         const char *content = 
            "<?xml version=\"1.0\"?>\n"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" version=\"0\" state=\"full\" entity=\"moh@panther.pingtel.com:5120\">\n"
            "<dialog id=\"1\" call-id=\"call-1116603513-270@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>moh@panther.pingtel.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "<dialog id=\"2\" call-id=\"call-1116603513-890@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>moh@panther.pingtel.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "<dialog id=\"3\" call-id=\"call-2226603513-890@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>moh@panther.pingtel.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "</dialog-info>\n"
            ;
       
         SipPublishContentMgr publisher;

         int bodyLength = strlen(content);
         HttpBody *body = new HttpBody(content, bodyLength, "text/xml");

         int numOldContents;
         HttpBody *oldContents[2];

         publisher.publish("moh@pingtel.com", "dialog", "dialog", 1, &body,
                           1, numOldContents, oldContents);

         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of contents should be zero",
                                      0, numOldContents);

         SipSubscribeServerEventHandler eventHandler;
         SipMessage notifyRequest;
         CPPUNIT_ASSERT(eventHandler.getNotifyContent("moh@pingtel.com", 
                                       "dialog",
                                        publisher,
                                        "text/xml",
                                        notifyRequest));
         const char* notifyBodyBytes = NULL;
         int notifyBodySize = 0;
         const HttpBody* notifyBody = notifyRequest.getBody();
         notifyBody->getBytes(&notifyBodyBytes, &notifyBodySize);
         CPPUNIT_ASSERT(notifyBodyBytes);
         CPPUNIT_ASSERT(strcmp(content, notifyBodyBytes) == 0);

         publisher.unpublish("moh@pingtel.com", "dialog", "dialog",
                             1, numOldContents, oldContents);

         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of contents are not the same",
                                      1, numOldContents);

         CPPUNIT_ASSERT_MESSAGE("bad body pointer", body == oldContents[0]);
      }

   void testGetContent()
      {
         const char *content = 
            "<?xml version=\"1.0\"?>\n"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" version=\"0\" state=\"full\" entity=\"moh@panther.pingtel.com:5120\">\n"
            "<dialog id=\"1\" call-id=\"call-1116603513-270@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>moh@panther.pingtel.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "<dialog id=\"2\" call-id=\"call-1116603513-890@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>moh@panther.pingtel.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "<dialog id=\"3\" call-id=\"call-2226603513-890@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>moh@panther.pingtel.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "</dialog-info>\n"
            ;
       
         SipPublishContentMgr publisher;

         int bodyLength = strlen(content);
         HttpBody *body = new HttpBody(content, bodyLength, "text/xml");

         int numOldContents;
         HttpBody *oldContents[2];

         publisher.publish("moh@pingtel.com", "dialog", "dialog", 1, &body,
                           1, numOldContents, oldContents);

         UtlBoolean foundContent;
         UtlBoolean isDefaultContent;

         foundContent = publisher.getContent("moh@pingtel.com", "dialog", "application/dialog-info+xml",
                                             oldContents[0], isDefaultContent);

         CPPUNIT_ASSERT(FALSE==isDefaultContent);

         CPPUNIT_ASSERT(TRUE==foundContent);

         int length;
         const char* contentBody = NULL;

         oldContents[0]->getBytes(&contentBody, &length);

         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of bytes are not the same",
                                      bodyLength, length);

         ASSERT_STR_EQUAL_MESSAGE("incorrect body value", content, contentBody);

         foundContent = publisher.getContent("something-else@pingtel.com", "dialog", "application/dialog-info+xml",
                                             oldContents[0], isDefaultContent);

         CPPUNIT_ASSERT(FALSE==foundContent);

         publisher.unpublish("moh@pingtel.com", "dialog", "dialog",
                             1, numOldContents, oldContents);

         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of contents are not the same",
                                      1, numOldContents);

         CPPUNIT_ASSERT_MESSAGE("bad body pointer", body == oldContents[0]);
      }

   void testContentChangeObserver()
      {
         const char *content = 
            "<?xml version=\"1.0\"?>\n"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" version=\"0\" state=\"full\" entity=\"moh@panther.pingtel.com:5120\">\n"
            "</dialog-info>\n"
            ;
         const char *resourceId = "moh@pingtel.com";
         const char *eventType = "dialog";
         const void* appData = "testContentChangeObserver";

         SipPublishContentMgr publisher;

         publisher.setContentChangeObserver(eventType, (void *)appData, contentChangeCallback);

         int bodyLength = strlen(content);
         HttpBody *body = new HttpBody(content, bodyLength, "text/xml");

         int numOldContents;
         HttpBody *oldContents[2];

         publisher.publish(resourceId, eventType, eventType, 1, &body,
                           1, numOldContents, oldContents);

         CPPUNIT_ASSERT_MESSAGE("bad app data pointer", appData == mAppData);
         ASSERT_STR_EQUAL_MESSAGE("incorrect resource Id", resourceId, mResourceId.data());
         ASSERT_STR_EQUAL_MESSAGE("incorrect event type key", eventType, mEventTypeKey.data());
         ASSERT_STR_EQUAL_MESSAGE("incorrect event type", eventType, mEventType.data());
         CPPUNIT_ASSERT(FALSE==mIsDefaultContent);

         void* myAppData = NULL;

         SipPublishContentMgr::SipPublisherContentChangeCallback myCallbackFunc;

         publisher.removeContentChangeObserver(eventType, ((void*&)myAppData), myCallbackFunc);

         CPPUNIT_ASSERT_MESSAGE("bad app data pointer", appData == myAppData);

         CPPUNIT_ASSERT_MESSAGE("bad callback founction pointer", contentChangeCallback == myCallbackFunc);
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipPublishContentMgrTest);
