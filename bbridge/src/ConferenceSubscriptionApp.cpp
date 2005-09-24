/*
 * Handler for subscriptions to conference events.
 */

#include "ConferenceSubscriptionApp.h"
#include "resip/dum/ServerSubscription.hxx"
#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"

namespace bbridge
{

ConferenceSubscriptionApp::ConferenceSubscriptionApp(resip::DialogUsageManager& dum,
                                                     const resip::SipMessage &msg) :
  resip::AppDialogSet(dum)
{
}

ConferenceSubscriptionApp::~ConferenceSubscriptionApp()
{
   if (mConference)
   {
      // The Conference knows all its subscriptions.
      mConference->removeSubscription(this);
      mConference = NULL;
   }
}

// Get the owning conference.
Conference* ConferenceSubscriptionApp::getConference()
{
   return mConference;
}

void ConferenceSubscriptionApp::attach(Conference* conf)
{
   // The Participant points to the Conference.
   mConference = conf;
   // The Conference knows all its Participants.
   conf->addSubscription(this);
}

// Record the ServerSubscriptionHandle.
void ConferenceSubscriptionApp::setSubscriptionHandle(resip::ServerSubscriptionHandle h)
{
   mSubscriptionHandle = h;
}

// Get the ServerSubscriptionHandle.
resip::ServerSubscriptionHandle ConferenceSubscriptionApp::getSubscriptionHandle()
{
   return mSubscriptionHandle;
}

// Functions to generate NOTIFYs.

// Generate notice for one subscriber.
void ConferenceSubscriptionApp::notifyOne()
{
   assert(mConference);

   // Construct the XML.
   const resip::Contents& notice = mConference->makeNotice();

   // Send it to the subscriber.
   resip::SipMessage& message = mSubscriptionHandle->update(&notice);
   mSubscriptionHandle->send(message);
}

}
