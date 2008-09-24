//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _ReceivedCallList_h_
#define _ReceivedCallList_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "CallList.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * List control for received calls.
 */
class ReceivedCallList : public CallList
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * ReceivedCallList contructor.
    */
   ReceivedCallList(wxWindow* parent, const wxPoint& pos, const wxSize& size);

   /**
    * ReceivedCallList destructor.
    */
   virtual ~ReceivedCallList();

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};


#endif
