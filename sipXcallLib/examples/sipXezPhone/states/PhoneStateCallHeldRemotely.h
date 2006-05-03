//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _PhoneStateCallHeldRemotely_h_
#define _PhoneStateCallHeldRemotely_h_

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


class PhoneStateCallHeldRemotely : public PhoneState
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * PhoneStateCallHeldRemotely contructor.
    */
   PhoneStateCallHeldRemotely();

   /**
    * PhoneStateCallHeldRemotely destructor.
    */
   virtual ~PhoneStateCallHeldRemotely();

   //virtual PhoneState* OnDial(const wxString phoneNumber);
   //virtual PhoneState* OnRinging();
   //virtual PhoneState* OnRemoteAlerting();
   //virtual PhoneState* OnRemoteBusy();
   virtual PhoneState* OnFlashButton();
   virtual PhoneState* OnConnected();
   virtual PhoneState* OnDisconnected(const SIPX_CALL hCall);
   virtual PhoneState* OnTransferRequested(const wxString phoneNumber);

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
