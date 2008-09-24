//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _PhoneStateMachine_h_
#define _PhoneStateMachine_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "PhoneState.h"
#include "PhoneStateMachineObserver.h"
#include "utl/UtlSList.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Holds a reference to the current state.  Drives the state transitions.
 */
class PhoneStateMachine : public IStateTransitions
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   static PhoneStateMachine& getInstance();
   
/* ============================ MANIPULATORS ============================== */
   void addObserver(const PhoneStateMachineObserver* pObserver);
   void removeObserver(const PhoneStateMachineObserver* pObserver);
   PhoneState* getState() const;
   void setState(const PhoneState* state);
   
   // IStateTransitions interface
   virtual PhoneState* OnDial(const wxString phoneNumber);
   virtual PhoneState* OnRinging(const SIPX_CALL hCall);
   virtual PhoneState* OnRemoteAlerting();
   virtual PhoneState* OnRemoteBusy();
   virtual PhoneState* OnFlashButton();
   virtual PhoneState* OnConnected();
   virtual PhoneState* OnDisconnected(const SIPX_CALL hCall);   
   virtual PhoneState* OnOutgoingCallRejected(); 
   virtual PhoneState* OnError(); 
   virtual PhoneState* OnTransferRequested(const wxString phoneNumber);
   virtual PhoneState* OnTransferComplete(const bool bFailure);
   virtual PhoneState* OnOffer(const SIPX_CALL hCall);
   virtual PhoneState* OnHoldButton();
   virtual PhoneState* OnLocalHoldRequested();
   virtual PhoneState* OnConnectedInactive();

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
/* ============================ CREATORS ================================== */
    /**
    * PhoneStateMachine contructor.
    */
    PhoneStateMachine();

    /**
    * PhoneStateMachine destructor.
    */
    virtual ~PhoneStateMachine();

    static PhoneStateMachine* spStateMachine; 
    PhoneState* mpState;
    UtlSList mObservers;
};

#endif

