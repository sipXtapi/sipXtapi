//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _TaoEventListener_h_
#define _TaoEventListener_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "ptapi/PtEvent.h"
#include "tao/TaoDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Base class for listener objects.

class TaoEventListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   TaoEventListener(const char* pTerminalName, int* pEventMask = NULL);
     //!param: (in) pTerminalName - name of listening terminal.
         //!param: (in) pMask - Event mask defining events the listener is interested in.  This must be a subset of the events that the listener supports.  The mask may be NULL where it is assumed that all events applicable to the derived listener are of interest.

   virtual
   ~TaoEventListener();
     //:Destructor

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */

   TaoStatus getEventMask(const int*& rpMask);
     //:Returns the event mask that defines the events of interest
     // The event mask is read only.  Unpredictable results will occur if
     // the event mask is modified after a listener is constructed.
     // The mask may be NULL where it is assumed that all events applicable
     // to the derived listener are of interest.
     //!param: (out) rpMask - Reference to a pointer to the event mask.
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus getTerminalName(char* rpTerminalName, int maxLen);
     //:Returns the listening terminal name
     // The event mask is read only.  Unpredictable results will occur if
     // the event mask is modified after a listener is constructed.
     // The mask may be NULL where it is assumed that all events applicable
     // to the derived listener are of interest.
     //!param: (out) rpTerminalName - Reference to a pointer to the listening terminal name.
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

   UtlBoolean isEventEnabled(PtEvent::PtEventId& eventId);
     //:Determines if the given event type is of interest to this listener.
     //!param: (in) eventId - The event id corresponding to the event type
     //!retcode: TRUE - If the given event type is of interest to this listener
     //!retcode: FALSE - If the given event type is NOT of interest to this listener

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        char*   mpTerminalName;
        int*    mpEventMask;

        TaoEventListener(const TaoEventListener& rTaoEventListener);
         //:Copy constructor

        TaoEventListener& operator=(const TaoEventListener& rhs);
     //:Assignment operator


};

/* ============================ INLINE METHODS ============================ */

#endif  // _TaoEventListener_h_
