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
#ifndef _PhoneStateMachineObserver_h_
#define _PhoneStateMachineObserver_h_


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "PhoneState.h"
#include "utl/UtlVoidPtr.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Base class for observing PhoneStateMachine transitions.
 * The PhoneStateMachine singleton keeps a collection of
 * PhoneStateMachineObserver objects which it has been
 * made aware of.
 * Derives from UtlVoidPointer, which is a 
 * UtlContainable. Provides a default implemenation for
 * the IStateTransitions interface. 
 */
class PhoneStateMachineObserver : public UtlVoidPtr, public IStateTransitions
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * PhoneStateMachineObserver contructor.
    */
   PhoneStateMachineObserver();

   /**
    * PhoneStateMachineObserver destructor.
    */
   virtual ~PhoneStateMachineObserver();


   virtual PhoneState* OnDial(const wxString phoneNumber)   { return NULL; }
   virtual PhoneState* OnRinging(SIPX_CALL hCall)     		{ return NULL; }
   virtual PhoneState* OnRemoteAlerting()	                { return NULL; }
   virtual PhoneState* OnRemoteBusy()		                { return NULL; }
   virtual PhoneState* OnFlashButton()		                { return NULL; }
   virtual PhoneState* OnConnected()		                { return NULL; }
   virtual PhoneState* OnDisconnected()                     { return NULL; }
   virtual PhoneState* OnOutgoingCallRejected()             { return NULL; }
   virtual PhoneState* OnError()                            { return NULL; }
   virtual PhoneState* OnHoldButton()                       { return NULL; }
   virtual PhoneState* OnTransferRequested(const wxString phoneNumber)      { return NULL; }
   virtual PhoneState* OnTransferComplete(const bool bFailure)              { return NULL; }

   virtual PhoneState* Execute() { return NULL; };

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};
#endif