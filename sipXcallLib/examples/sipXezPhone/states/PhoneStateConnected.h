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

   //virtual PhoneState* OnDial(const wxString phoneNumber);				
   //virtual PhoneState* OnRinging();			
   //virtual PhoneState* OnRemoteAlerting();	
   //virtual PhoneState* OnRemoteBusy();		
   virtual PhoneState* OnFlashButton();		
   //virtual PhoneState* OnConnected();		
   virtual PhoneState* OnDisconnected();	
   virtual PhoneState* OnHoldButton();
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