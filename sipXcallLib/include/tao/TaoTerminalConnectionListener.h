//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _TaoTerminalConnectionListener_h_
#define _TaoTerminalConnectionListener_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "tao/TaoDefs.h"
#include "os/OsConnectionSocket.h"
#include "ptapi/PtTerminalConnectionListener.h"
#include "ptapi/PtCallListener.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TaoTransportTask;

class TaoTerminalConnectionListener : public PtTerminalConnectionListener
{
public:
    TaoTerminalConnectionListener(PtEventMask* pMask = NULL);

        TaoTerminalConnectionListener(int objId,
                                                                 TaoObjHandle clientSocket,
                                                                 TaoTransportTask* pSvrTransport,
                                                                 const char * terminalName);

        TaoTerminalConnectionListener(const TaoTerminalConnectionListener& rTaoTerminalConnectionListener);
     //:Copy constructor (not implemented for this class)

    ~TaoTerminalConnectionListener();

    virtual void terminalConnectionCreated(const PtTerminalConnectionEvent& rEvent);
    virtual void terminalConnectionIdle(const PtTerminalConnectionEvent& rEvent);
    virtual void terminalConnectionRinging(const PtTerminalConnectionEvent& rEvent);
    virtual void terminalConnectionDropped(const PtTerminalConnectionEvent& rEvent);
    virtual void terminalConnectionUnknown(const PtTerminalConnectionEvent& rEvent);
    virtual void terminalConnectionHeld(const PtTerminalConnectionEvent& rEvent);
    virtual void terminalConnectionTalking(const PtTerminalConnectionEvent& rEvent);
    virtual void terminalConnectionInUse(const PtTerminalConnectionEvent& rEvent);

   virtual PtStatus getLocation(UtlString*& pLocation);

private:
        int                                             mObjId;
        char*                                   mTerminalName;

        TaoObjHandle                    mhClientSocket;
        TaoTransportTask*               mpSvrTransport;
        OsConnectionSocket*             mpConnectionSocket;


};

#endif // _TaoTerminalConnectionListener_h_
