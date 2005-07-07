// $Id$
//
// Copyright (C) 2005 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPXTAPI_EXCLUDE /* [ */

#ifndef _SipXHandleMap_h_
#define _SipXHandleMap_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsMutex.h"
#include "utl/UtlHashMap.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef unsigned int SIPXHANDLE ;
// FORWARD DECLARATIONS

/**
 * SipXHandleMap provides a very simple container that associates a void* 
 * with a handle value.  The handle value is a unique incrementing number.
 * In theory, we could get collisions if the numbers wrap, however, this
 * is not designed for that type of call volume (millions of call per 
 * hour?)
 * <p>
 * Generally, use the allocHandle, removeHandle, and findHandle methods.
 * lock() and unlock() methods are also provided for external iterators.
 */
class SipXHandleMap : public UtlHashMap
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   SipXHandleMap();
     
   /**
    * Destructor
    */
   virtual ~SipXHandleMap();

/* ============================ MANIPULATORS ============================== */

    /**
     * Lock/guard access to the allocHandle, findHandle, and removeHandle
     * routines.  This is called automatically for those routines, however,
     * should be called explicitly if using an external iterator on the map.
     */
    void lock() ;

    /**
     * Unlock access to the allocHandle, findHandle, and removeHandle
     * routines.  This is called automatically for those routines, however,
     * should be called explicitly if using an external iterator on the map.
     */
    void unlock() ;

    /**
     * Allocate a unique handle and associate the designed pData value
     * with that handle.
     *
     * @param pData Data to be associated with the newly allocated handle
     */ 
    SIPXHANDLE allocHandle(const void* pData) ;


    /**
     * Find the data associated with the designated handle and return it.
     */ 
    const void* findHandle(SIPXHANDLE handle) ;

    /**
     * Remove the handle and data assoicated with it from the map.
     */ 
    const void* removeHandle(SIPXHANDLE handle) ;

/* ============================ ACCESSORS ================================= */
    

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
	OsMutex    mLock ;       /**< Locked used for addEntry and removeEntry */
    SIPXHANDLE mNextHandle ; /**< Next available handle index */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipXHandleMap_h_

#endif /* ] SIPXTAPI_EXCLUDE */

