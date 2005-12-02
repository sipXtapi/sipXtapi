//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

#ifdef TEST
#include <utl/UtlMemCheck.h>
#endif

// APPLICATION INCLUDES
#include <os/OsTimerDb.h>
#include <os/OsBSem.h>
#include <os/OsTimer.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlInt.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Database of active timers.
// The OsTimerDb maintains a database of active timers (i.e., those timers
// that have been started by the low level OsSysTimer class).  Since the
// OsTimerTask is the only task that should be accessing the timer database
// there is no need to serialize access (and no locking).
// 
// Each entry in the database is a key/value pair where the key corresponds
// to a timer ID and the value is the pointer to the corresponding OsTimer
// object.  Duplicate keys are not allowed.

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsTimerDb::OsTimerDb()
:  mNumInserts(0), mNumRemoves(0)
{
#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      test();
   }
#endif //TEST

   // since we plan to store OsTimer pointers in the
   //  database, we make sure the sizes are compatible
   assert(sizeof(OsTimer*) <= sizeof(int));

   // no other work required
}

// Destructor
OsTimerDb::~OsTimerDb()
{
   mDict.destroyAll();   
}

/* ============================ MANIPULATORS ============================== */

// Insert the indicated timer into the database of active timers.
// Return OS_SUCCESS if successful, OS_NAME_IN_USE if the key is
// already in the database.
OsStatus OsTimerDb::insert(int timerId, OsTimer* pTimer)
{
   UtlInt* pDictKey;
   UtlInt* pDictValue;
   UtlInt* pInsertedKey;

   pDictKey   = new UtlInt(timerId);
   pDictValue = new UtlInt((int) pTimer);

   pInsertedKey = (UtlInt*)
                  mDict.insertKeyAndValue(pDictKey, pDictValue);

   if (pInsertedKey == NULL)
   {                             // insert failed
      delete pDictKey;           // clean up the key and value objects
      delete pDictValue;

      return OS_NAME_IN_USE;
   }
   else
   {
      mNumInserts++;
      return OS_SUCCESS;
   }
}

// Remove the indicated timer from the database of active timers.
// Return OS_SUCCESS if the indicated timerId is found, return
// OS_NOT_FOUND if there is no match for the specified key.
OsStatus OsTimerDb::remove(int timerId)
{
   UtlInt* pLookupKey;
   UtlInt* pDictKey;
   UtlInt* pDictValue;

   pLookupKey = new UtlInt(timerId);
   pDictKey   = (UtlInt*)
                mDict.removeKeyAndValue(pLookupKey,
                                        (UtlContainable*&) pDictValue);
   delete pLookupKey;

   if (pDictKey == NULL)
      return OS_NOT_FOUND;   // did not find the specified key
   else
      mNumRemoves++;

   delete pDictKey;          // before returning we need to destroy the
   delete pDictValue;        //  objects that were used to maintain the
                             //  dictionary entry
   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

// Get an array of pointers to the timers that are currently active.
// The caller provides an array that can hold up to "size" OsTimer
// pointers. This method will fill in the "activeTimers" array with
// up to "size" pointers. The method returns the number of pointers
// in the array that were actually filled in.
int OsTimerDb::getActiveTimers(OsTimer* activeTimers[], int size)
{
   UtlHashMapIterator iter(mDict);
   UtlContainable*    next;
   UtlInt* value;
   int      i;

   iter.reset();
   i = 0;
   while (next = iter())
   {
      if (i >= size) break;

      value = (UtlInt*) iter.value();
      activeTimers[i] = (OsTimer*) value->getValue();
      i++;
   }

   return i;
}

// Get the number of insertions and removals for the timer database.
void OsTimerDb::getDbStats(unsigned& nInserts, unsigned& nRemoves) const
{
   nInserts = mNumInserts;
   nRemoves = mNumRemoves;
}

// Return the number of key-value pairs in the timer database.
int OsTimerDb::numEntries(void) const
{
   return mDict.entries();
}

/* ============================ INQUIRY =================================== */

// Return TRUE if the timer database is empty.
UtlBoolean OsTimerDb::isEmpty(void) const
{
   return (numEntries() == 0);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool OsTimerDb::sIsTested = false;

// Test this class by running all of its assertion tests
void OsTimerDb::test()
{
   UtlMemCheck* pMemCheck = 0;
   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   testCreators();
   testManipulators();
   testAccessors();
   testInquiry();

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the creators (and destructor) methods for the class
void OsTimerDb::testCreators()
{
   unsigned     nInserts;
   unsigned     nRemoves;
   UtlMemCheck* pMemCheck  = 0;
   OsTimerDb*   pTimerDb;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   pTimerDb = new OsTimerDb();
   pTimerDb->getDbStats(nInserts, nRemoves);
   assert(pTimerDb->isEmpty() == TRUE);
   assert(pTimerDb->numEntries() == 0);
   assert(nInserts == 0 && nRemoves == 0);
   delete pTimerDb;

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the manipulator methods
void OsTimerDb::testManipulators()
{
   UtlMemCheck* pMemCheck  = 0;
   UtlMemCheck* pMemCheck2 = 0;
   unsigned     nInserts;
   unsigned     nRemoves;
   OsTimerDb*   pTimerDb;
   OsStatus     res;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check
   pTimerDb = new OsTimerDb();

   // test the insert() method

   // insert a timerId that is not yet in the database
   res = pTimerDb->insert(1, (OsTimer*) 0x11111);
   assert(res == OS_SUCCESS);
   assert(pTimerDb->isEmpty() == FALSE);
   assert(pTimerDb->numEntries() == 1);
   pTimerDb->getDbStats(nInserts, nRemoves);
   assert(nInserts == 1 && nRemoves == 0);

   // remove a timerId that is in the database
   res = pTimerDb->remove(1);
   assert(res == OS_SUCCESS);
   assert(pTimerDb->isEmpty() == TRUE);
   assert(pTimerDb->numEntries() == 0);
   pTimerDb->getDbStats(nInserts, nRemoves);
   assert(nInserts == 1 && nRemoves == 1);

   // The previous operations should have allocated whatever memory is needed
   // in the hash dictionary for hash buckets.  Now make sure we aren't
   // leaking.

   pMemCheck2 = new UtlMemCheck();     // checkpoint for memory leak check

   // insert a timerId that is not yet in the database
   res = pTimerDb->insert(1, (OsTimer*) 0x11111);
   assert(res == OS_SUCCESS);
   assert(pTimerDb->isEmpty() == FALSE);
   assert(pTimerDb->numEntries() == 1);
   pTimerDb->getDbStats(nInserts, nRemoves);
   assert(nInserts == 2 && nRemoves == 1);

   // insert a timerId that is already in the database
   res = pTimerDb->insert(1, (OsTimer*) 0x11111);
   assert(res == OS_NAME_IN_USE);
   assert(pTimerDb->isEmpty() == FALSE);
   assert(pTimerDb->numEntries() == 1);
   pTimerDb->getDbStats(nInserts, nRemoves);
   assert(nInserts == 2 && nRemoves == 1);

   // remove a timerId that is in the database
   res = pTimerDb->remove(1);
   assert(res == OS_SUCCESS);
   assert(pTimerDb->isEmpty() == TRUE);
   assert(pTimerDb->numEntries() == 0);
   pTimerDb->getDbStats(nInserts, nRemoves);
   assert(nInserts == 2 && nRemoves == 2);

   assert(pMemCheck2->delta() == 0);   // check for memory leak
   delete pMemCheck2;

   // Now test adding more than one entry and removing non-existent entries

   // insert a timerId that is not yet in the database
   res = pTimerDb->insert(1, (OsTimer*) 0x11111);
   assert(res == OS_SUCCESS);
   assert(pTimerDb->isEmpty() == FALSE);
   assert(pTimerDb->numEntries() == 1);
   pTimerDb->getDbStats(nInserts, nRemoves);
   assert(nInserts == 3 && nRemoves == 2);

   // insert another timerId that is not yet in the database
   res = pTimerDb->insert(2, (OsTimer*) 0x22222);
   assert(res == OS_SUCCESS);
   assert(pTimerDb->isEmpty() == FALSE);
   assert(pTimerDb->numEntries() == 2);
   pTimerDb->getDbStats(nInserts, nRemoves);
   assert(nInserts == 4 && nRemoves == 2);

   // remove a timerId that is in the database
   res = pTimerDb->remove(1);
   assert(res == OS_SUCCESS);
   assert(pTimerDb->isEmpty() == FALSE);
   assert(pTimerDb->numEntries() == 1);
   pTimerDb->getDbStats(nInserts, nRemoves);
   assert(nInserts == 4 && nRemoves == 3);

   // remove a timerId that is not in the database
   res = pTimerDb->remove(1);
   assert(res == OS_NOT_FOUND);
   assert(pTimerDb->isEmpty() == FALSE);
   assert(pTimerDb->numEntries() == 1);
   pTimerDb->getDbStats(nInserts, nRemoves);
   assert(nInserts == 4 && nRemoves == 3);

   // remove the last timerId in the database
   res = pTimerDb->remove(2);
   assert(res == OS_SUCCESS);
   assert(pTimerDb->isEmpty() == TRUE);
   assert(pTimerDb->numEntries() == 0);
   pTimerDb->getDbStats(nInserts, nRemoves);
   assert(nInserts == 4 && nRemoves == 4);

   // Delete the timer database
   delete pTimerDb;

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the accessor methods for the class
void OsTimerDb::testAccessors()
{
   UtlMemCheck* pMemCheck  = 0;
   OsTimer*     activeTimers[10];
   OsTimerDb*   pTimerDb;
   OsStatus     res;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // The numEntries() and getDbStats() methods were tested as a side effect
   // of other testing in testManipulators()

   // test the getActiveTimers() method

   pTimerDb = new OsTimerDb();

   // insert two timers into the database
   res = pTimerDb->insert(1, (OsTimer*) 0x11111);
   assert(res == OS_SUCCESS);

   res = pTimerDb->insert(2, (OsTimer*) 0x22222);
   assert(res == OS_SUCCESS);

   // try calling getActiveTimers with a zero length array
   assert(pTimerDb->getActiveTimers(activeTimers, 0)  == 0);

   // try calling getActiveTimers with an array of size 1
   assert(pTimerDb->getActiveTimers(activeTimers, 1)  == 1);
   assert(activeTimers[0] == (OsTimer*) 0x11111 ||
          activeTimers[0] == (OsTimer*) 0x22222);

   // try calling getActiveTimers with an array of size 2
   assert(pTimerDb->getActiveTimers(activeTimers, 2)  == 2);
   assert(activeTimers[0] == (OsTimer*) 0x11111 ||
          activeTimers[0] == (OsTimer*) 0x22222);
   if (activeTimers[0] == (OsTimer*) 0x11111)
      assert(activeTimers[1] == (OsTimer*) 0x22222);
   else
      assert(activeTimers[1] == (OsTimer*) 0x11111);

   // try calling getActiveTimers with an array bigger 
   assert(pTimerDb->getActiveTimers(activeTimers, 10) == 2);

   // Delete the timer database.
   delete pTimerDb;

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the inquiry methods for the class
void OsTimerDb::testInquiry()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // body of the test goes here

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

#endif //TEST

/* ============================ FUNCTIONS ================================= */


