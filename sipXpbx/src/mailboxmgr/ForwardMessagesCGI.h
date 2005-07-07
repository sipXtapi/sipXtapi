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
#ifndef FORWARDMESSAGE_H
#define FORWARDMESSAGE_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "net/Url.h"
#include "mailboxmgr/CGICommand.h"
#include "mailboxmgr/MailboxManager.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * ForwardMessagesCGI Class
 *
 * @author Harippriya M Sivapatham
 * @version 1.0
 */
class ForwardMessagesCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    ForwardMessagesCGI(	const char* comments,
						const UtlString& commentsDuration,
						const UtlString& commentsTimestamp,
						const int  commentsSize,
						const Url& fromMailboxIdentity,
						const UtlString& fromFolder, 
						const UtlString& messageIds,
						const UtlString& toExtension  ) ;
    /**
     * Virtual Destructor
     */
    virtual ~ForwardMessagesCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL) ;

protected:

private:
	const UtlString m_commentsDuration;
    const UtlString m_commentsTimestamp;
    // Note that this is an identity and not a full URL
    // we need to look up the real Url from the IMDB
    const Url      m_fromMailboxIdentity;
    const UtlString m_fromFolder;
	const UtlString m_messageIds;
	const UtlString m_toExtension;
    char* m_comments;
    int   m_commentsSize;
};

#endif //FORWARDMESSAGE_H
