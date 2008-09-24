//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsSyncBase_h_
#define _OsSyncBase_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"

// DEFINES
// If OS_SYNC_DEBUG, enable debugging information for binary semaphores and
// mutexes.
// #define OS_SYNC_DEBUG

#ifdef OS_SYNC_DEBUG
#  include "os/OsDateTime.h"
#endif

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

/**
 * @brief Base class for the synchronization mechanisms in the OS abstraction layer
 */
class OsSyncBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

     /// Destructor
   virtual
      ~OsSyncBase() { };

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

     /// Assignment operator
   OsSyncBase& operator=(const OsSyncBase& rhs);

     /// Block until the sync object is acquired or the timeout expires
   virtual OsStatus acquire(const OsTime& rTimeout = OsTime::OS_INFINITY) = 0;

     /// Conditionally acquire the semaphore (i.e., don't block)
   virtual OsStatus tryAcquire(void) = 0;
     /**
      * @return OS_BUSY if the sync object is held by some other task.
      */

     /// Release the sync object
   virtual OsStatus release(void) = 0;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

#ifdef OS_SYNC_DEBUG
     /// Operations on an OsSyncBase object
   typedef enum
   {
      crumbUnused,   ///< array slot not yet used
      crumbCreated,  ///< OsSyncBase constructed
      crumbAcquired, ///< OsSyncBase acquired
      crumbReleased, ///< OsSyncBase released
      crumbDeleted   ///< OsSyncBase deleted
   } OsSyncOperation;
#endif

#  ifdef OS_SYNC_DEBUG

   /// Track usage of any OsSyncBase object
   class OsSyncCrumbs
   {
#    define NUMBER_OF_CRUMBS 6

     public:

      /// Constructor to initialize crumb trail
      OsSyncCrumbs() :
         mCrumb(0)
      {
         mTrail[mCrumb].operation = crumbCreated;
         mTrail[mCrumb].taskId = 0; // default; parent constructor should call dropCrumb
         for ( unsigned int crumb=1; crumb < NUMBER_OF_CRUMBS; crumb++ )
         {
            mTrail[crumb].operation = crumbUnused;
            mTrail[crumb].taskId    = 0;
         }
      }
      
      /// record the task id and operation in the mTrail circular buffer
      void dropCrumb(int id, OsSyncOperation op)
      {
         mCrumb = (mCrumb + 1) % NUMBER_OF_CRUMBS;
         mTrail[mCrumb].operation = op;
         mTrail[mCrumb].taskId    = id;
         OsDateTime::getCurTime(mTrail[mCrumb].time);
      }
      
      ~OsSyncCrumbs()
      {
         // better if the destructor in the object calls, but make sure there is something.
         dropCrumb(0, crumbDeleted);
      }
      
     private:

      unsigned int mCrumb;  ///< circular index into mTrail: most recently used entry
      struct
      {         
         OsSyncOperation operation; ///< operation on the syncronizer
         unsigned int    taskId;    ///< the ID of the task that touched the syncronizer
         OsTime          time;      ///< when the operation happened
      } mTrail[NUMBER_OF_CRUMBS];
   } mSyncCrumbs;
   
#  endif

	 /// Default constructor
   OsSyncBase() { };

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor
   OsSyncBase(const OsSyncBase& rOsSyncBase);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsSyncBase_h_

