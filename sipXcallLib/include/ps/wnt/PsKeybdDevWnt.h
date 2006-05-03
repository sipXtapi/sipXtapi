//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _PsKeybdDevWnt_h_
#define _PsKeybdDevWnt_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "ps/PsButtonTask.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

//:Phone set keyboard device support on the Wnt platform
class PsKeybdDevWnt : public PsKeybdDev
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PsKeybdDevWnt(PsButtonTask* pButtonTask)
      : PsKeybdDev(pButtonTask)  { };
     //:Constructor
     // Initialize the keyboard device

   virtual
      ~PsKeybdDevWnt() { };
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual void disableIntr(void) { };
     //:Disable keyboard interrupts

   virtual void enableIntr(void) { };
     //:Enable keyboard interrupts

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   PsKeybdDevWnt(const PsKeybdDevWnt& rPsKeybdDevWnt);
     //:Copy constructor (not implemented for this class)

   PsKeybdDevWnt& operator=(const PsKeybdDevWnt& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsKeybdDevWnt_h_
