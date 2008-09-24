// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//
// Copyright (C) 2005-2008 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _CpMediaSocketMonitorTask_h_
#define _CpMediaSocketMonitorTask_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlSList.h"
#include "utl/UtlObservable.h"
#include "os/OsSocket.h"
#include "mediaInterface/IMediaInterface.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// ENUMS


// FORWARD DECLARATIONS

/**
 * This class monitors sockets for data traffic and will notify a listener 
 * if/when the silent becomes idle.  By default, the listener will not be
 * notified until "enabled".  
 */
class CpMediaSocketMonitorTask : public OsServerTask, public UtlObserver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Destructor
     */
    virtual ~CpMediaSocketMonitorTask();

    void setIdleTimeout(int idleTimeoutSeconds) {mIdleTimeoutSeconds=idleTimeoutSeconds;}
    
    bool monitor(OsSocket* pSocket, ISocketEvent* pSink, SocketPurpose purpose);
    bool unmonitor(OsSocket* pSocket);
    
    static CpMediaSocketMonitorTask* spInstance;
    static CpMediaSocketMonitorTask* createInstance(const int pollingFrequency = 5)
    {
        if (spInstance)
        {
            return spInstance;
        }
        else
        {
            spInstance = new CpMediaSocketMonitorTask(pollingFrequency);
            spInstance->start();
            return spInstance;
        }
    }

    static void releaseInstance()
    {
        if (spInstance)
        {
            delete spInstance;
            spInstance = NULL ;
        }

    }
    
    virtual void onNotify(UtlObservable*, int code, void *pUserData);
    
    static CpMediaSocketMonitorTask* getInstance()
    {
        return spInstance;
    }

    /**
     * For handling the timer message.
     */
    virtual UtlBoolean handleMessage(OsMsg& rMsg);    
/* ============================ MANIPULATORS ============================== */

    void enableSocket(OsSocket* pSocket) ;
    
    void disableSocket(OsSocket* pSocket) ;
    
    UtlBoolean isSocketEnabled(OsSocket* pSocket, OsDateTime& when) ;
   
/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /**
     * Default constructor
     */
    CpMediaSocketMonitorTask(const int pollingFrequency);
    
    /** Disabled copy constructor */
    CpMediaSocketMonitorTask(const CpMediaSocketMonitorTask& rCpMediaSocketMonitorTask);

    /** Disabled equals operator*/
    CpMediaSocketMonitorTask& operator=(const CpMediaSocketMonitorTask& rhs);
    
    /**
     * Map of ISocketEvent sinks, keyed by socket pointer.
     */
    UtlHashMap mSocketMap;
    UtlHashMap mSinkMap;
    UtlHashMap mSocketPurposeMap;
    UtlHashMap mSocketEnabledTime;
    
    int mIdleTimeoutSeconds;
    int mPollingFrequency;
    OsTimer* mpTimer;
    OsMutex mMapMutex;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpMediaSocketMonitorTask_h_

