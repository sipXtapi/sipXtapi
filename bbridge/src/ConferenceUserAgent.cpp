#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "resip/dum/ServerInviteSession.hxx"
#include "resip/dum/Handles.hxx"
#include "resip/stack/Aor.hxx"
#include "resip/stack/SdpContents.hxx"
#include "rutil/DnsUtil.hxx"
#include "ConferenceUserAgent.h"
#include "net/SdpCodec.h"

using namespace bbridge;
using namespace std;

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

const char *CODEC_G711_PCMU="258";
const char *CODEC_G711_PCMA="257";
const char *CODEC_DTMF_RFC2833="128";

ConferenceUserAgent::ConferenceUserAgent(OsConfigDb& db) :
   mConfigDb(db),
   mProfile(new MasterProfile),
   mSecurity(new Security(mCertPath)),
   mStack(mSecurity),
   mDum(mStack),
   mStackThread(mStack),
   mDumThread(mDum),
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

   // XXX ADD CODE TO READ FROM CONFIG INTO mUdpPort ET AL HERE.
   //Log::initialize(mLogType, mLogLevel, argv[0]);
   // hook up resip logging to sipX
   
   // !jf! should consider cases where these aren't set
   mConfigDb.get("SIP.UDP", mUdpPort);
   mConfigDb.get("SIP.TCP", mTcpPort);
   mConfigDb.get("SIP.TLS", mTlsPort);
   
   mDum.addTransport(UDP, mUdpPort);
   mDum.addTransport(TCP, mTcpPort);
   mDum.addTransport(TLS, mTlsPort, V4, Data::Empty, myAor.uri().host());
    
   mProfile->addSupportedMethod(INVITE);
   mProfile->validateAcceptEnabled() = false;
   mProfile->validateContentEnabled() = false;
   NameAddr myAor;
   mProfile->setDefaultFrom(myAor);
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
   Participant* part = dynamic_cast<Participant*>(h->getAppDialogSet().get());
   assert(part);
   if (!mConferences.count(aor))
   {
      mConferences[aor] = new Conference(*this,aor);
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

   Participant* part = dynamic_cast<Participant*>(h->getAppDialogSet().get());
   assert(part);
   assert(mConferences.count(h->myAddr().uri().getAor()));
   delete part;
   // should probably have the conference keep a reference count and remove when
   // all participants disappear
}

void
ConferenceUserAgent::onOffer(InviteSessionHandle h, const SipMessage& msg, const SdpContents& offer)
{         
   const Data& aor = msg.header(h_RequestLine).uri().getAor();
   Participant* part = dynamic_cast<Participant*>(h->getAppDialogSet().get());
   assert(mConferences.count(aor));
   assert(part);
   
   SdpContents answer;
   part->accept(offer, answer); // answer is returned
   h->provideAnswer(answer);

   ServerInviteSession *sis = dynamic_cast<ServerInviteSession*>(h.get());
   assert(sis);
   sis->accept();
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
  Copyright (c) 2005, Jason Fischl, Adam Roach
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
