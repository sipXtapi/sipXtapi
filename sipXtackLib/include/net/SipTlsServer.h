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

#ifndef _SipTlsServer_h_
#define _SipTlsServer_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <net/SipTcpServer.h>

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
class SipTlsServer : public SipTcpServer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipTlsServer(int sipPort = SIP_TLS_PORT, SipUserAgent* userAgent = NULL);
     //:Default constructor


   virtual
   ~SipTlsServer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */
    virtual UtlBoolean isOk();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    virtual OsSocket* buildClientSocket(int hostPort, const char* hostAddress);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

	SipTlsServer(const SipTlsServer& rSipTlsServer);
	//: disable Copy constructor

   SipTlsServer& operator=(const SipTlsServer& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipTlsServer_h_
