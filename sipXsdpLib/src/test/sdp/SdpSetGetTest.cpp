//  
// Copyright (C) 2006 SIPez LLC. 
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

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <iostream>
#include <utl/UtlSListIterator.h>

#include <sdp/SdpCodec.h>
#include <sdp/Sdp.h>
#include <sdp/SdpMediaLine.h>
#include <sdp/SdpHelperResip.h>

/**
* Unittest for Sdp object setters and getters.
*/
class SdpSetGetTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(SdpSetGetTest);
    CPPUNIT_TEST(testOriginatorInfo);
    CPPUNIT_TEST(testSessionInfo);
    CPPUNIT_TEST(testEmailAddrs);
    CPPUNIT_TEST(testPhoneNumbers);
    CPPUNIT_TEST(testBandwidth);
    CPPUNIT_TEST(testTime);
    CPPUNIT_TEST(testMisc);
    CPPUNIT_TEST(testSdpGroup);
    CPPUNIT_TEST(testLanguage);
    CPPUNIT_TEST(testMediaLine);
    CPPUNIT_TEST(testSdpToString);
    CPPUNIT_TEST(testFoundationId);
    CPPUNIT_TEST_SUITE_END();
private:
    Sdp mSdp;
public:
    void testOriginatorInfo()
    {
        mSdp.setSdpVersion(1);
        mSdp.setOriginatorUserName("Test");
        mSdp.setOriginatorSessionId(8888);
        mSdp.setOriginatorSessionVersion(9999);
        mSdp.setOriginatorNetType(Sdp::NET_TYPE_IN);
        mSdp.setOriginatorAddressType(Sdp::ADDRESS_TYPE_IP4);
        mSdp.setOriginatorUnicastAddress("127.0.0.1");
        
        // Verify that we get what we set.
        CPPUNIT_ASSERT(mSdp.getOriginatorUserName() == "Test");
        CPPUNIT_ASSERT(mSdp.getOriginatorSessionId() == 8888);
        CPPUNIT_ASSERT(mSdp.getOriginatorSessionVersion() == 9999);
        CPPUNIT_ASSERT(mSdp.getOriginatorNetType() == Sdp::NET_TYPE_IN);
        CPPUNIT_ASSERT(mSdp.getOriginatorAddressType() == Sdp::ADDRESS_TYPE_IP4);
        CPPUNIT_ASSERT(mSdp.getOriginatorUnicastAddress() == "127.0.0.1");
    }

    void testSessionInfo()
    {
        mSdp.setSessionName("SdpTestSession");
        mSdp.setSessionInformation("sample session information");
        mSdp.setSessionUri("http://www.sessionuri.com");
        CPPUNIT_ASSERT(mSdp.getSessionName() == "SdpTestSession");
        CPPUNIT_ASSERT(mSdp.getSessionInformation() == "sample session information");
        CPPUNIT_ASSERT(mSdp.getSessionUri() == "http://www.sessionuri.com");
    }

    void testEmailAddrs()
    {
        mSdp.addEmailAddress("me@here.com");
        mSdp.addEmailAddress("you@there.com");
        CPPUNIT_ASSERT(mSdp.getEmailAddresses().entries() == 2);
    }


    void testPhoneNumbers()
    {
        mSdp.addPhoneNumber("555-555-5555");
        mSdp.addPhoneNumber("123-123-1234");
        CPPUNIT_ASSERT(mSdp.getPhoneNumbers().entries() == 2);
    }

    void testBandwidth()
    {
        mSdp.addBandwidth(Sdp::BANDWIDTH_TYPE_CT, 1000);
        CPPUNIT_ASSERT(mSdp.getBandwidths().entries() == 1);
        
        mSdp.clearBandwidths();
        CPPUNIT_ASSERT(mSdp.getBandwidths().entries() == 0);
        
        mSdp.addBandwidth(Sdp::BANDWIDTH_TYPE_AS, 5000);
        mSdp.addBandwidth(Sdp::BANDWIDTH_TYPE_CT, 100);
        CPPUNIT_ASSERT(mSdp.getBandwidths().entries() == 2);
        CPPUNIT_ASSERT(((Sdp::SdpBandwidth*)mSdp.getBandwidths().at(0))->getType() == Sdp::BANDWIDTH_TYPE_AS);
        CPPUNIT_ASSERT(((Sdp::SdpBandwidth*)mSdp.getBandwidths().at(0))->getBandwidth() == 5000);
        CPPUNIT_ASSERT(((Sdp::SdpBandwidth*)mSdp.getBandwidths().at(1))->getType() == Sdp::BANDWIDTH_TYPE_CT);
        CPPUNIT_ASSERT(((Sdp::SdpBandwidth*)mSdp.getBandwidths().at(1))->getBandwidth() == 100);
    }

    void testTime()
    {
        Sdp::SdpTime* sdpTime = new Sdp::SdpTime(100, 200);
        UtlSList offsetsFromStartTime;  // empty
        Sdp::SdpTime::SdpTimeRepeat* repeat = new Sdp::SdpTime::SdpTimeRepeat(8, 800);
        sdpTime->addRepeat(repeat);
        repeat = new Sdp::SdpTime::SdpTimeRepeat(9, 900);
        repeat->addOffsetFromStartTime(20);
        repeat->addOffsetFromStartTime(21);
        sdpTime->addRepeat(repeat);
        mSdp.addTime(sdpTime);
        mSdp.addTime(300, 400);
        CPPUNIT_ASSERT(mSdp.getTimes().entries() == 2);      
        CPPUNIT_ASSERT(((Sdp::SdpTime*)mSdp.getTimes().at(0))->getStartTime() == 100);
        CPPUNIT_ASSERT(((Sdp::SdpTime*)mSdp.getTimes().at(0))->getStopTime() == 200);
        CPPUNIT_ASSERT(((Sdp::SdpTime*)mSdp.getTimes().at(0))->getRepeats().entries() == 2);
        CPPUNIT_ASSERT(((Sdp::SdpTime::SdpTimeRepeat*)((Sdp::SdpTime*)mSdp.getTimes().at(0))->getRepeats().at(0))->getRepeatInterval() == 8);
        CPPUNIT_ASSERT(((Sdp::SdpTime::SdpTimeRepeat*)((Sdp::SdpTime*)mSdp.getTimes().at(0))->getRepeats().at(0))->getActiveDuration() == 800);
        CPPUNIT_ASSERT(((Sdp::SdpTime::SdpTimeRepeat*)((Sdp::SdpTime*)mSdp.getTimes().at(0))->getRepeats().at(1))->getRepeatInterval() == 9);
        CPPUNIT_ASSERT(((Sdp::SdpTime::SdpTimeRepeat*)((Sdp::SdpTime*)mSdp.getTimes().at(0))->getRepeats().at(1))->getActiveDuration() == 900);
        CPPUNIT_ASSERT(((Sdp::SdpTime::SdpTimeRepeat*)((Sdp::SdpTime*)mSdp.getTimes().at(0))->getRepeats().at(1))->getOffsetsFromStartTime().entries() == 2);
        CPPUNIT_ASSERT(((UtlInt*)((Sdp::SdpTime::SdpTimeRepeat*)((Sdp::SdpTime*)mSdp.getTimes().at(0))->getRepeats().at(1))->getOffsetsFromStartTime().at(0))->getValue() == 20);
        CPPUNIT_ASSERT(((UtlInt*)((Sdp::SdpTime::SdpTimeRepeat*)((Sdp::SdpTime*)mSdp.getTimes().at(0))->getRepeats().at(1))->getOffsetsFromStartTime().at(1))->getValue() == 21);
        CPPUNIT_ASSERT(((Sdp::SdpTime*)mSdp.getTimes().at(1))->getStartTime() == 300);
        CPPUNIT_ASSERT(((Sdp::SdpTime*)mSdp.getTimes().at(1))->getStopTime() == 400);
        mSdp.addTimeZone(500, 600);
        mSdp.addTimeZone(700, 800);
        CPPUNIT_ASSERT(mSdp.getTimeZones().entries() == 2);
        CPPUNIT_ASSERT(((Sdp::SdpTimeZone*)mSdp.getTimeZones().at(0))->getAdjustmentTime() == 500);
        CPPUNIT_ASSERT(((Sdp::SdpTimeZone*)mSdp.getTimeZones().at(0))->getOffset() == 600);
        CPPUNIT_ASSERT(((Sdp::SdpTimeZone*)mSdp.getTimeZones().at(1))->getAdjustmentTime() == 700);
        CPPUNIT_ASSERT(((Sdp::SdpTimeZone*)mSdp.getTimeZones().at(1))->getOffset() == 800);
    }

    void testMisc()
    {
        mSdp.setCategory("sample sdp category");
        mSdp.setKeywords("sdp session description protocol");
        mSdp.setToolNameAndVersion("sipX session description 1.0");
        mSdp.setConferenceType(Sdp::CONFERENCE_TYPE_BROADCAST);
        mSdp.setCharSet("UTF-8");
        mSdp.setIcePassiveOnlyMode(true);
        CPPUNIT_ASSERT(mSdp.getCategory() == "sample sdp category");
        CPPUNIT_ASSERT(mSdp.getKeywords() == "sdp session description protocol");
        CPPUNIT_ASSERT(mSdp.getToolNameAndVersion() == "sipX session description 1.0");
        CPPUNIT_ASSERT(mSdp.getConferenceType() == Sdp::CONFERENCE_TYPE_BROADCAST);
        CPPUNIT_ASSERT(mSdp.getCharSet() == "UTF-8");
        CPPUNIT_ASSERT(mSdp.isIcePassiveOnlyMode() == true);
    }

    void testSdpGroup()
    {
        Sdp::SdpGroup* group = new Sdp::SdpGroup(Sdp::GROUP_SEMANTICS_LS);
        group->addIdentificationTag("media1");
        group->addIdentificationTag("media2");
        mSdp.addGroup(group);
        group = new Sdp::SdpGroup(Sdp::GROUP_SEMANTICS_FID);
        group->addIdentificationTag("fid1");
        mSdp.addGroup(group);
        CPPUNIT_ASSERT(mSdp.getGroups().entries() == 2);
        CPPUNIT_ASSERT(((Sdp::SdpGroup*)mSdp.getGroups().at(0))->getIdentificationTags().entries() == 2);
        CPPUNIT_ASSERT(((Sdp::SdpGroup*)mSdp.getGroups().at(0))->getSemantics() == Sdp::GROUP_SEMANTICS_LS);
        CPPUNIT_ASSERT(((UtlString*)((Sdp::SdpGroup*)mSdp.getGroups().at(0))->getIdentificationTags().at(0))->compareTo("media1") == 0);
        CPPUNIT_ASSERT(((UtlString*)((Sdp::SdpGroup*)mSdp.getGroups().at(0))->getIdentificationTags().at(1))->compareTo("media2") == 0);
        CPPUNIT_ASSERT(((Sdp::SdpGroup*)mSdp.getGroups().at(1))->getSemantics() == Sdp::GROUP_SEMANTICS_FID);
        CPPUNIT_ASSERT(((UtlString*)((Sdp::SdpGroup*)mSdp.getGroups().at(1))->getIdentificationTags().at(0))->compareTo("fid1") == 0);
    }

    void testLanguage()
    {
        mSdp.setSessionLanguage("EN-US");
        mSdp.setDescriptionLanguage("FR-CN");
        mSdp.setMaximumPacketRate(1.5);
        CPPUNIT_ASSERT(mSdp.getSessionLanguage() == "EN-US");
        CPPUNIT_ASSERT(mSdp.getDescriptionLanguage() == "FR-CN");
        CPPUNIT_ASSERT(mSdp.getMaximumPacketRate() == 1.5);
    }

    void testMediaLine()
    {
        SdpMediaLine* mediaLine = new SdpMediaLine();
        mediaLine->setMediaType(SdpMediaLine::MEDIA_TYPE_AUDIO);
        mediaLine->setTransportProtocolType(SdpMediaLine::PROTOCOL_TYPE_RTP_AVP);
        mediaLine->setTitle("G729a Audio Codec");
        mediaLine->addConnection(Sdp::NET_TYPE_IN, Sdp::ADDRESS_TYPE_IP4, "127.0.0.1", 6000, 90);
        mediaLine->addRtcpConnection(Sdp::NET_TYPE_IN, Sdp::ADDRESS_TYPE_IP4, "127.0.0.1", 6001);
        mediaLine->addBandwidth(Sdp::BANDWIDTH_TYPE_CT, 10);
        mediaLine->setEncryptionKey(SdpMediaLine::ENCRYPTION_METHOD_BASE64, "0123456789ABCD");
        mediaLine->setDirection(SdpMediaLine::DIRECTION_TYPE_SENDRECV);
        mediaLine->setPacketTime(20);
        mediaLine->setMaxPacketTime(60);
        mediaLine->setOrientation(SdpMediaLine::ORIENTATION_TYPE_LANDSCAPE);
        mediaLine->setDescriptionLanguage("English");
        mediaLine->setLanguage("EN");
        mediaLine->setFrameRate(256);
        mediaLine->setQuality(10);
        mediaLine->setTcpConnectionAttribute(SdpMediaLine::TCP_CONNECTION_ATTRIBUTE_NEW);
        mediaLine->setTcpSetupAttribute(SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTPASS);
        CPPUNIT_ASSERT(mediaLine->getMediaType() == SdpMediaLine::MEDIA_TYPE_AUDIO);
        CPPUNIT_ASSERT(mediaLine->getTransportProtocolType() == SdpMediaLine::PROTOCOL_TYPE_RTP_AVP);
        CPPUNIT_ASSERT(mediaLine->getTitle() == "G729a Audio Codec");
        CPPUNIT_ASSERT(mediaLine->getConnections().entries() == 1);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpConnection*)mediaLine->getConnections().at(0))->getNetType() == Sdp::NET_TYPE_IN);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpConnection*)mediaLine->getConnections().at(0))->getAddressType() == Sdp::ADDRESS_TYPE_IP4);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpConnection*)mediaLine->getConnections().at(0))->getAddress() == "127.0.0.1");
        CPPUNIT_ASSERT(((SdpMediaLine::SdpConnection*)mediaLine->getConnections().at(0))->getPort() == 6000);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpConnection*)mediaLine->getConnections().at(0))->getMulticastIpV4Ttl() == 90);
        CPPUNIT_ASSERT(mediaLine->getRtcpConnections().entries() == 1);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpConnection*)mediaLine->getRtcpConnections().at(0))->getNetType() == Sdp::NET_TYPE_IN);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpConnection*)mediaLine->getRtcpConnections().at(0))->getAddressType() == Sdp::ADDRESS_TYPE_IP4);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpConnection*)mediaLine->getRtcpConnections().at(0))->getAddress() == "127.0.0.1");
        CPPUNIT_ASSERT(((SdpMediaLine::SdpConnection*)mediaLine->getRtcpConnections().at(0))->getPort() == 6001);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpConnection*)mediaLine->getRtcpConnections().at(0))->getMulticastIpV4Ttl() == 0);
        CPPUNIT_ASSERT(mediaLine->getBandwidths().entries() == 1);
        CPPUNIT_ASSERT(mediaLine->getEncryptionKey() == "0123456789ABCD");
        CPPUNIT_ASSERT(mediaLine->getEncryptionMethod() == SdpMediaLine::ENCRYPTION_METHOD_BASE64);
        CPPUNIT_ASSERT(mediaLine->getDirection() == SdpMediaLine::DIRECTION_TYPE_SENDRECV);
        CPPUNIT_ASSERT(mediaLine->getPacketTime() == 20);
        CPPUNIT_ASSERT(mediaLine->getMaxPacketTime() == 60);
        CPPUNIT_ASSERT(mediaLine->getOrientation() == SdpMediaLine::ORIENTATION_TYPE_LANDSCAPE);
        CPPUNIT_ASSERT(mediaLine->getDescriptionLanguage() == "English");
        CPPUNIT_ASSERT(mediaLine->getLanguage() == "EN");
        CPPUNIT_ASSERT(mediaLine->getFrameRate() == 256);
        CPPUNIT_ASSERT(mediaLine->getQuality() == 10);
        CPPUNIT_ASSERT(mediaLine->getTcpConnectionAttribute() == SdpMediaLine::TCP_CONNECTION_ATTRIBUTE_NEW);
        CPPUNIT_ASSERT(mediaLine->getTcpSetupAttribute() == SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTPASS);

        SdpMediaLine::SdpCrypto *crypto = new SdpMediaLine::SdpCrypto;
        crypto->setTag(1);
        crypto->setSuite(SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32);
        crypto->setSrtpFecOrder(SdpMediaLine::CRYPTO_SRTP_FEC_ORDER_SRTP_FEC);
        crypto->setSrtpFecKey(SdpMediaLine::CRYPTO_KEY_METHOD_INLINE, "FECKEY", 200000, 2, 8);
        crypto->setAuthenticatedSrtp(true);
        crypto->setEncryptedSrtcp(true);
        crypto->setEncryptedSrtp(true);
        crypto->setSrtpKdr(64);
        crypto->setSrtpWsh(32);
        crypto->addCryptoKeyParam(SdpMediaLine::CRYPTO_KEY_METHOD_INLINE, "CyrptoKey", 10000, 1, 5);
        mediaLine->addCryptoSettings(crypto);
        crypto = new SdpMediaLine::SdpCrypto;
        crypto->setTag(2);
        crypto->setSuite(SdpMediaLine::CRYPTO_SUITE_TYPE_F8_128_HMAC_SHA1_80);
        crypto->addCryptoKeyParam(SdpMediaLine::CRYPTO_KEY_METHOD_INLINE, "CryptoKey2");
        crypto->addGenericSessionParam("param1");
        crypto->addGenericSessionParam("param2");
        CPPUNIT_ASSERT(crypto->getGenericSessionParams().entries() == 2);
        CPPUNIT_ASSERT(crypto->getTag() == 2);
        CPPUNIT_ASSERT(crypto->getSuite() == SdpMediaLine::CRYPTO_SUITE_TYPE_F8_128_HMAC_SHA1_80);
        CPPUNIT_ASSERT(crypto->getCryptoKeyParams().entries() == 1);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpCrypto::SdpCryptoKeyParam*)crypto->getCryptoKeyParams().at(0))->getKeyMethod() == SdpMediaLine::CRYPTO_KEY_METHOD_INLINE);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpCrypto::SdpCryptoKeyParam*)crypto->getCryptoKeyParams().at(0))->getKeyValue() == "CryptoKey2");
        CPPUNIT_ASSERT(crypto->getGenericSessionParams().entries() == 2);
        mediaLine->addCryptoSettings(crypto);
        CPPUNIT_ASSERT(mediaLine->getCryptos().entries() == 2);

        mediaLine->setFingerPrint(SdpMediaLine::FINGERPRINT_HASH_FUNC_MD5, "this-is-a-finger-print");
        mediaLine->setKeyManagementProtocol(SdpMediaLine::KEYMANAGEMENT_PROTOCOL_MIKEY, "this-is-some-key-management-data");
        mediaLine->addPreConditionCurrentStatus(SdpMediaLine::PRECONDITION_TYPE_QOS, SdpMediaLine::PRECONDITION_STATUS_E2E, SdpMediaLine::PRECONDITION_DIRECTION_SENDRECV);
        mediaLine->addPreConditionCurrentStatus(SdpMediaLine::PRECONDITION_TYPE_QOS, SdpMediaLine::PRECONDITION_STATUS_REMOTE, SdpMediaLine::PRECONDITION_DIRECTION_RECV);
        mediaLine->addPreConditionConfirmStatus(SdpMediaLine::PRECONDITION_TYPE_QOS, SdpMediaLine::PRECONDITION_STATUS_E2E, SdpMediaLine::PRECONDITION_DIRECTION_SENDRECV);
        mediaLine->addPreConditionConfirmStatus(SdpMediaLine::PRECONDITION_TYPE_QOS, SdpMediaLine::PRECONDITION_STATUS_REMOTE, SdpMediaLine::PRECONDITION_DIRECTION_RECV);
        mediaLine->addPreConditionDesiredStatus(SdpMediaLine::PRECONDITION_TYPE_QOS, SdpMediaLine::PRECONDITION_STRENGTH_MANDATORY, SdpMediaLine::PRECONDITION_STATUS_E2E, SdpMediaLine::PRECONDITION_DIRECTION_SENDRECV);
        mediaLine->addPreConditionDesiredStatus(SdpMediaLine::PRECONDITION_TYPE_QOS, SdpMediaLine::PRECONDITION_STRENGTH_OPTIONAL, SdpMediaLine::PRECONDITION_STATUS_REMOTE, SdpMediaLine::PRECONDITION_DIRECTION_RECV);
        CPPUNIT_ASSERT(mediaLine->getFingerPrint() == "this-is-a-finger-print");
        CPPUNIT_ASSERT(mediaLine->getFingerPrintHashFunction() == SdpMediaLine::FINGERPRINT_HASH_FUNC_MD5);
        CPPUNIT_ASSERT(mediaLine->getPreConditionCurrentStatus().entries() == 2);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpPreCondition*)mediaLine->getPreConditionCurrentStatus().at(0))->getType() == SdpMediaLine::PRECONDITION_TYPE_QOS);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpPreCondition*)mediaLine->getPreConditionCurrentStatus().at(0))->getStatus() == SdpMediaLine::PRECONDITION_STATUS_E2E);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpPreCondition*)mediaLine->getPreConditionCurrentStatus().at(1))->getDirection() == SdpMediaLine::PRECONDITION_DIRECTION_RECV);
        CPPUNIT_ASSERT(mediaLine->getPreConditionConfirmStatus().entries() == 2);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpPreCondition*)mediaLine->getPreConditionConfirmStatus().at(0))->getType() == SdpMediaLine::PRECONDITION_TYPE_QOS);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpPreCondition*)mediaLine->getPreConditionConfirmStatus().at(1))->getStatus() == SdpMediaLine::PRECONDITION_STATUS_REMOTE);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpPreCondition*)mediaLine->getPreConditionConfirmStatus().at(0))->getDirection() == SdpMediaLine::PRECONDITION_DIRECTION_SENDRECV);
        CPPUNIT_ASSERT(mediaLine->getPreConditionDesiredStatus().entries() == 2);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpPreConditionDesiredStatus*)mediaLine->getPreConditionDesiredStatus().at(0))->getStrength() == SdpMediaLine::PRECONDITION_STRENGTH_MANDATORY);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpPreConditionDesiredStatus*)mediaLine->getPreConditionDesiredStatus().at(1))->getStatus() == SdpMediaLine::PRECONDITION_STATUS_REMOTE);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpPreConditionDesiredStatus*)mediaLine->getPreConditionDesiredStatus().at(0))->getDirection() == SdpMediaLine::PRECONDITION_DIRECTION_SENDRECV);

        mediaLine->setMaximumPacketRate(20);
        mediaLine->setLabel("G711 Label");
        mediaLine->setIdentificationTag("item1");
        mediaLine->setIceUserFrag("ICEUSER");
        mediaLine->setIcePassword("ICEPASSWORD");
        mediaLine->addRemoteCandidate(1, "127.0.0.1", 5060);
        mediaLine->addRemoteCandidate(2, "127.0.0.2", 5061);
        CPPUNIT_ASSERT(mediaLine->getMaximumPacketRate() == 20);
        CPPUNIT_ASSERT(mediaLine->getLabel() == "G711 Label");
        CPPUNIT_ASSERT(mediaLine->getIdentificationTag() == "item1");
        CPPUNIT_ASSERT(mediaLine->getIceUserFrag() == "ICEUSER");
        CPPUNIT_ASSERT(mediaLine->getIcePassword() == "ICEPASSWORD");
        CPPUNIT_ASSERT(mediaLine->getRemoteCandidates().entries() == 2);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpRemoteCandidate*)mediaLine->getRemoteCandidates().at(0))->getComponentId() == 1);
        CPPUNIT_ASSERT(((SdpMediaLine::SdpRemoteCandidate*)mediaLine->getRemoteCandidates().at(0))->getConnectionAddress() == "127.0.0.1");
        CPPUNIT_ASSERT(((SdpMediaLine::SdpRemoteCandidate*)mediaLine->getRemoteCandidates().at(0))->getPort() == 5060);

        mediaLine->addCandidate("id1", 1, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 100, "127.0.0.1", 6000, SdpCandidate::CANDIDATE_TYPE_HOST);
        mediaLine->addCandidate("id1", 2, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 50, "127.0.0.1", 6000, SdpCandidate::CANDIDATE_TYPE_SRFLX);
        mediaLine->addCandidate("id2", 1, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 101, "192.168.1.2", 5060, SdpCandidate::CANDIDATE_TYPE_RELAY, "127.0.0.3", 5080);
        mediaLine->addCandidate("id2", 1, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 100, "127.0.0.1", 6001, SdpCandidate::CANDIDATE_TYPE_HOST);
        CPPUNIT_ASSERT(mediaLine->getCandidates().entries() == 4);
        // Note:  this list is ordered
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(0))->getPriority() == 101);
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(1))->getPriority() == 100);
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(2))->getPriority() == 100);
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(3))->getPriority() == 50);
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(0))->getFoundation() == "id2");
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(1))->getFoundation() == "id1");
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(2))->getFoundation() == "id2");
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(3))->getFoundation() == "id1");
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(0))->getId() == 1);
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(0))->getTransport() == SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP);
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(0))->getConnectionAddress() == "192.168.1.2");
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(0))->getPort() == 5060);
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(0))->getCandidateType() == SdpCandidate::CANDIDATE_TYPE_RELAY);
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(0))->getRelatedAddress() == "127.0.0.3");
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(0))->getRelatedPort() == 5080);
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(0))->getExtensionAttributes().entries() == 0);   
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(1))->getRelatedAddress() == "");
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(1))->getRelatedPort() == 0);
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(0))->isInUse() == false);
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(1))->isInUse() == true);
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(2))->isInUse() == true);
        CPPUNIT_ASSERT(((SdpCandidate*)mediaLine->getCandidates().at(3))->isInUse() == true);
        CPPUNIT_ASSERT(mediaLine->isIceSupported() == true);

        mSdp.addMediaLine(mediaLine);
        CPPUNIT_ASSERT(mSdp.getMediaLines().entries() == 1);
        mediaLine = new SdpMediaLine();
        SdpCodec* codec = new SdpCodec(19, MIME_TYPE_AUDIO, MIME_SUBTYPE_G729A, 8000, 20000, 1, "annexb=no");
        CPPUNIT_ASSERT(codec->getCodecType() == SdpCodec::SDP_CODEC_G729A);
        CPPUNIT_ASSERT(codec->getCodecPayloadFormat() == 19);
        CPPUNIT_ASSERT(codec->getSampleRate() == 8000);
        CPPUNIT_ASSERT(codec->getPacketLength() == 20000);
        CPPUNIT_ASSERT(codec->getNumChannels() == 1);
        UtlString fmtData;
        codec->getSdpFmtpField(fmtData);
        CPPUNIT_ASSERT(fmtData == "annexb=no");
        mediaLine->addCodec(codec);
        CPPUNIT_ASSERT(mediaLine->getCodecs().entries() == 1);

        SdpCandidate* sdpLocalCandidate = new SdpCandidate("f1", 1, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 100, "192.168.1.1", 6000, SdpCandidate::CANDIDATE_TYPE_HOST);
        SdpCandidate* sdpRemoteCandidate = new SdpCandidate("a1", 1, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 55, "192.168.1.1", 2345, SdpCandidate::CANDIDATE_TYPE_HOST);
        mediaLine->addCandidate(sdpLocalCandidate);
        CPPUNIT_ASSERT(mediaLine->getCandidates().entries() == 1);
        mediaLine->addCandidatePair(*sdpLocalCandidate, *sdpRemoteCandidate, SdpCandidatePair::OFFERER_REMOTE);
        CPPUNIT_ASSERT(mediaLine->getCandidatePairs().entries() == 1);
        delete sdpRemoteCandidate;
        sdpLocalCandidate = new SdpCandidate("f1", 2, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 101, "192.168.1.1", 6001, SdpCandidate::CANDIDATE_TYPE_HOST);
        sdpRemoteCandidate = new SdpCandidate("a1", 2, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 56, "192.168.1.1", 2346, SdpCandidate::CANDIDATE_TYPE_HOST);
        mediaLine->addCandidate(sdpLocalCandidate);
        CPPUNIT_ASSERT(mediaLine->getCandidates().entries() == 2);
        mediaLine->addCandidatePair(*sdpLocalCandidate, *sdpRemoteCandidate, SdpCandidatePair::OFFERER_LOCAL);
        CPPUNIT_ASSERT(mediaLine->getCandidatePairs().entries() == 2);
        CPPUNIT_ASSERT(((SdpCandidatePair*)mediaLine->getCandidatePairs().at(0))->getPriority() == 1993);
        CPPUNIT_ASSERT(((SdpCandidatePair*)mediaLine->getCandidatePairs().at(1))->getPriority() == 1962);
        CPPUNIT_ASSERT(((SdpCandidatePair*)mediaLine->getCandidatePairs().at(0))->getLocalCandidate().getPort() == 6001);
        CPPUNIT_ASSERT(((SdpCandidatePair*)mediaLine->getCandidatePairs().at(0))->getRemoteCandidate().getPort() == 2346);
        CPPUNIT_ASSERT(((SdpCandidatePair*)mediaLine->getCandidatePairs().at(0))->getOfferer() == SdpCandidatePair::OFFERER_LOCAL);
        CPPUNIT_ASSERT(((SdpCandidatePair*)mediaLine->getCandidatePairs().at(0))->getCheckState() == SdpCandidatePair::CHECK_STATE_FROZEN);
        ((SdpCandidatePair*)mediaLine->getCandidatePairs().at(0))->setCheckState(SdpCandidatePair::CHECK_STATE_WAITING);
        CPPUNIT_ASSERT(((SdpCandidatePair*)mediaLine->getCandidatePairs().at(0))->getCheckState() == SdpCandidatePair::CHECK_STATE_WAITING);

        delete sdpRemoteCandidate;

        mSdp.addMediaLine(mediaLine);
        CPPUNIT_ASSERT(mSdp.getMediaLines().entries() == 2);

    }

    void testSdpToString()
    {
        // Ensure string builder does not crash
        UtlString sdpString;
        mSdp.toString(sdpString);
        UtlString matchString = 
            "Sdp:\n"
            "SdpVersion: 1\n"
            "OrigUserName: ''\n"
            "OrigSessionId: 0\n"
            "OrigSessionVersion: 0\n"
            "OrigNetType: NO""NE\n"
            "OrigAddressType: NO""NE\n"
            "OrigUnicastAddr: ''\n"
            "SessionName: ''\n"
            "SessionInformation: ''\n"
            "SessionUri: ''\n"
            "Category: ''\n"
            "Keywords: ''\n"
            "ToolNameAndVersion: ''\n"
            "ConferenceType: NO""NE\n"
            "CharSet: ''\n"
            "IcePassiveOnlyMode: 0\n"
            "SessionLanguage: ''\n"
            "DescriptionLanguage: ''\n"
            "MaximumPacketRate: 0.000000\n";
        CPPUNIT_ASSERT(sdpString == matchString);
        //printf("%s\n", sdpString.data());
    }

    void testFoundationId()
    {
        // Test FoundationId interface
        CPPUNIT_ASSERT(mSdp.getLocalFoundationId(SdpCandidate::CANDIDATE_TYPE_HOST, "127.0.0.1") == "1");
        CPPUNIT_ASSERT(mSdp.getLocalFoundationId(SdpCandidate::CANDIDATE_TYPE_HOST, "127.0.0.1") == "1");
        CPPUNIT_ASSERT(mSdp.getLocalFoundationId(SdpCandidate::CANDIDATE_TYPE_HOST, "127.0.0.2") == "2");
        CPPUNIT_ASSERT(mSdp.getLocalFoundationId(SdpCandidate::CANDIDATE_TYPE_HOST, "127.0.0.2", "192.168.1.1") == "3");
        CPPUNIT_ASSERT(mSdp.getLocalFoundationId(SdpCandidate::CANDIDATE_TYPE_HOST, "127.0.0.2") == "2");
        CPPUNIT_ASSERT(mSdp.getLocalFoundationId(SdpCandidate::CANDIDATE_TYPE_HOST, "127.0.0.2", "192.168.1.1") == "3");
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SdpSetGetTest);
