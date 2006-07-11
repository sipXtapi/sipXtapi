//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsTimerTask_h_
#define _OsTimerTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsMsgQ.h"
#include "os/OsServerTask.h"
#include "os/OsSysTimer.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class OsTimer;
class OsTimerMsg;

//:Timer service request manager (runs as a separate task)
// This task is responsible for managing timer service requests. Timer
// requests are received via a message queue.

class OsTimerTask : public OsServerTask
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   static OsTimerTask* getTimerTask(void);
     //:Return a pointer to the timer task, creating it if necessary

   static void destroyTimer(void);
     //: Destroy the singleton instance of the sys timer

   virtual
   ~OsTimerTask();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   static void startTimer(OsTimer& rTimer);
      //:Submit a service request to start (arm) the specified timer

   static void stopTimer(OsTimer& rTimer);
      //:Submit a service request to cancel (disarm) the specified timer

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsTimerTask();
     //:Constructor (called only indirectly via getTimerTask())
     // We identify this as a protected (rather than a private) method so
     // that gcc doesn't complain that the class only defines a private
     // constructor and has no friends.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const int TIMER_MAX_REQUEST_MSGS;   // Maximum number of request messages
 
   virtual UtlBoolean handleMessage(OsMsg& rMsg);
     //:Handle a timer service request.
     // Return TRUE if the request was handled, otherwise FALSE.

   static OsTimerTask* spInstance;  // pointer to the single instance of
                                    //  the OsTimerTask class
   static OsBSem       sLock;       // semaphore used to ensure that there
                                    //  is only one instance of this class
   OsSysTimer*         mpTimerSubsys; // Object used to access the underlying
                                      //  operating system's timer subsystem

   OsTimerTask(const OsTimerTask& rOsTimerTask);
     //:Copy constructor (not implemented for this task)

   OsTimerTask& operator=(const OsTimerTask& rhs);
     //:Assignment operator (not implemented for this task)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsTimerTask_h_

