// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipUdpServer_h_
#define _SipUdpServer_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsDatagramSocket.h>
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
       int udpReadBufferSize = -1);
     //:Default constructor


   virtual
   ~SipUdpServer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
    int run(void* pArg);

    UtlBoolean startListener();

    void shutdownListener();

    UtlBoolean sendTo(const SipMessage& message,
                     const char* address,
                     int port);

/* ============================ ACCESSORS ================================= */

    void printStatus();

/* ============================ INQUIRY =================================== */

    virtual UtlBoolean isOk();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    OsSocket* buildClientSocket(int hostPort, const char* hostAddress);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
	int serverPort;
	OsDatagramSocket* serverSocket;
	SipClient* server;
    UtlString mNatPingUrl;
    int mNatPingFrequencySeconds;
    UtlString mNatPingMethod;

	SipUdpServer(const SipUdpServer& rSipUdpServer);
	//: disable Copy constructor

   SipUdpServer& operator=(const SipUdpServer& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipUdpServer_h_
