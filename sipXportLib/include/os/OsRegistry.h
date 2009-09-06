//
// Copyright (C) 2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2009 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _OsRegistry_h_
#define _OsRegistry_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "utl/UtlString.h"  

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS


// FORWARD DECLARATIONS


class OsRegistry
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   OsRegistry();
   virtual ~OsRegistry();

/* ============================ MANIPULATORS ============================== */
   bool writeInteger(UtlString keyPath, UtlString key, int value);

/* ============================ ACCESSORS ================================= */
   bool readInteger(UtlString keyPath, UtlString key, int& value);
   
/* ============================ INQUIRY =================================== */
    

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */




#endif  // _OsRegistry_h_


