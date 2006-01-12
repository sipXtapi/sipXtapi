// 
// Copyright (C) 2006 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _REGISTRARTEST_H_
#define _REGISTRARTEST_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * This thread is responsible for periodically attempting
 * to re-establish contact with an UnReachable peer.
 */
class RegistrarTest : public OsServerTask
{
  public:

   // constructor
   RegistrarTest();

   /// destructor
   virtual ~RegistrarTest()
      {
      };


   /// Check each unreachable peer.
   UtlBoolean handleMessage( OsMsg& eventMessage ///< Timer expiration msg
                            );
   /**<
    * Does a single check of each unreachable peer.
    * If any are still unreachable after all are checked, then
    * the timer is scheduled to retry, using a standard limited
    * exponential backoff.
    */
   
   /// Initiate checking of any unreachable peers.
   void check();
   /**<
    * If there is no currently running timer, start one to initiate
    * reachability checks (if already running, this is a no-op).
    */

  private:

   /// mutex must be locked with OsLock to access any other member variable.
   OsBSem mutex;

   /// Whether or not there is a timer running.
   bool mWaitingForNextCheck;
   
   /// Current value of the retry timer.
   size_t mRetryTime; 
   /**
    * Initialized to REGISTER_TEST_INITIAL_WAIT.
    *
    * If after a round of checks,
    * - all peers are Reachable
    *   Reset to REGISTER_TEST_INITIAL_WAIT.
    * - some peer is UnReachable
    *   Set to MIN( mRetryTime * 2, REGISTER_TEST_MAX_WAIT )
    *   and schedule a timer for this new time.
    */

   /// There is no copy constructor.
   RegistrarTest(const RegistrarTest&);

   /// There is no assignment operator.
   RegistrarTest& operator=(const RegistrarTest&);
    
};

#endif // _REGISTRARTEST_H_
