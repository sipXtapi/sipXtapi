#if !defined(DUM_ConferenceUserAgent_hxx)
#define DUM_ConferenceUserAgent_hxx

// repro includes
#include "resip/dum/AppDialogSetFactory.hxx"
#include "resip/dum/AppDialogSet.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/DumThread.hxx"
#include "resip/dum/InviteSessionHandler.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/stack/StackThread.hxx"

// sipX includes
#include "os/OsTask.h"
#include "os/OsConfigDb.h"
#include "sipXmediaFactoryImpl.h"
#include "net/QoS.h"
#include "net/SdpCodecFactory.h"
#include "mi/CpMediaInterface.h"

namespace resip
{

class ParticipantFactory : public AppDialogSetFactory
{
   public:
      AppDialogSet* createAppDialogSet(DialogUsageManager&, const SipMessage&);      
};

class Conference;

class Participant : public AppDialogSet
{
   public:
      Participant(DialogUsageManager& dum);
      ~Participant();

      void assign(Conference* conf, int connId);
      int id() const;
      
   private:
      Conference* mConference;
      int mConnId;
};

class Conference : public CpMediaInterface
{
   public:
      Conference(const Data& aor);
      ~Conference();

   private:
      Data mAor;
};



class ConferenceUserAgent : public InviteSessionHandler
{
   public:
      ConferenceUserAgent();
      virtual ~ConferenceUserAgent();

      void process();
      
   public:
      // Invite Session Handler /////////////////////////////////////////////////////
      virtual void onNewSession(ClientInviteSessionHandle h, InviteSession::OfferAnswerType oat, const SipMessage& msg);
      virtual void onNewSession(ServerInviteSessionHandle h, InviteSession::OfferAnswerType oat, const SipMessage& msg);
      virtual void onFailure(ClientInviteSessionHandle h, const SipMessage& msg);
      virtual void onEarlyMedia(ClientInviteSessionHandle, const SipMessage&, const SdpContents&);
      virtual void onProvisional(ClientInviteSessionHandle, const SipMessage& msg);
      virtual void onConnected(ClientInviteSessionHandle h, const SipMessage& msg);
      virtual void onConnected(InviteSessionHandle, const SipMessage& msg);
      virtual void onStaleCallTimeout(ClientInviteSessionHandle);
      virtual void onTerminated(InviteSessionHandle h, InviteSessionHandler::TerminatedReason reason, const SipMessage* msg);
      virtual void onRedirected(ClientInviteSessionHandle, const SipMessage& msg);
      virtual void onAnswer(InviteSessionHandle, const SipMessage& msg, const SdpContents&);
      virtual void onOffer(InviteSessionHandle handle, const SipMessage& msg, const SdpContents& offer);
      virtual void onOfferRequired(InviteSessionHandle, const SipMessage& msg);
      virtual void onOfferRejected(InviteSessionHandle, const SipMessage* msg);
      virtual void onDialogModified(InviteSessionHandle, InviteSession::OfferAnswerType oat, const SipMessage& msg);
      virtual void onInfo(InviteSessionHandle, const SipMessage& msg);
      virtual void onInfoSuccess(InviteSessionHandle, const SipMessage& msg);
      virtual void onInfoFailure(InviteSessionHandle, const SipMessage& msg);
      virtual void onRefer(InviteSessionHandle, ServerSubscriptionHandle, const SipMessage& msg);
      virtual void onReferAccepted(InviteSessionHandle, ClientSubscriptionHandle, const SipMessage& msg);
      virtual void onReferRejected(InviteSessionHandle, const SipMessage& msg);

      virtual void onMessage(InviteSessionHandle, const SipMessage& msg);
      virtual void onMessageSuccess(InviteSessionHandle, const SipMessage& msg);
      virtual void onMessageFailure(InviteSessionHandle, const SipMessage& msg);

   private:
      SharedPtr<MasterProfile> mProfile;
      Security* mSecurity;
      SipStack mStack;
      DialogUsageManager mDum;
      StackThread mStackThread;
      DumThread mDumThread;

      OsConfigDb mConfigDb;
      sipXmediaFactoryImpl mMediaFactory;
      SdpCodecFactory mCodecFactory;
      SdpCodec** mSdpCodecArray;
      int mNumCodecs;
      
      HashMap<Data, Conference*> mConferences;
      
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
