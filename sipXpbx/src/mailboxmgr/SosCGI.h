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

#ifndef SOSCGI_H
#define SOSCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "net/Url.h"
#include "mailboxmgr/VXMLCGICommand.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SosCGI : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    SosCGI (const Url& from);

    /**
     * Virtual Dtor
     */
    virtual ~SosCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

    OsStatus parseMappingFile(const UtlString& mapFile);

protected:

private:
    UtlString mFrom;
    UtlString mSosUrl;
};

#endif //SOSCGI_H

