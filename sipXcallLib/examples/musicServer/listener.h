// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
////////////////////////////////////////////////////////////////////////////

#ifndef _Listener_h_
#define _Listener_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <tao/TaoAdaptor.h>

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

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class Listener : public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   Listener(CallManager* callManager = NULL, UtlString playfile = "default.wav");
     //:Default constructor

/* ============================ MANIPULATORS ============================== */

   UtlBoolean handleMessage(OsMsg& rMsg);
     //: Method to process messages which get queued on this
     /**  The is the method that does all the work.  Telephony
       *  events get queued on this which consequently get
       *  processed in handleMessage in the contect of this
       *  task.
       */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    UtlString chooseAudioGreeting(UtlString callId, UtlString address);
    void dumpTaoMessageArgs(unsigned char eventId, TaoString& args);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlString mPlayfile;
    
    Listener(const Listener& rListener);
     //:Copy constructor

    Listener& operator=(const Listener& rhs);
     //:Assignment operator

    CallManager* mpCallManager;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _Listener_h_
