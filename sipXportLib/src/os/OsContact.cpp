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
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <string.h>
#include <limits.h>

// APPLICATION INCLUDES
#include "os/OsContact.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType OsContact::TYPE = "OsContact" ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor.
OsContact::OsContact(const char* szAddress,
                     int port,
                     OsSocket::IpProtocolSocketType protocol,
                     IpAddressType type)
{
    mAddress = szAddress;
    mPort = port;
    mProtocol = protocol;
    mType = type;
} 


OsContact::OsContact()
{
    mPort = PORT_NONE ;
    mProtocol = OsSocket::UDP ;
    mType = IP4 ;
}


// Copy constructor
OsContact::OsContact(const OsContact& ref)
{
    if (this != &ref)
    {
        mAddress = ref.mAddress;
        mPort = ref.mPort;
        mProtocol = ref.mProtocol;
        mType = ref.mType;
    }
}


// Equals operator
OsContact& OsContact::operator=(const OsContact& rhs) 
{
    if (this != &rhs)
    {
        mAddress = rhs.mAddress;
        mPort = rhs.mPort;
        mProtocol = rhs.mProtocol;
        mType = rhs.mType;
    }

    return *this ;
}


// Destructor
OsContact::~OsContact()
{
}


/* ============================ MANIPULATORS ============================== */

void OsContact::setAddress(const char* szAddress) 
{
    mAddress = szAddress ;
}


void OsContact::setPort(int port) 
{
    mPort = port ;
}


void OsContact::setProtocol(OsSocket::IpProtocolSocketType protocolType) 
{
    mProtocol = protocolType ;
}


void OsContact::setAddressType(IpAddressType addressType) 
{
    mType = addressType ;
}

/* ============================ ACCESSORS ================================= */

const int OsContact::getPort() const
{
    return mPort;
}

const OsSocket::IpProtocolSocketType OsContact::getProtocol() const
{
    return mProtocol;
}

const IpAddressType OsContact::getAddressType() const
{
    return mType;
}

unsigned OsContact::hash() const
{
   return mAddress.hash() + mPort + mProtocol + mType ; 
}


UtlContainableType OsContact::getContainableType() const
{
    return OsContact::TYPE ;
}

/* ============================ INQUIRY =================================== */

int OsContact::compareTo(UtlContainable const * inVal) const
{
   int result ; 
   
    const OsContact* pContact = dynamic_cast<const OsContact*>(inVal);
    if (inVal->isInstanceOf(OsContact::TYPE) && pContact)
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
    }
    else
    {
        result = INT_MAX ; 
    }

    return result ;
}

UtlBoolean OsContact::isEqual(UtlContainable const * inVal) const
{
    return (compareTo(inVal) == 0) ; 
}

const void OsContact::getAddress(UtlString& address) const
{
    address = mAddress;
}

const char* OsContact::getAddress() const
{
    if (mAddress.length() == 0)
        return NULL ;
    else
        return mAddress.data() ;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
const int OsContact::compareInt(const int first, const int second) const
{
    int ret;
    if (first == second)
    {
        ret = 0;
    }
    else if (first < second)
    {
        ret = -1;
    }
    else
    {
        ret = 1;
    }
    return ret;
}

/* ============================ FUNCTIONS ================================= */
