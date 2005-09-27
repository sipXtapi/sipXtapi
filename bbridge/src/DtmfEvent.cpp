#include "rutil/Logger.hxx"

#include "DtmfEvent.h"
#include "ConferenceUserAgent.h"

using namespace bbridge;

#define RESIPROCATE_SUBSYSTEM resip::Subsystem::TEST

DtmfEvent::DtmfEvent(Participant& part, char event, int duration, bool up) : 
   mParticipant(part),
   mDtmfTone(event),
   mDuration(duration),
   mUp(up)
{
}

void 
DtmfEvent::execute()
{
   mParticipant.onDtmfEvent(mDtmfTone, mDuration, mUp);
}

resip::Message* 
DtmfEvent::clone() const
{
   assert(0);
   return 0;
}

std::ostream& 
DtmfEvent::encode(std::ostream& strm) const
{
   strm << " dtmf event: " << mDtmfTone << " duration=" << mDuration << " up=" << mUp;
   return strm;
}

std::ostream& 
DtmfEvent::encodeBrief(std::ostream& strm) const
{
   return encode(strm);
}



