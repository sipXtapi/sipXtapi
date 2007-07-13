#if defined(_WIN32)
#   include <winsock2.h>
#endif

#include "os/OsConfigDb.h"
#include "os/OsTask.h"
#include "net/SdpCodec.h"

#include "resip/dum/ClientAuthManager.hxx"
#include "resip/dum/Handles.hxx"
#include "resip/dum/ServerInviteSession.hxx"
#include "resip/dum/ServerSubscription.hxx"
#include "resip/stack/Aor.hxx"
#include "resip/stack/PlainContents.hxx"
#include "resip/stack/SipMessage.hxx"
#include "resip/stack/SdpContents.hxx"
#include "rutil/DnsUtil.hxx"
#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "rutil/Logger.hxx"

#include "bbridge.h"

#include "ParticipantFactory.h"
#include "Participant.h"
#include "Conference.h"
#include "ConferenceUserAgent.h"
#include "ConferenceSubscriptionApp.h"
#include "Subsystem.h"

#define RESIPROCATE_SUBSYSTEM bbridge::Subsystem::BBRIDGE

using namespace bbridge;
using namespace std;

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
   mNumCodecs(0),
   mMime("application", "conference-info+xml")
{
   // Get the IP Address to use for media
   UtlString ipAddress;
   mConfigDb.get("BOSTON_BRIDGE_MEDIA_IP_ADDRESS", ipAddress);  // If setting not specified or blank then query OS for IP Address
   if(ipAddress.length() == 0)
   {
      mMediaIpAddress = resip::DnsUtil::getLocalIpAddress();
   }
   else
   {
      mMediaIpAddress = ipAddress.data();
   }

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
   
   // !jf! should consider cases where these aren't set
   mConfigDb.get("BOSTON_BRIDGE_UDP_PORT", mUdpPort);
   mConfigDb.get("BOSTON_BRIDGE_TCP_PORT", mTcpPort);
   mConfigDb.get("BOSTON_BRIDGE_TLS_PORT", mTlsPort);
   
   // Disable statistics Manager
   mStack.statisticsManagerEnabled() = false;

   resip::NameAddr myAor;
   mDum.addTransport(resip::UDP, mUdpPort);
   mDum.addTransport(resip::TCP, mTcpPort);
   mDum.addTransport(resip::TLS, mTlsPort, resip::V4, resip::Data::Empty, 
                     myAor.uri().host());
    
   mProfile->addSupportedMethod(resip::INVITE);
   mProfile->addSupportedMethod(resip::SUBSCRIBE);
   mProfile->addAllowedEvent(resip::Token("conference"));
   mProfile->validateAcceptEnabled() = false;
   mProfile->validateContentEnabled() = false;
   mProfile->setDefaultFrom(myAor);

   resip::Data uaId;
   uaId += "BostonBridge/";
   uaId += SIPX_BBRIDGE_VERSION;
   mProfile->setUserAgent(uaId);
   
   std::auto_ptr<resip::AppDialogSetFactory> rfactory(new ParticipantFactory);
   mDum.setAppDialogSetFactory(rfactory);
   mDum.setMasterProfile(mProfile);
   mDum.setInviteSessionHandler(this);
   mDum.setClientRegistrationHandler(this);

   std::auto_ptr<resip::ClientAuthManager> clam(new resip::ClientAuthManager());
   mDum.setClientAuthManager(clam);

   UtlString gw1Aor;
   mConfigDb.get("BOSTON_BRIDGE_GATEWAY1_AOR", gw1Aor);
   if (!gw1Aor.isNull())
   {
      UtlString gw1Username;
      UtlString gw1Password;
      UtlString gw1Realm;
      UtlString gw1Conference;
      
      mConfigDb.get("BOSTON_BRIDGE_GATEWAY1_USERNAME", gw1Username);
      mConfigDb.get("BOSTON_BRIDGE_GATEWAY1_PASSWORD", gw1Password);
      mConfigDb.get("BOSTON_BRIDGE_GATEWAY1_REALM", gw1Realm);
      mConfigDb.get("BOSTON_BRIDGE_GATEWAY1_CONFERENCE", gw1Conference);
      resip::SharedPtr<resip::UserProfile> gw1(new resip::UserProfile(mProfile));
      resip::NameAddr aor(gw1Aor.data());
      gw1->setDefaultFrom(aor);
      gw1->setDigestCredential(gw1Realm.data(), gw1Username.data(), gw1Password.data());
      resip::SharedPtr<resip::SipMessage> reg = mDum.makeRegistration(aor, gw1);
      InfoLog (<< "Registering  " << aor.uri().user() << " to map to " << gw1Conference.data());
      mDum.send(reg);
      mInBoundMap[aor.uri().user()] = gw1Conference.data();
   }

   mMimes.push_back(mMime);
   mDum.addServerSubscriptionHandler("conference", this);
   
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
ConferenceUserAgent::post(resip::DumCommand* cmd)
{
   mDum.post(cmd);
}

void 
ConferenceUserAgent::onSuccess(resip::ClientRegistrationHandle h, 
                               const resip::SipMessage& response)
{
   const resip::Data& aor = response.header(resip::h_From).uri().getAor();
   InfoLog (<< "Registered an AOR for inbound calls to " << aor);

}

void 
ConferenceUserAgent::onFailure(resip::ClientRegistrationHandle h, 
                               const resip::SipMessage& response)
{
   InfoLog (<< "Failed to register " << response.header(resip::h_From).uri().getAor());
}

void 
ConferenceUserAgent::onRemoved(resip::ClientRegistrationHandle h, const resip::SipMessage& response)
{
}

int 
ConferenceUserAgent::onRequestRetry(resip::ClientRegistrationHandle h, 
                                    int retrySeconds, 
                                    const resip::SipMessage& response)
{
   return -1;
}

resip::Data
ConferenceUserAgent::getConferenceUrl(const resip::Uri& uri)
{
   resip::Data aor = uri.getAor();
   if (mInBoundMap.count(uri.user()))
   {
     aor = mInBoundMap[uri.user()];
   }
     
   InfoLog (<< "uri: " << uri << " is mapped to  " << aor);

   return aor;
}

void
ConferenceUserAgent::onNewSession(resip::ServerInviteSessionHandle h,
                                  resip::InviteSession::OfferAnswerType oat,
                                  const resip::SipMessage& msg)
{
   InfoLog(<< h->myAddr().uri().user() << " INVITE from  " << h->peerAddr().uri().user());


   Participant* part = dynamic_cast<Participant*>(h->getAppDialogSet().get());
   assert(part);
   resip::Data aor = getConferenceUrl(msg.header(resip::h_RequestLine).uri());
   if (!mConferences.count(aor))
   {
     InfoLog (<< "Adding a conference for " << aor);

     mConferences[aor] = new Conference(*this, aor, mConfigDb, mMediaIpAddress);
   }
   part->assign(mConferences[aor]);

   // Generate notices for all subscribers to this conference's package.
   mConferences[aor]->notifyAll();
}

void
ConferenceUserAgent::onTerminated(resip::InviteSessionHandle h,
                                  resip::InviteSessionHandler::TerminatedReason reason,
                                  const resip::SipMessage* msg)
{
   if (reason != InviteSessionHandler::PeerEnded)
   {
      WarningLog(<< h->myAddr().uri().getAor() << " call terminated with " << h->peerAddr().uri().getAor());
   }
   else
   {
      WarningLog(<< h->myAddr().uri().getAor() << " ended call with " << h->peerAddr().uri().getAor());
   }

   Conference* conf = mConferences[getConferenceUrl(h->myAddr().uri())];
   assert(conf);

   // should probably have the conference keep a reference count and remove when
   // all participants disappear

   // Generate notices for all subscribers for this conference's events.
   conf->notifyAll();
}

void
ConferenceUserAgent::onOffer(resip::InviteSessionHandle h,
                             const resip::SipMessage& msg,
                             const resip::SdpContents& offer)
{         
  resip::Data aor = getConferenceUrl(msg.header(resip::h_RequestLine).uri());
   Participant* part = dynamic_cast<Participant*>(h->getAppDialogSet().get());
   assert(mConferences.count(aor));
   assert(part);
   
   resip::ServerInviteSession *sis =
     dynamic_cast<resip::ServerInviteSession*>(h.get());
   assert(sis);

   if (!h->isAccepted())
   {
      resip::SdpContents answer;
      part->accept(offer, answer); // answer is returned
      InfoLog (<< "Accepting offer with " << answer);
      h->provideAnswer(answer);

      InfoLog (<< "Accepting ");
      sis->accept();
   }
   else
   {
       // Re-invite - should probably be passed to Participant, incase SDP has changed
       // for now just assume a re-invite is only for session refresh and SDP has not changed
       // TODO
       h->provideAnswer(sis->getLocalSdp());
       InfoLog (<< "providing Answer to reInvite");
   }
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
ConferenceUserAgent::onReferNoSub(resip::InviteSessionHandle, 
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

// Implementation of the SUBSCRIBE handler.

void ConferenceUserAgent::onNewSubscription(resip::ServerSubscriptionHandle handle,
                                            const resip::SipMessage& sub)
{
   // Get the AOR that is being subscribed to.
   resip::Data subscribe_aor = sub.header(resip::h_RequestLine).uri().getAor();
   // Get the conference for the AOR.
   Conference* conference = mConferences[subscribe_aor];

   if (conference)
   {
      InfoLog(<< "Accepted subscription for AOR '" << subscribe_aor
              << "', subscriber '" << handle->getSubscriber()
              << "', for subscription ID '" << handle.getId()
              << "', dialog ID '" << handle->getDialogId()
              << "'");
      // Accept the subscription.
      handle->send(handle->accept());
      // Attach the subscription to the conference.
      ConferenceSubscriptionApp* subscribe_app =
         dynamic_cast<ConferenceSubscriptionApp*>(handle->getAppDialogSet().get());
      subscribe_app->attach(conference);
      // Record the ServerSubscriptionHandle in the application object.
      subscribe_app->setSubscriptionHandle(handle);
      // Send notify to the subscriber.
      subscribe_app->notifyOne();
   }
   else
   {
      InfoLog(<< "Rejected subscription for AOR '" << subscribe_aor
              << "'");
      // Reject the subscription.
      handle->send(handle->reject(404));
   }
}

void ConferenceUserAgent::onRefresh(resip::ServerSubscriptionHandle handle,
                                    const resip::SipMessage& sub)
{
   // Send notify to subscriber.
   ConferenceSubscriptionApp* subscribe_app =
      dynamic_cast<ConferenceSubscriptionApp*>(handle->getAppDialogSet().get());
   subscribe_app->notifyOne();
}

void ConferenceUserAgent::onPublished(resip::ServerSubscriptionHandle associated, 
                                      resip::ServerPublicationHandle publication, 
                                      const resip::Contents* contents,
                                      const resip::SecurityAttributes* attrs)
{
   // This should never be called, as we do not tell DUM that we handle PUBLISH.
   assert(false);
}

void ConferenceUserAgent::onNotifyRejected(resip::ServerSubscriptionHandle handle,
                                           const resip::SipMessage& msg)
{
   // If this is an expected sort of rejection, log at info level.  Otherwise, log at
   // warning level, because the subscriber may be rejecting our conference event
   // due to it being malformed.
   // The "expected" responses are:
   //        481 (Subscription does not exist)
   //        can happen because the client was reinitialized and forgot the subscription
   //        408 (Request timed out)
   //        client is no longer functioning
   int status = msg.header(resip::h_StatusLine).statusCode();
   if (status == 408 || status == 481)
   {
      InfoLog(<< "NOTIFY failed with status " << status
              << ", subscriber '" << handle->getSubscriber()
              << "', for subscription ID '" << handle.getId()
              << "', dialog ID '" << handle->getDialogId()
              << "'");
   }
   else
   {
      WarningLog(<< "NOTIFY failed with status " << status
                 << ", subscriber '" << handle->getSubscriber()
                 << "', for subscription ID '" << handle.getId()
                 << "', dialog ID '" << handle->getDialogId()
                 << "'");
   }
}

//called when this usage is destroyed for any reason. One of the following
//three methods will always be called before this, but this is the only
//method that MUST be implemented by a handler
void ConferenceUserAgent::onTerminated(resip::ServerSubscriptionHandle handle)
{
   // Disconnect the subscription from the conference.
   ConferenceSubscriptionApp* subscribe_app =
      dynamic_cast<ConferenceSubscriptionApp*>(handle->getAppDialogSet().get());
   subscribe_app->detach();
}

//will be called when a NOTIFY is not delivered(with a usage terminating
//statusCode), or the Dialog is destroyed
void ConferenceUserAgent::onError(resip::ServerSubscriptionHandle handle,
                                  const resip::SipMessage& msg)
{
}

//app can synchronously decorate terminating NOTIFY messages. The only
//graceful termination mechanism is expiration, but the client can
//explicity end a subscription with an Expires header of 0.
void ConferenceUserAgent::onExpiredByClient(resip::ServerSubscriptionHandle handle,
                                            const resip::SipMessage& sub,
                                            resip::SipMessage& notify)
{
}

void ConferenceUserAgent::onExpired(resip::ServerSubscriptionHandle handle,
                                    resip::SipMessage& notify)
{
}

bool ConferenceUserAgent::hasDefaultExpires() const
{
   return true;
}

UInt32 ConferenceUserAgent::getDefaultExpires() const
{
   return 3600;
}

const resip::Mimes& ConferenceUserAgent::getSupportedMimeTypes() const
{
   // Return the Mimes that we have prepared.
   return mMimes;
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
