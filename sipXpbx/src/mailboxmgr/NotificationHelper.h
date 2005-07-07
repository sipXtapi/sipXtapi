// 
//
// Copyright (c) PingTel Corp. (work in progress)
//
// This is an unpublished work containing PingTel Corporation's confidential
// and proprietary information.  Disclosure, use or reproduction without
// written authorization of PingTel Corp. is prohibited.
//
//! lib=mailboxmgr
//////////////////////////////////////////////////////////////////////////////
#ifndef NOTIFICATIONHELPER_H
#define NOTIFICATIONHELPER_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsDateTime.h"
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Url;
class UtlHashMap;

/**
 * Mailbox Class
 *
 * @author John P. Coffey
 * @version 1.0
 */
class NotificationHelper
{
public:
    /**
     * Singleton Accessor
     *
     * @return
     */
    static NotificationHelper* getInstance();

    /**
     * Virtual Destructor
     */
    virtual ~NotificationHelper();

    /**
     * 
     * @param rMailboxIdentity 
     * @param rSMTPServer           the outbound SMTP Server address
     * @param rServerRoot           root for apache web server
     * @param rContact              
     * @param rFrom
     * @param rReplyTo
     * @param rDate                 timestamp of the file
     * @param rDurationMSecs        duration specified in Milliseconds
     * @param rAttachmentFileName   the name of the attachment (relative to root)
     * @param rIsHtmlFormat
     * @param rAttachmentEnabled
     * 
     * @return 
     */
    OsStatus send (
        const UtlString& rMailboxIdentity,
        const UtlString& rSMTPServer,
        const Url&      mediaserverUrl,
        const UtlString& rContact,
        const UtlString& rFrom,
        const UtlString& rReplyTo,
        const UtlString& rDate,
        const UtlString& rDurationMSecs,
        const UtlString& wavFileName,
        const char*     pAudioData,
        const int&      rAudioDatasize,
        const UtlBoolean& rAttachmentEnabled = FALSE ) const;
protected:

private:
    /**
     * Private constructor (singleton)
     */
    NotificationHelper();

    // Singtleton instance
    static NotificationHelper* spInstance;

    // Exclusive binary lock
    static OsMutex sLockMutex;
};

#endif //NOTIFICATIONHELPER_H

