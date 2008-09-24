//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _PhoneStateTransferRequested_h_
#define _PhoneStateTransferRequested_h_

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


class PhoneStateTransferRequested : public PhoneState
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * PhoneStateTransferRequested contructor.
    */
   PhoneStateTransferRequested(const wxString phoneNumber);

   /**
    * PhoneStateTransferRequested destructor.
    */
   virtual ~PhoneStateTransferRequested();

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
    wxString mPhoneNumber;
};
#endif
