//
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <stdio.h>


// APPLICATION INCLUDES
#include <os/OsNatServerSocket.h>
#include <os/OsNatConnectionSocket.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsNatServerSocket::OsNatServerSocket(int connectionQueueSize,
    int serverPort,
    const char* szBindAddr,
    const bool bPerformBind) :
    OsServerSocket(connectionQueueSize, serverPort, szBindAddr, bPerformBind)
{
}

// Destructor
OsNatServerSocket::~OsNatServerSocket()
{
}

/* ============================ MANIPULATORS ============================== */




/* //////////////////////////// PROTECTED ///////////////////////////////// */
OsConnectionSocket* OsNatServerSocket::createConnectionSocket(UtlString localIp, int descriptor)
{
    return new OsNatConnectionSocket(localIp, descriptor, RTP_TCP_ROLE_PASSIVE);
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
