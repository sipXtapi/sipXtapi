// $Id$
//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "RegistrarPeer.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

// Constructor
RegistrarPeer::RegistrarPeer(const UtlString& name
                             ,int             rpcPort
                             ,const char*     rpcPath
                             )
   : UtlString(name), mLock(NULL), mSyncState(PeerSyncStateUnknown), mSentTo(0), 
     mReceivedFrom(0), mUrl(NULL)
{
}

// Destructor
RegistrarPeer::~RegistrarPeer()
{
}

/// The full URL to be used to make an XML RPC request of this peer.
void RegistrarPeer::rpcURL(Url& url)
{
   // :TODO: implement this method
}

/// Whether or not the most recent attempt to reach this peer succeeded.
RegistrarPeer::SynchronizationState RegistrarPeer::synchronizationState()
{
   return mSyncState;
}

/// Indicate that a request to this peer failed.
void RegistrarPeer::markUnReachable()
{
   mSyncState = PeerUnReachable;
}

/// Indicate that a request to this peer succeeded or a request was received from it.
void RegistrarPeer::markReachable()
{
   mSyncState = PeerReachable;
}

/// The oldest update successfully sent to this peer.
intll RegistrarPeer::sentTo()
{
   return mSentTo;
}

/// The last update received from this peer.
intll RegistrarPeer::receivedFrom()
{
   return mReceivedFrom;
}

void RegistrarPeer::setSentTo(intll updateNumber)
{
   mSentTo = updateNumber;
}

void RegistrarPeer::setReceivedFrom(intll updateNumber)
{
   mReceivedFrom = updateNumber;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

