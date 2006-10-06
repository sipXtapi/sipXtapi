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
OsContact::OsContact(UtlString address,
                       int port,
                       OsSocket::SocketProtocolTypes protocol,
                       IpAddressType type)
{
    mAddress = address;
    mPort = port;
    mProtocol = protocol;
    mType = type;
} 


// Copy constructor
OsContact::OsContact(const OsContact& ref)
{
    this->mAddress = ref.mAddress;
    this->mPort = ref.mPort;
    this->mProtocol = ref.mProtocol;
    this->mType = ref.mType;
}

// Destructor
OsContact::~OsContact()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

const int OsContact::getPort() const
{
    return mPort;
}

const OsSocket::SocketProtocolTypes OsContact::getProtocol() const
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
