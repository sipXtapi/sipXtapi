#if !defined(BBRIDGE_DTMF_H)
#define BBRIDGE_DTMF_H

#include "resip/dum/DumCommand.hxx"

namespace bbridge
{

class Participant;
class Message;

class DtmfEvent : public resip::DumCommand
{
   public:
      DtmfEvent(Participant& ua, char event, int duration, bool up);
      virtual void executeCommand();

      Message* clone() const;
      std::ostream& encode(std::ostream& strm) const;
      std::ostream& encodeBrief(std::ostream& strm) const;

   private:
      Participant& mParticipant;
      char mDtmfTone;
      int mDuration;
      bool mUp;
};


}

#endif
