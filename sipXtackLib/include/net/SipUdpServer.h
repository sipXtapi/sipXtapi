//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _SipUdpServer_h_
#define _SipUdpServer_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <net/SipProtocolServerBase.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;
class OsNatDatagramSocket ;
class OsNotification ;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipUdpServer : public SipProtocolServerBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipUdpServer(int sipPort = SIP_PORT,
       SipUserAgent* userAgent = NULL,
       const char* natPingUrl = "",
       int natPingFrequency = 0,
       const char* natPingMethod = "PING",
       int udpReadBufferSize = -1,
       UtlBoolean bUseNextAvailablePort = FALSE,
       const char* szBoundIp = NULL);
     //:Default constructor


   virtual
   ~SipUdpServer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

    int run(void* pArg);

    void shutdownListener();

    void enableStun(const char* szStunServer, 
                    int iStunPort,
                    const char* szLocalIp, 
                    int refreshPeriodInSecs, 
                    OsNotification* pNotification) ;
      //:Enable stun lookups for UDP signaling
      // Use a NULL szStunServer to disable

    UtlBoolean sendTo(const SipMessage& message,
                     const char* address,
                     int port,
                     const char* szLocalSipIp = NULL);


    UtlBoolean addCrLfKeepAlive(const char* szLocalIp,
                                const char* szRemoteIp,
                                const int   remotePort,
                                const int   keepAliveSecs) ;

    UtlBoolean removeCrLfKeepAlive(const char* szLocalIp,
                                   const char* szRemoteIp,
                                   const int   remotePort) ;

    UtlBoolean addStunKeepAlive(const char* szLocalIp,
                                const char* szRemoteIp,
                                const int   remotePort,
                                const int   keepAliveSecs) ;

    UtlBoolean removeStunKeepAlive(const char* szLocalIp,
                                   const char* szRemoteIp,
                                   const int   remotePort) ;

/* ============================ ACCESSORS ================================= */

    void printStatus();

    int getServerPort(const char* szLocalIp = NULL) ;

    UtlBoolean getStunAddress(UtlString* pIpAddress,
                              int* pPort,
                              const char* szLocalIp = NULL) ;

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    OsSocket* buildClientSocket(int hostPort,
                                const char* hostAddress,
                                const char* localIp);

    OsMutex mMapLock;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlString mNatPingUrl;
    int mNatPingFrequencySeconds;
    UtlString mNatPingMethod;
    UtlString mStunServer ;
    int mStunRefreshSecs ;
    int mStunPort ;
    OsStatus createServerSocket(const char* localIp,
                                 int& localPort,
                                 const UtlBoolean& bUseNextAvailablePort,
                                 int udpReadBufferSize);

    SipUdpServer(const SipUdpServer& rSipUdpServer);
    //: disable Copy constructor

    SipUdpServer& operator=(const SipUdpServer& rhs);
     //:disable Assignment operator
     

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipUdpServer_h_
