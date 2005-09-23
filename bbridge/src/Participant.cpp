#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "resip/dum/ServerInviteSession.hxx"
#include "resip/dum/Handles.hxx"
#include "resip/stack/Aor.hxx"
#include "resip/stack/SdpContents.hxx"
#include "rutil/DnsUtil.hxx"
#include "net/SdpCodec.h"

#include "Conference.h"
#include "ConferenceUserAgent.h"
#include "Participant.h"

using namespace bbridge;
using namespace std;

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

Participant::Participant(resip::DialogUsageManager& dum,
                         const resip::SipMessage& msg) :
  AppDialogSet(dum),
  mConnId(-1)
{
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
Participant::assign(Conference* conf)
{
   mConference = conf;
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
   resip::Data peerHost;
   int peerRtpPort = 0;
   int peerRtcpPort = 0;
   int dummyPort = 0;
   
   int numCodecs = 0;
   ::SdpCodec **sendCodecs = NULL;
   ::SdpSrtpParameters srtpParams;

   resip::Data offerData = resip::Data::from(offer);
   
   // !ah! - offerData ready to convert to sipX SDP (yes we know this is gross)
   
   ::SdpBody offerSipX(offerData.c_str(), offerData.size());
   ::SdpBody answerSipX;
    
    int numCodecsInCommon = 0;
    SdpCodec **codecsInCommon = 0;
    UtlString remoteAddress;
    UtlString localAddress;
    SdpCodecFactory supportedCodecs;
    int localRtpPort;
    int localRtcpPort;
    
    mConference->mMedia->getCapabilities(mConnId, localAddress, 
                                         localRtpPort, localRtcpPort,
                                         dummyPort, dummyPort,
                                         supportedCodecs, srtpParams);
    
    offerSipX.getBestAudioCodecs(supportedCodecs,
                                 numCodecsInCommon,
                                 codecsInCommon,
                                 remoteAddress,
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
   resip::Data sdata(sdpData);
   resip::HeaderFieldValue hfv(sdata.data(), sdata.size());
   answer = resip::SdpContents(&hfv, resip::Mime("application","sdp"));
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
