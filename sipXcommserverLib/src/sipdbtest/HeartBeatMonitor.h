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

#ifndef HEARTBEATMONITOR_H
#define HEARTBEATMONITOR_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "IMDBWorkerTask.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsEvent;

class HeartBeatMonitor : public IMDBWorkerTask
{
public:
    /**
     * Ctor
     * 
     * @param rCommand
     * @param rArgument
     * @param rMsgQ
     */
    HeartBeatMonitor ( 
        const UtlString& rArgument, 
        OsMsgQ& rMsgQ,
        OsEvent& rCommandEvent);

    /**
     * Dtor
     */
    virtual ~HeartBeatMonitor();
    
    /**
     * The worker method, this is where all the command types execute, they
     * run as separate threads
     * 
     * @param runArg
     * 
     * @return 
     */
    virtual int run( void* runArg );
};

#endif  // HEARTBEATMONITOR_H

