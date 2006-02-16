#if !defined(_REDIRECTROUTER_H_)
#define _REDIRECTROUTER_H_ 

#include "repro/Processor.hxx"

namespace repro
{
class NameAddrs;
class RouteStore;

class RedirectRouter: public Processor
{
   public:
      RedirectRouter(const resip::NameAddrs& routeSet);
      virtual ~RedirectRouter();
      
      virtual processor_action_t process(RequestContext &);
      virtual void dump(std::ostream &os) const;
      
   private:
      resip::NameAddrs mRouteSet;
};

}
#endif
