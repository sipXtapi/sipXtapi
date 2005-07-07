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

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsgQ.h"
#include "sipdb/SIPDBManager.h"
#include "IMDBTaskMonitor.h"
#include "KeepAliveTask.h"

KeepAliveTask::KeepAliveTask ( 
    const UtlString& rArgument, OsMsgQ& rMsgQ, OsEvent& rCommandEvent) : 
    IMDBWorkerTask( rArgument, rMsgQ, rCommandEvent )
{}

KeepAliveTask::~KeepAliveTask()
{}

int 
KeepAliveTask::run( void* runArg )
{
    osPrintf ("Starting Thread\n");
    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "Starting Thread\n");

    if ( openIMDBTables() == OS_SUCCESS ) 
    {
        // this task is never busy
        setBusy (FALSE);
        while ( !isShuttingDown() )
        {
            // we're actually doing nothing here except signalling the TaskMonitor
            OsSysLog::add( LOG_FACILITY, PRI_DEBUG, "Keeping IMDB Tables Alive\n");

            // send a success message to the sleeping monitor
            notifyMonitor( USER_HEARTBEAT_SUCCESS_EVENT );

            // wait for the next event to be signaled by the notifier
            mCommandEvent.wait();
            mCommandEvent.reset();
        }
    }

    cleanupIMDBResources();

    SIPDBManager::getInstance()->releaseAllDatabase();
    delete SIPDBManager::getInstance();

    osPrintf ("Stopping Thread");
    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "Stopping Thread\n");
    return( TRUE );
}

OsStatus 
KeepAliveTask::openIMDBTables() const
{
    // preload the following tables to ensure that
    // their reference count does not to to 0 
    // causing an expensive load by another process
    SIPDBManager::getInstance()->preloadAllDatabase();

    return OS_SUCCESS;
}
