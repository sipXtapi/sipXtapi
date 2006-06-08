//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include "TestExternalTransport.h"
#include "os/OsMutex.h"
#include "os/OsTime.h"
#include "sipXtapiTest.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"

static OsMutex transportMutex(OsMutex::Q_FIFO);
static void* gpData1;
static size_t gnData1;
static char gpUserData1[256];
static void* gpData2;
static size_t gnData2;
static char gpUserData2[256];

SIPX_TRANSPORT ghTransport1;
SIPX_TRANSPORT ghTransport2;


bool transportProc1(SIPX_TRANSPORT hTransport,
                 const char* szDestinationIp,
                 const int   iDestPort,
                 const char* szLocalIp,
                 const int   iLocalPort,
                 const void* pData,
                 const size_t nData,
                 const void* pUserData)
{
    
    transportMutex.acquire();
    
    ghTransport1 = hTransport;
    gpData1 = malloc(nData);
    gnData1 = nData;
    memset(gpData1, 0, nData);
    memcpy(gpData1, pData, nData);
    strcpy(gpUserData1, (char*)pUserData);
    
    transportMutex.release();
    
    return nData > 0;
} 

bool transportProc2(SIPX_TRANSPORT hTransport,
                 const char* szDestinationIp,
                 const int   iDestPort,
                 const char* szLocalIp,
                 const int   iLocalPort,
                 const void* pData,
                 const size_t nData,
                 const void* pUserData)
{
    transportMutex.acquire();
    
    ghTransport2 = hTransport;
    gpData2 = malloc(nData);
    gnData2 = nData;
    memset(gpData2, 0, nData);
    memcpy(gpData2, pData, nData);
    strcpy(gpUserData2, (char*)pUserData);
    
    transportMutex.release();
    
    return nData > 0;
} 


TransportTask::TransportTask(int id,
                         const UtlString& name,
                         void* pArg,
                         const int priority,
                         const int options,
                         const int stackSize) :
    OsTask(name, pArg, priority, options, stackSize),
    mId(id)
{
}

TransportTask::~TransportTask()
{
}

int TransportTask::run(void* pArg)
{
    OsTime timeout(100);
    int bytesRead = 0;
    void* buffer = NULL;
    
    while (mState != SHUTTING_DOWN && 
           mState != SHUT_DOWN )
    {
        OsStatus rc = transportMutex.acquire(timeout);
        
        
        if (OS_SUCCESS == rc)
        {
            SIPX_TRANSPORT hTransport = NULL;

            if (1 == mId)
            {
                hTransport = ghTransport1;
                bytesRead = gnData2;
                buffer = gpData2;
            }
            else if (2 == mId)
            {
                hTransport = ghTransport2;
                bytesRead = gnData1;
                buffer = gpData1;
            }
            if (hTransport && buffer)
            {
                sipxConfigExternalTransportHandleMessage(hTransport,
                                                        "127.0.0.1",
                                                        -1,
                                                        "127.0.0.1",
                                                        -1,
                                                        buffer,
                                                        bytesRead);
            }                                                    
            
            if (1 == mId)
            {
                gpData2 = NULL;
            }
            else if (2 == mId)
            {
                gpData1 = NULL;
            }
            transportMutex.release();                                                 
        }            
        
        
    }
    return 0;
}    




