#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "resip/dum/ServerInviteSession.hxx"
#include "resip/dum/Handles.hxx"
#include "resip/stack/Aor.hxx"
#include "resip/stack/SdpContents.hxx"
#include "resip/stack/PlainContents.hxx"
#include "rutil/DnsUtil.hxx"
#include "net/SdpCodec.h"

#include "ConferenceUserAgent.h"
#include "Conference.h"
#include "Subsystem.h"

#define RESIPROCATE_SUBSYSTEM bbridge::Subsystem::BBRIDGE

using namespace bbridge;
using namespace std;

Conference::Conference(bbridge::ConferenceUserAgent& ua,
                       const resip::Data& aor,
                       OsConfigDb &configDb,
                       const resip::Data& ipAddress) : 
   mAor(aor),
   mUa(ua),
   mMedia(ua.mMediaFactory->createMediaInterface(ipAddress.c_str(),
                                                 ipAddress.c_str(),
                                                 ua.mNumCodecs, 
                                                 ua.mSdpCodecArray, 
                                                 "",  // locale
                                                 QOS_LAYER3_LOW_DELAY_IP_TOS,
                                                 "", // stun server
                                                 0, // stun options
                                                 25)), // stun keep alive
   mConfigDb(configDb),
   mMime("application", "conference-info+xml")
{
}

Conference::~Conference()
{
   // Terminate all the subscriptions to this conference.
   for(std::set<ConferenceSubscriptionApp*>::const_iterator i = mSubscriptions.begin();
       i != mSubscriptions.end(); ++i)
   {
      (*i)->terminate();
   }

   // !jf! Do I need to do anything other than delete mMedia to free up resources? 
   mMedia->release();
   mUa.mConferences.erase(mAor);
}

// Get the AOR to reach this conference.
const resip::Data& Conference::getAor() const
{
   return mAor;
}

// Add a Participant to this conference's list of Participants.
void Conference::addParticipant(Participant* part)
{
   mParticipants.insert(part);
}

// Delete a Participant from this conference's list of Participants.
void Conference::removeParticipant(Participant* part)
{
   mParticipants.erase(part);
   if (mParticipants.empty())
   {
      InfoLog (<< "Last participant has left the conference");
      delete this;
   }
}

// Add a subscripton to this conference's list of subscriptions.
void Conference::addSubscription(ConferenceSubscriptionApp* sub)
{
   mSubscriptions.insert(sub);
}

// Delete a subscription from this conference's list of subscriptions.
void Conference::removeSubscription(ConferenceSubscriptionApp* sub)
{
   mSubscriptions.erase(sub);
}

// Support routines for generating NOTIFYs.

// Generate notices for all subscribers to the conference.
void Conference::notifyAll()
{
   // Construct the XML.
   const resip::Contents& notice = makeNotice();

   // Send it to all the subscribers for the conference.
   for(std::set<ConferenceSubscriptionApp*>::const_iterator i = mSubscriptions.begin();
       i != mSubscriptions.end(); ++i)
   {
      // Get the subscription dialog handle for the subscription.
      resip::ServerSubscriptionHandle handle = (*i)->getSubscriptionHandle();
      // Create the NOTIFY message.
      resip::SharedPtr<resip::SipMessage> message = handle->update(&notice);
      // Send it.
      handle->send(message);

      InfoLog(<< "Sent NOTIFY to subscriber '" << handle->getSubscriber()
              << "', for subscription ID '" << handle.getId()
              << "', dialog ID '" << handle->getDialogId()
              << "'");
   }
}

bool
Conference::shouldPlayMusic() const
{
   return mParticipants.size() == 1;
}

// Make the Contents which is the conference event body.
resip::Contents& Conference::makeNotice()
{
   resip::Data d;
   d +=
      "<conference-info>\n"
      "<conference-description>\n"
      "<conf-uris>\n"
      "<entry>\n"
      "<uri>";
   d += getAor();
   d += 
      "</uri>\n"
      "<purpose>participation</purpose>\n"
      "</entry>\n"
      "</conf-uris>\n"
      "</conference-description>\n"
      "<conference-state>\n"
      "<user-count>0</user-count>\n"
      "<active>true</active>\n"
      "<locked>false</locked>"
      "</conference-state\n"
      "</conference-info>\n";

/*
 *   |-- users
 *   |    |-- user
 *   |    |    |-- endpoint
 *   |    |    |    |-- media
 *   |    |    |    |-- media
 *   |    |    |    |-- call-info
 *   |    |    |
 *   |    |    |-- endpoint
 *   |    |         |-- media
 *   |    |-- user
 *   |         |-- endpoint
 *   |              |-- media
 */

   resip::Contents* c = new resip::PlainContents(d, mMime);
   return *c;
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
