//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
#ifndef _PhoneStateConnected_h_
#define _PhoneStateConnected_h_

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


class PhoneStateConnected : public PhoneState
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * PhoneStateConnected contructor.
    */
   PhoneStateConnected();

   /**
    * PhoneStateConnected destructor.
    */
   virtual ~PhoneStateConnected();


   virtual PhoneState* OnFlashButton();
   virtual PhoneState* OnDisconnected(const SIPX_CALL hCall);
   virtual PhoneState* OnHoldButton();
   virtual PhoneState* OnTransferRequested(const wxString phoneNumber);
   virtual PhoneState* OnConnectedInactive();

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
