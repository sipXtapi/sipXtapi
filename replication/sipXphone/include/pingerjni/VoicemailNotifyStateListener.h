// $Id: //depot/OPENDEV/sipXphone/include/pingerjni/VoicemailNotifyStateListener.h#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _VoicemailNotifyStateListener_h_
#define _VoicemailNotifyStateListener_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsDefs.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class VoicemailNotifyStateListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    static void doReboot();

    static void doBinaryMessageWaiting(const char* toUrl,
                                      UtlBoolean newMessages);

    static void doDetailMessageWaiting(const char* toUrl,
                                       const char* messageMediaType,
                                       UtlBoolean absoluteValues,
                                       int totalNewMessages,
                                       int totalOldMessages,
                                       int totalUntouchedMessages,
                                       int urgentUntouchedMessages,
                                       int totalSkippedMessages,
                                       int urgentSkippedMessages,
                                       int totalFlaggedMessages,
                                       int urgentFlaggedMessages,
                                       int totalReadMessages,
                                       int urgentReadMessages,
                                       int totalAnsweredMessages,
                                       int urgentAnsweredMessages,
                                       int totalDeletedMessages,
                                       int urgentDeletedMessages);
    // totalNewMessages and totalOldMessages are summary totals
    //      accross all other categories.
    //! param: messageMediaType = "Voicemail", "Email", "Fax", "Video", etc
    //! param: absoluteValues - the message counts are absolute counts (TRUE) or deltas (FALSE)
    // If these are absolute values negative number for a message count means unspecified
    // If these are deltas there is know way to know if the values are not set


/* ============================ CREATORS ================================== */

   VoicemailNotifyStateListener();
     //:Default constructor

   virtual
   ~VoicemailNotifyStateListener();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


   VoicemailNotifyStateListener(const VoicemailNotifyStateListener& rVoicemailNotifyStateListener);
     //:Copy constructor (disabled)

   VoicemailNotifyStateListener& operator=(const VoicemailNotifyStateListener& rhs);
     //:Assignment operator (disabled)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _VoicemailNotifyStateListener_h_
