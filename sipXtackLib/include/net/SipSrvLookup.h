//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _SipSrvLookup_h_
#define _SipSrvLookup_h_

// SYSTEM INCLUDES
#if defined(_WIN32)
//#   include <resparse/wnt/netinet/in.h>
#elif defined(_VXWORKS)
#  include <netinet/in.h>
#elif defined(__pingtel_on_posix__)
#   include <netinet/in.h>
#else
#  error Unsupported target platform.
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

// FORWARD DECLARATIONS
class server_t;

/**
 * A class (with no members) whose 'servers' method implements the RFC
 * 3263 process for determining a list of server entries for a SIP
 * domain name.
 */
class SipSrvLookup
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// Get the list of server entries for SIP domain name 'domain'.
   static server_t* servers(const char *domain,
                            ///< SIP domain name or host name
                            const char *service,
                            ///< "sip" or "sips"
                            OsSocket::SocketProtocolTypes socketType,
                            ///< types of transport
                            int port,
                            ///< port number from URI, or PORT_NONE
                            const char* srcIp
                            ///< the outgoing interface ip to send the request on
      );
   /**<
    * Returns the list of server entries for SIP domain name 'domain'.
    * Implements the processes of RFC 2543.
    * The search process is modified by the parameters:
    *
    * The 'service' is the URI scheme, "sip" or "sips".
    *
    * The 'port' argument is the port number supplied in the URI, or a
    * negative number if none was supplied.
    * If none was supplied, SRV records are consulted.  If no SRV records
    * are found, port defaults to 5060 for "sip" service and 5061 for
    * "sips" service.
    *
    * The 'socketType' restricts the search to that sort of transport,
    * and may have the values (within enum SocketProtocolTypes):
    *    TCP
    *    UDP
    *    SSL_SOCKET
    *    UNKNOWN (all of the above are acceptable)
    * It is used if the URI or other context information specifies a transport.
    * If UNKNOWN is specified, servers() attempts to return addresses for
    * all transports that it knows about that are compatible with the service.
    * (If socketType is incompatible with service, no addresses will
    * be returned.)
    *
    * @returns Allocates an array of server_t objects and returns
    * the pointer to it.  Caller is responsible for delete[]'ing the array.
    * The servers are listed in the array in preference order, with a final
    * entry with a host value of NULL.
    */

   /// Option codes for server lookup.
   enum OptionCode {
      OptionCodeNone = 0,       ///< Special value
      OptionCodeFirst,          ///< Start of range
      OptionCodeIgnoreSRV,      ///< If 1, do not search for SRV records.
      OptionCodeIgnoreNAPTR,    ///< If 1, do not search for NAPTR records.
      OptionCodeSortAnswers,    /**< If 1, sort DNS answers before using them.
                                 *   (For testing only.) */
      OptionCodePrintAnswers,   /**< If 1, print DNS answers.
                                 *   (For testing only.) */
      OptionCodeCNAMELimit,     ///< Max. number of CNAMEs to follow.
      OptionCodeNoDefaultTCP,   /**< If 1, do not add TCP contacts by default,
                                 *   for better RFC 3263 conformance. */
      OptionCodeLast,           ///< End of range
   };
   /**<
    * All options have a code name in this enumeration.  All codes are in the
    * range OptionCodeFirst:OptionCodeLast.  OptionCodeNone is not in that
    * range and may be used as a special value.
    *
    * Option values are signed integers.
    *
    * Getting and setting all option codes in that range is guaranteed to
    * restore the server lookup algorithm to the preceeding codition.
    *
    * The way enum values are assigned is somewhat sloppy (it allows some
    * code values that are not used to be in the valid range), but it makes
    * editing the enum correctly quite easy.  Do not reorder the list without
    * checking the initializer in SipSrvLookup.cpp; some options have non-zero
    * initial values.
    */

   /// Get an option value.
   static inline int getOption(OptionCode option)
      {
         return options[option];
      }

   /// Set an option value.
   static void setOption(OptionCode option, int value);
   /**
    * setOption can be called at any time from any thread without
    * causing harm to any requests that may be being processed.
    * It may, however, block until all current requests are processed
    * before returning to the caller, and current requests may be processed
    * partly under the old configuration and partly under the new
    * configuration.
    */

   /// Sets the timing parameters for DNS SRV queries.
   static void setDnsSrvTimeouts(int initialTimeoutInSecs,
                                 /**< Timeout in seconds for first query,
                                  *   or 0 for no change. */
                                 int retries
                                 /**< Number of retries to attempt,
                                  *   or 0 for no change. */
      );
   ///< Defaults are: timeout = 5, retries = 4.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /// Mutex to keep the routines thread-safe.
   static OsMutex sMutex;

   /// The array of option values.
   static int options[OptionCodeLast+1];

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};


/**
 * Structure to describe a server found for a SIP domain.
 */
class server_t {
  public:
   
   //! Sets the mDnsSrvResolveEnabled flag
   static void setDnsSrvResolveEnabled(UtlBoolean& enabled);   

   char *host;                  ///< Host name. (Owned by this object.)
   OsSocket::SocketProtocolTypes type;
                                ///< OsSocket:{TCP,UDP,SSL_SOCKET}
   struct sockaddr_in sin;      ///< IP address and port
   unsigned int priority;       ///< SRV priority value
   unsigned int weight;         ///< SRV weight
   float score;                 ///< Calculated sorting score

   /// Initializer for server_t
   server_t();
   static UtlBoolean mDnsSrvResolveEnabled;
   

   /// Copy constructor for server_t
   server_t(const server_t& rserver_t);

   /// Copy assignment constructor for server_t
   server_t& operator=(const server_t& rhs);

   /// Destructor for server_t
   ~server_t();

   /// Inquire if this is a valid SRV record
   UtlBoolean isValidServerT();
   ///< Tests whether the host name element is non-NULL.

   /// Accessor for host name
   void getHostNameFromServerT(UtlString& hostName);

   /// Accessor for host IP address
   void getIpAddressFromServerT(UtlString& hostName);

   /// Accessor for port
   int getPortFromServerT();

   /// Accessor for weight
   unsigned int getWeightFromServerT();

   /// Accessor for score
   float getScoreFromServerT();

   /// Accessor for priority
   unsigned int getPriorityFromServerT();

   /// Accessor for protocol
   OsSocket::SocketProtocolTypes getProtocolFromServerT();
};

#endif  // _SipSrvLookup_h_
