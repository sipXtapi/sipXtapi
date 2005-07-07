// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipPublishServerEventStateMgr_h_
#define _SipPublishServerEventStateMgr_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <os/OsDefs.h>
#include <os/OsMsgQ.h>
#include <os/OsMutex.h>
#include <utl/UtlDefs.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlHashBag.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// FORWARD DECLARATIONS
class SipMessage;
class UtlString;

// TYPEDEFS

//! Class for maintaining PUBLISH event state information in publish server
/*! 
 *
 * \par 
 */
class SipPublishServerEventStateMgr
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:



/* ============================ CREATORS ================================== */

    //! Default constructor
    SipPublishServerEventStateMgr();


    //! Destructor
    virtual
    ~SipPublishServerEventStateMgr();


/* ============================ MANIPULATORS ============================== */
                 
    //! Generate a new entity tag
    virtual void generateETag(UtlString& entity);

    //! Add a new publication
    void addPublish(UtlString& entity, UtlString& resourceId,
                    UtlString& eventTypeKey, int expiration);

    //! Update the publication
    void updatePublish(UtlString& oldEntity, UtlString& entity, UtlString& resourceId,
                       UtlString& eventTypeKey, int expiration);
                    
    //! Remove old publication
    void removePublish(UtlString& entity);

    //! Remove old publication that expired before given date
    void removeOldPublication(long oldEpochTimeSeconds);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

    //! inquire if the publish exists
    virtual UtlBoolean publishExists(UtlString& entityTag);

    //! inquire if the publish has already expired
    virtual UtlBoolean isExpired(UtlString& entityTag);

    //! inquire if the expiration is correct
    virtual UtlBoolean checkExpiration(int* expiration);
    
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    //! Copy constructor NOT ALLOWED
    SipPublishServerEventStateMgr(const SipPublishServerEventStateMgr& rSipPublishServerEventStateMgr);

    //! Assignment operator NOT ALLOWED
    SipPublishServerEventStateMgr& operator=(const SipPublishServerEventStateMgr& rhs);

    //! lock for single thread use
    void lock();

    //! unlock for use
    void unlock();

    OsMutex mEventStateMgrMutex;
    int mMinExpiration;
    int mDefaultExpiration;
    int mMaxExpiration;

    // Container for the event states
    UtlHashMap mEventStatesByEntityTag;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipPublishServerEventStateMgr_h_
