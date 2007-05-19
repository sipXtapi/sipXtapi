//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <net/XmlRpcMethod.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

XmlRpcMethod* XmlRpcMethod::get()
{
   assert(0);
   return NULL;
}

// Copy constructor
XmlRpcMethod::XmlRpcMethod(const XmlRpcMethod& rXmlRpcMethod)
{
}

// Destructor
XmlRpcMethod::~XmlRpcMethod()
{
}

/* ============================ MANIPULATORS ============================== */


// Assignment operator
XmlRpcMethod&
XmlRpcMethod::operator=(const XmlRpcMethod& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;
}

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Constructor
XmlRpcMethod::XmlRpcMethod()
{
}

/* ============================ FUNCTIONS ================================= */

