//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _PtProviderCapabilities_h_
#define _PtProviderCapabilities_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The ProviderCapabilities interface represents the initial capabilities interface for the Provider.
// This interface supports basic queries for the core package.
// <p>
// Applications obtain the static Provider capabilities via the Provider.getProviderCapabilities()
// method, and the dynamic capabilities via the Provider.getCapabilities() method. This interface
// is used to represent both static and dynamic capabilities.
// <p>
// Any package which extends the core Provider interface should also extend this interface to
// provide additional capability queries for that particular package.
// <p>

class PtProviderCapabilities
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtProviderCapabilities();
     //:Default constructor

   PtProviderCapabilities(const PtProviderCapabilities& rPtProviderCapabilities);
     //:Copy constructor

   virtual
   ~PtProviderCapabilities();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtProviderCapabilities& operator=(const PtProviderCapabilities& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

        UtlBoolean isObservable();
     //:Returns true if this Provider can be observed, false otherwise.
     //!retcode: True if this Provider can be observed,
         //!retcode: false otherwise.


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtProviderCapabilities_h_
