//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _SipMessageEvent_h_
#define _SipMessageEvent_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <net/SipMessage.h>
#include <os/OsMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipMessageEvent : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

        enum MessageStatusTypes
        {
                APPLICATION = 0,
                TRANSPORT_ERROR = 1,
        AUTHENTICATION_RETRY,
        SESSION_REINVITE_TIMER,
        TRANSACTION_GARBAGE_COLLECTION,
        TRANSACTION_RESEND,
        TRANSACTION_EXPIRATION
        };

/* ============================ CREATORS ================================== */

   SipMessageEvent(SipMessage* message = NULL, int status = APPLICATION);
     //:Default constructor


   virtual
   ~SipMessageEvent();
     //:Destructor

   virtual OsMsg* createCopy(void) const;
/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */
const SipMessage* getMessage();

void setMessageStatus(int status);
int getMessageStatus() const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        SipMessage* sipMessage;
        int messageStatus;

   SipMessageEvent(const SipMessageEvent& rSipMessageEvent);
     //:disable Copy constructor

   SipMessageEvent& operator=(const SipMessageEvent& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipMessageEvent_h_
