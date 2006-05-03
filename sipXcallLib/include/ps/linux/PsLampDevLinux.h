//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _PsLampDevLinux_h_
#define _PsLampDevLinux_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "ps/PsLampDev.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class PsLampTask;

//:Phone set lamp device support on the WIN32 platform
class PsLampDevLinux : public PsLampDev
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PsLampDevLinux(PsLampTask* pLampTask=NULL);
     //:Constructor
     // Initialize the lamp device

   virtual
   ~PsLampDevLinux();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   void lightLamps(unsigned long onLamps);
     //:Turn on the indicated lamps

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   PsLampDevLinux(const PsLampDevLinux& rPsLampDevLinux);
     //:Copy constructor (not implemented for this class)

   PsLampDevLinux& operator=(const PsLampDevLinux& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsLampDevLinux_h_
