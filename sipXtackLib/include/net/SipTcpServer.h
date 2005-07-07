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

#ifndef _SipTcpServer_h_
#define _SipTcpServer_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsServerSocket.h>
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
class SipTcpServer : public SipProtocolServerBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipTcpServer(int sipPort = SIP_PORT, 
                SipUserAgent* userAgent = NULL,
                const char* protocolString = SIP_TRANSPORT_TCP, 
                const char* taskName  = "SipTcpServer-%d");
     //:Default constructor


   virtual
   ~SipTcpServer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
	UtlBoolean startListener();

	//void addEventConsumer(OsServerTask* messageEventListener);
	//void removeEventConsumer(OsServerTask* messageEventListener);

	void shutdownListener();

	int run(void* pArg);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */
    virtual UtlBoolean isOk();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    virtual OsSocket* buildClientSocket(int hostPort, const char* hostAddress);

    int mServerPort;
    OsServerSocket* mServerSocket;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

	SipTcpServer(const SipTcpServer& rSipTcpServer);
	//: disable Copy constructor

   SipTcpServer& operator=(const SipTcpServer& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipTcpServer_h_
