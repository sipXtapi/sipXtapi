//
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include "CallStateEventBuilder_XML.h"

// Note: these tests will fail if PRETTYPRINT_EVENTS is defined in CallStateEventBuilder_XML.cpp

class CallStateEventBuilder_XMLTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(CallStateEventBuilder_XMLTest);
   CPPUNIT_TEST(testInitial);
   CPPUNIT_TEST(testRequest);
   CPPUNIT_TEST(testSetup);
   CPPUNIT_TEST(testFailure);
   CPPUNIT_TEST(testEnd);
   CPPUNIT_TEST_SUITE_END();


public:
   void testInitial()
      {
         UtlString event;
         
         CallStateEventBuilder_XML builder("observer.example.com");
         builder.observerEvent(0, "2004-12-15T11:42:41.010+0000", CallStateEventBuilder::ObserverReset, "testInitial");
         
         CPPUNIT_ASSERT(builder.xmlElement(event));
         CPPUNIT_ASSERT(0==event.compareTo("<call_event xmlns='http://www.sipfoundry.org/sipX/schema/xml/cse-01-00'><observer>observer.example.com</observer><obs_seq>0</obs_seq><obs_time>2004-12-15T11:42:41.010+0000</obs_time><obs_msg><obs_status>0</obs_status><obs_text>testInitial</obs_text></obs_msg></call_event></call_event>\n"));

         builder.xmlElement(event);
         CPPUNIT_ASSERT(event.isNull());
      }

   void testRequest()
      {
         UtlString event;
         
         CallStateEventBuilder_XML builder("observer.example.com");
         builder.observerEvent(0, "2004-12-15T11:42:41.011+0000", CallStateEventBuilder::ObserverReset, "testRequest");
         
         CPPUNIT_ASSERT(builder.xmlElement(event));
         CPPUNIT_ASSERT(0==event.compareTo("<call_event xmlns='http://www.sipfoundry.org/sipX/schema/xml/cse-01-00'><observer>observer.example.com</observer><obs_seq>0</obs_seq><obs_time>2004-12-15T11:42:41.011+0000</obs_time><obs_msg><obs_status>0</obs_status><obs_text>testRequest</obs_text></obs_msg></call_event></call_event>\n"));

         builder.callRequestEvent(1, "2004-12-15T11:42:41.012+0000", "Contact <sip:requestor@sip.net>");
         CPPUNIT_ASSERT(!builder.xmlElement(event));

         UtlString callId("08799710-9147-486B-A28D-FFDEB031106B@10.90.10.98");
         UtlString fromTag("8633744");
         UtlString toTag;
         UtlString fromField("\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=8633744");
         UtlString toField("\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>");
         builder.addCallData(callId, fromTag, toTag, fromField, toField);
         CPPUNIT_ASSERT(!builder.xmlElement(event));
         
         UtlString viaField("SIP/2.0/UDP 10.1.30.248:7003");
         builder.addEventVia(0, viaField);
         CPPUNIT_ASSERT(!builder.xmlElement(event));

         builder.completeCallEvent();
         CPPUNIT_ASSERT(builder.xmlElement(event));
         CPPUNIT_ASSERT(0==event.compareTo("<call_event xmlns='http://www.sipfoundry.org/sipX/schema/xml/cse-01-00'><observer>observer.example.com</observer><obs_seq>1</obs_seq><obs_time>2004-12-15T11:42:41.012+0000</obs_time><call_request><call><dialog><call_id>08799710-9147-486B-A28D-FFDEB031106B@10.90.10.98</call_id><from_tag>8633744</from_tag></dialog><from>&quot;Éê½­ÌÎ&quot;&lt;sip:1002@sip.net&gt;;tag=8633744</from><to>&quot;Joe Caller&quot;&lt;sip:jcaller@rhe-sipx.example.com&gt;</to></call><contact>Contact &lt;sip:requestor@sip.net&gt;</contact><via>SIP/2.0/UDP 10.1.30.248:7003</via></call_request></call_event>\n"));

         CPPUNIT_ASSERT(!builder.xmlElement(event));
         CPPUNIT_ASSERT(event.isNull());
      }

   void testSetup()
      {
         UtlString event;
         
         CallStateEventBuilder_XML builder("observer.example.com");
         builder.observerEvent(0, "2004-12-15T11:42:41.013+0000", CallStateEventBuilder::ObserverReset, "testSetup");
         
         CPPUNIT_ASSERT(builder.xmlElement(event));
         CPPUNIT_ASSERT(0==event.compareTo("<call_event xmlns='http://www.sipfoundry.org/sipX/schema/xml/cse-01-00'><observer>observer.example.com</observer><obs_seq>0</obs_seq><obs_time>2004-12-15T11:42:41.013+0000</obs_time><obs_msg><obs_status>0</obs_status><obs_text>testSetup</obs_text></obs_msg></call_event></call_event>\n"));

         builder.callSetupEvent(1, "2004-12-15T11:42:41.014+0000", "Contact <sip:responder@sip.net>");
         CPPUNIT_ASSERT(!builder.xmlElement(event));

         UtlString callId("08799710-9147-A28D-486B-FFDEB031106B@10.90.10.98");
         UtlString fromTag("3744863");
         UtlString toTag("19b8e5bK3a");
         UtlString fromField("\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=3744863");
         UtlString toField("\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>;tag=19b8e5bK3a");
         builder.addCallData(callId, fromTag, toTag, fromField, toField);
         CPPUNIT_ASSERT(!builder.xmlElement(event));
         
         UtlString viaField("SIP/2.0/UDP 10.1.30.248:7004");
         builder.addEventVia(0, viaField);
         CPPUNIT_ASSERT(!builder.xmlElement(event));

         builder.completeCallEvent();
         CPPUNIT_ASSERT(builder.xmlElement(event));
         CPPUNIT_ASSERT(0==event.compareTo("<call_event xmlns='http://www.sipfoundry.org/sipX/schema/xml/cse-01-00'><observer>observer.example.com</observer><obs_seq>1</obs_seq><obs_time>2004-12-15T11:42:41.014+0000</obs_time><call_setup><call><dialog><call_id>08799710-9147-A28D-486B-FFDEB031106B@10.90.10.98</call_id><from_tag>3744863</from_tag><to_tag>19b8e5bK3a</to_tag></dialog><from>&quot;Éê½­ÌÎ&quot;&lt;sip:1002@sip.net&gt;;tag=3744863</from><to>&quot;Joe Caller&quot;&lt;sip:jcaller@rhe-sipx.example.com&gt;;tag=19b8e5bK3a</to></call><contact>Contact &lt;sip:responder@sip.net&gt;</contact><via>SIP/2.0/UDP 10.1.30.248:7004</via></call_setup></call_event>\n"));

         CPPUNIT_ASSERT(!builder.xmlElement(event));
         CPPUNIT_ASSERT(event.isNull());
      }

   void testFailure()
      {
         UtlString event;
         
         CallStateEventBuilder_XML builder("observer.example.com");
         builder.observerEvent(0, "2004-12-15T11:42:41.015+0000", CallStateEventBuilder::ObserverReset, "testFailure");
         
         CPPUNIT_ASSERT(builder.xmlElement(event));
         CPPUNIT_ASSERT(0==event.compareTo("<call_event xmlns='http://www.sipfoundry.org/sipX/schema/xml/cse-01-00'><observer>observer.example.com</observer><obs_seq>0</obs_seq><obs_time>2004-12-15T11:42:41.015+0000</obs_time><obs_msg><obs_status>0</obs_status><obs_text>testFailure</obs_text></obs_msg></call_event></call_event>\n"));

         builder.callFailureEvent(1, "2004-12-15T11:42:41.016+0000", 403, "Forbidden <dummy>");
         CPPUNIT_ASSERT(!builder.xmlElement(event));

         UtlString callId("9147-08799710-A28D-486B-FFDEB031106B@10.90.10.98");
         UtlString fromTag("7448633");
         UtlString toTag("b8e5bK3a19");
         UtlString fromField("\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=7448633");
         UtlString toField("\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>;tag=b8e5bK3a19");
         builder.addCallData(callId, fromTag, toTag, fromField, toField);
         CPPUNIT_ASSERT(!builder.xmlElement(event));
         
         UtlString viaField("SIP/2.0/UDP 10.1.30.248:7005");
         builder.addEventVia(0, viaField);
         CPPUNIT_ASSERT(!builder.xmlElement(event));

         builder.completeCallEvent();
         CPPUNIT_ASSERT(builder.xmlElement(event));
         CPPUNIT_ASSERT(0==event.compareTo("<call_event xmlns='http://www.sipfoundry.org/sipX/schema/xml/cse-01-00'><observer>observer.example.com</observer><obs_seq>1</obs_seq><obs_time>2004-12-15T11:42:41.016+0000</obs_time><call_failure><call><dialog><call_id>9147-08799710-A28D-486B-FFDEB031106B@10.90.10.98</call_id><from_tag>7448633</from_tag><to_tag>b8e5bK3a19</to_tag></dialog><from>&quot;Éê½­ÌÎ&quot;&lt;sip:1002@sip.net&gt;;tag=7448633</from><to>&quot;Joe Caller&quot;&lt;sip:jcaller@rhe-sipx.example.com&gt;;tag=b8e5bK3a19</to></call><response><status>403</status><reason>Forbidden &lt;dummy&gt;</reason></response><via>SIP/2.0/UDP 10.1.30.248:7005</via></call_failure></call_event>\n"));

         CPPUNIT_ASSERT(!builder.xmlElement(event));
         CPPUNIT_ASSERT(event.isNull());
      }

   void testEnd()
      {
         UtlString event;
         
         CallStateEventBuilder_XML builder("observer.example.com");
         builder.observerEvent(0, "2004-12-15T11:42:41.017+0000", CallStateEventBuilder::ObserverReset, "testEnd");
         
         CPPUNIT_ASSERT(builder.xmlElement(event));
         CPPUNIT_ASSERT(0==event.compareTo("<call_event xmlns='http://www.sipfoundry.org/sipX/schema/xml/cse-01-00'><observer>observer.example.com</observer><obs_seq>0</obs_seq><obs_time>2004-12-15T11:42:41.017+0000</obs_time><obs_msg><obs_status>0</obs_status><obs_text>testEnd</obs_text></obs_msg></call_event></call_event>\n"));

         builder.callEndEvent(1, "2004-12-15T11:42:41.018+0000");
         CPPUNIT_ASSERT(!builder.xmlElement(event));

         UtlString callId("9147-A28D-08799710-486B-FFDEB031106B@10.90.10.98");
         UtlString fromTag("3374486");
         UtlString toTag("a19b8e5bK3");
         UtlString fromField("\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=3374486");
         UtlString toField("\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>;tag=a19b8e5bK3");
         builder.addCallData(callId, fromTag, toTag, fromField, toField);
         CPPUNIT_ASSERT(!builder.xmlElement(event));
         
         UtlString viaField("SIP/2.0/UDP 10.1.30.248:7006");
         builder.addEventVia(0, viaField);
         CPPUNIT_ASSERT(!builder.xmlElement(event));

         builder.completeCallEvent();
         CPPUNIT_ASSERT(builder.xmlElement(event));
         CPPUNIT_ASSERT(0==event.compareTo("<call_event xmlns='http://www.sipfoundry.org/sipX/schema/xml/cse-01-00'><observer>observer.example.com</observer><obs_seq>1</obs_seq><obs_time>2004-12-15T11:42:41.018+0000</obs_time><call_end><call><dialog><call_id>9147-A28D-08799710-486B-FFDEB031106B@10.90.10.98</call_id><from_tag>3374486</from_tag><to_tag>a19b8e5bK3</to_tag></dialog><from>&quot;Éê½­ÌÎ&quot;&lt;sip:1002@sip.net&gt;;tag=3374486</from><to>&quot;Joe Caller&quot;&lt;sip:jcaller@rhe-sipx.example.com&gt;;tag=a19b8e5bK3</to></call><via>SIP/2.0/UDP 10.1.30.248:7006</via></call_end></call_event>\n"));

         CPPUNIT_ASSERT(!builder.xmlElement(event));
         CPPUNIT_ASSERT(event.isNull());
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CallStateEventBuilder_XMLTest);

