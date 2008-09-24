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
