//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
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


/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};
#endif



