#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "resip/dum/ServerInviteSession.hxx"
#include "resip/dum/Handles.hxx"
#include "resip/stack/Aor.hxx"
#include "resip/stack/SdpContents.hxx"
#include "rutil/DnsUtil.hxx"
#include "net/SdpCodec.h"

#include "os/OsCallback.h"

#include "Conference.h"
#include "ConferenceUserAgent.h"
#include "DtmfEvent.h"
#include "Participant.h"
#include "Subsystem.h"

using namespace bbridge;
using namespace std;

#define RESIPROCATE_SUBSYSTEM bbridge::Subsystem::BBRIDGE

Participant::Participant(resip::DialogUsageManager& dum, 
                         const resip::SipMessage& msg) :
   AppDialogSet(dum),
   mConference(0),
   mConnId(-1)
{
}

Participant::~Participant()
{
   if (mConference)
   {
      assert(mConference->mMedia);

      if (mConference->shouldPlayMusic())
      {
         // Start the hold music
         UtlString holdMusic;
         mConference->mConfigDb.get("BOSTON_BRIDGE_HOLD_MUSIC",holdMusic);
         OsStatus status = mConference->mMedia->playAudio(holdMusic,
                                                  true,  // repeat
                                                  false, // no local party
                                                  true); // remote parties

         // If the hold music failed, play the built-in default exit tone
         if (status != OS_SUCCESS)
         {
           mConference->mMedia->playBuffer (const_cast<char *>(sExitBuffer),
                                            sizeof(sExitBuffer),
                                            0,
                                            false, // no repeat
                                            false, // no local party
                                            true); // remote parties
         }
      }
      else
      {
         // Play the exit sound
         UtlString exitSound;
         mConference->mConfigDb.get("BOSTON_BRIDGE_EXIT_SOUND",exitSound);
         OsStatus status = mConference->mMedia->playAudio(exitSound,
                                                  false, // no repeat
                                                  false, // no local party
                                                  true); // remote parties
         if (status != OS_SUCCESS)
         {
           mConference->mMedia->playBuffer (const_cast<char *>(sExitBuffer),
                                            sizeof(sExitBuffer),
                                            0,
                                            false, // no repeat
                                            false, // no local party
                                            true); // remote parties
         }
      }

      mConference->mMedia->stopRtpReceive(mConnId);
      mConference->mMedia->stopRtpSend(mConnId);
      mConference->mMedia->deleteConnection(mConnId);

      // The Conference knows all its Participants.
      mConference->removeParticipant(this);

      mConference = 0;
   }
}

void
Participant::assign(Conference* conf)
{
   // The Participant points to the Conference.
   mConference = conf;
   // The Conference knows all its Participants.
   conf->addParticipant(this);

   if (mConference->shouldPlayMusic())
   {
      // Start the hold music
      UtlString holdMusic;
      mConference->mConfigDb.get("BOSTON_BRIDGE_HOLD_MUSIC",holdMusic);
      OsStatus status = mConference->mMedia->playAudio(holdMusic,
                                                       true,  // repeat
                                                       false, // no local party
                                                       true); // remote parties
      (void)status;
   }
   else
   {
      // Play the enter sound
      UtlString enterSound;
      mConference->mConfigDb.get("BOSTON_BRIDGE_ENTER_SOUND",enterSound);
      OsStatus status = mConference->mMedia->playAudio(enterSound,
                                                       false, // no repeat
                                                       false, // no local party
                                                       true); // remote parties
      if (status != OS_SUCCESS)
      {
        mConference->mMedia->playBuffer (const_cast<char *>(sEnterBuffer),
                                         sizeof(sEnterBuffer),
                                         0,
                                         false, // no repeat
                                         false, // no local party
                                         true); // remote parties
     }
   }
}

int 
Participant::id() const
{
   return mConnId;
}

void
Participant::accept(const resip::SdpContents& offer,
                    resip::SdpContents& answer)
{
   assert(mConference);
   assert(mConference->mMedia); 

   mConference->mMedia->createConnection(mConnId, 0);
   UtlString peerHost;
   int peerRtpPort = 0;
   int peerRtcpPort = 0;
   int dummyPort = 0;
   
   //int numCodecs = 0;
   ::SdpSrtpParameters srtpParams;

   resip::Data offerData = resip::Data::from(offer);
   
   // !ah! - offerData ready to convert to sipX SDP (yes we know this is gross)
   
   ::SdpBody offerSipX(offerData.c_str(), offerData.size());
   ::SdpBody answerSipX;

   InfoLog (<< "Offer: " << offer);
   
    int numCodecsInCommon = 0;
    SdpCodec **codecsInCommon = 0;
    UtlString localAddress;
    SdpCodecFactory supportedCodecs;
    int localRtpPort;
    int localRtcpPort;
    
    mConference->mMedia->getCapabilities(mConnId, localAddress, 
                                         localRtpPort, localRtcpPort,
                                         dummyPort, dummyPort,
                                         supportedCodecs, srtpParams);
    InfoLog (<< "mConnId=" << mConnId
             << " localAddress=" << localAddress
             << " localRtpPort=" << localRtpPort
             << " localRtcpPort=" << localRtcpPort);
    
    offerSipX.getBestAudioCodecs(supportedCodecs,
                                 numCodecsInCommon,
                                 codecsInCommon,
                                 peerHost,
                                 peerRtpPort, 
                                 peerRtcpPort,
                                 dummyPort,
                                 dummyPort); // !ah! dummy used for video

    answerSipX.setStandardHeaderFields("conference",0,0,localAddress);
    answerSipX.addAudioCodecs(localAddress,
                              localRtpPort,
                              localRtcpPort,
                              dummyPort, dummyPort, // !ah! --video
                              numCodecsInCommon,
                              codecsInCommon, 
                              srtpParams);

    // !ah! convert to resiprocate SDP

   int sdpSize = answerSipX.getLength();
   const char * sdpData = new  char[sdpSize];
   answerSipX.getBytes(&sdpData,&sdpSize);

   //InfoLog( <<"Answer so far: " << sdpData);
   resip::HeaderFieldValue *hfv = new resip::HeaderFieldValue(sdpData, sdpSize);

   answer = resip::SdpContents(hfv, resip::Mime("application","sdp"));

   
   InfoLog (<< mConnId << " peer=" << peerHost
            << ":" << peerRtpPort
            << " conf=" << mConference);
   assert(numCodecsInCommon > 0);
   mConference->mMedia->setConnectionDestination(mConnId, peerHost,
                                                 peerRtpPort, peerRtcpPort,
                                                 0, 0);

   mConference->mMedia->startRtpReceive(mConnId, numCodecsInCommon, 
                                        codecsInCommon,
                                        srtpParams);
   mConference->mMedia->startRtpSend(mConnId,numCodecsInCommon,
                                     codecsInCommon,
                                     srtpParams);
   // !jf! delete codecsInCommon


   // add some code to install DtmfEvent handler
   // need to pass in enough information to be able to create a DtmfEvent and
   // have it get posted to mDum.
   OsCallback* cb=new OsCallback((const int)(this), Participant::dtmfCallback);
   mConference->mMedia->addToneListener(cb , mConnId);
}

void
Participant::dtmfCallback(const int thisi, const int code)
{
   Participant* thisp = reinterpret_cast<Participant*>(thisi);
   int tone = ((code >> 16) & 0x3fff);
   int up = code & 0x80000000;
   int duration = code & 0xFFFF;
   char tonec = (char)tone + '0';
   DtmfEvent* devent = new DtmfEvent(*thisp, tonec, duration, up);
   thisp->mDum.post(devent);
}


void 
Participant::onDtmfEvent(char event, int duration, bool up)
{
   InfoLog (<< " DTMF event handled for " << event << " duration=" << duration << " up=" << up);
   // dan - insert stuff here
}

/*
  Copyright (c) 2005, Jason Fischl, Adam Roach, Alan Hawrylyshen
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
