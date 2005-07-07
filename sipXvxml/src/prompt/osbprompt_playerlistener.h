// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _osbprompt_playerlistener_h_
#define _osbprompt_playerlistener_h_

// SYSTEM INCLUDES
#include "os/OsBSem.h"
#include "os/OsDefs.h"
#include "os/OsTime.h"
#include "os/OsQueuedEvent.h"

// APPLICATION INCLUDES
#include "mp/MpQueuePlayerListener.h"

// DEFINES
#define MAX_NUM_LISTENERS 10

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Class used to wait for state changes
class OSBQueuePlayerListener : public MpQueuePlayerListener
{
public:
   OSBQueuePlayerListener(int iTimeoutSecs = 5);

   virtual
   ~OSBQueuePlayerListener();
     //:Destructor

   virtual void clearState();

   virtual UtlBoolean waitForState(PlayerState state);
  
   virtual UtlBoolean isState(PlayerState state);

   virtual void addListeningEvent(OsQueuedEvent* promptEvent);

   virtual void removeListeningEvent(OsQueuedEvent* promptEvent);

   virtual void queuePlayerStarted();
     //:Called when a queue player has started playing its playlist.

   virtual void queuePlayerStopped();
     //:Called when a queue player has stopped playing its playlist.  
     // This event will occur after the play list completes or when aborted.    

   virtual void queuePlayerAdvanced();
     //:Called when the queue player advances to a new playlist element.
     // This method is called before the new playlist element is played and
     // may occur multiple times before a queuePlayerStopped.

   virtual int addRef();
     //:Called when the rec module gets a handle of the listener.

   virtual int release();
     //:Called when the rec and prompt modules' EndSession.
     // when return value (mRef) is <= 0, caller should delete this listener.


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
  int mEventQLen;
        OsQueuedEvent*  mPromptEvents[MAX_NUM_LISTENERS];

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int       miTimeoutSec ;
   UtlBoolean mStates[16] ;
   OsBSem    mSemStateChange;
   OsBSem    mSemGuard ;

   int       mShutdown;
   int       mRef;

#ifdef TEST
   static bool sIsTested;
     //:Set to true after the tests for this class have been executed once

   void test();
     //:Verify assertions for this class

   // Test helper functions
   void testCreators();
   void testManipulators();
   void testAccessors();
   void testInquiry();

#endif //TEST
};

/* ============================ INLINE METHODS ============================ */

#endif  // _osbprompt_playerlistener_h_


