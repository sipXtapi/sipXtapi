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

#ifndef NOTIFIERBASE_H
#define NOTIFIERBASE_H

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
class SipMessage;

class NotifierBase  
{
public:   
    virtual void sendNotifyForeachSubscription(
        const char* key,
        const char* eventType,
        SipMessage& notify) = 0;

    virtual void sendNotifyForSubscription(
        const char* key,
        const char* eventType,
        SipMessage& notify) = 0;
};

#endif // NOTIFIERBASE_H
