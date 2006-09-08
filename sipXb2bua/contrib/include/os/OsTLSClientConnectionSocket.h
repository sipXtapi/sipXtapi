//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsTLSClientConnectionSocket_h_
#define _OsTLSClientConnectionSocket_h_

#ifdef SIP_TLS
#ifdef SIP_TLS_NSS

// SYSTEM INCLUDES
#include <prio.h>

// APPLICATION INCLUDES                      
#include "os/OsTLSConnectionSocket.h"
#include "os/OsTLS.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS                                       
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/// Implements TLS version of OsSocket
class OsTLSClientConnectionSocket : public OsTLSConnectionSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsTLSClientConnectionSocket(int serverPort,
                                             const char* serverName,
                                             const UtlString certNickname,
                                             const UtlString password,
                                             const UtlString dbLocation,
                                             long timeoutInSecs,
                                             const char* localIp,
                                             ITlsSink* pSink);

  virtual
   ~OsTLSClientConnectionSocket();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
 

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   void NSSInitSocket(int socket, long timeoutInSecs, const char* szPassword);
 
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsTLSClientConnectionSocket(const OsTLSClientConnectionSocket& rOsTLSClientConnectionSocket);
     //:Disable copy constructor

   OsTLSClientConnectionSocket();
     //:Disable default constructor

   OsTLSClientConnectionSocket& operator=(const OsTLSClientConnectionSocket& rhs);
     //:Assignment operator

   ITlsSink* mpTlsSink;

   

};

/* ============================ INLINE METHODS ============================ */

#endif // SIP_TLS
#endif // SIP_TLS_NSS


#endif  // _OsTLSClientConnectionSocket_h_

