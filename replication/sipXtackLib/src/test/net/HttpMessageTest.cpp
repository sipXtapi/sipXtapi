//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <net/HttpMessage.h>
#include <net/SdpBody.h>


/**
 * Unittest for HttpMessage
 */
class HttpMessageTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(HttpMessageTest);
    CPPUNIT_TEST(testCreator);
    CPPUNIT_TEST(testMessage);
    CPPUNIT_TEST(testSdp);
    CPPUNIT_TEST(testMd5Digest);
    CPPUNIT_TEST_SUITE_END();

public:

    void testCreator()
    {
        const char *hdr = "\r\nContent-Length: 0\r\n\r\n";
        int expectedLen = strlen(hdr);
        HttpMessage* msg = new HttpMessage();
        UtlString buf;
        int bufLen = 0;

        msg->getBytes(&buf, &bufLen);
        ASSERT_STR_EQUAL_MESSAGE("Set header comes back intact", hdr, (char *)buf.data());
        CPPUNIT_ASSERT_MESSAGE("bytes should not be null", !buf.isNull());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer should contain only content length field",
                expectedLen, bufLen);
        ASSERT_STR_EQUAL_MESSAGE("buffer should contain only content length field",
                buf.data(), hdr);
        delete msg;
    }

    /**
     * Test header, message, body, message contructor
     */
    void testMessage()
    {
        // TODO break this up into several tests. To intertwined

        const char* name = "Content-Type";
        const char* value = "text/plain";
        const char* httpTopLine = "GET /index.html HTTP/1.0";
        const char* valueRef = NULL;
        const char* n2 = "yyy";
        const char* v2 = "yyy-value";
        const char* v2a = "yyy-value2";
        UtlString messageBytes;
        UtlString messageBytes2;
        int messageLen = 0;
        int messageLen2 = 0;
        const char* body = "<HTML>\n<H3>Hello\n<BR>\n</HTML>\n";
        const HttpBody *bodyRef;
        int bodyLength = 0;
        UtlString headerLinePart;
        HttpMessage *msg;
        HttpMessage *msg2;

        msg = new HttpMessage();

        // H E A D E R
        int fieldCount = msg->getCountHeaderFields();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be zero",
                0, fieldCount);

        msg->addHeaderField(name, value);
        fieldCount = msg->getCountHeaderFields();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be zero", 1,
                fieldCount);

        valueRef = msg->getHeaderValue(0);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value,
                valueRef);

        msg->setFirstHeaderLine(httpTopLine);
        valueRef = msg->getFirstHeaderLine();
        ASSERT_STR_EQUAL_MESSAGE("incorrect top header line value",
                valueRef, httpTopLine);

        valueRef = msg->getHeaderValue(0, name);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value,
            valueRef);

        msg->addHeaderField(n2, v2);
        fieldCount = msg->getCountHeaderFields();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be 2", 2,
                fieldCount);

        valueRef = msg->getHeaderValue(0, n2);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", v2,
            valueRef);

                msg->addHeaderField(n2, v2a);
        fieldCount = msg->getCountHeaderFields();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be 3", 3,
                fieldCount);

        valueRef = msg->getHeaderValue(1, n2);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", v2a,
            valueRef);

        // B O D Y
        HttpBody *httpBody = new HttpBody(body, strlen(body));
        msg->setBody(httpBody);
        bodyRef = msg->getBody();
        CPPUNIT_ASSERT_MESSAGE("bad body pointer", httpBody == bodyRef);

        bodyRef->getBytes(&valueRef, &bodyLength);
        CPPUNIT_ASSERT_MESSAGE("bad body pointer", valueRef != NULL);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect body len", (int)strlen(body),
                bodyLength);
        ASSERT_STR_EQUAL_MESSAGE("incorrect body value", body, valueRef);

        const char* expectedLinePart[] = {
            "GET", "/index.html", "HTTP/1.0"
        };
        int n = sizeof(expectedLinePart) / sizeof(expectedLinePart[0]);

        for (int i = 0; i < n; i++)
        {
            msg->getFirstHeaderLinePart(i, &headerLinePart);
            CPPUNIT_ASSERT_MESSAGE("NULL header line part pointer",
                !headerLinePart.isNull());
            ASSERT_STR_EQUAL_MESSAGE("incorrect hdr line", expectedLinePart[i],
                headerLinePart.data());
            headerLinePart.remove(0);
        }

        msg->getBytes(&messageBytes, &messageLen);
        CPPUNIT_ASSERT_MESSAGE("NULL body pointer", !messageBytes.isNull());

        // message constructor
        msg2 = new HttpMessage(messageBytes.data(), messageLen);
        msg2->getBytes(&messageBytes2, &messageLen2);
        valueRef = msg2->getHeaderValue(0, name);
        ASSERT_STR_EQUAL_MESSAGE("incorrect message bytes", value, valueRef);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect message byte length",
                messageLen, messageLen2);
        delete msg2;

        delete msg;

        // AS DESIGNED: body delete is handled by delete msg
        // delete httpBody;
    }

    void testSdp()
    {
        const char* sip = "INVITE 14 SIP/2.0\nContent-Type:application/sdp\n\n"
            "v=0\nm=audio 49170 RTP/AVP 0\nc=IN IP4 224.2.17.12/127";

        HttpMessage *msg = new HttpMessage(sip);
        SdpBody *sdp = (SdpBody *)msg->getBody();

        CPPUNIT_ASSERT_MESSAGE("Null sdp buffer", sdp != NULL);

        int mediaCount = sdp->getMediaSetCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect media count", 1, mediaCount);

        const char* referenceSdp = 
            "v=0\r\nm=audio 49170 RTP/AVP 0\r\nc=IN IP4 224.2.17.12/127\r\n";
        const char* sdpBytes = NULL;
        int sdpByteLength = 0;
        sdp->getBytes(&sdpBytes, &sdpByteLength);
        for(int iii = 0; iii < sdpByteLength; iii++)
        {
            if(referenceSdp[iii] != sdpBytes[iii])
            {
                printf("index[%d]: expected: %d got: %d\n",
                    iii, referenceSdp[iii], sdpBytes[iii]);
            }
        }
        CPPUNIT_ASSERT_MESSAGE("Null sdp serialized content", sdpBytes != NULL);
        CPPUNIT_ASSERT_MESSAGE("SDP does not match expected content",
            strcmp(referenceSdp, sdpBytes) == 0);

        HttpMessage* msgCopy = new HttpMessage(*msg);
        CPPUNIT_ASSERT_MESSAGE("NULL message copy", msgCopy != NULL);
        SdpBody *sdpCopy = (SdpBody *)msgCopy->getBody();
        CPPUNIT_ASSERT_MESSAGE("NULL SDP copy", sdpCopy != NULL);
        const char* sdpCopyBytes = NULL;
        int sdpCopyLen = 0;
        sdpCopy->getBytes(&sdpCopyBytes, &sdpCopyLen);
        //printf("SDP copy length: %d\n%s\n", sdpCopyLen, sdpCopyBytes);
        CPPUNIT_ASSERT_MESSAGE("Null sdp copy serialized content", sdpCopyBytes != NULL);
        CPPUNIT_ASSERT_MESSAGE("SDP does not match expected content",
            strcmp(referenceSdp, sdpCopyBytes) == 0);
    }


    void testMd5Digest()
    {
        const char* passToken = "john.salesman:sales@www/example.com:5+5=10";
        const char* nonce = "dcd98b7102dd2f0e8b11d0f600bfb0c093";
        const char* alg = HTTP_MD5_SESSION_ALGORITHM;
        const char *cnonce = NULL;
        int nonceCount = 0;
        const char* method = "GET";
        const char* qop = NULL;
        const char* uri = "/private/prices.html";
        const char *bodyDigest = NULL;
        const char* response = "739fc56a88db37aeebefe70572aeda5f";
        UtlString responseToken;

        HttpMessage::buildMd5Digest(passToken, alg, nonce, cnonce,  nonceCount,
                                   qop, method, uri, bodyDigest, &responseToken);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("httpmessage digest test",
            0, responseToken.compareTo(response));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(HttpMessageTest);
