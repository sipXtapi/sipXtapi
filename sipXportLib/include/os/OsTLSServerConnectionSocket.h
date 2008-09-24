//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsTLSServerConnectionSocket_h_
#define _OsTLSServerConnectionSocket_h_

#ifdef SIP_TLS
#ifdef SIP_TLS_NSS

// SYSTEM INCLUDES
#include <prio.h>

// APPLICATION INCLUDES                      
#include <os/OsTLSConnectionSocket.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS                                       
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Implements TLS version of OsSocket
class OsTLSServerConnectionSocket : public OsTLSConnectionSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsTLSServerConnectionSocket(int socketDescriptor,
                         const UtlString certNickname,
                         const UtlString password,
                         const UtlString dbLocation,
                         long timeoutInSecs = 0); 

  virtual
   ~OsTLSServerConnectionSocket();
     //:Destructor

  bool mbInitializeFailed;

/* ============================ MANIPULATORS ============================== */
 

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   void NSSInitSocket(PRFileDesc* pDesc, long timeoutInSecs, const char* szPassword);
   void NSSInitSocket(int socketDescriptor, long timeoutInSecs, const char* szPassword);
 
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


   OsTLSServerConnectionSocket(const OsTLSServerConnectionSocket& rOsTLSServerConnectionSocket);
     //:Disable copy constructor

   OsTLSServerConnectionSocket();
     //:Disable default constructor

   OsTLSServerConnectionSocket& operator=(const OsTLSServerConnectionSocket& rhs);
     //:Assignment operator


   

};

/* ============================ INLINE METHODS ============================ */

#endif // SIP_TLS
#endif // SIP_TLS_NSS

#endif  // _OsTLSServerConnectionSocket_h_

