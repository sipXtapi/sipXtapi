//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsTimerDb_h_
#define _OsTimerDb_h_

// SYSTEM INCLUDES
#include <os/OsDefs.h>
#include <os/OsBSem.h>
#include <os/OsStatus.h>
#include <os/OsTimer.h>
#include <utl/UtlHashMap.h>

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Database of active timers.
// The OsTimerDb maintains a database of active timers (i.e., those timers
// that have been started by the low level OsSysTimer class).  Since the
// OsTimerTask is the only task that should be accessing the timer database
// there is no need to serialize access (and no locking).<br>
// <br>
// Each entry in the database is a key/value pair where the key corresponds
// to a timer ID and the value is the pointer to the corresponding OsTimer
// object.  Duplicate keys are not allowed.
class OsTimerDb
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum DbInitSize { TIMER_DB_INIT_SIZE = 30 };

/* ============================ CREATORS ================================== */

   OsTimerDb();
     //:Default constructor

   virtual
   ~OsTimerDb();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus insert(int timerId, OsTimer* pTimer);
     //:Insert the indicated timer into the database of active timers.
     // Return OS_SUCCESS if successful, OS_NAME_IN_USE if the key is
     // already in the database.

   OsStatus remove(int timerId);
     //:Remove the indicated timer from the database of active timers.
     // Return OS_SUCCESS if the indicated timerId is found, return
     // OS_NOT_FOUND if there is no match for the specified key.

/* ============================ ACCESSORS ================================= */

   int getActiveTimers(OsTimer* activeTimers[], int size);
     //:Get an array of pointers to the timers that are currently active.
     // The caller provides an array that can hold up to <i>size</i> OsTimer
     // pointers. This method will fill in the <i>activeTimers</i> array with
     // up to <i>size</i> pointers. The method returns the number of pointers
     // in the array that were actually filled in.

   void getDbStats(unsigned& nInserts, unsigned& nRemoves) const;
     //:Get the number of insertions and removals for the timer database.

   int numEntries(void) const;
     //:Return the number of key-value pairs in the name database.

/* ============================ INQUIRY =================================== */

   UtlBoolean isEmpty(void) const;
     //:Return TRUE if the timer database is empty.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlHashMap mDict;          // hash table used to store the key/value
                                    //  pairs
   unsigned mNumInserts;            // number of insertions into the database
   unsigned mNumRemoves;            // number of removals from the database

   OsTimerDb(const OsTimerDb& rOsTimerDb);
     //:Copy constructor (not implemented for this class)

   OsTimerDb& operator=(const OsTimerDb& rhs);
     //:Assignment operator (not implemented for this class)

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

#endif  // _OsTimerDb_h_

