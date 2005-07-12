// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/XmlContent.h>
#include <net/XmlRpcBody.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
XmlRpcBody::XmlRpcBody()
{
   mBody = XML_VERSION_1_0;
}

// Copy constructor
XmlRpcBody::XmlRpcBody(const XmlRpcBody& rXmlRpcBody)
{
}

// Destructor
XmlRpcBody::~XmlRpcBody()
{
}

/* ============================ MANIPULATORS ============================== */


// Assignment operator
XmlRpcBody&
XmlRpcBody::operator=(const XmlRpcBody& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;
}


void XmlRpcBody::append(const char* string)
{
   mBody.append(string);
}


/* ============================ ACCESSORS ================================= */

int XmlRpcBody::getLength() const
{
   return (mBody.length());
}

void XmlRpcBody::getBytes(const char** bytes, int* length) const
{
   // This version of getBytes exists so that a caller who is
   // calling this method through an HttpBody will get the right
   // thing - we fill in the mBody string and then return that.
   UtlString tempBody;
   getBytes( &tempBody, length );
   ((XmlRpcBody*)this)->mBody = tempBody.data();
   *bytes = mBody.data();
}

void XmlRpcBody::getBytes(UtlString* bytes, int* length) const
{
   *bytes = mBody;
   *length = bytes->length();
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

