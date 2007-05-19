//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MyPlayerListenerPoller_h_
#define _MyPlayerListenerPoller_h_

#include "test/mp/MpTestConfig.h"
#include "mp/MpPlayerListener.h"
#include "os/OsDefs.h"
#include "mp/MpPlayerEvent.h"
#include "mp/MpMisc.h"

class MyPlayerListenerPoller : public MpPlayerListener
{

 protected:
    int       miTimeoutSec ;
    UtlBoolean mStates[MAX_STATES] ;
    OsBSem    mSemStateChange;
    OsBSem    mSemGuard ;

 public:
   MyPlayerListenerPoller(int iTimeoutSecs = 15);
   virtual ~MyPlayerListenerPoller(void);

   virtual void clearState();
   virtual UtlBoolean waitForState(PlayerState state);
   virtual void playerRealized(MpPlayerEvent& event);
   virtual void playerPrefetched(MpPlayerEvent& event);
   virtual void playerPlaying(MpPlayerEvent& event);
   virtual void playerPaused(MpPlayerEvent& event);
   virtual void playerStopped(MpPlayerEvent& event);
   virtual void playerFailed(MpPlayerEvent& event);
};

#endif // MyPlayerListenerPoller_h_
