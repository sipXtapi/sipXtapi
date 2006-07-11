//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
#ifndef _PhoneStateLocalHoldRequested_h_
#define _PhoneStateLocalHoldRequested_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "PhoneState.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


class PhoneStateLocalHoldRequested : public PhoneState
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * PhoneStateLocalHoldRequested contructor.
    */
   PhoneStateLocalHoldRequested(SIPX_CALL hCall);

   /**
    * PhoneStateLocalHoldRequested destructor.
    */
   virtual ~PhoneStateLocalHoldRequested();

   virtual PhoneState* OnConnectedInactive();
   //virtual PhoneState* OnDial(const wxString phoneNumber);
   //virtual PhoneState* OnRinging();
   //virtual PhoneState* OnRemoteAlerting();
   //virtual PhoneState* OnRemoteBusy();
   //virtual PhoneState* OnFlashButton();
   //virtual PhoneState* OnConnected();
   //virtual PhoneState* OnDisconnected();

   virtual PhoneState* Execute();


/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif
