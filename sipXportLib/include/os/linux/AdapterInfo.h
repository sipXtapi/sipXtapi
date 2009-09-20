//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef getDNSServers_h_
#define getDNSServers_h_

#ifdef __cplusplus
#include <utl/UtlString.h>

#define MAX_ADAPTERS 20  //if you have more than 20 adapters, something is wrong with you. :)

#ifndef MAX_ADAPTER_NAME_LENGTH
   #define MAX_ADAPTER_NAME_LENGTH 256
#endif

#ifndef MAX_GATEWAYS_PER_ADAPTER
   #define MAX_GATEWAYS_PER_ADAPTER 16
#endif

#ifndef MAX_DNS_SERVERS_PER_ADAPTER
   #define MAX_DNS_SERVERS_PER_ADAPTER 16
#endif

typedef struct _AdapterInfo
{
        char AdapterName[MAX_ADAPTER_NAME_LENGTH + 4];  //long adapter name
        char IpAddress[40];                             //ip address of adapter
        char GatewayList[40*MAX_GATEWAYS_PER_ADAPTER];
        char DnsList[40*MAX_DNS_SERVERS_PER_ADAPTER];
        unsigned char MacAddress[MAX_ADAPTER_NAME_LENGTH];       //mac address of adapter
} AdapterInfoRec, *pAdapterInfoRec;

  /// Return this host's IP addresses.
extern "C" 
bool getAllLocalHostIps(const class HostAdapterAddress* localHostAddresses[],
                        int &numAddresses);
  /**<
  *  Does not include the "loopback" address.
  */

  /// Return a generated adapter name associated with the IP address.
extern "C"
bool getContactAdapterName(UtlString &adapterName, const UtlString &ipAddress,
                           bool unusedHere);

extern "C" AdapterInfoRec* getAdaptersInfo(int& numAdapters, bool bForceLookup);

#endif

#endif // getDNSServers_h_
