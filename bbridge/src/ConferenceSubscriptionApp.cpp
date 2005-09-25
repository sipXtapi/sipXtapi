/*
 * Handler for subscriptions to conference events.
 */

#include "ConferenceSubscriptionApp.h"
#include "resip/dum/ServerSubscription.hxx"
#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"

#include "Subsystem.h"

#define RESIPROCATE_SUBSYSTEM bbridge::Subsystem::BBRIDGE

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

void ConferenceSubscriptionApp::detach()
{
   if (mConference)
   {
      // The Conference knows all its Participants.
      mConference->removeSubscription(this);
      // The Participant points to the Conference.
      mConference = NULL;
   }
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

// Terminate this subscription.
void ConferenceSubscriptionApp::terminate()
{
   if (mConference)
   {
      // Construct the XML for the final NOTIFY.
      const resip::Contents& notice = mConference->makeNotice();
      // Send the final NOTIFY.
      // Give as termination reason "noresource", which means that the
      // subscribed-to resource ceased to exist.
      mSubscriptionHandle->end(resip::NoResource, &notice);
      // Detach the subscription now from the conference.
      // (It is safe to call detach() multiple times.)
      detach();
   }
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

   InfoLog(<< "Sent NOTIFY to subscriber '" << mSubscriptionHandle->getSubscriber()
           << "', for subscription ID '" << mSubscriptionHandle.getId()
           << "', dialog ID '" << mSubscriptionHandle->getDialogId()
           << "'");
}

}
