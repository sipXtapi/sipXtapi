//
// Copyright (C) 2007-2010 SIPez LLC  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <os/OsExcept.h>
#include <os/OsMsg.h>
#include <os/OsMsgQ.h>
#include <sipxunittests.h>

UtlBoolean gMsgReceived;

UtlBoolean msgSendHook(const OsMsg& rOsMsg)
{
    gMsgReceived = TRUE;

    return FALSE;
}

class OsMsgQTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(OsMsgQTest);
    CPPUNIT_TEST(testMessageQueue);
    CPPUNIT_TEST_SUITE_END();

public:

    void testMessageQueue()
    {
        OsMsgQ* pMsgQ1;
        OsMsg* pMsg1;
        OsMsg* pMsg2;
        OsMsg* pRecvMsg;
        
        pMsgQ1 = new OsMsgQ(OsMsgQ::DEF_MAX_MSGS, OsMsgQ::DEF_MAX_MSG_LEN,
                       OsMsgQ::Q_PRIORITY, "MQ1");

        pMsg1  = new OsMsg(OsMsg::UNSPECIFIED, 0);
        pMsg2  = new OsMsg(OsMsg::UNSPECIFIED, 0);

        CPPUNIT_ASSERT(pMsgQ1->isEmpty());
        CPPUNIT_ASSERT_EQUAL(0, pMsgQ1->numMsgs());
        CPPUNIT_ASSERT(pMsgQ1->getSendHook() == NULL);
        pMsgQ1->setSendHook(msgSendHook);
        CPPUNIT_ASSERT(pMsgQ1->getSendHook() == msgSendHook);

        gMsgReceived = FALSE;
        OsStatus stat = pMsgQ1->send(*pMsg1);
        CPPUNIT_ASSERT(gMsgReceived);

        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, stat);
        CPPUNIT_ASSERT(!pMsgQ1->isEmpty());
        CPPUNIT_ASSERT_EQUAL(1, pMsgQ1->numMsgs());

        stat = pMsgQ1->send(*pMsg2);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, stat);

        CPPUNIT_ASSERT_EQUAL(2, pMsgQ1->numMsgs());

        stat = pMsgQ1->receive(pRecvMsg);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, stat);

        delete pRecvMsg;
        CPPUNIT_ASSERT_EQUAL(1, pMsgQ1->numMsgs());

        stat = pMsgQ1->receive(pRecvMsg);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, stat);
        delete pRecvMsg;

        CPPUNIT_ASSERT_EQUAL(0, pMsgQ1->numMsgs());

        delete pMsg1;
        delete pMsg2;
        delete pMsgQ1;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsMsgQTest);



