//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _WINDOWSADAPTERINFO_H
#define _WINDOWSADAPTERINFO_H

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
        BYTE MacAddress[MAX_ADAPTER_NAME_LENGTH];       //mac address of adapter
} AdapterInfoRec, *pAdapterInfoRec;

#ifdef WIN32


//FUNCTIONS

        //pass in amac address and get out the current ip address assigned to it
        extern int lookupIpAddressByMacAddress(char *mac_address, char *ipaddress);

        // pass in ipaddress and get the adapter info record
        extern AdapterInfoRec* lookupAdapterInfoRecord(const char *ipaddress);
        
        //initializes internal adapter structure
        extern AdapterInfoRec* getAdaptersInfo(int& numAdapters, bool bForceLookup);

        extern int getWindowsDomainName(char *domain_name);


#endif  //WIN32

#endif //_WINDOWSADAPTERINFO_H
