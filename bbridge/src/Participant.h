#ifndef __PARTICIPANT_H
#define __PARTICIPANT_H

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

namespace bbridge
{

class Conference;
class ConferenceUserAgent;

/**
   A Participant object is created for each inbound INVITE request. The
   Participant derives from AppDialogSet and is created before the Participant
   gets associated with a Conference by the ParticipantFactory (which is created
   by the ConferenceUserAgent and managed by DUM). The Participant is also
   responsible for taking DTMF events from sipXmedia and passing them to DUM as
   DumCommand objects - which will result in the onDtmfEvent method call on the
   Participant. 
 */
class Participant : public resip::AppDialogSet
{
   public:
      Participant(resip::DialogUsageManager& dum, 
                  const resip::SipMessage &msg);
      /**
         On destruction, the Participant may decide to tell the conference to
         play music on hold if the conference indicates to do so. Exit sounds
         are also handled here. Media connection resources are also released.
       */
      ~Participant();

      /**
         Assigns this Participant to a particular conference. Plays hold music
         if necessary, plays entry sounds. Starts playing audio. 
       */
      void assign(Conference* conf);

      /**
         Takes an SDP offer from the incoming INVITE and produces an answer
         based on the CpMediaInterface's current capabilities. Sets up media
         flow graph to start sending and receiving audio and returns the answer
         so that it can be sent back in the 200 OK. 
       */
      void accept(const resip::SdpContents& offer, resip::SdpContents& answer);

      /// Returns the flow graph's associated connection ID. 
      int id() const;
      
      /**
         Called whenever this Participant provides DTMF. The event is a
         character '0' - '9', '*', '#' plus duration and whether the key was
         pressed or released. 
       */
      void onDtmfEvent(char event, int duration, bool up);

      /**
         Callback from within the CpMediaInterface indicating the dtmf
         data. This gets packaged up as a DtmfEvent (DumCommand) and posted to
         the DialogUsageManager asynchronously. This provides thread-safe
         communication between the CpMediaInterface thread and the
         ConferenceUserAgent thread. 
      */
      static void dtmfCallback(const int thisp, const int eventData);

   private:
      Conference* mConference;
      int mConnId;

      static const char sEnterBuffer[8000];
      static const char sExitBuffer[8000];
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
