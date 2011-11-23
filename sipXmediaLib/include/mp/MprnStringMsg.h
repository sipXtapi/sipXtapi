//  
// Copyright (C) 2007-2011 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprnStringMsg_h_
#define _MprnStringMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "utl/UtlString.h"
#include "MpResNotificationMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message notification object used to send an abstract integer.
class MprnStringMsg : public MpResNotificationMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprnStringMsg(MpResNotificationMsg::RNMsgType msgType,
              const UtlString& namedResOriginator,
              const UtlString& value,
              MpConnectionID connId = MP_INVALID_CONNECTION_ID,
              int streamId = -1);

     /// Copy constructor
   MprnStringMsg(const MprnStringMsg& rMsg);

     /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy() const;

     /// Destructor
   virtual ~MprnStringMsg();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MprnStringMsg& operator=(const MprnStringMsg& rhs);

     /// Set the value this notification reports.
   void setValue(const UtlString& value);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get the value this notification reports.
   void getValue(UtlString& value) const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlString mValue; ///< Reported value.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprnStringMsg_h_
