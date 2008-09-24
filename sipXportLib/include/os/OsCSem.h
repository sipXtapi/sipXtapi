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

#ifndef _OsCSem_h_
#define _OsCSem_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSyncBase.h"
#include "os/OsBSem.h"

// DEFINES

//Uncomment next line to see semaphore errors when they occur
//#define OS_CSEM_DEBUG

//Uncomment next line (as well as above line) to see all acquires and releases.
//#define OS_CSEM_DEBUG_SHOWALL

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Counting semaphore
class OsCSemBase : public OsSyncBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum QueueOptions
   {
      Q_FIFO     = 0x0, ///< queue blocked tasks on a first-in, first-out basis
      Q_PRIORITY = 0x1  ///< queue blocked tasks based on their priority
   };

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

     /// Block the task until the semaphore is acquired or the timeout expires
   virtual OsStatus acquire(const OsTime& rTimeout = OsTime::OS_INFINITY) = 0;

     /// Conditionally acquire the semaphore (i.e., don't block)
   virtual OsStatus tryAcquire(void) = 0;
     /**
      * @return OS_BUSY if the semaphore is held by some other task.
      */

     /// Release the semaphore
   virtual OsStatus release(void) = 0;


/* ============================ ACCESSORS ================================= */

#ifdef OS_CSEM_DEBUG
     /// Print statistics gathered
   virtual void show(void) ;

     /// Returns the current value of the semaphone
   virtual int getValue(void) ;
#endif

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

#ifdef OS_CSEM_DEBUG
   OsBSem       mGuard;         ///< Guard to protect the statistics
                                ///< used for debugging
   int          mQueueOptions;
   int          mInitialCount;
   int          mMaxCount;
   int          mCurCount;
   int          mHighCount;
   int          mLowCount;
   int          mNumAcquires;
   int          mNumReleases;
#endif

     /// Default constructor
   OsCSemBase(const int queueOptions, const int maxCount,
                     const int initCount) ;

     /// Destructor
   virtual ~OsCSemBase()  {  };

#ifdef OS_CSEM_DEBUG
     /// Update the statistics associated with acquiring a counting semaphore
   void updateAcquireStats(void);

     /// Update the statistics associated with releasing a counting semaphore
   void updateReleaseStats(void);
#endif

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


     /// Copy constructor (not implemented for this class)
   OsCSemBase(const OsCSemBase& rOsCSemBase);

     /// Assignment operator (not implemented for this class) 
   OsCSemBase& operator=(const OsCSemBase& rhs);

};

/* ============================ INLINE METHODS ============================ */

/// Depending on the native OS that we are running on, we include the class
/// declaration for the appropriate lower level implementation and use a
/// "typedef" statement to associate the OS-independent class name (OsCSem)
/// with the OS-dependent realization of that type (e.g., OsCSemWnt).
#if defined(_WIN32)
#  include "os/Wnt/OsCSemWnt.h"
   typedef class OsCSemWnt OsCSem;
#elif defined(_VXWORKS)
#  include "os/Vxw/OsCSemVxw.h"
   typedef class OsCSemVxw OsCSem;
#elif defined(__pingtel_on_posix__)
#  include "os/linux/OsCSemLinux.h"
   typedef class OsCSemLinux OsCSem;
#else
#  error Unsupported target platform.
#endif

#endif  // _OsCSem_h_

