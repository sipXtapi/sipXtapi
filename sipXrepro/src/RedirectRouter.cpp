#if defined(HAVE_CONFIG_H)
#include "resip/stack/config.hxx"
#endif

#include "repro/Proxy.hxx"
#include "repro/RequestContext.hxx"
#include "RedirectRouter.h"
#include "resip/stack/Helper.hxx"
#include "resip/stack/NameAddr.hxx"
#include "resip/stack/SipMessage.hxx"
#include "rutil/Inserter.hxx"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM resip::Subsystem::REPRO

using namespace resip;
using namespace repro;
using namespace std;


RedirectRouter::RedirectRouter(const resip::NameAddrs& routes) :
   mRouteSet(routes)
{}

RedirectRouter::~RedirectRouter()
{}


Processor::processor_action_t
RedirectRouter::process(RequestContext& context)
{
   DebugLog(<< "Monkey handling request: " << *this 
            << "; reqcontext = " << context);
   
   resip::SipMessage& request = context.getOriginalRequest();

   // if there are routes set, then just let it go
   if (   !request.exists(h_Routes)
       || request.header(h_Routes).empty()
       )
   {
      // no routes, so send it to the redirect server for routing
      const Uri& uri = request.header(h_RequestLine).uri();
      if (context.getProxy().isMyUri(uri))
      {
         /* :TODO: the current argument parser wants route to be a set;
          *        we only need one value, so pull off the first one */
         resip::NameAddrs redirectRoutes = mRouteSet;
         resip::NameAddr  redirectServiceRoute  = mRouteSet.front();

         redirectServiceRoute.uri().param(p_lr) = true;
         request.header(h_Routes).push_front(redirectServiceRoute);

         InfoLog (<< "RedirectService route is: " << Inserter(request.header(h_Routes)));
      }
      else
      {
         InfoLog (<< "Non-local target : " << Inserter(uri) << " proxied");
      }

      context.addTarget(NameAddr(uri));
   }

   return Processor::Continue;
}

void
RedirectRouter::dump(std::ostream &os) const
{
   os << "RedirectRouter Monkey" << std::endl;
}
