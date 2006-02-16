#if !defined(_REDIRECT_SERVICE_RECURSE_H_)
#define _REDIRECT_SERVICE_RECURSE_H_

#include "repro/Processor.hxx"

namespace repro
{

class RedirectServiceRecurse: public Processor
{
   public:
      RedirectServiceRecurse(const resip::NameAddrs& redirectServiceRoute);
      virtual ~RedirectServiceRecurse();
      
      virtual processor_action_t process(RequestContext &);
      virtual void dump(std::ostream &os) const;

  protected:
      resip::NameAddr mRedirectServiceRoute;
};

}
#endif
