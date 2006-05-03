//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _CpTestSupport_h_
#define _CpTestSupport_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class CallManager;
class SipUserAgent;

class CpTestSupport
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    /**
     * A testable call manager initialized to testable defaults
     */
    static CallManager *newCallManager(SipUserAgent *ua);

    /**
     * A testable user agent initialized to testable defaults
     */
    static SipUserAgent *newSipUserAgent();

};

#endif  // _CpTestSupport_h_
