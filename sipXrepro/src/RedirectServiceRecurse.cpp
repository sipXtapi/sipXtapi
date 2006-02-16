#if defined(HAVE_CONFIG_H)
#include "resip/stack/config.hxx"
#endif

#include "resip/stack/SipMessage.hxx"
#include "resip/stack/NameAddr.hxx"
#include "repro/RequestContext.hxx"
#include "rutil/Logger.hxx"

#include "RedirectServiceRecurse.h"

#define RESIPROCATE_SUBSYSTEM resip::Subsystem::REPRO

using namespace resip;
using namespace repro;
using namespace std;

RedirectServiceRecurse::RedirectServiceRecurse(const resip::NameAddrs& redirectServiceRoutes) :
   mRedirectServiceRoute(redirectServiceRoutes.front())
{}


RedirectServiceRecurse::~RedirectServiceRecurse()
{}


Processor::processor_action_t
RedirectServiceRecurse::process(RequestContext& context)
{
   DebugLog(<< "Lemur handling request: " << *this 
            << "; reqcontext = " << context);
   Processor::processor_action_t doNext = Processor::Continue;

   const SipMessage* response = dynamic_cast<const SipMessage*>(context.getCurrentEvent());
   if (response && response->isResponse() )
   {
      int status = response->header(h_StatusLine).statusCode();
      if (status / 100 == 3) // is this a redirect (3xx)?
      {
         resip::SipMessage& request = context.getOriginalRequest();

         if ( request.exists(h_Routes) && ! request.header(h_Routes).empty() )
         {
//:TODO: figure out how to check that this is just the server route
//            resip::NameAddr topRoute = request.header(h_Routes).front();
//            if ( topRoute == mRedirectServiceRoute )
//            {
               request.header(h_Routes).pop_front(); // remove the route to the redirect server
//            }
         }

#if 0
         if (305 == status) // special case
         {
            NameAddr useProxy = response->header(h_Contacts).front();
            request.header(h_Routes).push_front(useProxy); 

            InfoLog(<< "Use Proxy : " << useProxy );
         }
         else 
#endif
         {
            for (NameAddrs::const_iterator i=response->header(h_Contacts).begin(); 
                 i != response->header(h_Contacts).end(); ++i)
            {
               context.addTarget(*i);
               InfoLog(<< "Redirect to : " << *i );
            }
         }
         doNext = Processor::SkipAllChains;
      }
   }

   return doNext;
}

void
RedirectServiceRecurse::dump(std::ostream &os) const
{
   os << "Redirect Service Recurse Processor" << std::endl;
}
