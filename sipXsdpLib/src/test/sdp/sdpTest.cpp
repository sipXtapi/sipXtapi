//
// Copyright (C) 2007 Plantronics
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////
// Author: Scott Godin (sgodin AT SipSpectrum DOT com)

#ifdef OLD_SDPLIB_UTESTS

#include <iostream>
#include <utl/UtlSListIterator.h>

#include <sdp/SdpCodec.h>
#include <sdp/Sdp.h>
#include <sdp/SdpMediaLine.h>
#include <sdp/SdpHelperResip.h>

#ifndef EXCLUDE_SIPX_SDP_HELPER
#include <net/SdpBody.h>
#include <net/SdpHelper.h>
#endif

#ifdef BUILD_RESIP_SDP_HELPER
#include <rutil/ParseBuffer.hxx>
#include <resip/stack/Symbols.hxx>
#include <resip/stack/SdpContents.hxx>
#include <resip/stack/HeaderFieldValue.hxx>
using namespace resip;
#endif

using namespace std;

#if defined(WINCE)
int main( int argc, char* argv[] );

// wWinMain is not defined in winbase.h.
//extern "C" int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd);
//****************************************************************
int
WINAPI
WinMain(
		HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPWSTR lpCmdLine,
		int nShowCmd
	   )
{
	printf( "entering WinMain( ) - lpCmdLine is *%s*\n", lpCmdLine );
	wchar_t	*pW			= NULL;
	int		iRet		= 1;


	iRet = main( 0, NULL );

	printf( "  main( ) returned %d\n", iRet );
	return iRet;
}

#endif
int main(int argc, char* argv[])
{
   {  // Test get/set interfaces
      Sdp sdp;
      sdp.setSdpVersion(1);
      sdp.setOriginatorUserName("Test");
      sdp.setOriginatorSessionId(8888);
      sdp.setOriginatorSessionVersion(9999);
      sdp.setOriginatorNetType(Sdp::NET_TYPE_IN);
      sdp.setOriginatorAddressType(Sdp::ADDRESS_TYPE_IP4);
      sdp.setOriginatorUnicastAddress("127.0.0.1");
      sdp.setSessionName("SdpTestSession");
      sdp.setSessionInformation("sample session information");
      sdp.setSessionUri("http://www.sessionuri.com");
      assert(sdp.getSdpVersion() == 1);
      assert(sdp.getOriginatorUserName() == "Test");
      assert(sdp.getOriginatorSessionId() == 8888);
      assert(sdp.getOriginatorSessionVersion() == 9999);
      assert(sdp.getOriginatorNetType() == Sdp::NET_TYPE_IN);
      assert(sdp.getOriginatorAddressType() == Sdp::ADDRESS_TYPE_IP4);
      assert(sdp.getOriginatorUnicastAddress() == "127.0.0.1");
      assert(sdp.getSessionName() == "SdpTestSession");
      assert(sdp.getSessionInformation() == "sample session information");
      assert(sdp.getSessionUri() == "http://www.sessionuri.com");

      sdp.addEmailAddress("me@here.com");
      sdp.addEmailAddress("you@there.com");
      assert(sdp.getEmailAddresses().entries() == 2);

      sdp.addPhoneNumber("555-555-5555");
      sdp.addPhoneNumber("123-123-1234");
      assert(sdp.getPhoneNumbers().entries() == 2);

      sdp.addBandwidth(Sdp::BANDWIDTH_TYPE_CT, 1000);
      assert(sdp.getBandwidths().entries() == 1);
      sdp.clearBandwidths();
      assert(sdp.getBandwidths().entries() == 0);
      sdp.addBandwidth(Sdp::BANDWIDTH_TYPE_AS, 5000);
      sdp.addBandwidth(Sdp::BANDWIDTH_TYPE_CT, 100);
      assert(sdp.getBandwidths().entries() == 2);
      assert(((Sdp::SdpBandwidth*)sdp.getBandwidths().at(0))->getType() == Sdp::BANDWIDTH_TYPE_AS);
      assert(((Sdp::SdpBandwidth*)sdp.getBandwidths().at(0))->getBandwidth() == 5000);
      assert(((Sdp::SdpBandwidth*)sdp.getBandwidths().at(1))->getType() == Sdp::BANDWIDTH_TYPE_CT);
      assert(((Sdp::SdpBandwidth*)sdp.getBandwidths().at(1))->getBandwidth() == 100);      

      Sdp::SdpTime* sdpTime = new Sdp::SdpTime(100, 200);
      UtlSList offsetsFromStartTime;  // empty
      Sdp::SdpTime::SdpTimeRepeat* repeat = new Sdp::SdpTime::SdpTimeRepeat(8, 800);
      sdpTime->addRepeat(repeat);
      repeat = new Sdp::SdpTime::SdpTimeRepeat(9, 900);
      repeat->addOffsetFromStartTime(20);
      repeat->addOffsetFromStartTime(21);
      sdpTime->addRepeat(repeat);
      sdp.addTime(sdpTime);
      sdp.addTime(300, 400);
      assert(sdp.getTimes().entries() == 2);      
      assert(((Sdp::SdpTime*)sdp.getTimes().at(0))->getStartTime() == 100);
      assert(((Sdp::SdpTime*)sdp.getTimes().at(0))->getStopTime() == 200);
      assert(((Sdp::SdpTime*)sdp.getTimes().at(0))->getRepeats().entries() == 2);
      assert(((Sdp::SdpTime::SdpTimeRepeat*)((Sdp::SdpTime*)sdp.getTimes().at(0))->getRepeats().at(0))->getRepeatInterval() == 8);
      assert(((Sdp::SdpTime::SdpTimeRepeat*)((Sdp::SdpTime*)sdp.getTimes().at(0))->getRepeats().at(0))->getActiveDuration() == 800);
      assert(((Sdp::SdpTime::SdpTimeRepeat*)((Sdp::SdpTime*)sdp.getTimes().at(0))->getRepeats().at(1))->getRepeatInterval() == 9);
      assert(((Sdp::SdpTime::SdpTimeRepeat*)((Sdp::SdpTime*)sdp.getTimes().at(0))->getRepeats().at(1))->getActiveDuration() == 900);
      assert(((Sdp::SdpTime::SdpTimeRepeat*)((Sdp::SdpTime*)sdp.getTimes().at(0))->getRepeats().at(1))->getOffsetsFromStartTime().entries() == 2);
      assert(((UtlInt*)((Sdp::SdpTime::SdpTimeRepeat*)((Sdp::SdpTime*)sdp.getTimes().at(0))->getRepeats().at(1))->getOffsetsFromStartTime().at(0))->getValue() == 20);
      assert(((UtlInt*)((Sdp::SdpTime::SdpTimeRepeat*)((Sdp::SdpTime*)sdp.getTimes().at(0))->getRepeats().at(1))->getOffsetsFromStartTime().at(1))->getValue() == 21);
      assert(((Sdp::SdpTime*)sdp.getTimes().at(1))->getStartTime() == 300);
      assert(((Sdp::SdpTime*)sdp.getTimes().at(1))->getStopTime() == 400);
      sdp.addTimeZone(500, 600);
      sdp.addTimeZone(700, 800);
      assert(sdp.getTimeZones().entries() == 2);
      assert(((Sdp::SdpTimeZone*)sdp.getTimeZones().at(0))->getAdjustmentTime() == 500);
      assert(((Sdp::SdpTimeZone*)sdp.getTimeZones().at(0))->getOffset() == 600);
      assert(((Sdp::SdpTimeZone*)sdp.getTimeZones().at(1))->getAdjustmentTime() == 700);
      assert(((Sdp::SdpTimeZone*)sdp.getTimeZones().at(1))->getOffset() == 800);

      sdp.setCategory("sample sdp category");
      sdp.setKeywords("sdp session description protocol");
      sdp.setToolNameAndVersion("sipX session description 1.0");
      sdp.setConferenceType(Sdp::CONFERENCE_TYPE_BROADCAST);
      sdp.setCharSet("UTF-8");
      sdp.setIcePassiveOnlyMode(true);
      assert(sdp.getCategory() == "sample sdp category");
      assert(sdp.getKeywords() == "sdp session description protocol");
      assert(sdp.getToolNameAndVersion() == "sipX session description 1.0");
      assert(sdp.getConferenceType() == Sdp::CONFERENCE_TYPE_BROADCAST);
      assert(sdp.getCharSet() == "UTF-8");
      assert(sdp.isIcePassiveOnlyMode() == true);

      Sdp::SdpGroup* group = new Sdp::SdpGroup(Sdp::GROUP_SEMANTICS_LS);
      group->addIdentificationTag("media1");
      group->addIdentificationTag("media2");
      sdp.addGroup(group);
      group = new Sdp::SdpGroup(Sdp::GROUP_SEMANTICS_FID);
      group->addIdentificationTag("fid1");
      sdp.addGroup(group);
      assert(sdp.getGroups().entries() == 2);
      assert(((Sdp::SdpGroup*)sdp.getGroups().at(0))->getIdentificationTags().entries() == 2);
      assert(((Sdp::SdpGroup*)sdp.getGroups().at(0))->getSemantics() == Sdp::GROUP_SEMANTICS_LS);
      assert(((UtlString*)((Sdp::SdpGroup*)sdp.getGroups().at(0))->getIdentificationTags().at(0))->compareTo("media1") == 0);
      assert(((UtlString*)((Sdp::SdpGroup*)sdp.getGroups().at(0))->getIdentificationTags().at(1))->compareTo("media2") == 0);
      assert(((Sdp::SdpGroup*)sdp.getGroups().at(1))->getSemantics() == Sdp::GROUP_SEMANTICS_FID);
      assert(((UtlString*)((Sdp::SdpGroup*)sdp.getGroups().at(1))->getIdentificationTags().at(0))->compareTo("fid1") == 0);

      sdp.setSessionLanguage("EN-US");
      sdp.setDescriptionLanguage("FR-CN");
      sdp.setMaximumPacketRate(1.5);
      assert(sdp.getSessionLanguage() == "EN-US");
      assert(sdp.getDescriptionLanguage() == "FR-CN");
      assert(sdp.getMaximumPacketRate() == 1.5);

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
      assert(mediaLine->getMediaType() == SdpMediaLine::MEDIA_TYPE_AUDIO);
      assert(mediaLine->getTransportProtocolType() == SdpMediaLine::PROTOCOL_TYPE_RTP_AVP);
      assert(mediaLine->getTitle() == "G729a Audio Codec");
      assert(mediaLine->getConnections().entries() == 1);
      assert(((SdpMediaLine::SdpConnection*)mediaLine->getConnections().at(0))->getNetType() == Sdp::NET_TYPE_IN);
      assert(((SdpMediaLine::SdpConnection*)mediaLine->getConnections().at(0))->getAddressType() == Sdp::ADDRESS_TYPE_IP4);
      assert(((SdpMediaLine::SdpConnection*)mediaLine->getConnections().at(0))->getAddress() == "127.0.0.1");
      assert(((SdpMediaLine::SdpConnection*)mediaLine->getConnections().at(0))->getPort() == 6000);
      assert(((SdpMediaLine::SdpConnection*)mediaLine->getConnections().at(0))->getMulticastIpV4Ttl() == 90);
      assert(mediaLine->getRtcpConnections().entries() == 1);
      assert(((SdpMediaLine::SdpConnection*)mediaLine->getRtcpConnections().at(0))->getNetType() == Sdp::NET_TYPE_IN);
      assert(((SdpMediaLine::SdpConnection*)mediaLine->getRtcpConnections().at(0))->getAddressType() == Sdp::ADDRESS_TYPE_IP4);
      assert(((SdpMediaLine::SdpConnection*)mediaLine->getRtcpConnections().at(0))->getAddress() == "127.0.0.1");
      assert(((SdpMediaLine::SdpConnection*)mediaLine->getRtcpConnections().at(0))->getPort() == 6001);
      assert(((SdpMediaLine::SdpConnection*)mediaLine->getRtcpConnections().at(0))->getMulticastIpV4Ttl() == 0);
      assert(mediaLine->getBandwidths().entries() == 1);
      assert(mediaLine->getEncryptionKey() == "0123456789ABCD");
      assert(mediaLine->getEncryptionMethod() == SdpMediaLine::ENCRYPTION_METHOD_BASE64);
      assert(mediaLine->getDirection() == SdpMediaLine::DIRECTION_TYPE_SENDRECV);
      assert(mediaLine->getPacketTime() == 20);
      assert(mediaLine->getMaxPacketTime() == 60);
      assert(mediaLine->getOrientation() == SdpMediaLine::ORIENTATION_TYPE_LANDSCAPE);
      assert(mediaLine->getDescriptionLanguage() == "English");
      assert(mediaLine->getLanguage() == "EN");
      assert(mediaLine->getFrameRate() == 256);
      assert(mediaLine->getQuality() == 10);
      assert(mediaLine->getTcpConnectionAttribute() == SdpMediaLine::TCP_CONNECTION_ATTRIBUTE_NEW);
      assert(mediaLine->getTcpSetupAttribute() == SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTPASS);

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
      assert(crypto->getGenericSessionParams().entries() == 2);
      assert(crypto->getTag() == 2);
      assert(crypto->getSuite() == SdpMediaLine::CRYPTO_SUITE_TYPE_F8_128_HMAC_SHA1_80);
      assert(crypto->getCryptoKeyParams().entries() == 1);
      assert(((SdpMediaLine::SdpCrypto::SdpCryptoKeyParam*)crypto->getCryptoKeyParams().at(0))->getKeyMethod() == SdpMediaLine::CRYPTO_KEY_METHOD_INLINE);
      assert(((SdpMediaLine::SdpCrypto::SdpCryptoKeyParam*)crypto->getCryptoKeyParams().at(0))->getKeyValue() == "CryptoKey2");
      assert(crypto->getGenericSessionParams().entries() == 2);
      mediaLine->addCryptoSettings(crypto);
      assert(mediaLine->getCryptos().entries() == 2);

      mediaLine->setFingerPrint(SdpMediaLine::FINGERPRINT_HASH_FUNC_MD5, "this-is-a-finger-print");
      mediaLine->setKeyManagementProtocol(SdpMediaLine::KEYMANAGEMENT_PROTOCOL_MIKEY, "this-is-some-key-management-data");
      mediaLine->addPreConditionCurrentStatus(SdpMediaLine::PRECONDITION_TYPE_QOS, SdpMediaLine::PRECONDITION_STATUS_E2E, SdpMediaLine::PRECONDITION_DIRECTION_SENDRECV);
      mediaLine->addPreConditionCurrentStatus(SdpMediaLine::PRECONDITION_TYPE_QOS, SdpMediaLine::PRECONDITION_STATUS_REMOTE, SdpMediaLine::PRECONDITION_DIRECTION_RECV);
      mediaLine->addPreConditionConfirmStatus(SdpMediaLine::PRECONDITION_TYPE_QOS, SdpMediaLine::PRECONDITION_STATUS_E2E, SdpMediaLine::PRECONDITION_DIRECTION_SENDRECV);
      mediaLine->addPreConditionConfirmStatus(SdpMediaLine::PRECONDITION_TYPE_QOS, SdpMediaLine::PRECONDITION_STATUS_REMOTE, SdpMediaLine::PRECONDITION_DIRECTION_RECV);
      mediaLine->addPreConditionDesiredStatus(SdpMediaLine::PRECONDITION_TYPE_QOS, SdpMediaLine::PRECONDITION_STRENGTH_MANDATORY, SdpMediaLine::PRECONDITION_STATUS_E2E, SdpMediaLine::PRECONDITION_DIRECTION_SENDRECV);
      mediaLine->addPreConditionDesiredStatus(SdpMediaLine::PRECONDITION_TYPE_QOS, SdpMediaLine::PRECONDITION_STRENGTH_OPTIONAL, SdpMediaLine::PRECONDITION_STATUS_REMOTE, SdpMediaLine::PRECONDITION_DIRECTION_RECV);
      assert(mediaLine->getFingerPrint() == "this-is-a-finger-print");
      assert(mediaLine->getFingerPrintHashFunction() == SdpMediaLine::FINGERPRINT_HASH_FUNC_MD5);
      assert(mediaLine->getPreConditionCurrentStatus().entries() == 2);
      assert(((SdpMediaLine::SdpPreCondition*)mediaLine->getPreConditionCurrentStatus().at(0))->getType() == SdpMediaLine::PRECONDITION_TYPE_QOS);
      assert(((SdpMediaLine::SdpPreCondition*)mediaLine->getPreConditionCurrentStatus().at(0))->getStatus() == SdpMediaLine::PRECONDITION_STATUS_E2E);
      assert(((SdpMediaLine::SdpPreCondition*)mediaLine->getPreConditionCurrentStatus().at(1))->getDirection() == SdpMediaLine::PRECONDITION_DIRECTION_RECV);
      assert(mediaLine->getPreConditionConfirmStatus().entries() == 2);
      assert(((SdpMediaLine::SdpPreCondition*)mediaLine->getPreConditionConfirmStatus().at(0))->getType() == SdpMediaLine::PRECONDITION_TYPE_QOS);
      assert(((SdpMediaLine::SdpPreCondition*)mediaLine->getPreConditionConfirmStatus().at(1))->getStatus() == SdpMediaLine::PRECONDITION_STATUS_REMOTE);
      assert(((SdpMediaLine::SdpPreCondition*)mediaLine->getPreConditionConfirmStatus().at(0))->getDirection() == SdpMediaLine::PRECONDITION_DIRECTION_SENDRECV);
      assert(mediaLine->getPreConditionDesiredStatus().entries() == 2);
      assert(((SdpMediaLine::SdpPreConditionDesiredStatus*)mediaLine->getPreConditionDesiredStatus().at(0))->getStrength() == SdpMediaLine::PRECONDITION_STRENGTH_MANDATORY);
      assert(((SdpMediaLine::SdpPreConditionDesiredStatus*)mediaLine->getPreConditionDesiredStatus().at(1))->getStatus() == SdpMediaLine::PRECONDITION_STATUS_REMOTE);
      assert(((SdpMediaLine::SdpPreConditionDesiredStatus*)mediaLine->getPreConditionDesiredStatus().at(0))->getDirection() == SdpMediaLine::PRECONDITION_DIRECTION_SENDRECV);

      mediaLine->setMaximumPacketRate(20);
      mediaLine->setLabel("G711 Label");
      mediaLine->setIdentificationTag("item1");
      mediaLine->setIceUserFrag("ICEUSER");
      mediaLine->setIcePassword("ICEPASSWORD");
      mediaLine->addRemoteCandidate(1, "127.0.0.1", 5060);
      mediaLine->addRemoteCandidate(2, "127.0.0.2", 5061);
      assert(mediaLine->getMaximumPacketRate() == 20);
      assert(mediaLine->getLabel() == "G711 Label");
      assert(mediaLine->getIdentificationTag() == "item1");
      assert(mediaLine->getIceUserFrag() == "ICEUSER");
      assert(mediaLine->getIcePassword() == "ICEPASSWORD");
      assert(mediaLine->getRemoteCandidates().entries() == 2);
      assert(((SdpMediaLine::SdpRemoteCandidate*)mediaLine->getRemoteCandidates().at(0))->getComponentId() == 1);
      assert(((SdpMediaLine::SdpRemoteCandidate*)mediaLine->getRemoteCandidates().at(0))->getConnectionAddress() == "127.0.0.1");
      assert(((SdpMediaLine::SdpRemoteCandidate*)mediaLine->getRemoteCandidates().at(0))->getPort() == 5060);

      mediaLine->addCandidate("id1", 1, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 100, "127.0.0.1", 6000, SdpCandidate::CANDIDATE_TYPE_HOST);
      mediaLine->addCandidate("id1", 2, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 50, "127.0.0.1", 6000, SdpCandidate::CANDIDATE_TYPE_SRFLX);
      mediaLine->addCandidate("id2", 1, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 101, "192.168.1.2", 5060, SdpCandidate::CANDIDATE_TYPE_RELAY, "127.0.0.3", 5080);
      mediaLine->addCandidate("id2", 1, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 100, "127.0.0.1", 6001, SdpCandidate::CANDIDATE_TYPE_HOST);
      assert(mediaLine->getCandidates().entries() == 4);
      // Note:  this list is ordered
      assert(((SdpCandidate*)mediaLine->getCandidates().at(0))->getPriority() == 101);
      assert(((SdpCandidate*)mediaLine->getCandidates().at(1))->getPriority() == 100);
      assert(((SdpCandidate*)mediaLine->getCandidates().at(2))->getPriority() == 100);
      assert(((SdpCandidate*)mediaLine->getCandidates().at(3))->getPriority() == 50);
      assert(((SdpCandidate*)mediaLine->getCandidates().at(0))->getFoundation() == "id2");
      assert(((SdpCandidate*)mediaLine->getCandidates().at(1))->getFoundation() == "id1");
      assert(((SdpCandidate*)mediaLine->getCandidates().at(2))->getFoundation() == "id2");
      assert(((SdpCandidate*)mediaLine->getCandidates().at(3))->getFoundation() == "id1");
      assert(((SdpCandidate*)mediaLine->getCandidates().at(0))->getId() == 1);
      assert(((SdpCandidate*)mediaLine->getCandidates().at(0))->getTransport() == SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP);
      assert(((SdpCandidate*)mediaLine->getCandidates().at(0))->getConnectionAddress() == "192.168.1.2");
      assert(((SdpCandidate*)mediaLine->getCandidates().at(0))->getPort() == 5060);
      assert(((SdpCandidate*)mediaLine->getCandidates().at(0))->getCandidateType() == SdpCandidate::CANDIDATE_TYPE_RELAY);
      assert(((SdpCandidate*)mediaLine->getCandidates().at(0))->getRelatedAddress() == "127.0.0.3");
      assert(((SdpCandidate*)mediaLine->getCandidates().at(0))->getRelatedPort() == 5080);
      assert(((SdpCandidate*)mediaLine->getCandidates().at(0))->getExtensionAttributes().entries() == 0);   
      assert(((SdpCandidate*)mediaLine->getCandidates().at(1))->getRelatedAddress() == "");
      assert(((SdpCandidate*)mediaLine->getCandidates().at(1))->getRelatedPort() == 0);
      assert(((SdpCandidate*)mediaLine->getCandidates().at(0))->isInUse() == false);
      assert(((SdpCandidate*)mediaLine->getCandidates().at(1))->isInUse() == true);
      assert(((SdpCandidate*)mediaLine->getCandidates().at(2))->isInUse() == true);
      assert(((SdpCandidate*)mediaLine->getCandidates().at(3))->isInUse() == true);
      assert(mediaLine->isIceSupported() == true);

      sdp.addMediaLine(mediaLine);
      assert(sdp.getMediaLines().entries() == 1);

      mediaLine = new SdpMediaLine();
      SdpCodec* codec = new SdpCodec(19, MIME_TYPE_AUDIO, MIME_SUBTYPE_G729A, 8000, 20000, 1, "annexb=no");
      assert(codec->getCodecType() == SdpCodec::SDP_CODEC_G729A);
      assert(codec->getCodecPayloadFormat() == 19);
      assert(codec->getSampleRate() == 8000);
      assert(codec->getPacketLength() == 20000);
      assert(codec->getNumChannels() == 1);
      UtlString fmtData;
      codec->getSdpFmtpField(fmtData);
      assert(fmtData == "annexb=no");
      mediaLine->addCodec(codec);
      assert(mediaLine->getCodecs().entries() == 1);

      SdpCandidate* sdpLocalCandidate = new SdpCandidate("f1", 1, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 100, "192.168.1.1", 6000, SdpCandidate::CANDIDATE_TYPE_HOST);
      SdpCandidate* sdpRemoteCandidate = new SdpCandidate("a1", 1, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 55, "192.168.1.1", 2345, SdpCandidate::CANDIDATE_TYPE_HOST);
      mediaLine->addCandidate(sdpLocalCandidate);
      assert(mediaLine->getCandidates().entries() == 1);
      mediaLine->addCandidatePair(*sdpLocalCandidate, *sdpRemoteCandidate, SdpCandidatePair::OFFERER_REMOTE);
      assert(mediaLine->getCandidatePairs().entries() == 1);
      delete sdpRemoteCandidate;
      sdpLocalCandidate = new SdpCandidate("f1", 2, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 101, "192.168.1.1", 6001, SdpCandidate::CANDIDATE_TYPE_HOST);
      sdpRemoteCandidate = new SdpCandidate("a1", 2, SdpCandidate::CANDIDATE_TRANSPORT_TYPE_UDP, 56, "192.168.1.1", 2346, SdpCandidate::CANDIDATE_TYPE_HOST);
      mediaLine->addCandidate(sdpLocalCandidate);
      assert(mediaLine->getCandidates().entries() == 2);
      mediaLine->addCandidatePair(*sdpLocalCandidate, *sdpRemoteCandidate, SdpCandidatePair::OFFERER_LOCAL);
      assert(mediaLine->getCandidatePairs().entries() == 2);
      assert(((SdpCandidatePair*)mediaLine->getCandidatePairs().at(0))->getPriority() == 1993);
      assert(((SdpCandidatePair*)mediaLine->getCandidatePairs().at(1))->getPriority() == 1962);
      assert(((SdpCandidatePair*)mediaLine->getCandidatePairs().at(0))->getLocalCandidate().getPort() == 6001);
      assert(((SdpCandidatePair*)mediaLine->getCandidatePairs().at(0))->getRemoteCandidate().getPort() == 2346);
      assert(((SdpCandidatePair*)mediaLine->getCandidatePairs().at(0))->getOfferer() == SdpCandidatePair::OFFERER_LOCAL);
      assert(((SdpCandidatePair*)mediaLine->getCandidatePairs().at(0))->getCheckState() == SdpCandidatePair::CHECK_STATE_FROZEN);
      ((SdpCandidatePair*)mediaLine->getCandidatePairs().at(0))->setCheckState(SdpCandidatePair::CHECK_STATE_WAITING);
      assert(((SdpCandidatePair*)mediaLine->getCandidatePairs().at(0))->getCheckState() == SdpCandidatePair::CHECK_STATE_WAITING);

      delete sdpRemoteCandidate;

      sdp.addMediaLine(mediaLine);
      assert(sdp.getMediaLines().entries() == 2);

      // Ensure string builder does not crash
      UtlString sdpString;
      sdp.toString(sdpString);
      printf("%s\n", sdpString.data());

      // Test FoundationId interface
      assert(sdp.getLocalFoundationId(SdpCandidate::CANDIDATE_TYPE_HOST, "127.0.0.1") == "1");
      assert(sdp.getLocalFoundationId(SdpCandidate::CANDIDATE_TYPE_HOST, "127.0.0.1") == "1");
      assert(sdp.getLocalFoundationId(SdpCandidate::CANDIDATE_TYPE_HOST, "127.0.0.2") == "2");
      assert(sdp.getLocalFoundationId(SdpCandidate::CANDIDATE_TYPE_HOST, "127.0.0.2", "192.168.1.1") == "3");
      assert(sdp.getLocalFoundationId(SdpCandidate::CANDIDATE_TYPE_HOST, "127.0.0.2") == "2");
      assert(sdp.getLocalFoundationId(SdpCandidate::CANDIDATE_TYPE_HOST, "127.0.0.2", "192.168.1.1") == "3");
   }

   {
      // Test helpers
      char txt[] = "v=0\r\n"  
         "o=- 333525334858460 333525334858460 IN IP4 192.168.0.156\r\n"
         "s=test123\r\n"
         "e=unknown@invalid.net\r\n"
         "p=+972 683 1000\r\n"
         "c=IN IP4 127.0.0.1\r\n"
         "b=RR:0\r\n"
         "b=RS:0\r\n"
         "b=CT:10000\r\n"
         "t=4058038202 0\r\n"
         "k=base64:base64key\r\n"
         "a=tool:ResipParserTester\r\n"
         "a=inactive\r\n"
         "m=audio 41466/6 RTP/AVP 0 101\r\n"
         "i=Audio Stream\r\n"
         "c=IN IP4 192.168.0.156/100/3\r\n"
         "c=IN IP6 FF15::101/3\r\n"
         "k=clear:base64clearkey\r\n"
         "a=fmtp:101 0-11\r\n"
         "a=ptime:20\r\n"
         "a=fmtp:0 annexb=no\r\n"
         "a=maxptime:40\r\n"
         "a=setup:active\r\n"
         "a=sendrecv\r\n"
         "a=rtpmap:101 telephone-event/8000\r\n"
         "a=crypto:1 F8_128_HMAC_SHA1_80 inline:MTIzNDU2Nzg5QUJDREUwMTIzNDU2Nzg5QUJjZGVm|2^20|1:4;inline:QUJjZGVmMTIzNDU2Nzg5QUJDREUwMTIzNDU2Nzg5|2^20|2:4 FEC_ORDER=FEC_SRTP\r\n"
         "m=video 21234 RTP/AVP 140\r\n"
         "b=RR:1\r\n"
         "b=RS:0\r\n"
         "a=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:QUJjZGVmMTIzNDU2Nzg5QUJDREUwMTIzNDU2Nzg5|2:18;inline:QUJjZGVmMTIzNDU2Nzg5QUJDREUwMTIzNDU2Nzg5|21|3:4 KDR=23 FEC_ORDER=SRTP_FEC UNENCRYPTED_SRTP\r\n"
         "a=crypto:2 AES_CM_128_HMAC_SHA1_32 inline:QUJjZGVmMTIzNDU2Nzg5QUJDREUwMTIzNDU2Nzg5|2^20 FEC_KEY=inline:QUJjZGVmMTIzNDU2Nzg5QUJDREUwMTIzNDU2Nzg5|2^20|2:4 WSH=60\r\n"
         "a=fingerprint:sha-1 0123456789\r\n"
         "a=key-mgmt:mikey thisissomebase64data\r\n"
         "a=curr:qos e2e sendrecv\r\n"
         "a=curr:qos local send\r\n"
         "a=des:qos mandatory e2e sendrecv\r\n"
         "a=des:qos optional local send\r\n"
         "a=conf:qos e2e none\r\n"
         "a=conf:qos remote recv\r\n"
         "a=remote-candidates:1 192.168.0.1 5060 2 192.168.0.1 5061\r\n"
         "a=remote-candidates:3 192.168.0.2 5063\r\n"
         "a=candidate:foundation1 1 udp 100000 127.0.0.1 21234 typ host raddr 127.0.0.8 rport 6667 name value name2 value2\r\n"
         "a=candidate:foundation2 2 udp 100001 192.168.0.1 6667 raddr 127.0.0.9 rport 6668 name value name2 value2\r\n"
         "a=candidate:foundation3 3 udp 100002 192.168.0.2 6668 raddr 127.0.0.9 name value name2 value2\r\n"
         "a=candidate:foundation3 3 udp 100002 123.123.123.124 127 name value name2 value2\r\n"
         "a=candidate:foundation3 3 udp 100002 192.168.0.2 6668 typ relay\r\n"
         "a=rtcp:127 IN IP4 123.123.123.124/60\r\n"
         "a=rtpmap:140 vp71/144000\r\n"
         "a=fmtp:140 CIF=1 QCIF=2 SQCIF\r\n";

      Sdp* convSdp;
      UtlString sdpString;

#ifdef BUILD_RESIP_SDP_HELPER
      { // Test resip helper
         HeaderFieldValue hfv(txt, sizeof(txt));
         Mime type("application", "sdp");
         SdpContents resipSdp(&hfv, type);

         convSdp = SdpHelperResip::createSdpFromResipSdp(resipSdp);
         if(convSdp)
         {
            // Ensure string builder does not crash
            printf("\n\nResip Sdp Helper Test:\n");
            convSdp->toString(sdpString);
            printf("%s\n", sdpString.data());

            // Perform some random assertions
            assert(convSdp->getLocalFoundationId(SdpCandidate::CANDIDATE_TYPE_HOST, "127.0.0.1") == "1");
            assert(convSdp->getOriginatorUnicastAddress() == "192.168.0.156");
            assert(convSdp->getSessionName() == "test123");
            assert(convSdp->getEmailAddresses().entries() == 1);
            assert(convSdp->getPhoneNumbers().entries() == 1);
            assert(convSdp->getBandwidths().entries() == 3);
            assert(convSdp->getTimes().entries() == 1);
            assert(convSdp->getToolNameAndVersion() == "ResipParserTester");
            assert(convSdp->getMediaLines().entries() == 2);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getMediaType() == SdpMediaLine::MEDIA_TYPE_AUDIO);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getMediaType() == SdpMediaLine::MEDIA_TYPE_VIDEO);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getTransportProtocolType() == SdpMediaLine::PROTOCOL_TYPE_RTP_AVP);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCodecs().entries() == 2);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getCodecs().entries() == 1);
            assert(((SdpCodec*)((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCodecs().at(0))->getCodecType() == SdpCodec::SDP_CODEC_GIPS_PCMU);
            assert(((SdpCodec*)((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCodecs().at(1))->getCodecType() == SdpCodec::SDP_CODEC_TONES);
            assert(((SdpCodec*)((SdpMediaLine*)convSdp->getMediaLines().at(1))->getCodecs().at(0))->getCodecType() == SdpCodec::SDP_CODEC_VP71_CIF);
            UtlString fmtp;
            ((SdpCodec*)((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCodecs().at(1))->getSdpFmtpField(fmtp);
            assert(fmtp == "0-11");
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getConnections().entries() == 6);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getConnections().entries() == 1);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getRtcpConnections().entries() == 0);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getRtcpConnections().entries() == 1);
            assert(((SdpMediaLine::SdpConnection*)((SdpMediaLine*)convSdp->getMediaLines().at(0))->getConnections().at(0))->getAddress() == "192.168.0.156");
            assert(((SdpMediaLine::SdpConnection*)((SdpMediaLine*)convSdp->getMediaLines().at(0))->getConnections().at(0))->getPort() == 41466);
            assert(((SdpMediaLine::SdpConnection*)((SdpMediaLine*)convSdp->getMediaLines().at(1))->getRtcpConnections().at(0))->getPort() == 127);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getBandwidths().entries() == 0);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getBandwidths().entries() == 2);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getEncryptionKey() == "base64clearkey");
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getDirection() == SdpMediaLine::DIRECTION_TYPE_SENDRECV);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getDirection() == SdpMediaLine::DIRECTION_TYPE_INACTIVE);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getPacketTime() == 20);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getTcpConnectionAttribute() == SdpMediaLine::TCP_CONNECTION_ATTRIBUTE_NONE);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getTcpSetupAttribute() == SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTIVE);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCryptos().entries() == 1);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getCryptos().entries() == 2);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCandidates().entries() == 0);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getCandidates().entries() == 5);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getRemoteCandidates().entries() == 3);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getPreConditionCurrentStatus().entries() == 2);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getPreConditionConfirmStatus().entries() == 2);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getPreConditionDesiredStatus().entries() == 2);

            // Copy test
            Sdp copySdp(*convSdp);

            // Ensure string builder does not crash
            copySdp.toString(sdpString);
            printf("%s\n", sdpString.data());

            // Perform some random assertions on copy
            assert(copySdp.getLocalFoundationId(SdpCandidate::CANDIDATE_TYPE_HOST, "127.0.0.2") == "2");
            assert(copySdp.getOriginatorUnicastAddress() == "192.168.0.156");
            assert(copySdp.getSessionName() == "test123");
            assert(copySdp.getEmailAddresses().entries() == 1);
            assert(copySdp.getPhoneNumbers().entries() == 1);
            assert(copySdp.getBandwidths().entries() == 2);
            assert(copySdp.getTimes().entries() == 1);
            assert(copySdp.getToolNameAndVersion() == "ResipParserTester");
            assert(copySdp.getMediaLines().entries() == 2);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(0))->getMediaType() == SdpMediaLine::MEDIA_TYPE_AUDIO);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(1))->getMediaType() == SdpMediaLine::MEDIA_TYPE_VIDEO);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(0))->getTransportProtocolType() == SdpMediaLine::PROTOCOL_TYPE_RTP_AVP);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(0))->getCodecs().entries() == 2);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(1))->getCodecs().entries() == 1);
            assert(((SdpCodec*)((SdpMediaLine*)copySdp.getMediaLines().at(0))->getCodecs().at(0))->getCodecType() == SdpCodec::SDP_CODEC_GIPS_PCMU);
            assert(((SdpCodec*)((SdpMediaLine*)copySdp.getMediaLines().at(0))->getCodecs().at(1))->getCodecType() == SdpCodec::SDP_CODEC_TONES);
            assert(((SdpCodec*)((SdpMediaLine*)copySdp.getMediaLines().at(1))->getCodecs().at(0))->getCodecType() == SdpCodec::SDP_CODEC_VP71_CIF);
            ((SdpCodec*)((SdpMediaLine*)copySdp.getMediaLines().at(0))->getCodecs().at(1))->getSdpFmtpField(fmtp);
            assert(fmtp == "0-11");
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(0))->getConnections().entries() == 6);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(1))->getConnections().entries() == 1);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(0))->getRtcpConnections().entries() == 0);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(1))->getRtcpConnections().entries() == 1);
            assert(((SdpMediaLine::SdpConnection*)((SdpMediaLine*)copySdp.getMediaLines().at(0))->getConnections().at(0))->getAddress() == "192.168.0.156");
            assert(((SdpMediaLine::SdpConnection*)((SdpMediaLine*)copySdp.getMediaLines().at(0))->getConnections().at(0))->getPort() == 41466);
            assert(((SdpMediaLine::SdpConnection*)((SdpMediaLine*)copySdp.getMediaLines().at(1))->getRtcpConnections().at(0))->getPort() == 127);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(0))->getBandwidths().entries() == 0);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(1))->getBandwidths().entries() == 2);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(0))->getEncryptionKey() == "base64clearkey");
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(0))->getDirection() == SdpMediaLine::DIRECTION_TYPE_SENDRECV);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(1))->getDirection() == SdpMediaLine::DIRECTION_TYPE_INACTIVE);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(0))->getPacketTime() == 20);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(0))->getTcpConnectionAttribute() == SdpMediaLine::TCP_CONNECTION_ATTRIBUTE_NONE);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(0))->getTcpSetupAttribute() == SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTIVE);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(0))->getCryptos().entries() == 1);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(1))->getCryptos().entries() == 2);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(0))->getCandidates().entries() == 0);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(1))->getCandidates().entries() == 5);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(1))->getRemoteCandidates().entries() == 3);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(1))->getPreConditionCurrentStatus().entries() == 2);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(1))->getPreConditionConfirmStatus().entries() == 2);
            assert(((SdpMediaLine*)copySdp.getMediaLines().at(1))->getPreConditionDesiredStatus().entries() == 2);

            delete convSdp;
         }
      }
#endif

#ifndef EXCLUDE_SIPX_SDP_HELPER
      {  // Test sipX helper
         SdpBody sdpBody(txt, sizeof(txt));
         convSdp = SdpHelper::createSdpFromSdpBody(sdpBody);
         if(convSdp)
         {
            // Ensure string builder does not crash
            printf("\n\nsipX Sdp Helper Test:\n");
            convSdp->toString(sdpString);
            printf("%s\n", sdpString.data());

            // Perform some random assertions
            // Note:  The SdpBody class implementation is far from complete 
            //         - therefor the SdpHelper for sipX is incomplete 
            //         - therefor some assertions are commented out

            //assert(convSdp->getOriginatorUnicastAddress() == "192.168.0.156");
            //assert(convSdp->getSessionName() == "test123");
            //assert(convSdp->getEmailAddresses().entries() == 1);
            //assert(convSdp->getPhoneNumbers().entries() == 1);
            //assert(convSdp->getBandwidths().entries() == 2);
            //assert(convSdp->getTimes().entries() == 1);
            //assert(convSdp->getToolNameAndVersion() == "ResipParserTester");
            assert(convSdp->getMediaLines().entries() == 2);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getMediaType() == SdpMediaLine::MEDIA_TYPE_AUDIO);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getMediaType() == SdpMediaLine::MEDIA_TYPE_VIDEO);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getTransportProtocolType() == SdpMediaLine::PROTOCOL_TYPE_RTP_AVP);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCodecs().entries() == 2);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getCodecs().entries() == 1);
            assert(((SdpCodec*)((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCodecs().at(0))->getCodecType() == SdpCodec::SDP_CODEC_GIPS_PCMU);
            assert(((SdpCodec*)((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCodecs().at(1))->getCodecType() == SdpCodec::SDP_CODEC_TONES);
            assert(((SdpCodec*)((SdpMediaLine*)convSdp->getMediaLines().at(1))->getCodecs().at(0))->getCodecType() == SdpCodec::SDP_CODEC_VP71_CIF);
            UtlString fmtp;
            ((SdpCodec*)((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCodecs().at(1))->getSdpFmtpField(fmtp);
            assert(fmtp == "0-11");
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getConnections().entries() == 6);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getConnections().entries() == 1);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getRtcpConnections().entries() == 0);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getRtcpConnections().entries() == 1);
            assert(((SdpMediaLine::SdpConnection*)((SdpMediaLine*)convSdp->getMediaLines().at(0))->getConnections().at(0))->getAddress() == "192.168.0.156");
            assert(((SdpMediaLine::SdpConnection*)((SdpMediaLine*)convSdp->getMediaLines().at(0))->getConnections().at(0))->getPort() == 41466);
            assert(((SdpMediaLine::SdpConnection*)((SdpMediaLine*)convSdp->getMediaLines().at(1))->getRtcpConnections().at(0))->getPort() == 127);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getBandwidths().entries() == 0);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getBandwidths().entries() == 2);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getEncryptionKey() == "base64clearkey");
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getDirection() == SdpMediaLine::DIRECTION_TYPE_SENDRECV);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getDirection() == SdpMediaLine::DIRECTION_TYPE_INACTIVE);  // !slg! SdpBody only reads first occurance of a= direction keyword
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getPacketTime() == 20);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getTcpConnectionAttribute() == SdpMediaLine::TCP_CONNECTION_ATTRIBUTE_NONE);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getTcpSetupAttribute() == SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTIVE);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCryptos().entries() == 1);
            assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getCryptos().entries() == 2);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(0))->getCandidates().entries() == 0);   // !slg! candidate code in SdpBody is for old draft and doesn't work for this test
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getCandidates().entries() == 5); 
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getRemoteCandidates().entries() == 3);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getPreConditionCurrentStatus().entries() == 2);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getPreConditionConfirmStatus().entries() == 2);
            //assert(((SdpMediaLine*)convSdp->getMediaLines().at(1))->getPreConditionDesiredStatus().entries() == 2);
         }
         delete convSdp;
      }
#endif
   }

	return 0;
}

#endif // OLD_SDPLIB_UTESTS
