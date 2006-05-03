//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include "test/mp/MyStreamQueueHistoryKeeper.h"

MyStreamQueueHistoryKeeper::~MyStreamQueueHistoryKeeper()
{
}

void MyStreamQueueHistoryKeeper::queuePlayerStarted()
{
    mHistory.append("queuePlayerStarted ") ;
}


void MyStreamQueueHistoryKeeper::queuePlayerStopped()
{
    mHistory.append("queuePlayerStopped ") ;
}


void MyStreamQueueHistoryKeeper::queuePlayerAdvanced()
{
    mHistory.append("queuePlayerAdvanced ") ;
}

const char* MyStreamQueueHistoryKeeper::getHistory()
{
    return mHistory.data() ;
}
