//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
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
   virtual PhoneState* OnDial(const wxString phoneNumber) {return NULL;}
   virtual PhoneState* OnRinging(const SIPX_CALL hCall)                 {return NULL;}
   virtual PhoneState* OnRemoteAlerting()       {return NULL;}
   virtual PhoneState* OnRemoteBusy()           {return NULL;}
   virtual PhoneState* OnFlashButton()          {return NULL;}
   virtual PhoneState* OnHoldButton()           {return NULL;}
   virtual PhoneState* OnConnected()            {return NULL;}
   virtual PhoneState* OnDisconnected(const SIPX_CALL hCall)    {return NULL;}
   virtual PhoneState* OnOutgoingCallRejected() {return NULL;}
   virtual PhoneState* OnError() {return NULL;}
   virtual PhoneState* OnTransferRequested(const wxString phoneNumber) {return NULL;}
   virtual PhoneState* OnTransferComplete(const bool bFailure) {return NULL;}
   virtual PhoneState* OnOffer(const SIPX_CALL hCall) {return NULL;}
   virtual PhoneState* OnConnectedInactive() {return NULL;}
   virtual PhoneState* OnLocalHoldRequested() {return NULL;}
   virtual PhoneState* Execute() {return NULL;}


/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif
