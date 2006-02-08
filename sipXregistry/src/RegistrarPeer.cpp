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
     mSyncState(SyncStateUnknown),
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
   bool notifyTestThread = false;
   
   { // lock scope
      OsLock mutex(mLock);
   
      // If the peer was previously UnReachable, then marking it UnReachable again
      // is a noop.  If the peer was in any other state (Reachable, SyncStateUnknown,
      // Incompatible) then notify the test thread so that we can try to reach it
      // again later.
      if (mSyncState != UnReachable)
      {
         mSyncState = UnReachable;
         notifyTestThread = true;
      }
   }  // release lock before signalling RegistrarTest thread
   
   if (notifyTestThread)
   {
      RegistrarTest* registrarTestThread = mRegistrar->getRegistrarTest();
      assert(registrarTestThread);

      // Tell the RegistrarTest thread to start polling
      registrarTestThread->check();
   }
}

/// Indicate that a request to this peer succeeded or a request was received from it.
void RegistrarPeer::markReachable()
{
   RegistrarSync* notifySyncThread;
   
   { // lock scope
      OsLock mutex(mLock);
   
      notifySyncThread = (  UnReachable == mSyncState
                          ? mRegistrar->getRegistrarSync() // was not reachable, so get the thread
                          : NULL // was reachable, so no need to notify the thread again
                          );   
      mSyncState = Reachable;

      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "RegistrarPeer::markReachable called on peer %s", name());
   }  // release lock before signalling RegistrarSync thread
   
   if (notifySyncThread)
   {
      // Tell the RegistrarSync thread to start updating
      notifySyncThread->sendUpdates();
   }
}

/// Indicate that a permanent error has occurred with this peer.
void RegistrarPeer::markIncompatible()
{
   OsLock mutex(mLock);

   mSyncState = Incompatible;
}

/// Set the peer state to a known state (not SyncStateUnknown)
void RegistrarPeer::setState(SynchronizationState state)
{
   switch(state)
   {
   case SyncStateUnknown:
      assert(false);
      break;

   case Reachable:
      markReachable();
      break;
   
   case UnReachable:
      markUnReachable();
      break;

   case Incompatible:
      markIncompatible();
      break;

   default:
      assert(false);
      break;
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

