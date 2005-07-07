/*
* Expands a SIP domain name into a list of suitable servers, in the
* following order:
*  (0) IP address
*  (1) SRV records      (negative preferences starting at -65536)
*  (3) A and RR records (assigned a preference of 65536)
*
* Based on DomainSearch by Christian Zahl (1996);
*   Cleaned up by Henning Schulzrinne, 1997-07-13;
*   Extended to handle SRV records and port, 1998-08-03
*
* Copyright 1998-1999 by Columbia University; all rights reserved 
*/

#if defined(_WIN32)
#	include "resparse/wnt/sysdep.h"
#	include <resparse/wnt/netinet/in.h>
#	include <resparse/wnt/arpa/nameser.h>
#	include <resparse/wnt/resolv/resolv.h>
#	include <winsock2.h>
#elif defined(_VXWORKS)
#	include <netdb.h>
#	include <netinet/in.h>
/* Use local lnameser.h for info missing from VxWorks version --GAT */
/* lnameser.h is a subset of resparse/wnt/arpa/nameser.h                */
#	include <resolv/nameser.h>
#	include <resparse/vxw/arpa/lnameser.h>
/* Use local lresolv.h for info missing from VxWorks version --GAT */
/* lresolv.h is a subset of resparse/wnt/resolv/resolv.h               */
#	include <resolv/resolv.h>
#	include <resparse/vxw/resolv/lresolv.h>
/* #include <sys/socket.h> used sockLib.h instead --GAT */
#	include <sockLib.h>
#	include <resolvLib.h>
#	include <resparse/vxw/hd_string.h>
#elif defined(__pingtel_on_posix__)
#	include <arpa/inet.h>
#	include <netinet/in.h>
#	include <sys/socket.h>
#	include <resolv.h>
#	include <netdb.h>
#else
#	error Unsupported target platform.
#endif

#include <stdlib.h>   /* qsort() --GAT */
#include <assert.h>

#include <stdio.h>
#include <sys/types.h>

#if HAVE_ARPA_NAMESER_COMPAT_H
#include <resparse/arpa/nameser_compat.h> /* T_SRV, etc., on recent BIND */
#endif

#include <ctype.h>
#include <string.h>

/* #include "resparse/pthread.h" For testing, ignore threading --GAT */

#include "resparse/rr.h"       /* from resparse package --GAT */
#include "net/SipSrvLookup.h"

#ifdef _VXWORKS
extern "C" int enetIsLinkActive(void);
#endif

#define DEBUG
#undef DEBUG
#ifdef DEBUG
extern "C" int srvlookup(char *name);
static int res_search_debug(const char* name, int queryClass, int queryType,
                            u_char* answer, int anslen);

#define RES_SEARCH(a, b, c, d, e)   res_search_debug(a, b, c, d, e)
#else
#define RES_SEARCH res_search
#endif

/*
 * Use a mutex lock to protect MT-unsafe res_search().
 * Since all occurences of res_search are called from within
 * servers() the lock is set and unset in that function.
 */
OsMutex SipSrvLookup::sLock(OsMutex::Q_PRIORITY |
                            OsMutex::DELETE_SAFE |
                            OsMutex::INVERSION_SAFE);

/*
 * Insert host if address if not a duplicate.  Update preference if lower
 * one found.
 */
server_t* SipSrvLookup::server_insert(server_t *server, 
                                      const char *host, 
                                      enum OsSocket::SocketProtocolTypes type,
                                      struct sockaddr_in sin, 
                                      int preference, 
                                      int weight, 
                                      int *entries)
{
  int i;

  for (i = 0; i < *entries; i++) {
    if (server[i].sin.sin_addr.s_addr == sin.sin_addr.s_addr
        && server[i].sin.sin_port == sin.sin_port
        && server[i].type == type) {
      if (preference < server[i].preference)
      {
        server[i].preference = preference;
        server[i].weight = weight;
      }
      break;
    }
  }
  /* no matching entry found, thus insert */
  if (i == *entries) {
    (*entries)++;
    server = (server_t *) (server ? realloc(server, sizeof(server_t) * (*entries)) :
                    malloc(sizeof(server_t)));
    server[i].host = host ? strdup(host) : NULL;
    server[i].sin  = sin;
    server[i].type = type;
    server[i].preference = preference;
    server[i].weight = weight;
  }
  return server;
} /* server_insert */


/*
 * Sort function:  by preference (lowest first) and then weight (highest
 * first).  For equal preference, UDP wins.
 */
int SipSrvLookup::server_sort(const server_t *s1, const server_t *s2)
{
    int compare = 0;

    if (s1->preference > s2->preference)
    {
        compare = 1;
    }
    else if (s1->preference < s2->preference)
    {
        compare = -1;
    }
    else if (s1->weight < s2->weight)
    {
        compare = 1;
    }
    else if (s1->weight > s2->weight)
    {
        compare = -1;
    }
    else if (s1->type == s2->type) 
    {
        compare = 0;
    }
    // TCP is the lowest priority
    else if(s1->type == OsSocket::TCP)
    {
        compare = 1;
    }
    else if(s2->type == OsSocket::TCP)
    {
        compare = -1;
    }
    // TLS is the highest priority
    else if(s1->type == OsSocket::SSL_SOCKET)
    {
        compare = -1;
    }
    else if(s2->type == OsSocket::SSL_SOCKET)
    {
        compare = 1;
    }

    return compare;
} /* server_sort */

/*
* Find the address of a host with domain name 'domain' and add it to the
* address list.
*/
server_t* SipSrvLookup::server_addr(server_t *server, 
  const char *domain, enum OsSocket::SocketProtocolTypes type, int port, 
  int preference, int weight, int *entries, int depth, UtlBoolean doCnameQuery)
{
  struct hostent *h;
  char msg[PACKETSZ];
#if defined(_VXWORKS)  /* needed for call to resolvGetHostByName --GAT */
  char buf[512];
#endif
  s_rr **rr;
  res_response *res;
  struct sockaddr_in sin;
  unsigned int i;
  UtlBoolean foundAddress = FALSE;

#ifdef DEBUG
  osPrintf("SipSrvLookup::server_addr() called with\n");
  osPrintf("  domain = \"%s\", type = %d, port = %d, preference = %d, weight = %d,\n",
           domain, type, port, preference, weight);
  osPrintf("  depth = %d, doCnameQuery = %s\n",
           depth, doCnameQuery ? "TRUE" : "FALSE");
#endif

  /* Allow nesting to 5 levels. */
  if (depth > 5) return server;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);

  /* Query for any CNAMEs and recurse.
   * Only do the CNAME query if doCnameQuery is TRUE.  This flag is used to avoid
   * CNAME lookups for SRV target hosts which, according to RFC 2782, MUST NOT
   * be aliases.
   */
  if (!foundAddress &&
      doCnameQuery &&
      (RES_SEARCH(domain, C_IN, T_CNAME, (unsigned char *)msg, sizeof (msg)) != -1) &&
      ((res = res_parse((char *)&msg)) != NULL)) {

    if (res->header.ancount > 0)
      foundAddress = TRUE;

    rr = res->answer;
    for (i = 0; i < res->header.ancount; i++) {
      if ((rr[i]->type == T_CNAME)
          && (rr[i]->rclass == C_IN))
        server = server_addr(server, rr[i]->rdata.string, type, port,
                             preference, weight, entries, depth + 1, doCnameQuery);
    }
    res_free(res);
  }

  /*
   * Try A RRs next if no CNAME.
   */
  if (!foundAddress &&
      (RES_SEARCH(domain, C_IN, T_A, (unsigned char *)msg, sizeof (msg)) != -1) &&
      ((res = res_parse((char *)&msg)) != NULL)) {

    if (res->header.ancount > 0)
      foundAddress = TRUE;

    rr = res->answer;
    for (i = 0; i < res->header.ancount; i++) {
      if ((rr[i]->type == T_A)
          && (rr[i]->rclass == C_IN)) 
      {
        sin.sin_addr = rr[i]->rdata.address;
        if(type == OsSocket::UNKNOWN)
        {
            // The type is unspecified and A records do not specify
            // the protocol, so add one for each
            server = server_insert(server, rr[i]->name, OsSocket::UDP,
                       sin, preference, weight, entries);
            server = server_insert(server, rr[i]->name, OsSocket::TCP,
                       sin, preference, weight, entries);
#ifdef TLS
            server = server_insert(server, rr[i]->name, OsSocket::SSL_SOCKET,
                       sin, preference, weight, entries);
#endif

        }
        else
        {
            server = server_insert(server, rr[i]->name, type,
                       sin, preference, weight, entries);
        }
      }
    }
    res_free(res);
  }

  /*
   * If that fails, try gethostbyname (this also uses local resources
   *   such as NIS or /etc/hosts).
   */
#ifdef DEBUG
  if (!foundAddress) {
    osPrintf("SipSrvLookup::server_addr() calling gethostbyname(\"%s\")\n",
             domain);
  }
#endif
  if (!foundAddress &&
#if defined(_WIN32) || defined(__pingtel_on_posix__)
      ((h = gethostbyname (domain)) != NULL) &&
#elif defined(_VXWORKS)
      ((h = resolvGetHostByName (domain, buf, sizeof(buf))) != NULL) &&
#else
#    error need function to resolve DNA a record look
#endif
      (h->h_addrtype == AF_INET)) {
    for (i = 0; h->h_addr_list[i] != NULL; i++) {
      sin.sin_addr = *(struct in_addr *)(h->h_addr_list[i]); 
      server = server_insert(server, h->h_name, type,
                 sin, preference, weight, entries);
    }
  }
  return server;
} /* server_addr */


/*
* Add to address list for 'domain'.  First, try for SRV RRs, then A RRs.
* Update 'entries' with number of entries.
*/
server_t* SipSrvLookup::server_hosts(const char *domain, 
                                     const char *service, 
                                     enum OsSocket::SocketProtocolTypes type, 
                                     int port,
                                     int *entries)
{
  char msg[PACKETSZ];
  //DWW was 1024 but this was being overwritten by long uri's
  //  char name[1024];
  char *name = NULL;

  s_rr **rr;
  res_response *res;
  unsigned int i;
  server_t *server = NULL;
  struct sockaddr_in sin;
  UtlBoolean foundAddress = FALSE;

  /*
   * Check if the domain is just a numeric IP address.
   */

  if ((sin.sin_addr.s_addr = inet_addr(domain)) != OS_INVALID_INET_ADDRESS) {
    sin.sin_family = AF_INET;
    sin.sin_port = htons(abs(port));
    if (type == OsSocket::UNKNOWN || type == OsSocket::UDP) {
      server = server_insert(server, domain, OsSocket::UDP, sin, 0, 0, entries);
    }
    if (type == OsSocket::UNKNOWN || type == OsSocket::TCP) {
      server = server_insert(server, domain, OsSocket::TCP, sin, 0, 0, entries);
    }
#ifdef TLS
        if (type == OsSocket::UNKNOWN || type == OsSocket::SSL_SOCKET) {
      server = server_insert(server, domain, OsSocket::SSL_SOCKET, sin, 0, 0, entries);
    }
#endif
    return server;
  }

  /*
   * Look for SRV RRs of type 'sip' in the domain, first UDP, then TCP.
   */

#ifdef DEBUG
  if (_res.options & RES_DEBUG) {
    osPrintf("SipSrvLookup::server_hosts() called with\n");
    osPrintf("  domain = \"%s\", type = %d, service = %s, port = %d\n",
             domain, type, service, port);
  }
#endif

  //+20 padding for format string in sprintf
  name = new char[strlen(service)+strlen(domain)+20]; 

  /* UDP */
  if ((type == OsSocket::UNKNOWN || type == OsSocket::UDP) && port <= 0) {
    res = NULL;

    /* New-style (RFC 2782) format */
    sprintf(name, "_%s._udp.%s", service, domain);
    if (RES_SEARCH(name, C_IN, T_SRV, (unsigned char *)msg, sizeof (msg)) != -1) {
      res = res_parse((char *)&msg);
    }

    if (res != NULL) {
      rr = res->answer;
      for (i = 0; i < res->header.ancount; i++) {
        if ((rr[i]->type == T_SRV) && (rr[i]->rclass == C_IN) &&
            (strcasecmp(name, rr[i]->name) == 0)) {
#ifdef DEBUG
          osPrintf("SipSrvLookup::server_hosts() - Received SRV record %d\n", i);
          osPrintf("  target=\"%s\", port=%d, pri=%d, weight=%d, type=DGRAM\n",
                   rr[i]->rdata.srv.target, rr[i]->rdata.srv.port,
                   rr[i]->rdata.srv.priority, rr[i]->rdata.srv.weight);
#endif
          foundAddress = TRUE;
          /* Check the "additional data" section for a corresponding A record */
          if (lookup_addr_in_addl_info(&sin, res, rr[i]->rdata.srv.target,
                                       rr[i]->rdata.srv.port)) {
            server = server_insert(server, rr[i]->rdata.srv.target, OsSocket::UDP,
                                   sin, rr[i]->rdata.srv.priority - 65536,
                                   (int)(rr[i]->rdata.srv.weight * rand()),
                                   entries);
          }
          else {
            server = server_addr(server, rr[i]->rdata.srv.target,
                                 OsSocket::UDP, rr[i]->rdata.srv.port,
                                 rr[i]->rdata.srv.priority - 65536, 
                                 (int)(rr[i]->rdata.srv.weight * rand()),
                                 entries,
                                 0, FALSE);
          }
        }
      }
      res_free(res);
    }
  }
  /* TCP */
  // don't care if UDP SRV records are found, if the type == unknown/any we need
  // to lookup TCP as well
  if (//!foundAddress &&
      ((type == OsSocket::UNKNOWN || type == OsSocket::TCP) && port <= 0)) {
    res = NULL;

    /* New-style (RFC 2782) format */
    sprintf(name, "_%s._tcp.%s", service, domain);
    if (RES_SEARCH(name, C_IN, T_SRV, (unsigned char *)msg, sizeof (msg)) != -1) {
      res = res_parse((char *)&msg);
    }

    if (res != NULL) {
      rr = res->answer;
      for (i = 0; i < res->header.ancount; i++) {
        if ((rr[i]->type == T_SRV) && (rr[i]->rclass == C_IN) &&
            (strcasecmp(name, rr[i]->name) == 0)) {
#ifdef DEBUG
          osPrintf("SipSrvLookup::server_hosts() - Received SRV record %d\n", i);
          osPrintf("  target=\"%s\", port=%d, pri=%d, weight=%d, type=STREAM\n",
                   rr[i]->rdata.srv.target, rr[i]->rdata.srv.port,
                   rr[i]->rdata.srv.priority, rr[i]->rdata.srv.weight);
#endif
          foundAddress = TRUE;
          /* Check the "additional data" section for a corresponding A record */
          if (lookup_addr_in_addl_info(&sin, res, rr[i]->rdata.srv.target,
                                       rr[i]->rdata.srv.port)) {
            server = server_insert(server, rr[i]->rdata.srv.target, OsSocket::TCP,
                                   sin, rr[i]->rdata.srv.priority - 65536,
                                   (int)(rr[i]->rdata.srv.weight * rand()),
                                   entries);
          }
          else {
            server = server_addr(server, rr[i]->rdata.srv.target,
                                 OsSocket::TCP, rr[i]->rdata.srv.port,
                                 rr[i]->rdata.srv.priority - 65536, 
                                 (int)(rr[i]->rdata.srv.weight * rand()),
                                 entries,
                                 0, FALSE);
          }
        }
      }
      res_free(res);
    }
  }

#ifdef TLS
  /* TLS */
  // don't care if UDP or TCP SRV records are found, if the type == unknown/any we need
  // to lookup TLS as well
    if (((type == OsSocket::UNKNOWN || type == OsSocket::SSL_SOCKET) && port <= 0)) 
    {
        res = NULL;

        /* New-style (RFC 2782) format */
        sprintf(name, "_%s._tls.%s", service, domain);
        if (RES_SEARCH(name, C_IN, T_SRV, (unsigned char *)msg, sizeof (msg)) != -1) {
          res = res_parse((char *)&msg);
        }

        if (res != NULL) {
          rr = res->answer;
          for (i = 0; i < res->header.ancount; i++) {
            if ((rr[i]->type == T_SRV) && (rr[i]->rclass == C_IN) &&
                (strcasecmp(name, rr[i]->name) == 0)) {
#ifdef DEBUG
              osPrintf("SipSrvLookup::server_hosts() - Received SRV record %d\n", i);
              osPrintf("  target=\"%s\", port=%d, pri=%d, weight=%d, type=TLS\n",
                       rr[i]->rdata.srv.target, rr[i]->rdata.srv.port,
                       rr[i]->rdata.srv.priority, rr[i]->rdata.srv.weight);
#endif
              foundAddress = TRUE;
              /* Check the "additional data" section for a corresponding A record */
              if (lookup_addr_in_addl_info(&sin, res, rr[i]->rdata.srv.target,
                                           rr[i]->rdata.srv.port)) {
                server = server_insert(server, rr[i]->rdata.srv.target, OsSocket::SSL_SOCKET,
                                       sin, rr[i]->rdata.srv.priority - 65536,
                                       (int)(rr[i]->rdata.srv.weight * rand()),
                                       entries);
              }
              else {
                server = server_addr(server, rr[i]->rdata.srv.target,
                                     OsSocket::SSL_SOCKET, rr[i]->rdata.srv.port,
                                     rr[i]->rdata.srv.priority - 65536, 
                                     (int)(rr[i]->rdata.srv.weight * rand()),
                                     entries,
                                     0, FALSE);
              }
            }
          }
          res_free(res);
        }
    }

#endif

  /*
   * Finally, we take any host with that name, but give it the lowest
   * preference. We use UDP first, then TCP.
   */
  if (!foundAddress) 
  {
    int savEntries;

    savEntries = *entries;
    server = server_addr(server, domain, type,  
                         abs(port), 65536, 0, entries, 0, TRUE);

    if (*entries > savEntries)
      foundAddress = TRUE;
  }

  //if (!foundAddress &&
  //    type == OsSocket::TCP) {
  //  server = server_addr(server, domain, OsSocket::TCP,
  //                       abs(port), 65536, 0, entries, 0, TRUE);
  //}
  
  if (name)
      delete [] name;

  return server;
} /* server_hosts */


/*
* External interface; return list of server entries for 'domain', with
* last entry having a host value of NULL.  The service is a protocol
* such as 'sip' or 'rtsp'.  The 'port' argument is used when the DNS
* entry doesn't contain one.  A negative number indicates the default
* port.  (This is indicated explicitly since SRV records are skipped
* unless the default port is used.) The 'type' restricts whether UDP or
* TCP addresses are sought.  If 0 both are used.
*/

server_t* SipSrvLookup::servers(const char *domain, 
                                const char *service, 
                                enum OsSocket::SocketProtocolTypes socketType, 
                                int port)
{
  enum OsSocket::SocketProtocolTypes type = socketType;

  struct sockaddr_in sin;
  server_t *server = NULL;
  int entries = 0;

#ifdef _VXWORKS
  // $$$ (rschaaf)
  // ToDo: We should extend the OS abstraction layer with a method to
  //       determine whether the network interface is available.
  // Don't attempt to query the DNS server if we know that the network
  // is unavailable.
  if (enetIsLinkActive())
  {
#endif
  sLock.acquire();   /* start of critical section */

  memset(&sin, '\0', sizeof(sin));

  server = server_hosts(domain, service, type, port, &entries);
  qsort(server, entries, sizeof(server_t),
        (int (*)(const void*, const void*)) server_sort);

  /* insert terminator */
  server = server_insert(server, NULL, OsSocket::UNKNOWN, sin, 0, 0, &entries);  

#ifdef DEBUG
  osPrintf("SipSrvLookup::servers(domain=\"%s\", service=%s, type=%d, port=%d) returned %d entries\n",
           domain, service, type, port, entries);
#endif
  
  sLock.release();   /* end of critical section */
#ifdef _VXWORKS
  }
#endif

  return server;
} /* servers */

/*
 * Search the additional info returned with the response
 * for the specified address (A) record.
 * Returns 1 if the A record is found; otherwise returns 0
 */
UtlBoolean SipSrvLookup::lookup_addr_in_addl_info(struct sockaddr_in *sin,
  void *opaque_response, char *name, int port)
{
  
  res_response *response = (res_response *) opaque_response;

  unsigned int i;
  s_rr **rr;

#ifdef DEBUG
  osPrintf("SipSrvLookup::lookup_addr_in_addl_info: DNS Response:\n");
  osPrintf("  qdcount=%d, ancount=%d, nscount=%d, arcount=%d\n",
           response->header.qdcount, response->header.ancount,
           response->header.nscount, response->header.arcount);
#endif

  if (response && response->header.arcount > 0) {
    rr = response->additional;
#ifdef DEBUG
    osPrintf("SipSrvLookup::lookup_addr_in_addl_info\n");
    osPrintf("   name=%s, arcount=%d\n", name, response->header.arcount);
#endif

    for (i = 0; i < response->header.arcount; i++) {

#ifdef DEBUG
      osPrintf("   %d: name=\"%s\", type=%d (%s), class=%d (%s)\n",
               i, rr[i]->name,
               rr[i]->type,   rr[i]->type == T_A ? "T_A" : "",
               rr[i]->rclass, rr[i]->rclass == C_IN ? "C_IN" : "");
#endif

      if ((rr[i]->type == T_A) &&
          (rr[i]->rclass == C_IN) &&
          (strcasecmp(name, rr[i]->name) == 0)) {
        sin->sin_addr = rr[i]->rdata.address;
        sin->sin_family = AF_INET;
        sin->sin_port = htons(abs(port));
#ifdef DEBUG
        osPrintf("SipSrvLookup::lookup_addr_in_addl_info(): Found match\n");
#endif
        return 1;
      }
    }
  }

  /* if we made it to here, we didn't find the address record */
  return 0;
}

//! Inquire if this is a valid SRV record
UtlBoolean SipSrvLookup::isValidServerT(server_t& srvRecord)
{
    return(srvRecord.host != NULL);
}

//! Accessor for host name
void SipSrvLookup::getHostNameFromServerT(server_t& srvRecord,
                                  UtlString& hostName)
{
    hostName = srvRecord.host ? srvRecord.host : "";
}

//! Accessor for host IP address
 void SipSrvLookup::getIpAddressFromServerT(server_t& srvRecord,
                                  UtlString& hostName)
{
    OsSocket::inet_ntoa_pt(srvRecord.sin.sin_addr, hostName);
}

//! Accessor for port
int SipSrvLookup::getPortFromServerT(server_t& srvRecord)
{
    return(ntohs(srvRecord.sin.sin_port));
}

//! Accessor for weight
int SipSrvLookup::getWeightFromServerT(server_t& srvRecord)
{
    return(srvRecord.weight);
}

//! Accessor for preference
int SipSrvLookup::getPreferenceFromServerT(server_t& srvRecord)
{
    return(srvRecord.preference);
}

//! Accessor for protocol
enum OsSocket::SocketProtocolTypes SipSrvLookup::getProtocolFromServerT(server_t& srvRecord)
{
    return(srvRecord.type);
}

//! Initializer for server_t
void SipSrvLookup::initServerT(server_t& srvRecord)
{
    srvRecord.host = NULL;
}

// destructor for server_t
void SipSrvLookup::freeServerT(server_t* srvRecordArray)
{
    // remove the memory for each of the entries
    for (int i=0; srvRecordArray[i].host != NULL; i++)
    {
        // remove the host string first
        free(srvRecordArray[i].host);
    }
    free(srvRecordArray);
}

#ifdef DEBUG
static int res_search_debug(const char* name, int queryClass,
                                   int queryType, u_char* answer,
                                   int anslen)
{
  int   result;
  const char* strClass;
  const char* strType;

  result = RES_SEARCH(name, queryClass, queryType, answer, anslen);

  switch (queryClass) {
  case C_IN: strClass = "C_IN";    break;
  default:   strClass = "Unknown"; break;
  }

  switch (queryType) {
  case T_CNAME: strType = "T_CNAME"; break;
  case T_SRV:   strType = "T_SRV";   break;
  case T_A:     strType = "T_A";     break;
  default:      strType = "Unknown"; break;
  }

  osPrintf("res_search(name=\"%s\", class=%s, type=%s) returned %d\n",
           name, strClass, strType, result);

  return result;
}

int srvlookup(char *name)
{
  int i;
  server_t *server_list = NULL;

  server_list = SipSrvLookup::servers(name, 
                                      "sip", 
                                      OsSocket::UNKNOWN, 
                                      0); // port

  if (server_list) {
    for (i=0; server_list[i].host != NULL; i++)
    {
      osPrintf("host=\"%s\", port=%d, type=%d, preference=%d, weight=%d\n",
               server_list[i].host,
               htons(server_list[i].sin.sin_port),
               server_list[i].type,
               server_list[i].preference,
               server_list[i].weight);
      free(server_list[i].host);
    }
    free(server_list);
  }
  return 0;
}
#endif


//! Sets the DNS SRV times.  Defaults: timeout=5, retries=4
void SipSrvLookup::setDnsSrvTimeouts(int initialTimeoutInSecs, int retries)
{
    assert(initialTimeoutInSecs > 0) ;
    assert(retries > 0) ;

    if (initialTimeoutInSecs > 0)
    {
        _res.retrans = initialTimeoutInSecs;
    }

    if (retries > 0)
    {
	    _res.retry = retries;
    }
}
