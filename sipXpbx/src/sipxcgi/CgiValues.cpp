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
#include <cgicc/Cgicc.h>
#include <string>
#include <sipxcgi/CgiValues.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */
                                                                                                             
CgiValues::CgiValues(cgicc::Cgicc *cgi)
{
    m_cgi = cgi;
}


const char* CgiValues::valueOf(const char *name)
{
    const char *cstrValue = NULL;
    cgicc::form_iterator i = m_cgi->getElement(name);
    if (i != m_cgi->getElements().end())
    {
        // save every returned value, caller expects values to remain
        // valid until this class instance is destructed
        std::string strValue = i->getValue();
        m_values.insert(m_values.end(), strValue);
        cstrValue = strValue.c_str();
    }

    return cstrValue;
}
