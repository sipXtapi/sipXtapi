//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _DialogEventPublisher_h_
#define _DialogEventPublisher_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <tao/TaoAdaptor.h>
#include <net/SipDialogEvent.h>
#include <net/SipPublishContentMgr.h>
#include <utl/UtlHashMap.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class CallManager;
class TaoString;

//! Class for publishing the dialog state change for each call
/**
 * This class keeps tracking the dialog state changes for each call and generates
 * a dialog event package as described in draft-ietf-sipping-dialog-package-06.txt
 * (An INVITE Initiated Dialog Event Package for SIP) and sends it to a generic
 * RFC 3265 SUBSCRIBE server or NOTIFIER.
 */

class DialogEventPublisher: public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   DialogEventPublisher(CallManager* callMgr, SipPublishContentMgr* contentMgr);
     //:Default constructor

   virtual
   ~DialogEventPublisher();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& eventMessage);

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    void dumpTaoMessageArgs(unsigned char eventId, TaoString& args);

    void insertEntry(UtlString& callId, SipDialogEvent* call);
    SipDialogEvent* getEntry(UtlString& callId);
    SipDialogEvent* removeEntry(UtlString& callId);
 
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    CallManager* mpCallManager;
    
    SipPublishContentMgr* mpSipPublishContentMgr;

    UtlHashMap mCalls;

    unsigned long mDialogId;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _DialogEventPublisher_h_
