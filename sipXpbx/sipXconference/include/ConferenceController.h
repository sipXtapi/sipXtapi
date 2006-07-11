//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _ConferenceController_h_
#define _ConferenceController_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsServerTask.h>
#include <os/OsRWMutex.h>
#include "os/OsProtectEvent.h"
#include "os/OsQueuedEvent.h"

#include "ptapi/PtEvent.h"
#include "ptapi/PtDefs.h"
#include "net/SipMessage.h"
#include "net/SipContactDb.h"
#include "net/SipDialog.h"
#include "cp/Connection.h"
#include <tapi/sipXtapi.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Conference;
class Leg;

// class ConferenceController is the master object of the conference bridge
// and/or the master management task for the conference bridge.

class ConferenceController : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

    // The sipXtapi user agent instance for receiving calls.
    SIPX_INST mSipxInstance;

    // A list to hold all the conferences.
    UtlSList mConferences;

    // The unique conference in the bridge.
    Conference* mConference;

    // The domain or host-port for the conference URIs.
    UtlString mDomainHostPort;

    // The index mapping call handles to Leg objects.
    UtlHashMap mIndex;

/* ============================ CREATORS ================================== */

   //! Default constructor
   ConferenceController(int udpPort,
                        int tcpPort,
                        int rtpBase,
                        int maxSessions,
                        UtlString& domain);

   //! Destructor
   ~ConferenceController();

/* ============================ MANIPULATORS ============================== */

    virtual UtlBoolean handleMessage(OsMsg& eventMessage);

    virtual void requestShutdown(void);

    // Tapi event listener.
    // The event listener function has to be static, as sipxListenerAdd
    // cannot take a member function.
    static void eventListener(SIPX_CALL hCall,
                              SIPX_LINE hLine,
                              SIPX_CALLSTATE_MAJOR eMajor,
                              SIPX_CALLSTATE_MINOR eMinor,  
                              void* pUserData);

    void indexAddCall(SIPX_CALL hCall,
                      Leg* leg);

    void indexDeleteCall(SIPX_CALL hCall);

    Leg* indexLookupCall(SIPX_CALL hCall);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

    // Semaphore to serialize access to the conference control structures.
    // Externally activated operations (e.g., management requests) can
    // acquire mMutex and do what they want.  Operations that may be
    // stimulated from within control actions (especially sipXtapi callback
    // events) must be packaged as messages on the ConferenceController's
    // message queue to be processed asynchronously.
    OsBSem mMutex;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _ConferenceController_h_
