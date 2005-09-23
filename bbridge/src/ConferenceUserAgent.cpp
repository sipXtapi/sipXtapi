#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "resip/dum/ServerInviteSession.hxx"
#include "rutil/DnsUtil.hxx"
#include "ConferenceUserAgent.h"
#include "net/SdpCodec.h"

using namespace resip;
using namespace std;

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

const char *CODEC_G711_PCMU="258";
const char *CODEC_G711_PCMA="257";
const char *CODEC_DTMF_RFC2833="128";

Participant::Participant(DialogUsageManager& dum, const SipMessage& msg) : AppDialogSet(dum)
{
   ConferenceUserAgent& ua = dynamic_cast<ConferenceUserAgent&>(dum);
   ((void *)&ua); // suppress "unused variable" warning
}

Participant::~Participant()
{
   if (mConference)
   {
      assert(mConference->mMedia);
      mConference->mMedia->stopRtpReceive(mConnId);
      mConference->mMedia->stopRtpSend(mConnId);
      mConference->mMedia->deleteConnection(mConnId);
      // should check to see if all participants are gone now and delete the
      // conference if needed
   }
}

void
Participant::assign(Conference* conf, int connId)
{
   mConference = conf;
   mConnId = connId;
}

int 
Participant::id() const
{
   return mConnId;
}

void
Participant::accept(const SdpContents& offer, SdpContents& answer)
{
   assert(mConference);
   
   mConference->mMedia->createConnection(mConnId, 0);
   Data peerHost;
   int peerRtpPort = 0;
   int peerRtcpPort = 0;
   int dummyPort = 0;
   
   int numCodecs = 0;
   SdpCodec **sendCodecs = NULL;
   SdpSrtpParameters srtpParams;


   assert(mConference->mMedia); // !ah! pointless -- see above, we already indirected it
   
   resip::Data offerData;
   DataStream ds(offerData);
   ds << offer; // !ah! yetch
   // !ah! - offerData ready to convert to sipX SDP (yes we know this is gross)
   
   ::SdpBody offerSipX(offer.c_str(), offer.size());
   ::SdpBody answerSipX;
    
    int numCodecsInCommon = 0;
    SdpCodec **codecsInCommon = 0;
    UtlString remoteAddress;
    UtlString localAddress;
    SdpCodecFactory supportedCodecs;
    SdpSrtpParameters srtpParams;
    
    mConference->mMedia->getCapabilities(mConnId, localAddress, localRtpPort, localRtcpPort, dummyPort, dummyPort, supportedCodecs, srtpParams);
    
    offerSipX->getBestAudioCodecs(supportedCodecs,
            numCodecsInCommon,
            codecsInCommon,
            &remoteAddress,
            &peerRtpPort, 
            &peerRtcpPort,
            dummyPort,
            dummyPort); // !ah! dummy used for video

    sdpBody.setStandardHeaderFields("conference",0,0,localAddress);
    sdpBody.addAudioCodecs(localAddress,
			   localRtpPort,
			   localRtcpPort,
			   dummyPort, dummyPort, // !ah! --video
			   numCodecsInCommon,
			   codecsInCommon, 
			   srtpParams);

    // !ah! convert to resiprocate SDP

   int sdpSize = sdpBody.getLength();
   char * sdpData = new  char[sdpSize];
   sdpBody.getBytes(&sdpData,&sdpSize);
   answer = SdpContents(sdpData,sdpSize);
   delete [] sdpData;

   mConference->mMedia->setConnectionDestination(mConnId, peerHost.c_str(), 
                                                 peerRtpPort, peerRtcpPort,
                                                 0, 0);
   mConference->mMedia->startRtpReceive(mConnId, numCodecs, 
                                        sendCodecs,
                                        srtpParams);
   mConference->mMedia->startRtpSend(mConnId,numCodecs,
                                     sendCodecs,
                                     srtpParams);
}

Conference::Conference(ConferenceUserAgent& ua, const Data& aor) : 
   mAor(aor),
   mMedia(ua.mMediaFactory.createMediaInterface(0, // not used
                                                0, // not used
                                                ua.mNumCodecs, 
                                                ua.mSdpCodecArray, 
                                                "",  // locale
                                                QOS_LAYER3_LOW_DELAY_IP_TOS,
                                                "", // stun server
                                                0, // stun options
                                                25)) // stun keep alive
{
}

Conference::~Conference()
{
   
}

AppDialogSet* 
ParticipantFactory::createAppDialogSet(DialogUsageManager& dum, const SipMessage& msg)
{
   return new Participant(dum, msg);
}


ConferenceUserAgent::ConferenceUserAgent(const NameAddr& myAor) :
   mProfile(new MasterProfile),
   mSecurity(new Security(mCertPath)),
   mStack(mSecurity),
   mDum(mStack),
   mStackThread(mStack),
   mDumThread(mDum),
   mConfigDb(),
   mMediaFactory(&mConfigDb),
   mCodecFactory(),
   mSdpCodecArray(0),
   mNumCodecs(0)
{
   const int numCodecs = 3;
   SdpCodec::SdpCodecTypes sdpCodecEnumArray[numCodecs];
   sdpCodecEnumArray[0] = SdpCodecFactory::getCodecType(CODEC_G711_PCMU);
   sdpCodecEnumArray[1] = SdpCodecFactory::getCodecType(CODEC_G711_PCMA);
   sdpCodecEnumArray[2] = SdpCodecFactory::getCodecType(CODEC_DTMF_RFC2833);

   mCodecFactory.buildSdpCodecFactory(numCodecs, sdpCodecEnumArray);
   mCodecFactory.getCodecs(mNumCodecs, mSdpCodecArray);
    
   //Log::initialize(mLogType, mLogLevel, argv[0]);
   addTransport(UDP, mUdpPort);
   addTransport(TCP, mTcpPort);
   addTransport(TLS, mTlsPort, V4, Data::Empty, myAor.host());
    
   mProfile->addSupportedMethod(INVITE);
   mProfile->validateAcceptEnabled() = false;
   mProfile->validateContentEnabled() = false;
   mProfile->setDefaultFrom(myAor);
   
   if (!mContact.host().empty())
   {
      mProfile->setOverrideHostAndPort(mContact);
   }
   mProfile->setUserAgent("BostonBridge/0.1");
   
   mDum.setMasterProfile(mProfile);
   mDum.setInviteSessionHandler(this);
   
   mStackThread.run(); 
   mDumThread.run();
}

ConferenceUserAgent::~ConferenceUserAgent()
{
   mDumThread.shutdown();
   mDumThread.join();
   mStackThread.shutdown();
   mStackThread.join();
   
}

void
ConferenceUserAgent::onNewSession(ServerInviteSessionHandle h, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
   InfoLog(<< h->myAddr().uri().user() << " INVITE from  " << h->peerAddr().uri().user());

   const Data& aor = msg.header(h_RequestLine).uri().getAor();
   Participant* part = dynamic_cast<Participant*>(getAppDialogSet().get());
   assert(part);
   if (!mConferences.count(aor))
   {
      mConferences[aor] = new Conference(aor);
   }
   part->assign(mConferences[aor]);
}

void
ConferenceUserAgent::onTerminated(InviteSessionHandle h, InviteSessionHandler::TerminatedReason reason, const SipMessage* msg)
{
   if (reason != InviteSessionHandler::PeerEnded)
   {
      WarningLog(<< h->myAddr().uri().user() << " call terminated with " << h->peerAddr().uri().user());
   }
   else
   {
      WarningLog(<< h->myAddr().uri().user() << " ended call with " << h->peerAddr().uri().user());
   }

   Participant* part = dynamic_cast<Participant*>(getAppDialogSet().get());
   assert(part);
   assert(mConferences.count(aor));
   delete part;
   // should probably have the conference keep a reference count and remove when
   // all participants disappear
}

void
ConferenceUserAgent::onOffer(InviteSessionHandle handle, const SipMessage& msg, const SdpContents& offer)
{         
   const Data& aor = msg.header(h_RequestLine).uri().getAor();
   Participant* part = dynamic_cast<Participant*>(getAppDialogSet().get());
   assert(mConferences.count(aor));
   assert(part);
   
   SdpContents answer;
   part->accept(offer, answer); // answer is returned
   h->provideAnswer(answer);
   h->accept();
}

void
ConferenceUserAgent::onNewSession(ClientInviteSessionHandle h, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
   assert(0);
}


void
ConferenceUserAgent::onAnswer(InviteSessionHandle, const SipMessage& msg, const SdpContents&)
{
   assert(0);
}

void
ConferenceUserAgent::onFailure(ClientInviteSessionHandle h, const SipMessage& msg)
{
   assert(0);
}
      
void
ConferenceUserAgent::onEarlyMedia(ClientInviteSessionHandle, const SipMessage&, const SdpContents&)
{
   assert(0);
}

void
ConferenceUserAgent::onProvisional(ClientInviteSessionHandle, const SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onConnected(ClientInviteSessionHandle h, const SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onConnected(InviteSessionHandle, const SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onStaleCallTimeout(ClientInviteSessionHandle)
{
   assert(0);
}

void
ConferenceUserAgent::onRedirected(ClientInviteSessionHandle, const SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onOfferRequired(InviteSessionHandle, const SipMessage& msg)
{
   assert(false);
}

void
ConferenceUserAgent::onOfferRejected(InviteSessionHandle, const SipMessage* msg)
{
   assert(0);
}

void
ConferenceUserAgent::onDialogModified(InviteSessionHandle, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onInfo(InviteSessionHandle, const SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onInfoSuccess(InviteSessionHandle, const SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onInfoFailure(InviteSessionHandle, const SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onRefer(InviteSessionHandle, ServerSubscriptionHandle, const SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onReferAccepted(InviteSessionHandle, ClientSubscriptionHandle, const SipMessage& msg)
{
   assert(false);
}

void
ConferenceUserAgent::onReferRejected(InviteSessionHandle, const SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onMessage(InviteSessionHandle, const SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onMessageSuccess(InviteSessionHandle, const SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onMessageFailure(InviteSessionHandle, const SipMessage& msg)
{
   assert(0);
}


void
ConferenceUserAgent::onForkDestroyed(ClientInviteSessionHandle)
{
   assert(0);
}

/*
  Copyright (c) 2005, Jason Fischl
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the name of PurpleComm, Inc. nor the names of its contributors may
    be used to endorse or promote products derived from this software without
    specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
