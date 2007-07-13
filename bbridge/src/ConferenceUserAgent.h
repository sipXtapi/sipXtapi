#ifndef __CONFERENCE_USER_AGENT_H
#define __CONFERENCE_USER_AGENT_H

// repro includes
#include "resip/dum/AppDialogSet.hxx"
#include "resip/dum/AppDialogSetFactory.hxx"
#include "resip/dum/ClientRegistration.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/DumThread.hxx"
#include "resip/dum/InviteSessionHandler.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/dum/RegistrationHandler.hxx"
#include "resip/dum/SubscriptionHandler.hxx"
#include "resip/stack/StackThread.hxx"

// sipX includes
#include "mi/CpMediaInterfaceFactoryFactory.h"
#include "mi/CpMediaInterface.h"
#include "net/QoS.h"
#include "net/SdpCodecFactory.h"

// bbridge includes
#include "ConferenceSubscriptionApp.h"

class OsConfig;

namespace resip
{
class DumCommand;
}

namespace bbridge
{

class Conference;

/**
   There is one ConferenceUserAgent per conference bridge. Its responsibilities
   include managing the resip::SipStack and resip::Dum objects. These objects are
   run in their own respective threads using resip::StackThread and
   resip::DumThread respectively. ConferenceUserAgent also creates and manages
   Conference objects. A conference is one bridge and is identified by a unique
   sip url. It is also possible for the ConferenceUserAgent to maintain some
   registrations to services like Vonage or Stanaphone for the purposes of
   providing DID access to specific conferences. 
 */
class ConferenceUserAgent : public resip::InviteSessionHandler,
                            public resip::ServerSubscriptionHandler,
                            public resip::ClientRegistrationHandler
{
   public:
      /** 
          On construction, the ConferenceUserAgent will create the SipStack,
          DialogUsageManager and their associated threads. All configuration
          data is pulled out of the OsConfigDb. Some of this data is used to
          populate the resip::MasterProfile. When the constructor finishes the
          stack and dum threads are running. 
       */ 
      ConferenceUserAgent(OsConfigDb& db);

      /** 
          On destruction, the ConferenceUserAgent shuts down its child threads
          and waits for them to finish shutting down
      */
      virtual ~ConferenceUserAgent();

      /** In order to post an event (command pattern) to be executed later by dum
          in the ConferenceUserAgent's thread. Specifically, this is used by the
          Participant in a conference to post DtmfEvent objects asynchronously
          for later processing. 
      */
      void post(resip::DumCommand* cmd);

   public:
      // resip::Invite Session Handler /////////////////////////////////////////////////////
      virtual void onNewSession(resip::ClientInviteSessionHandle h, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg);
      virtual void onNewSession(resip::ServerInviteSessionHandle h, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg);
      virtual void onFailure(resip::ClientInviteSessionHandle h, const resip::SipMessage& msg);
      virtual void onEarlyMedia(resip::ClientInviteSessionHandle, const resip::SipMessage&, const resip::SdpContents&);
      virtual void onProvisional(resip::ClientInviteSessionHandle, const resip::SipMessage& msg);
      virtual void onConnected(resip::ClientInviteSessionHandle h, const resip::SipMessage& msg);
      virtual void onConnected(resip::InviteSessionHandle, const resip::SipMessage& msg);
      virtual void onStaleCallTimeout(resip::ClientInviteSessionHandle);
      virtual void onTerminated(resip::InviteSessionHandle h, resip::InviteSessionHandler::TerminatedReason reason, const resip::SipMessage* msg);
      virtual void onRedirected(resip::ClientInviteSessionHandle, const resip::SipMessage& msg);
      virtual void onAnswer(resip::InviteSessionHandle, const resip::SipMessage& msg, const resip::SdpContents&);
      virtual void onOffer(resip::InviteSessionHandle handle, const resip::SipMessage& msg, const resip::SdpContents& offer);
      virtual void onOfferRequired(resip::InviteSessionHandle, const resip::SipMessage& msg);
      virtual void onOfferRejected(resip::InviteSessionHandle, const resip::SipMessage* msg);
      virtual void onDialogModified(resip::InviteSessionHandle, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg);
      virtual void onInfo(resip::InviteSessionHandle, const resip::SipMessage& msg);
      virtual void onInfoSuccess(resip::InviteSessionHandle, const resip::SipMessage& msg);
      virtual void onInfoFailure(resip::InviteSessionHandle, const resip::SipMessage& msg);
      virtual void onRefer(resip::InviteSessionHandle, resip::ServerSubscriptionHandle, const resip::SipMessage& msg);
      virtual void onReferNoSub(resip::InviteSessionHandle, const resip::SipMessage& msg);
      virtual void onReferAccepted(resip::InviteSessionHandle, resip::ClientSubscriptionHandle, const resip::SipMessage& msg);
      virtual void onReferRejected(resip::InviteSessionHandle, const resip::SipMessage& msg);

      virtual void onMessage(resip::InviteSessionHandle, const resip::SipMessage& msg);
      virtual void onMessageSuccess(resip::InviteSessionHandle, const resip::SipMessage& msg);
      virtual void onMessageFailure(resip::InviteSessionHandle, const resip::SipMessage& msg);

      virtual void onForkDestroyed(resip::ClientInviteSessionHandle);

      // resip::Client Registration Handler ///////////////////////////////////////////
      virtual void onSuccess(resip::ClientRegistrationHandle, const resip::SipMessage& response);
      virtual void onRemoved(resip::ClientRegistrationHandle, const resip::SipMessage& response);
      virtual int onRequestRetry(resip::ClientRegistrationHandle, int retrySeconds, const resip::SipMessage& response);
      virtual void onFailure(resip::ClientRegistrationHandle, const resip::SipMessage& response);

      // resip::Subscribe Session Handler /////////////////////////////////////////////////////
      virtual void onNewSubscription(resip::ServerSubscriptionHandle,
                                     const resip::SipMessage& sub);
      virtual void onRefresh(resip::ServerSubscriptionHandle,
                             const resip::SipMessage& sub);
      virtual void onPublished(resip::ServerSubscriptionHandle associated, 
                               resip::ServerPublicationHandle publication, 
                               const resip::Contents* contents,
                               const resip::SecurityAttributes* attrs);
      virtual void onNotifyRejected(resip::ServerSubscriptionHandle,
                                    const resip::SipMessage& msg);      
      virtual void onTerminated(resip::ServerSubscriptionHandle);
      virtual void onError(resip::ServerSubscriptionHandle,
                           const resip::SipMessage& msg);      
      virtual void onExpiredByClient(resip::ServerSubscriptionHandle,
                                     const resip::SipMessage& sub,
                                     resip::SipMessage& notify);
      virtual void onExpired(resip::ServerSubscriptionHandle,
                             resip::SipMessage& notify);
      virtual bool hasDefaultExpires() const;
      virtual UInt32 getDefaultExpires() const;
      const resip::Mimes& getSupportedMimeTypes() const;

   private:
      /**
         When provided a Request-URI, this method will determine if the URI maps
         to an existing conference URI using mInBoundMap (used to map from a DID
         to a conference). This mechanism is only useful in small enterprise
         environments. 
       */
      resip::Data getConferenceUrl(const resip::Uri& msg);

      OsConfigDb &mConfigDb;
      resip::Data mMediaIpAddress;
      resip::SharedPtr<resip::MasterProfile> mProfile;
      resip::Security* mSecurity;
      resip::SipStack mStack;
      resip::DialogUsageManager mDum;
      resip::StackThread mStackThread;
      resip::DumThread mDumThread;
      
      CpMediaInterfaceFactory* mMediaFactory;
      SdpCodecFactory mCodecFactory;
      SdpCodec** mSdpCodecArray;
      int mNumCodecs;
      int mUdpPort;
      int mTcpPort;
      int mTlsPort;

      /// Map from AORs to the conferences that have those AORs.
      HashMap<resip::Data, bbridge::Conference*> mConferences;

      /// mapping table from registered AOR -> conference URI
      HashMap<resip::Data, resip::Data> mInBoundMap;
      
      /// A Mime object for application/conference-info+xml.
      resip::Mime mMime;

      /// A Mimes object to list { application/conference-info+xml }.
      resip::Mimes mMimes;

      friend class Conference;
};
 
}

#endif

/* ====================================================================
 * The Vovida Software License, Version 1.0 
 * 
 * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 
 * 3. The names "VOCAL", "Vovida Open Communication Application Library",
 *    and "Vovida Open Communication Application Library (VOCAL)" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact vocal@vovida.org.
 *
 * 4. Products derived from this software may not be called "VOCAL", nor
 *    may "VOCAL" appear in their name, without prior written
 *    permission of Vovida Networks, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA
 * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * 
 * ====================================================================
 * 
 * This software consists of voluntary contributions made by Vovida
 * Networks, Inc. and many individuals on behalf of Vovida Networks,
 * Inc.  For more information on Vovida Networks, Inc., please see
 * <http://www.vovida.org/>.
 *
 */
