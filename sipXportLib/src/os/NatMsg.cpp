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


// APPLICATION INCLUDES
#include "os/NatMsg.h"


NatMsg::NatMsg(int          type,
               char*        szBuffer, 
               int          nLength, 
               IStunSocket* pSocket,
               UtlString    receivedIp,
               int          iReceivedPort)
    : OsMsg(NAT_MSG_TYPE, 0)
{
    miType = type ;
    mBuffer = szBuffer ;    // Shallow copy
    mLength = nLength ;
    mpSocket = pSocket ;
    mReceivedIp = receivedIp ;
    miReceivedPort = iReceivedPort ;
    mpContext = NULL ;
}

NatMsg::NatMsg(int   type,
               void* pContext)
    : OsMsg(NAT_MSG_TYPE, 0)
{
    miType = type ;
    mBuffer = NULL ;
    mLength = 0;
    mpSocket = NULL ;
    mReceivedIp.remove(0) ;
    miReceivedPort = 0 ;
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


IStunSocket* NatMsg::getSocket() const 
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

