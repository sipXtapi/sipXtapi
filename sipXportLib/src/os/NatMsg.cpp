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


// APPLICATION INCLUDES
#include "os/NatMsg.h"


NatMsg::NatMsg(int          type,
               char*        szBuffer, 
               int          nLength, 
               IOsNatSocket* pSocket,
               UtlString    receivedIp,
               int          iReceivedPort,
               UtlString*   pRelayIp,
               int*         pRelayPort)
    : OsMsg(NAT_MSG_TYPE, 0)
{
    miType = type ;
    mBuffer = szBuffer ;    // Shallow copy
    mLength = nLength ;
    mpSocket = pSocket ;
    mReceivedIp = receivedIp ;
    miReceivedPort = iReceivedPort ;
    mpContext = NULL ;
    if (pRelayIp)
        mRelayIp = *pRelayIp ;
    miRelayPort = (pRelayPort == NULL) ? 0 : *pRelayPort ;
}

NatMsg::NatMsg(int   type,
               void* pContext)
    : OsMsg(NAT_MSG_TYPE, 0)
{
    miType = type ;
    mBuffer = NULL ;
    mLength = 0;
    mpSocket = NULL ;
    miReceivedPort = 0 ;
    miRelayPort = 0 ;
    mpContext = pContext ;
}


NatMsg::NatMsg(const NatMsg& rNatMsg)
    : OsMsg(NAT_MSG_TYPE, 0)
{
    miType = rNatMsg.miType ;
    mBuffer = rNatMsg.mBuffer ;
    mLength  = rNatMsg.mLength ;
    mpSocket = rNatMsg.mpSocket ;
    mReceivedIp = rNatMsg.mReceivedIp ;
    miReceivedPort = rNatMsg.miReceivedPort ;
    mpContext = rNatMsg.mpContext ;
    mRelayIp = rNatMsg.mRelayIp ;
    miRelayPort = rNatMsg.miRelayPort ;
}


OsMsg* NatMsg::createCopy(void) const
{
    return new NatMsg(*this);
}


NatMsg::~NatMsg()
{

}


NatMsg& NatMsg::operator=(const NatMsg& rhs)
{
    if (this != &rhs)            // handle the assignment to self case
    {
        miType = rhs.miType ;
        mBuffer = rhs.mBuffer ;
        mLength = rhs.mLength ;
        mpSocket = rhs.mpSocket ;
        mReceivedIp = rhs.mReceivedIp ;
        miReceivedPort = rhs.miReceivedPort ;
        mpContext = rhs.mpContext ;
        mRelayIp = rhs.mRelayIp ;
        miRelayPort = rhs.miRelayPort ;
    }

    return *this ;
}


char* NatMsg::getBuffer() const
{
    return mBuffer ;
}

   
int NatMsg::getLength() const
{
    return mLength ;
}


IOsNatSocket* NatMsg::getSocket() const 
{
    return mpSocket ;
}

UtlString NatMsg::getReceivedIp() const 
{
    return mReceivedIp ;
}


int NatMsg::getReceivedPort() const 
{
    return miReceivedPort ;
}

int NatMsg::getType() const 
{
    return miType ;
}

void* NatMsg::getContext() const
{
    return mpContext ;
}

UtlString NatMsg::getRelayIp() const 
{
    return mRelayIp ;
}


int NatMsg::getRelayPort() const 
{
    return miRelayPort ;
}


