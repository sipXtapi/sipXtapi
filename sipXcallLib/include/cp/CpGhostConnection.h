//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _CpGhostConnection_h_
#define _CpGhostConnection_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <cp/Connection.h>
#include <net/SipContactDb.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


//:logical Connection within a call
// The Connection encapsulates the call setup protocol and state
// information for the leg of a call to a particular address.
class CpGhostConnection: public Connection
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   CpGhostConnection(CpCallManager* callMgr = NULL,
              CpCall* call = NULL, const char* callId = NULL);
     //:Default constructor


   virtual
   ~CpGhostConnection();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean dequeue(UtlBoolean callInFocus);

   virtual UtlBoolean dial(const char* dialString,
                           const char* callerId,
                           const char* callId,
                           const char* callController = NULL,
                           const char* originalCallConnection = NULL,
                           UtlBoolean requestQueuedCall = FALSE,
                           const void* pDisplay = NULL,
                           const void* pSecurity = NULL,
                           const char* locationHeader = NULL,
                           const int bandWidth = AUDIO_MICODEC_BW_DEFAULT,
                           UtlBoolean bOnHold = FALSE);

   //! param: requestQueuedCall - indicates that the caller wishes to have the callee queue the call if busy

   virtual UtlBoolean originalCallTransfer(UtlString& transferTargetAddress,
                                                           const char* transferControllerAddress,
                                   const char* targetCallId);
   // Initiate blind transfer on transfer controller connection in
   // the original call.

   virtual UtlBoolean targetCallBlindTransfer(const char* transferTargetAddress,
                                                           const char* transferControllerAddress);
   // Communicate blind transfer on transfer controller connection in
   // the target call.  This is signaled by the transfer controller in the
   // original call.

   virtual UtlBoolean transfereeStatus(int connectionState, int response);
   // Method to communicate status to original call on transferee side

   virtual UtlBoolean transferControllerStatus(int connectionState, int cause);
   // Method to communicate status to target call on transfer
   // controller side

   virtual UtlBoolean answer(const void* hWnd = NULL);

   virtual void outOfFocus();

   virtual UtlBoolean hangUp();

   virtual UtlBoolean hold();

   virtual UtlBoolean reject();

   virtual UtlBoolean redirect(const char* forwardAddress);

   virtual UtlBoolean offHold();

   virtual UtlBoolean renegotiateCodecs();

   virtual UtlBoolean silentRemoteHold() ;

   virtual UtlBoolean accept(int forwardOnNoAnswerSeconds, 
                             const void *pSecurity = NULL,
                             const char* locationHeader = NULL,
                             const int bandWidth = AUDIO_MICODEC_BW_DEFAULT);

   virtual UtlBoolean processMessage(OsMsg& eventMessage,
                                    UtlBoolean callInFocus, UtlBoolean onHook);

/* ============================ ACCESSORS ================================= */

   virtual UtlBoolean getRemoteAddress(UtlString* remoteAddress) const;
   //: get Connection address
   //! returns: TRUE/FALSE if the connection has an address.  The connection may not have an address assigned yet (i.e. if it is not fully setup).

   virtual UtlBoolean getRemoteAddress(UtlString* remoteAddress, UtlBoolean leaveFieldParametersIn) const;
   //: get Connection address
   //! returns: TRUE/FALSE if the connection has an address.  The connection may not have an address assigned yet (i.e. if it is not fully setup).
   
           virtual UtlBoolean getSession(SipSession& session);

    /**
     * Enumerate possible contact addresses
     */
    virtual void getLocalContactAddresses( CONTACT_ADDRESS contacts[],
                                           size_t nMaxContacts,
                                           size_t& nActualContacts) ;

    virtual void getRemoteUserAgent(UtlString* pUserAgent);


/* ============================ INQUIRY =================================== */

   virtual UtlBoolean willHandleMessage(OsMsg& eventMessage) const;

   virtual UtlBoolean isConnection(const char* callId,
                                  const char* toTag,
                                  const char* fromTag,
                                  UtlBoolean strictCompare) const;

   virtual UtlBoolean isSameRemoteAddress(Url& remoteAddress) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:



/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

        CpGhostConnection(const CpGhostConnection& rCpGhostConnection);
     //:Copy constructor (disabled)
        CpGhostConnection& operator=(const CpGhostConnection& rhs);
     //:Assignment operator (disabled)

    UtlString mRemoteAddress;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpGhostConnection_h_
