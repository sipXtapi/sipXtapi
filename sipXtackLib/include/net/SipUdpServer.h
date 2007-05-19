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
// APPLICATION INCLUDES
#include <net/SipProtocolServerBase.h>
#include <os/OsNatKeepaliveListener.h>
#include <utl/UtlSList.h>
#include <os/OsRWMutex.h>

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
class OsTimer;


/**
 * The SipUdpServer is owner/container of the sockets used for UDP 
 * communications.  The SipUdpServer has a number of NAT APIs and
 * coordinates NAT detection/keepalives with the OsNatDatagramSocket / 
 * OsNatAgentTask.
 * 
 * The SipUdpServer may contain multiple sockets if being used in a multi-
 * home configuration.  This mostly works, but needs additional testing
 * and tweaking.
 *
 * Most of the NAT keepalives are delegated to the OsNatAgentTask.  The 
 * exception is the Sip keepalive.  For this, the SipUdpServer uses 
 * callbacks to send off the Sip messages (as opposed to introducing a 
 * OsMsgQueue).  The SipUdpServer never listens for responses, however, 
 * the SipUserAgent itself pays attention to rport results and notifies 
 * the OsNatAgentTask of local ip -> remote IP NAT bindings.
 */
class SipUdpServer : public SipProtocolServerBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipUdpServer(int sipPort = SIP_PORT,
       SipUserAgent* userAgent = NULL,
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
                                const int   keepAliveSecs,
                                OsNatKeepaliveListener* pListener) ;

    UtlBoolean removeCrLfKeepAlive(const char* szLocalIp,
                                   const char* szRemoteIp,
                                   const int   remotePort) ;

    UtlBoolean addStunKeepAlive(const char* szLocalIp,
                                const char* szRemoteIp,
                                const int   remotePort,
                                const int   keepAliveSecs,
                                OsNatKeepaliveListener* pListener) ;

    UtlBoolean removeStunKeepAlive(const char* szLocalIp,
                                   const char* szRemoteIp,
                                   const int   remotePort) ;

    UtlBoolean addSipKeepAlive(const char* szLocalIp,
                               const char* szRemoteIp,
                               const int   remotePort,
                               const char* szMethod,
                               const int   keepAliveSecs,
                               OsNatKeepaliveListener* pListener) ;

    UtlBoolean removeSipKeepAlive(const char* szLocalIp,
                                  const char* szRemoteIp,
                                  const int   remotePort,
                                  const char* szMethod) ;

    void updateSipKeepAlive(const char* szLocalIp,
                            const char* szMethod,
                            const char* szRemoteIp,
                            const int   remotePort,
                            const char* szContactIp,
                            const int   contactPort) ;


    void sendSipKeepAlive(OsTimer* pTimer) ; 

    static void SipKeepAliveCallback(const int userData, 
                                     const int eventData) ;

/* ============================ ACCESSORS ================================= */

    void printStatus();

    int getServerPort(const char* szLocalIp = NULL) ;

    UtlBoolean getStunAddress(UtlString* pIpAddress,
                              int* pPort,
                              const char* szLocalIp = NULL) ;

    UtlBoolean addKeepAliveBinding(void* pBinding) ;
    UtlBoolean removeKeepAliveBinding(void* pBinding) ;

    void* findKeepAliveBinding(OsTimer* pTimer) ;

    void* findKeepAliveBinding(OsSocket*   pSocket,
                               const char* szRemoteIp,
                               const int   remotePort,
                               const char* szMethod) ;


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    OsSocket* buildClientSocket(int hostPort,
                                const char* hostAddress,
                                const char* localIp);

    OsMutex mMapLock;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    UtlString mStunServer ;
    int mStunRefreshSecs ;
    int mStunPort ;
    UtlSList mSipKeepAliveBindings ;
    OsRWMutex mKeepAliveMutex ;

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
