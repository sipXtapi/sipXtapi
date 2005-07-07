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
#ifndef _PhoneState_h_
#define _PhoneState_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "IStateTransitions.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Abstract Base class from which all PhoneStates are derived.
 * Implements default methods for the transitions.
 */
class PhoneState : public IStateTransitions
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * PhoneState contructor.
    */
   PhoneState();

   /**
    * PhoneState destructor.
    */
   virtual ~PhoneState();

   virtual PhoneState* OnDial(const wxString phoneNumber)   { return this; }
   virtual PhoneState* OnRinging(SIPX_CALL hCall)     		{ return this; }
   virtual PhoneState* OnRemoteAlerting()	{ return this; }
   virtual PhoneState* OnRemoteBusy()		{ return this; }
   virtual PhoneState* OnFlashButton()		{ return this; }
   virtual PhoneState* OnConnected()		{ return this; }
   virtual PhoneState* OnDisconnected();
   virtual PhoneState* OnOutgoingCallRejected() { return this; }
   virtual PhoneState* OnError()          { return this; }
   virtual PhoneState* OnHoldButton()       { return this; }
   virtual PhoneState* OnTransferRequested(const wxString phoneNumber)   { return this; }
   virtual PhoneState* OnTransferComplete(const bool bFailure) { return this; }

   virtual PhoneState* Execute() { return NULL; };


/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   SIPX_CALL mhCall;
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};
#endif