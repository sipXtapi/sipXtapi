//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _MyPlayerListenerHistoryKeeper_h_
#define _MyPlayerListenerHistoryKeeper_h_

#include "utl/UtlString.h"
#include "mp/MpPlayerListener.h"
#include "mp/MpPlayerEvent.h"


class MyPlayerListenerHistoryKeeper : public MpPlayerListener
{
 protected:
   UtlString mHistory ;
   UtlString mExpectedHistory;

 public:
   virtual ~MyPlayerListenerHistoryKeeper(void);

   virtual void playerRealized(MpPlayerEvent& event);
   virtual void playerPrefetched(MpPlayerEvent& event);
   virtual void playerPlaying(MpPlayerEvent& event);
   virtual void playerPaused(MpPlayerEvent& event);
   virtual void playerStopped(MpPlayerEvent& event);
   virtual void playerFailed(MpPlayerEvent& event);
   const char* getHistory();
   UtlBoolean matchesHistory(void* userData, int* pPlayerStates);
};

#endif // MyPlayerListenerHistoryKeeper_h_
