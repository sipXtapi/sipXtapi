//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

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
