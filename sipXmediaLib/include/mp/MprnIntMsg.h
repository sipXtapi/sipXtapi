//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprnIntMsg_h_
#define _MprnIntMsg_h_

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
class MprnIntMsg : public MpResNotificationMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprnIntMsg(MpResNotificationMsg::RNMsgType msgType,
              const UtlString& namedResOriginator,
              int value,
              MpConnectionID connId = MP_INVALID_CONNECTION_ID,
              int streamId = -1);

     /// Copy constructor
   MprnIntMsg(const MprnIntMsg& rMsg);

     /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy() const;

     /// Destructor
   virtual ~MprnIntMsg();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MprnIntMsg& operator=(const MprnIntMsg& rhs);

     /// Set the value this notification reports.
   void setValue(int value);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get the value this notification reports.
   int getValue() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int mValue; ///< Reported value.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprnIntMsg_h_
