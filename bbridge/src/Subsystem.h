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

   private:
      explicit Subsystem(const char* rhs) : resip::Subsystem(rhs) {};
      explicit Subsystem(const resip::Data& rhs);
      Subsystem& operator=(const resip::Data& rhs);
};


}

#endif
