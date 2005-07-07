// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipRequestContext_h_
#define _SipRequestContext_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlDList.h>


// DEFINES


// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class Container for context variables for SIP request API
// The class is passed as an additional argument to contain
// context information that is not contained in the SIP request
// (which is also passed with this container).  Effectively this
// is supplimental informatiion to the SIP Request.

class SipRequestContext 
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const char* sAUTH_USER;
   static const char* sAUTH_REALM;
   static const char* sREQUEST_METHOD;
   static const char* sSERVER_DOMAIN;

/* ============================ CREATORS ================================== */

   SipRequestContext(const char* requestMethod = NULL);
     //:Default constructor


   virtual
   ~SipRequestContext();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */
   UtlBoolean getVariable(const char* name, 
                          UtlString& value, 
                          int occurance = 0) const;
   //: Get context variables provided in this request context.
   // As it is possible to have multiple occurances of a named value
   // the occurance argument indicates which occurance.  The default is 
   // the first.

   UtlBoolean getVariable(int index, 
                             UtlString& name, 
                             UtlString& value) const;
   //: Get the name and value of the variable at the given index

   void addVariable( const char* name, const char* value);
   //: Add a variable to the context.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipRequestContext(const SipRequestContext& rSipRequestContext);
     //:Copy constructor 
   SipRequestContext& operator=(const SipRequestContext& rhs);
     //:Assignment operator

   UtlDList mVariableList;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipRequestContext_h_
