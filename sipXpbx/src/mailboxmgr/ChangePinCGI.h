// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef CHANGEPINCGI_H
#define CHANGEPINCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "mailboxmgr/VXMLCGICommand.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class ChangePinCGI : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    ChangePinCGI ( const UtlString& userid,
                   const UtlString& password,
                   const UtlString& newpassword);

    /**
     * Virtual Destructor
     */
    virtual ~ChangePinCGI();

    /** This does the work */
    virtual OsStatus execute ( UtlString* out = NULL);

protected:

private:
    const UtlString mUserId;
    const UtlString mOldPassword;
    const UtlString mNewPassword;
};

#endif //CHANGEPINCGI_H

