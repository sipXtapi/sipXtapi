//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _TestRegistryCollection_h_
#define _TestRegistryCollection_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
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
 */
class TestRegistryCollection : public UtlSList
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   TestRegistryCollection();
     
   /**
    * Copy constructor
    */
   TestRegistryCollection(const TestRegistryCollection& rTestRegistryCollection);     

   /**
    * Destructor
    */
   virtual ~TestRegistryCollection();
   

/* ============================ MANIPULATORS ============================== */

   /**
    * Assignment operator
    *
    * @param rhs right hand side of the equals operator
    */
   TestRegistryCollection& operator=(const TestRegistryCollection& rhs);  

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _TestRegistryCollection_h_
