// 
// Copyright (C) 2006 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsEventMsg.h"
#include "os/OsTime.h"
#include "os/OsLock.h"
#include "utl/UtlSListIterator.h"
#include "SipRegistrar.h"
#include "SyncRpc.h"
#include "RegistrarPeer.h"
#include "RegistrarTest.h"

// DEFINES
// CONSTANTS

const size_t REGISTER_TEST_INITIAL_WAIT = 2; // Seconds until the first test
const size_t REGISTER_TEST_MAX_WAIT = 32;    // Maximum seconds between tests

// TYPEDEFS
// FORWARD DECLARATIONS

/// constructor
RegistrarTest::RegistrarTest(SipRegistrar& sipRegistrar) :
   mLock(OsBSem::Q_PRIORITY, OsBSem::FULL),
   mWaitingForNextCheck(false),
   mRetryTimer(getMessageQueue(),0),
   mRetryTime(0),
   mSipRegistrar(sipRegistrar)
{
};

/// Signal that a peer has become UnReachable
void RegistrarTest::check()
{
   OsLock mutex(mLock);

   if ( !mWaitingForNextCheck )
   {
      mRetryTime = REGISTER_TEST_INITIAL_WAIT;
      mRetryTimer.oneshotAfter(OsTime(mRetryTime,0));
      mWaitingForNextCheck = true;
   }
   else
   {
      /*
       * There is already a timer running, so just let it expire and do the check then.
       */
   }
}


/// Check each unreachable peer.
UtlBoolean RegistrarTest::handleMessage( OsMsg& eventMessage ///< Timer expiration msg
                                        )
{
   UtlBoolean handled = TRUE;
   
   int msgType    = eventMessage.getMsgType();
   int msgSubType = eventMessage.getMsgSubType();

   if (   OsMsg::OS_EVENT    == msgType
       && OsEventMsg::NOTIFY == msgSubType
       )
   {
      // A timer has expired, so it's time to check on UnReachable peers
      SipRegistrar* sipRegistrar = NULL;
      UtlSListIterator* peers = NULL;

      sipRegistrar = &mSipRegistrar;
      peers = sipRegistrar ? sipRegistrar->getPeers() : NULL;

      if (sipRegistrar && peers)
      {
         /*
          * Do a single check of each unreachable peer.
          */
         RegistrarPeer* peer;
         while (   !isShuttingDown()
                && (peer = dynamic_cast<RegistrarPeer*>((*peers)()))
                )
         {
            if ( RegistrarPeer::UnReachable == peer->synchronizationState() )
            {
               OsSysLog::add( FAC_SIP, PRI_DEBUG, "RegistrarTest SyncRpcReset::invoke(%s, %s)"
                              ,sipRegistrar->primaryName().data(), peer->name());
               RegistrarPeer::SynchronizationState state =
                  SyncRpcReset::invoke(sipRegistrar->primaryName(), *peer);
               peer->setState(state);
            }
         }
         
         if ( !isShuttingDown() )
         {
            /*
             * If any are still unreachable after all are checked, then
             * the timer is scheduled to retry, using a standard limited
             * exponential backoff.
             */

            bool somePeerIsUnreachable = false;  // be optimistic 
            peers->reset();
            OsLock mutex(mLock); // do not do any asynchronous operations holding the lock

            while (   (peer = dynamic_cast<RegistrarPeer*>((*peers)()))
                   && !somePeerIsUnreachable // it only takes one, so don't keep checking
                   )
            {
               if ( RegistrarPeer::UnReachable == peer->synchronizationState() )
               {
                  somePeerIsUnreachable = true;
               }
            }

            if (somePeerIsUnreachable)
            {
               if ( mRetryTime < REGISTER_TEST_MAX_WAIT ) // has timer reached the backoff limit?
               {
                  // no - so back off by doubling it
                  mRetryTime *= 2;
               }

               // start the timer
               mRetryTimer.oneshotAfter(OsTime(mRetryTime,0));
            }
            else // there are no UnReachable peers
            {
               mWaitingForNextCheck = false;
               mRetryTime = 0;
            }
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_NOTICE,
                       "RegistrarTest::handleMessage no peers? %p %p",
                       sipRegistrar, peers
                       );
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "RegistrarTest::handleMessage %d/%d",
                    msgType, msgSubType
                    );
   }
   
   return handled;
}

/// destructor
RegistrarTest::~RegistrarTest()
{
   // :TODO: 
}
