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
#include "os/OsLock.h"
#include "SipRegistrar.h"
#include "RegistrarTest.h"
#include "RegistrarSync.h"
#include "RegistrarPeer.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

// Constructor
RegistrarPeer::RegistrarPeer( SipRegistrar*   registrar
                             ,const UtlString& name
                             ,int             rpcPort
                             ,const char*     rpcPath
                             )
   : UtlString(name),
     mLock(OsBSem::Q_PRIORITY, OsBSem::FULL),
     mSyncState(PeerSyncStateUnknown),
     mSentTo(0),
     mReceivedFrom(0),
     mRegistrar(registrar)
{

   mUrl.setScheme(Url::HttpsUrlScheme);
   mUrl.setHostAddress(name);
   mUrl.setHostPort(rpcPort);
   mUrl.setPath(rpcPath);
}

// Destructor
RegistrarPeer::~RegistrarPeer()
{
}

/// The full URL to be used to make an XML RPC request of this peer.
void RegistrarPeer::rpcURL(Url& url)
{
   // no need for lock;  url is set in constructor and never changed.
   url = mUrl;
}

/// Whether or not the most recent attempt to reach this peer succeeded.
RegistrarPeer::SynchronizationState RegistrarPeer::synchronizationState()
{
   OsLock mutex(mLock);
   
   return mSyncState;
}

/// Indicate that a request to this peer failed.
void RegistrarPeer::markUnReachable()
{
   RegistrarTest* notifyTestThread;
   
   { // lock scope
      OsLock mutex(mLock);
   
      notifyTestThread = (  PeerReachable == mSyncState
                          ? mRegistrar->getRegistrarTest() // was reachabe, so get the thread
                          : NULL // was not reachable, so no need to notify the thread again
                          );   

      mSyncState = PeerUnReachable;
   }  // release lock before signalling RegistrarTest thread
   
   if (notifyTestThread)
   {
      // Tell the RegistrarTest thread to start polling
      notifyTestThread->check();
   }
}

/// Indicate that a request to this peer succeeded or a request was received from it.
void RegistrarPeer::markReachable()
{
   RegistrarSync* notifySyncThread;
   
   { // lock scope
      OsLock mutex(mLock);
   
      notifySyncThread = (  PeerUnReachable == mSyncState
                          ? mRegistrar->getRegistrarSync() // was not reachabe, so get the thread
                          : NULL // was reachable, so no need to notify the thread again
                          );   
      mSyncState = PeerReachable;

   }  // release lock before signalling RegistrarSync thread
   
   if (notifySyncThread)
   {
      // Tell the RegistrarSync thread to start updating
      notifySyncThread->sendUpdates();
   }
}

/// The oldest update successfully sent to this peer.
intll RegistrarPeer::sentTo()
{
   OsLock mutex(mLock);
   
   return mSentTo;
}

/// The last update received from this peer.
intll RegistrarPeer::receivedFrom()
{
   OsLock mutex(mLock);
   
   return mReceivedFrom;
}

void RegistrarPeer::setSentTo(intll updateNumber)
{
   OsLock mutex(mLock);
   
   mSentTo = updateNumber;
}

void RegistrarPeer::setReceivedFrom(intll updateNumber)
{
   OsLock mutex(mLock);
   
   mReceivedFrom = updateNumber;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

