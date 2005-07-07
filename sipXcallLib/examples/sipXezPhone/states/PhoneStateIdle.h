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
#ifndef _PhoneStateIdle_h_
#define _PhoneStateIdle_h_

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
class PhoneState;

class PhoneStateIdle : public PhoneState
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * PhoneStateIdle contructor.
    */
   PhoneStateIdle();

   /**
    * PhoneStateIdle destructor.
    */
   virtual ~PhoneStateIdle();

   virtual PhoneState* OnDial(const wxString phoneNumber);				
   virtual PhoneState* OnRinging(SIPX_CALL hCall);			
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