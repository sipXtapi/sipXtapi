// $Id$
//
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _IStateTransitions_h_
#define _IStateTransitions_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "../stdwx.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class PhoneState;  // forward declaration
/**
 * Interface class.  Defines the transition method signatures.
 */
class IStateTransitions
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   virtual PhoneState* OnDial(const wxString phoneNumber) = 0;
   virtual PhoneState* OnRinging(const SIPX_CALL hCall)                 = 0;
   virtual PhoneState* OnRemoteAlerting()       = 0;
   virtual PhoneState* OnRemoteBusy()           = 0;
   virtual PhoneState* OnFlashButton()          = 0;
   virtual PhoneState* OnConnected()            = 0;
   virtual PhoneState* OnDisconnected(const SIPX_CALL hCall)    = 0;
   virtual PhoneState* OnOutgoingCallRejected() = 0;
   virtual PhoneState* OnError() = 0;
   virtual PhoneState* OnHoldButton() = 0;
   virtual PhoneState* OnTransferRequested(const wxString phoneNumber) = 0;
   virtual PhoneState* OnTransferComplete(const bool bFailure) = 0;
   virtual PhoneState* OnOffer(const SIPX_CALL hCall) = 0;


/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};
#endif
