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
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
// Author: Mike Cohen
// 
#ifndef _UPnpDiscovery_h_
#define _UPnpDiscovery_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "net/HttpMessage.h"
#include "os/OsDatagramSocket.h" 

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class UPnpDiscovery
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
    UPnpDiscovery(const int timeout);
    UPnpDiscovery(const UtlString srcIp, const int srcPort, const int timeout);
    UPnpDiscovery(const UPnpDiscovery& src);
    virtual ~UPnpDiscovery();

    UPnpDiscovery& operator= (const UPnpDiscovery& src);

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
    UtlString discoverRootLocation() const;
    UtlString discoverGatewayLocation() const;
    UtlString discoverWANIPConnectionLocation() const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    UtlString mSrcIp;
    int mSrcPort;

    const HttpMessage sendDiscoveryRequest(const HttpMessage& message,
                                           const int timeout) const;
    const HttpMessage createDiscoveryRequest(const UtlString serviceType) const;
    const UtlString getLocation(const HttpMessage sResponse) const;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    OsDatagramSocket* mpSocket;
    int mTimeout;
};

#endif
