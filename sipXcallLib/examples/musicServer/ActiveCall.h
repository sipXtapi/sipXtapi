//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _ActiveCall_h_
#define _ActiveCall_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <CallObject.h>
#include <utl/UtlContainable.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class ActiveCall : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   ActiveCall(UtlString& callId);

   ActiveCall(UtlString& callId, CallObject* call);
   
   ~ActiveCall();

   virtual UtlContainableType getContainableType() const;

   virtual unsigned int hash() const;

   int compareTo(const UtlContainable *b) const;

   CallObject* getCallObject() { return mpCall; };

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   
   UtlString mCallId;
   CallObject* mpCall;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _ActiveCall_h_
