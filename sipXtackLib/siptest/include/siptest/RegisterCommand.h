//
// Copyright (C) 2006-2012 SIPez LLC.  All rights reserved.
//
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _RegisterCommand_h_
#define _RegisterCommand_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsDefs.h>

#include "siptest/Command.h"
#include "net/SipLineMgr.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class implement siptest 'register' command

class RegisterCommand : public Command
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   RegisterCommand(SipLineMgr* lineMgr);
     //:Default constructor

   virtual
   ~RegisterCommand();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual int execute(int argc, char* argv[]);

/* ============================ ACCESSORS ================================= */

   virtual void getUsage(const char* commandName, UtlString* usage) const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   SipLineMgr& mLineMgr;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

        RegisterCommand& operator=(const RegisterCommand& rhs);
        //:Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _RegisterCommand_h_

