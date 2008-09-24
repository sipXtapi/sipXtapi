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
// Author: Bob Andreasen
//

// SYSTEM INCLUDES
#include <string.h>
#include <limits.h>

// APPLICATION INCLUDES
#include "os/OsMediaContact.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType OsMediaContact::TYPE = "OsMediaContact" ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor.
OsMediaContact::OsMediaContact(const char* szAddress,
                                int rtpPort,
                                int rtcpPort,
                                OsSocket::IpProtocolSocketType protocol,
                                RTP_TRANSPORT transportType,
                                IpAddressType addressType)
    : OsContact(szAddress, rtpPort, protocol, addressType)
{
    mRtcpPort = rtcpPort ;
    mTransportType = transportType ;
} 


OsMediaContact::OsMediaContact()
    : OsContact()
{
    mRtcpPort = PORT_NONE ;
    mTransportType = RTP_TRANSPORT_UNKNOWN ;
}


// Copy constructor
OsMediaContact::OsMediaContact(const OsMediaContact& ref)
    : OsContact(ref)
{    
    if (this != &ref)
    {
        mRtcpPort = ref.mRtcpPort ;
        mTransportType = ref.mTransportType ;
    }
}


// Equals operator
OsMediaContact& OsMediaContact::operator=(const OsMediaContact& rhs) 
{
    if (this != &rhs)
    {
        OsContact::operator=(rhs);

        mRtcpPort = rhs.mRtcpPort ;
        mTransportType = rhs.mTransportType ;
    }

    return *this ;
}


// Destructor
OsMediaContact::~OsMediaContact()
{
}


/* ============================ MANIPULATORS ============================== */

void OsMediaContact::setRtcpPort(int port) 
{
    mRtcpPort = port ;    
}

void OsMediaContact::setTransportType(RTP_TRANSPORT transportType) 
{
    mTransportType = transportType ;
}


/* ============================ ACCESSORS ================================= */

int OsMediaContact::getRtcpPort() const
{
    return mRtcpPort ;
}

RTP_TRANSPORT OsMediaContact::getTransportType() const 
{
    return mTransportType ;
}

unsigned int OsMediaContact::hash() const 
{
    int hash = OsContact::hash() ;

    hash ^= mRtcpPort ;
    hash ^= mTransportType ;

    return hash ; 
}


UtlContainableType OsMediaContact::getContainableType() const
{
    return OsMediaContact::TYPE ;
}

/* ============================ INQUIRY =================================== */

int OsMediaContact::compareTo(UtlContainable const * inVal) const
{
   int result ; 
   
    const OsMediaContact* pContact = dynamic_cast<const OsMediaContact*>(inVal);
    if (inVal && inVal->isInstanceOf(OsMediaContact::TYPE) && pContact)
    {
        UtlString address;
        pContact->getAddress(address);
        result = mAddress.compareTo(address);
        if (0 == result)
        {
            result = compareInt(mPort, pContact->getPort());
        }
        if (0 == result)
        {
            result = compareInt(mProtocol, pContact->getProtocol());
        }
        if (0 == result)
        {
            result = compareInt((const int)mType, (const int)pContact->getAddressType());
        }
        if (0 == result)
        {
            result = compareInt((const int)mRtcpPort, (const int)pContact->getRtcpPort());
        }
        if (0 == result)
        {
            result = compareInt((const int)mTransportType, (const int)pContact->getTransportType());
        }
    }
    else
    {
        result = INT_MAX ; 
    }

    return result ;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
