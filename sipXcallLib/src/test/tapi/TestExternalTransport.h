//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"

bool transportProc1(SIPX_TRANSPORT hTransport,
                 const char* szDestinationIp,
                 const int   iDestPort,
                 const char* szLocalIp,
                 const int   iLocalPort,
                 const void* pData,
                 const size_t nData,
                 const void* pUserData);
                 
bool transportProc2(SIPX_TRANSPORT hTransport,
                 const char* szDestinationIp,
                 const int   iDestPort,
                 const char* szLocalIp,
                 const int   iLocalPort,
                 const void* pData,
                 const size_t nData,
                 const void* pUserData);
                 
                 
class TransportTask : public OsTask
{
public:
   TransportTask(int id,
             const UtlString& name="",
             void* pArg=NULL,
             const int priority=DEF_PRIO,
             const int options=DEF_OPTIONS,
             const int stackSize=DEF_STACKSIZE);
    virtual ~TransportTask();
protected:
    virtual int run(void* pArg);
private:
    int mId;
    
                
};                 
