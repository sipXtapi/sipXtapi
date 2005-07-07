// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _MyStreamQueueHistoryKeeper_h_
#define _MyStreamQueueHistoryKeeper_h_

#include "utl/UtlString.h"
#include "mp/MpQueuePlayerListener.h"

class MyStreamQueueHistoryKeeper : public MpQueuePlayerListener
{
 protected:
   UtlString mHistory ;
   UtlString mExpectedHistory;

 public:
   virtual ~MyStreamQueueHistoryKeeper(void);

   virtual void queuePlayerStarted();
   virtual void queuePlayerStopped();
   virtual void queuePlayerAdvanced();
   const char* getHistory();
};

#endif // MyStreamQueueHistoryKeeper_h_
