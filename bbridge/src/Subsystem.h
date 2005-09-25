#if !defined(SIPX_SUBSYSTEM_HXX)
#define SIPX_SUBSYSTEM_HXX 

#include <iostream>
#include "rutil/Subsystem.hxx"

namespace bbridge
{

class Subsystem : public resip::Subsystem
{
   public:
      // Add new systems below
      static const Subsystem BBRIDGE;
};


}

#endif
