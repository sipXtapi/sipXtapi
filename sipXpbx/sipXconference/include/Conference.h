//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _Conference_h_
#define _Conference_h_

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

#include "ConferenceController.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Leg;

class Conference : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   static UtlContainableType TYPE ;    /** < Class type used for runtime checking */ 

   UtlString mUserName;

   ConferenceController* mConferenceController;

   SIPX_LINE mSipxLine;

   SIPX_CONF mSipxConference;

    // A list to hold all the legs.
    UtlSList mLegs;

/* ============================ CREATORS ================================== */

   //! Default constructor
   Conference(ConferenceController* parent,
              UtlString& user_name);

   //! Destructor
   ~Conference();

/* ============================ MANIPULATORS ============================== */

Leg* handleNewCall(SIPX_CALL hCall,
                   SIPX_CALLSTATE_MINOR eMinor);

/* ============================ ACCESSORS ================================= */

   // To implement containability.
   virtual UtlContainableType getContainableType() const;
   virtual unsigned hash() const;
   virtual int compareTo(const UtlContainable* otherObject) const;    

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _Conference_h_
