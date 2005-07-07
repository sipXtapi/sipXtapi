// 
//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _CgiValues_h_
#define _CgiValues_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <cgicc/Cgicc.h>
#include <string>
#include <vector>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Wraps CGICC with helper routines.
 */
class CgiValues 
{
private:
    cgicc::Cgicc* m_cgi;

    std::vector<std::string> m_values;

public:

    CgiValues(cgicc::Cgicc *cgi);

    const char* valueOf(const char *name);
};

#endif // _CgiValues_h_
