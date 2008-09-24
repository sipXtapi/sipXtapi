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
#include <assert.h>

// APPLICATION INCLUDES
#include <net/SipUserAgentBase.h>
#include <os/OsWriteLock.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipUserAgentBase::SipUserAgentBase(int sipTcpPort, 
                                   int sipUdpPort,
                                   int sipTlsPort,
                                   int queueSize) :
    OsServerTask("SipUserAgent-%d", NULL, queueSize),
    mObserverMutex(OsRWMutex::Q_FIFO)
{
    mTcpPort = sipTcpPort;
    mUdpPort = sipUdpPort;
    mTlsPort = sipTlsPort;
    mMessageLogEnabled = TRUE;
}



// Destructor
SipUserAgentBase::~SipUserAgentBase()
{
}

/* ============================ MANIPULATORS ============================== */

void SipUserAgentBase::addConfigChangeConsumer(OsMsgQ& messageQueue)
{
    UtlInt* observer = new UtlInt((int) &messageQueue);
    OsWriteLock lock(mObserverMutex);
    mConfigChangeObservers.insert(observer);
}

/* ============================ ACCESSORS ================================= */

void SipUserAgentBase::getContactUri(UtlString* contactUri)
{
    contactUri->remove(0);
    contactUri->append(mContactAddress);
}

/* ============================ INQUIRY =================================== */

UtlBoolean SipUserAgentBase::isReady()
{
    return(isStarted());
}

UtlBoolean SipUserAgentBase::waitUntilReady()
{
    // Lazy hack, should be a semaphore or event
    while(!isReady())
    {
        delay(500);
    }
    return(TRUE);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Assignment operator
SipUserAgentBase& 
SipUserAgentBase::operator=(const SipUserAgentBase& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

// Copy constructor
SipUserAgentBase::SipUserAgentBase(const SipUserAgentBase& rSipUserAgentBase) :
    mObserverMutex(OsRWMutex::Q_FIFO)
{
}

/* ============================ FUNCTIONS ================================= */

