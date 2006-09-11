// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef REDIRECTRESUMEMSG_H
#define REDIRECTRESUMEMSG_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "registry/RedirectPlugin.h"

// DEFINES
   /** Message type code.
    */
#define REDIRECT_RESTART OsMsg::USER_START

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Message to the redirect server to resume processing a redirection request.
class RedirectResumeMsg : public OsMsg
{
public:

   /** Construct a message saying that redirector redirectorNo is willing to
    * reprocess request seqNo.
    */
   RedirectResumeMsg(RedirectPlugin::RequestSeqNo seqNo,
                     int redirectorNo);

   /**
    * Copy this message.
    */
   virtual OsMsg* createCopy(void) const;

   /** Get the sequence number
    */
   inline RedirectPlugin::RequestSeqNo getRequestSeqNo() const
   {
      return mSeqNo;
   }

   /** Get the redirector number
    */
   inline int getRedirectorNo() const
   {
      return mRedirectorNo;
   }

private:

   /** Sequence number of the request
    */
   RedirectPlugin::RequestSeqNo mSeqNo;

   /** Number of the redirector
    */
   int mRedirectorNo;
};

#endif /*  REDIRECTRESUMEMSG_H */
