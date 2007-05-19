//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _SipTlsServer_h_
#define _SipTlsServer_h_

#ifdef SIP_TLS

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <net/SipProtocolServerBase.h>
#include <os/OsTLSServerSocket.h>
#include "os/OsTLSClientConnectionSocket.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipTlsServer : public SipProtocolServerBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipTlsServer(int sipPort = SIP_TLS_PORT, 
                SipUserAgent* userAgent = NULL,
                UtlBoolean bUseNextAvailablePort = FALSE,
                UtlString certNickname = "",
                UtlString certPassword = "",
                UtlString dbLocation = ".",
                const char* szBoundIp = "0.0.0.0");
     //:Default constructor


   virtual
   ~SipTlsServer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

    int getServerPort() const ;
    //: The the local server port for this server

    OsStatus getTlsInitCode() { return mTlsInitCode; }

    UtlBoolean startListener();
    void shutdownListener();
    int run(void* pArg);

    UtlHashMap mServerBrokers;


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    virtual OsSocket* buildClientSocket(int hostPort, const char* hostAddress, const char* localIp);
    
    OsStatus createServerSocket(const char* szBindAddr,
                                int& port,
                                const UtlBoolean& bUseNextAvailablePort);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    UtlString mCertNickname;
    UtlString mCertPassword;
    UtlString mDbLocation;
    int mServerPort;
    OsStatus mTlsInitCode;

    SipTlsServer(const SipTlsServer& rSipTlsServer);
    //: disable Copy constructor

    SipTlsServer& operator=(const SipTlsServer& rhs);
    //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // SIP_TLS
#endif  // _SipTlsServer_h_
