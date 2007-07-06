#ifndef __CONFERENCE_H
#define __CONFERENCE_H

// repro includes
#include "resip/dum/AppDialogSet.hxx"
#include "resip/dum/AppDialogSetFactory.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/DumThread.hxx"
#include "resip/dum/InviteSessionHandler.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/stack/StackThread.hxx"

// sipX includes
#include "mi/CpMediaInterface.h"
#include "mi/CpMediaInterfaceFactoryFactory.h"
#include "net/QoS.h"
#include "net/SdpCodecFactory.h"
#include "os/OsConfigDb.h"
#include "os/OsTask.h"

// bbridge includes
#include "Participant.h"

namespace bbridge
{

class ConferenceUserAgent;
class ConferenceSubscriptionApp;

/**
   Conferences are objects that take care of managing a group of Participant
   objects that share media. The Conference also takes care of knowing when to
   play music on hold, managing subscriptions to the conference package, as well
   as creation and deletion of CpMediaInterface resources. 
*/
class Conference 
{
   public:
      /** 
          Called to create a new Conference which will always be associated with
          an address of record (AOR). At creation, there are no participants in
          a conference. Makes a CpMediaInterface (Flow Graph) using its
          associated factory.
      */ 
      Conference(ConferenceUserAgent &ua,
                 const resip::Data& aor,
                 OsConfigDb &configDb,
                 const resip::Data& ipAddress);

      /**
         Terminates all subscriptions to the conference event package and
         releases the resources associated with the CpMediaInterface.
       */
      ~Conference();
      
      /// Get the AOR to reach this conference.
      const resip::Data& getAor() const;

      /// Add a Participant to this conference's list of Participants.
      void addParticipant(Participant*);

      /// Delete a Participant from this conference's list of Participants.
      void Conference::removeParticipant(Participant*);

      /** Add a subscription (watcher) to this conference's list of
          subscriptions.  */
      void addSubscription(ConferenceSubscriptionApp*);

      /** Delete a subscription (watcher) from this conference's list of
          subscriptions. */
      void Conference::removeSubscription(ConferenceSubscriptionApp*);

      /// Generate notices for all subscribers (watchers) to the conference.
      void notifyAll();

      /// Make the Contents which is the conference event body for the current state.
      resip::Contents& makeNotice();

      /// Return true if there is only one participant
      bool shouldPlayMusic() const;
      
   private:
      resip::Data mAor;
      ConferenceUserAgent& mUa;
      CpMediaInterface* mMedia;
      OsConfigDb &mConfigDb;

      /// The list of the Participants in this Conference.
      std::set<Participant*> mParticipants;

      /// The list of the subscriptions to this Conference.
      std::set<ConferenceSubscriptionApp*> mSubscriptions;

      /// A Mime object for application/conference-info+xml.
      resip::Mime mMime;

      friend class Participant;
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
