// 
// 
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _RlsSubscribe_h_
#define _RlsSubscribe_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/SipSubscribeServerEventHandler.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// FORWARD DECLARATIONS
class SipMessage;
class UtlString;
class SipPublishContentMgr;

// TYPEDEFS

//! Specialize SipSubscribeServerEventHandler to enforce RLS policies.
/*! In particular, SUBSCRIBEs that do not support 'eventlist' are to be
 *  rejected.
 */
class RlsSubscribe : public SipSubscribeServerEventHandler
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    //! Default Dialog constructor
    RlsSubscribe();

    //! Destructor
    virtual
    ~RlsSubscribe();

/* ============================ MANIPULATORS ============================== */

    //! Determine if the given SUBSCRIBE request is authorized to subscribe
    /*! Only allowed if 'eventlist' is supported.
     */
    virtual UtlBoolean isAuthorized(const SipMessage& subscribeRequest,
                                    const UtlString& resourceId,
                                    const UtlString& eventTypeKey,
                                    SipMessage& subscribeResponse);

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    //! Copy constructor NOT ALLOWED
    RlsSubscribe(const RlsSubscribe& rRlsSubscribe);

    //! Assignment operator NOT ALLOWED
    RlsSubscribe& operator=(const RlsSubscribe& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _RlsSubscribe_h_
