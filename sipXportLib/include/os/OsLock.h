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


#ifndef _OsLock_h_
#define _OsLock_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "OsSyncBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * @brief Lock class for mutual exclusion in a critical section.
 *
 * This class uses OsSyncBase objects for synchronization.
 * The constructor for the class automatically blocks until the designated
 * semaphore is acquired. Similarly, the destructor automatically releases
 * the lock. The easiest way to use this object as a guard for a critical
 * section is to create the object as a variable on the stack just before
 * the critical section. When the variable goes out of scope, the lock will
 * be automatically released. An example of this form of use is shown below.
 * 
 * <code>
 * &nbsp;&nbsp;                      someMethod()                    <br>
 * &nbsp;&nbsp;                      {                               <br>
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      OsLock lock(myBSemaphore);    <br>
 *                                                                   <br>
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      < critical section >          <br>
 * &nbsp;&nbsp;                      }                               
 * </code>
 */
class OsLock
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

     /// Constructor
   OsLock(OsSyncBase& rSemaphore)
   : mrSemaphore(rSemaphore) { rSemaphore.acquire(); };

     /// Destructor
   virtual
   ~OsLock()  { mrSemaphore.release(); };

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsSyncBase& mrSemaphore;

     /// Default constructor (not implemented for this class)
   OsLock();

     /// Copy constructor (not implemented for this class)
   OsLock(const OsLock& rOsLock);

     /// Assignment operator (not implemented for this class)
   OsLock& operator=(const OsLock& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsLock_h_

