//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef ExternalTransport_h

#include "os/OsDefs.h"
#include "os/OsTask.h"
#include "os/OsNatDatagramSocket.h"

SIPX_CONTACT_ID lookupContactId(const char* szIp, const char* transport, const SIPX_TRANSPORT hTransport);


class FlibbleTask : public OsTask
{
public:
   FlibbleTask(const char* szBoundIp = NULL,
             const UtlString& name="",
             void* pArg=NULL,
             const int priority=DEF_PRIO,
             const int options=DEF_OPTIONS,
             const int stackSize=DEF_STACKSIZE);
    virtual ~FlibbleTask();
protected:
    virtual int run(void* pArg);
private:
    OsSocket* mpServerSocket;  
    void initSocket(const char* szBoundIp);
    UtlString mBoundIp;
                
};

#endif // #ifndef ExternalTransport_h
