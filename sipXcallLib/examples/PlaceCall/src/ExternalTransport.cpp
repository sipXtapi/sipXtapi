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
#include "ExternalTransport.h"
extern SIPX_TRANSPORT ghTransport;
extern SIPX_INST g_hInst;      // Handle to the sipXtapi instanance

FlibbleTask::FlibbleTask(const char* szBoundIp,
                         const UtlString& name,
                         void* pArg,
                         const int priority,
                         const int options,
                         const int stackSize) :
    OsTask(name, pArg, priority, options, stackSize),
    mpServerSocket(NULL),
    mBoundIp(szBoundIp)
{
}

FlibbleTask::~FlibbleTask()
{
}

int FlibbleTask::run(void* pArg)
{
    char buffer[2048];
    UtlString ipAddress;
    int nPort = -1;
    int bytesRead = 0;
    int remotePort;
    UtlString remoteIp;
    
    if (!mpServerSocket)
    {
        initSocket(mBoundIp.data());
    }
    while (mState != SHUTTING_DOWN && 
           mState != SHUT_DOWN )
    {
        bytesRead = mpServerSocket->read(buffer, 2048, &remoteIp, &remotePort);
        sipxConfigExternalTransportHandleMessage(ghTransport,
                                                 remoteIp.data(),
                                                 -1,
                                                 mBoundIp.data(),
                                                 -1,
                                                 buffer,
                                                 bytesRead);
        
    }
    return 0;
}    

void FlibbleTask::initSocket(const char* szBoundIp)
{
    mpServerSocket = 
      new OsNatDatagramSocket(0,
                              NULL,
                              42424, // flibble listen port = 42424;
                              mBoundIp.data(),
                              FALSE);                                          
}

bool tribbleProc(SIPX_TRANSPORT hTransport,
                 const char* szDestinationIp,
                 const int   iDestPort,
                 const char* szLocalIp,
                 const int   iLocalPort,
                 const void* pData,
                 const size_t nData,
                 const void* pUserData)
{
    bool bRet = false;
    
    return bRet;
}                                      

bool flibbleProc(SIPX_TRANSPORT hTransport,
                 const char* szDestinationIp,
                 const int   iDestPort,
                 const char* szLocalIp,
                 const int   iLocalPort,
                 const void* pData,
                 const size_t nData,
                 const void* pUserData)
{
    bool bRet = false;
    OsNatDatagramSocket sendSocket(42424, szDestinationIp, 42425);
    int bytesSent = sendSocket.write((const char*) pData, nData, szDestinationIp, 42424);
    
    return bytesSent > 0;
} 

SIPX_CONTACT_ID lookupContactId(const char* szIp, const char* transport, const SIPX_TRANSPORT hTransport)
{
    SIPX_CONTACT_ID contactId = CONTACT_AUTO;
    size_t numAddresses = 0;
    SIPX_CONTACT_ADDRESS addresses[32];
    
    sipxConfigGetLocalContacts(g_hInst, addresses, 32, numAddresses);
    for (int i = 0; i < numAddresses; i++)
    {
        if (0 == strcmp(addresses[i].cIpAddress, szIp) &&
            0 == strcmp(addresses[i].cCustomTransportName, transport) &&
            addresses[i].eTransportType == hTransport)
        {
            contactId = addresses[i].id;
        }
    }
    
    return contactId;
}
