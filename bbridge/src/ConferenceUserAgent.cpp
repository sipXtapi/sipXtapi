#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "resip/dum/ServerInviteSession.hxx"
#include "resip/dum/Handles.hxx"
#include "resip/stack/Aor.hxx"
#include "resip/stack/SdpContents.hxx"
#include "rutil/DnsUtil.hxx"
#include "net/SdpCodec.h"

#include "ParticipantFactory.h"
#include "Participant.h"
#include "Conference.h"
#include "ConferenceUserAgent.h"

using namespace bbridge;
using namespace std;

#ifndef SIPX_CONFDIR
#   define SIPX_CONFDIR "."
#endif

#define RESIPROCATE_SUBSYSTEM resip::Subsystem::TEST

const char *CODEC_G711_PCMU="258";
const char *CODEC_G711_PCMA="257";
const char *CODEC_DTMF_RFC2833="128";

ConferenceUserAgent::ConferenceUserAgent(OsConfigDb& db) :
   mConfigDb(db),
   mProfile(new resip::MasterProfile),
   mSecurity(new resip::Security(resip::Data(SIPX_CONFDIR "/ssl"))),
   mStack(mSecurity),
   mDum(mStack),
   mStackThread(mStack),
   mDumThread(mDum),
   mMediaFactory(sipXmediaFactoryFactory(NULL)),
   mCodecFactory(),
   mSdpCodecArray(0),
   mNumCodecs(0)
{
   // Instruct the factory to use the specified port range
   int rtpPortStart, rtpPortEnd;
   mConfigDb.get("BOSTON_BRIDGE_RTP_START", rtpPortStart);
   mConfigDb.get("BOSTON_BRIDGE_RTP_END", rtpPortEnd);
   mMediaFactory->getFactoryImplementation()->setRtpPortRange(rtpPortStart, rtpPortEnd) ;
   
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
   mConfigDb.get("BOSTON_BRIDGE_UDP_PORT", mUdpPort);
   mConfigDb.get("BOSTON_BRIDGE_TCP_PORT", mTcpPort);
   mConfigDb.get("BOSTON_BRIDGE_TLS_PORT", mTlsPort);
   
   resip::NameAddr myAor;
   mDum.addTransport(resip::UDP, mUdpPort);
   mDum.addTransport(resip::TCP, mTcpPort);
   mDum.addTransport(resip::TLS, mTlsPort, resip::V4, resip::Data::Empty, 
                     myAor.uri().host());
    
   mProfile->addSupportedMethod(resip::INVITE);
   mProfile->validateAcceptEnabled() = false;
   mProfile->validateContentEnabled() = false;
   mProfile->setDefaultFrom(myAor);
   mProfile->setUserAgent("BostonBridge/0.1");
   
   std::auto_ptr<resip::AppDialogSetFactory> rfactory(new ParticipantFactory);
   mDum.setAppDialogSetFactory(rfactory);
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
ConferenceUserAgent::onNewSession(resip::ServerInviteSessionHandle h,
                                  resip::InviteSession::OfferAnswerType oat,
                                  const resip::SipMessage& msg)
{
   InfoLog(<< h->myAddr().uri().user() << " INVITE from  " << h->peerAddr().uri().user());

   const resip::Data& aor = msg.header(resip::h_RequestLine).uri().getAor();
   Participant* part = dynamic_cast<Participant*>(h->getAppDialogSet().get());
   assert(part);
   if (!mConferences.count(aor))
   {
      mConferences[aor] = new Conference(*this,aor);
   }
   part->assign(mConferences[aor]);
}

void
ConferenceUserAgent::onTerminated(resip::InviteSessionHandle h,
                                  resip::InviteSessionHandler::TerminatedReason reason,
                                  const resip::SipMessage* msg)
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
ConferenceUserAgent::onOffer(resip::InviteSessionHandle h,
                             const resip::SipMessage& msg,
                             const resip::SdpContents& offer)
{         
   const resip::Data& aor = msg.header(resip::h_RequestLine).uri().getAor();
   Participant* part = dynamic_cast<Participant*>(h->getAppDialogSet().get());
   assert(mConferences.count(aor));
   assert(part);
   
   resip::SdpContents answer;
   part->accept(offer, answer); // answer is returned
   InfoLog (<< "Accepting offer with " << answer);
   h->provideAnswer(answer);
   
   resip::ServerInviteSession *sis =
     dynamic_cast<resip::ServerInviteSession*>(h.get());
   assert(sis);
   InfoLog (<< "Accepting ");
   sis->accept();
}

void
ConferenceUserAgent::onNewSession(resip::ClientInviteSessionHandle h,
                                  resip::InviteSession::OfferAnswerType oat,
                                  const resip::SipMessage& msg)
{
   assert(0);
}


void
ConferenceUserAgent::onAnswer(resip::InviteSessionHandle,
                              const resip::SipMessage& msg,
                              const resip::SdpContents&)
{
   assert(0);
}

void
ConferenceUserAgent::onFailure(resip::ClientInviteSessionHandle h,
                               const resip::SipMessage& msg)
{
   assert(0);
}
      
void
ConferenceUserAgent::onEarlyMedia(resip::ClientInviteSessionHandle,
                                  const resip::SipMessage&,
                                  const resip::SdpContents&)
{
   assert(0);
}

void
ConferenceUserAgent::onProvisional(resip::ClientInviteSessionHandle,
                                   const resip::SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onConnected(resip::ClientInviteSessionHandle h,
                                 const resip::SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onConnected(resip::InviteSessionHandle,
                                 const resip::SipMessage& msg)
{
   //assert(0);
}

void
ConferenceUserAgent::onStaleCallTimeout(resip::ClientInviteSessionHandle)
{
   assert(0);
}

void
ConferenceUserAgent::onRedirected(resip::ClientInviteSessionHandle,
                                  const resip::SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onOfferRequired(resip::InviteSessionHandle,
                                     const resip::SipMessage& msg)
{
   assert(false);
}

void
ConferenceUserAgent::onOfferRejected(resip::InviteSessionHandle,
                                     const resip::SipMessage* msg)
{
   assert(0);
}

void
ConferenceUserAgent::onDialogModified(resip::InviteSessionHandle,
                                      resip::InviteSession::OfferAnswerType oat,
                                      const resip::SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onInfo(resip::InviteSessionHandle,
                            const resip::SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onInfoSuccess(resip::InviteSessionHandle,
                                   const resip::SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onInfoFailure(resip::InviteSessionHandle,
                                   const resip::SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onRefer(resip::InviteSessionHandle,
                             resip::ServerSubscriptionHandle,
                             const resip::SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onReferAccepted(resip::InviteSessionHandle,
                                     resip::ClientSubscriptionHandle,
                                     const resip::SipMessage& msg)
{
   assert(false);
}

void
ConferenceUserAgent::onReferRejected(resip::InviteSessionHandle,
                                     const resip::SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onMessage(resip::InviteSessionHandle,
                               const resip::SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onMessageSuccess(resip::InviteSessionHandle,
                                      const resip::SipMessage& msg)
{
   assert(0);
}

void
ConferenceUserAgent::onMessageFailure(resip::InviteSessionHandle,
                                      const resip::SipMessage& msg)
{
   assert(0);
}


void
ConferenceUserAgent::onForkDestroyed(resip::ClientInviteSessionHandle)
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
