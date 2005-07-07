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

#ifndef _SipSrvLookup_h_
#define _SipSrvLookup_h_

// SYSTEM INCLUDES
#if defined(_WIN32)
//#	include <resparse/wnt/netinet/in.h>
#elif defined(_VXWORKS)
#	include <netinet/in.h>
#elif defined(__pingtel_on_posix__)
#   include <netinet/in.h>
#else
#	error Unsupported target platform.
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsMutex.h"
#include "os/OsSocket.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

// TYPEDEFS
typedef struct server_t {
  char *host;              /* host name */
  enum OsSocket::SocketProtocolTypes type; /* SOCK_DGRAM or SOCK_STREAM */
  struct sockaddr_in sin;  /* IP address and port */
  int preference;          /* MX and SRV preference value (lower is better) */
  int weight;              /* load balancing weight (higher is better) */
} server_t;

// FORWARD DECLARATIONS

//:Return list of server entries for 'domain', with last entry having a 
//:host value of NULL.
// The service is a protocol such as 'sip' or 'rtsp'.  The 'port' argument
// is used when the DNS entry doesn't contain one.  A negative number
// indicates the default port.  (This is indicated explicitly since SRV
// records are skipped unless the default port is used.) The 'type'
// restricts whether UDP or TCP addresses are sought.  If 0 both are used.
class SipSrvLookup
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static server_t* servers(const char *domain, 
                            const char *service, 
                            enum OsSocket::SocketProtocolTypes socketType, 
                            int port);

   //! Inquire if this is a valid SRV record
   static UtlBoolean isValidServerT(server_t& srvRecord);

   //! Accessor for host name
   static void getHostNameFromServerT(server_t& srvRecord,
                                      UtlString& hostName);

   //! Accessor for host IP address
   static void getIpAddressFromServerT(server_t& srvRecord,
                                      UtlString& hostName);

   //! Accessor for port
   static int getPortFromServerT(server_t& srvRecord);

   static int getWeightFromServerT(server_t& srvRecord);

   //! Accessor for preference
   static int getPreferenceFromServerT(server_t& srvRecord);

   //! Accessor for protocol
   static enum OsSocket::SocketProtocolTypes getProtocolFromServerT(server_t& srvRecord);

   //! Initializer for server_t
   static void initServerT(server_t& srvRecord);

   // destructor for server_t
   static void freeServerT(server_t* srvRecordArray);

   //! Sets the DNS SRV times.  Defaults: timeout=5, retries=4
   static void setDnsSrvTimeouts(int initialTimeoutInSecs, int retries);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static OsMutex sLock;

   static server_t* server_insert(server_t *server, 
                                  const char *host, 
                                  enum OsSocket::SocketProtocolTypes type,
                                  struct sockaddr_in sin, 
                                  int preference,
                                  int weight, 
                                  int *entries);

   static int server_sort(const server_t *s1, const server_t *s2);

   static server_t* server_addr(server_t *server, 
                                const char *domain, 
                                enum OsSocket::SocketProtocolTypes type,
                                int port, 
                                int preference, 
                                int weight,
                                int *entries, 
                                int depth,
                                UtlBoolean doCnameQuery);

   static server_t* server_hosts(const char *domain, 
                                 const char *service, 
                                 enum OsSocket::SocketProtocolTypes type,
                                 int port, 
                                 int *entries);

   static UtlBoolean lookup_addr_in_addl_info(struct sockaddr_in *sin,
                                       void *response, char *name,
                                       int port);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipSrvLookup_h_
