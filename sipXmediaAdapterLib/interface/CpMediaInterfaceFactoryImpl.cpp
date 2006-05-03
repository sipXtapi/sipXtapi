//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mi/CpMediaInterfaceFactoryImpl.h"
#include "os/OsLock.h"
#include "os/OsDatagramSocket.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpMediaInterfaceFactoryImpl::CpMediaInterfaceFactoryImpl()
    : mlockList(OsMutex::Q_FIFO)
{
    miStartRtpPort = 0 ;
    miLastRtpPort = 0;
    miNextRtpPort = miStartRtpPort ;
}

// Destructor
CpMediaInterfaceFactoryImpl::~CpMediaInterfaceFactoryImpl()
{
    OsLock lock(mlockList) ;
    
    mlistFreePorts.destroyAll() ;
    mlistBusyPorts.destroyAll() ;
}

/* =========================== DESTRUCTORS ================================ */

/**
 * public interface for destroying this media interface
 */ 
void CpMediaInterfaceFactoryImpl::release()
{
   delete this;
}

/* ============================ MANIPULATORS ============================== */


void CpMediaInterfaceFactoryImpl::setRtpPortRange(int startRtpPort, int lastRtpPort) 
{
    miStartRtpPort = startRtpPort ;
    if (miStartRtpPort < 0)
    {
        miStartRtpPort = 0 ;
    }
    miLastRtpPort = lastRtpPort ;
    miNextRtpPort = miStartRtpPort ;
}

#define MAX_PORT_CHECK_ATTEMPTS     4
#define MAX_PORT_CHECK_WAIT_MS      50
OsStatus CpMediaInterfaceFactoryImpl::getNextRtpPort(int &rtpPort) 
{
    OsLock lock(mlockList) ;
    bool bGoodPort = false ;
    int iAttempts = 0 ;

    // Re-add busy ports to end of free list
    while (mlistBusyPorts.entries())
    {
        UtlInt* pInt = (UtlInt*) mlistBusyPorts.first() ;
        mlistBusyPorts.remove(pInt) ;

        mlistFreePorts.append(pInt) ;
    }

    while (!bGoodPort && (iAttempts < MAX_PORT_CHECK_ATTEMPTS))
    {
        iAttempts++ ;

        // First attempt to get a free port for the free list, if that
        // fails, return a new one. 
        if (mlistFreePorts.entries())
        {
            UtlInt* pInt = (UtlInt*) mlistFreePorts.first() ;
            mlistFreePorts.remove(pInt) ;
            rtpPort = pInt->getValue() ;
            delete pInt ;
        }
        else
        {
            rtpPort = miNextRtpPort ;

            // Only allocate if the nextRtpPort is greater then 0 -- otherwise we
            // are allowing the system to allocate ports.
            if (miNextRtpPort > 0)
            {
                miNextRtpPort += 2 ; 
            }
        }

        bGoodPort = !isPortBusy(rtpPort, MAX_PORT_CHECK_WAIT_MS) ;
        if (!bGoodPort)
        {
            mlistBusyPorts.insert(new UtlInt(rtpPort)) ;
        }
    }

    // If unable to find a usable port, let the system pick one.
    if (!bGoodPort)
    {
        rtpPort = 0 ;
    }
    
    return OS_SUCCESS ;
}


OsStatus CpMediaInterfaceFactoryImpl::releaseRtpPort(const int rtpPort) 
{
    OsLock lock(mlockList) ;

    // Only bother noting the free port if the next port isn't 0 (OS selects 
    // port)
    if (miNextRtpPort != 0)
    {
        // Release port to head of list (generally want to reuse ports)
        mlistFreePorts.insert(new UtlInt(rtpPort)) ;
    }

    return OS_SUCCESS ;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

bool CpMediaInterfaceFactoryImpl::isPortBusy(int iPort, int checkTimeMS) 
{
    bool bBusy = false ;

    if (iPort > 0)
    {
        OsDatagramSocket* pSocket = new OsDatagramSocket(0, NULL, iPort, NULL) ;
        if (pSocket != NULL)
        {
            if (pSocket->isReadyToRead(checkTimeMS))
            {
                bBusy = true ;
            }

            pSocket->close() ;
            delete pSocket ;
        }
    }

    return bBusy ;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


