// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
// USA. 
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

// SYSTEM INCLUDES
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#ifdef __MACH__
# include <ifaddrs.h>
#endif
// APPLICATION INCLUDES
#include <os/HostAdapterAddress.h>
#include <os/OsSysLog.h>

static int AdapterCount = 0;
AdapterInfoRec adapters[MAX_ADAPTERS+1];                          //used to store all the adapters it finds

/**
*  Get the addresses associated with all of the IP interfaces.
*
*  The core work is done by the SIOCGIFCONF ioctl, documented in the
*  netdevice(7) manual page, with additional hints in ioctl(2), ip(7) and inet(3).
*/
bool getAllLocalHostIps(const HostAdapterAddress* localHostAddresses[],
                        int &numAddresses)
{
   UtlBoolean rc;
   numAddresses = 0;
#ifdef __MACH__
   struct ifaddrs* ifap ;
   if (getifaddrs(&ifap) == 0)
   {
      struct ifaddrs* pTraverse = ifap ;
      while (pTraverse)
      {
         if ((pTraverse->ifa_flags & IFF_UP) && !(pTraverse->ifa_flags & IFF_LOOPBACK))
         {
            sockaddr_in* pAddr = (sockaddr_in*) pTraverse->ifa_addr ;
            if (pAddr->sin_family == AF_INET)
            {
			   char* s = inet_ntoa(pAddr->sin_addr);
               UtlString address(s);
               if (address.compareTo("127.0.0.1") != 0 && address.compareTo("0.0.0.0") != 0)
               {
                  // Put the interface name and address into a HostAdapterAddress.
                  localHostAddresses[numAddresses++] = new HostAdapterAddress(pTraverse->ifa_name, s);			
               }
            }
         }
		pTraverse = pTraverse->ifa_next ;
      }
      freeifaddrs(ifap);
	  
	  if (numAddresses > 0)
	     rc = true ;
   }
#else
   // Allocate array of struct ifreq's.
   struct ifreq ifreq_array[MAX_IP_ADDRESSES];
   // Allocate struct ifconf.
   struct ifconf ifconf_structure;
   // Point ifconf to ifreq's.
   ifconf_structure.ifc_len = sizeof (ifreq_array);
   ifconf_structure.ifc_req = ifreq_array;

   // Open an arbitrary network socket on which to perform the ioctl.
   int sock = socket(PF_INET, SOCK_DGRAM, 0);
   if (sock < 0)
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR,
                    "getAllLocalHostIps unable to open socket, errno = %d",
                    errno);
      rc = FALSE;
   }
   else
   {
      // Perform the SIOCGIFCONF ioctl to get the interface addresses.
      int ret = ioctl(sock, SIOCGIFCONF, (void*) &ifconf_structure);
      if (ret < 0)
      {
         OsSysLog::add(FAC_KERNEL, PRI_ERR,
                       "getAllLocalHostIps error performing SIOCGIFCONF, errno = %d",
                       errno);
         rc = FALSE;
      }
      else
      {
         rc = TRUE;
         // Get the number of returned addresses from ifc_len.
         numAddresses = ifconf_structure.ifc_len / sizeof (struct ifreq);
         int j = 0;
         // Iterate through the returned addresses.
         for (int i = 0; i < numAddresses; i++)
         {
            
            if (ifreq_array[i].ifr_addr.sa_family != AF_INET)
                continue;

            // Get transient pointer to address in text format.
            char* s = inet_ntoa(((struct sockaddr_in&) (ifreq_array[i].ifr_addr)).sin_addr);

            // Ignore the loopback address, because opening ports on the
            // loopback interface interferes with STUN operation.
            UtlString address(s);
            if (address.compareTo("127.0.0.1") != 0 && address.compareTo("0.0.0.0") != 0)
            {
               // Put the interface name and address into a HostAdapterAddress.
               localHostAddresses[j] = new HostAdapterAddress(ifreq_array[i].ifr_name, s);
/*
               OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                             "getAllLocalHostIps entry %d, interface '%s', address '%s'",
                             j, ifreq_array[i].ifr_name, s);
*/
               j++;
            }
         }
         numAddresses = j;
      }
      close(sock);
   }
#endif
   return rc;
}

bool getContactAdapterName(UtlString &adapterName, const UtlString &ipAddress, bool unusedHere)
{
   bool found = false;

   // Explicitly check for loopback adapter, because getAllLocalHostIps never
   // name it.
   if (ipAddress == "127.0.0.1")
   {
      found = true;
      // TODO:: It is not always true. You may set any name to loopback
      //        adapter, afaik. So we may want to do real search here.
      adapterName = "lo";
      return found;
   }
   else
   {
      int numAddresses = 0;
      const HostAdapterAddress* adapterAddresses[MAX_IP_ADDRESSES];
      getAllLocalHostIps(adapterAddresses, numAddresses);

      // Return empty string if nothing will be found
      adapterName.remove(0);

      for (int i = 0; i < numAddresses; i++)
      {
         if (ipAddress.compareTo(adapterAddresses[i]->mAddress.data()) == 0)
         {
            adapterName = adapterAddresses[i]->mAdapter;
/*
            OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                        "getContactAdapterName found name %s for ip %s",
                        szAdapter, szIp);
*/
            found = true;
         }
         delete adapterAddresses[i];
      }
   }
   
   return found;
}


///////////////////////////////////////////
//
// getAdaptersInfo
//
//
// numAdapters - is set to the number of adapters found
//               or is set to 0 for a failure
// 
// bForceLookup - if false, will used cached adapters info
//
// returns adapterInfoArray
//
//////////////////////////////////////////
AdapterInfoRec* getAdaptersInfo(int& numAdapters,
                                 bool bForceLookup)
{
        char MacAddressStr[256]; //mac address converted to a string
        char MacOneByteStr[10]; //used to hold one byte of mac address

	//just return count if we already did this before
	if (!bForceLookup && AdapterCount)
	{
		numAdapters = AdapterCount;
		return adapters;
	}

	numAdapters = 0;
	AdapterCount = 0;
	// clear the adapters struct
	memset(adapters, 0, sizeof(adapters));

/*	
	IP_ADAPTER_INFO  *pAdapterInfo; //points to buffer hold linked list adapter info

	DWORD dwSize = (sizeof(IP_ADAPTER_INFO) * MAX_ADAPTERS) + sizeof(DWORD); //size for lots of adapters
	char *buffer = new char[dwSize];  //allocate space for lots of adapters
	if (buffer)
	{
		pAdapterInfo = (IP_ADAPTER_INFO *)buffer;  //point to buffer
		if (GetAdaptersInfo(
			pAdapterInfo,  // buffer for mapping table
			&dwSize) == NO_ERROR)                     // sort the table
		{
			while (pAdapterInfo)
			{
				strcpy(adapters[AdapterCount].AdapterName, pAdapterInfo->Description);
				strcpy(adapters[AdapterCount].IpAddress, (const char *)pAdapterInfo->IpAddressList.IpAddress.String);
				// get a list of gateways (comma separated) for the adapter
				_IP_ADDR_STRING* pGateway = &pAdapterInfo->GatewayList;
				memset(adapters[AdapterCount].GatewayList, 0, sizeof(adapters[AdapterCount].GatewayList));
				int gatewayCount = 0;
				while (pGateway)
				{
					if (gatewayCount)
					{
					strncat(adapters[AdapterCount].GatewayList,
						",",
						sizeof(adapters[AdapterCount].GatewayList));
					}
					gatewayCount++;
					strncat(adapters[AdapterCount].GatewayList,
					(const char *)pGateway->IpAddress.String,
						sizeof(adapters[AdapterCount].GatewayList));
					pGateway = pAdapterInfo->GatewayList.Next;
				}

				// get a current Dns Server
				// now that we have the index, we
				// can call GetPerAdapterInfo
				unsigned long outBufLen = 0;
				GetPerAdapterInfo(pAdapterInfo->Index, NULL, &outBufLen);
				if (outBufLen)
				{
					IP_PER_ADAPTER_INFO* pPerAdapterInfo = (IP_PER_ADAPTER_INFO*) malloc(outBufLen);
					DWORD dwResult = GetPerAdapterInfo(pAdapterInfo->Index, pPerAdapterInfo, &outBufLen);  
					if (ERROR_SUCCESS == dwResult)
					{
					IP_ADDR_STRING* pDns = &pPerAdapterInfo->DnsServerList;
					int dnsCount = 0;
					while (pDns)
					{
						if (dnsCount)
						{
						strncat(adapters[AdapterCount].DnsList,
							",",
							sizeof(adapters[AdapterCount].DnsList));
						}
						dnsCount++;
						strncat(adapters[AdapterCount].DnsList,
						pDns->IpAddress.String, 
						sizeof(adapters[AdapterCount].DnsList));
						pDns = pDns->Next;
					}
					}              
					free(pPerAdapterInfo);
				}
				
				//build mac address as a string
				*MacAddressStr = '\0';
				for (unsigned int loop = 0; loop < pAdapterInfo->AddressLength; loop++)
				{
					if (strlen(MacAddressStr))
						strcat(MacAddressStr,"-");
					sprintf(MacOneByteStr,"%02X",pAdapterInfo->Address[loop]);
					strcat(MacAddressStr,MacOneByteStr);
				}
				strcpy((char *)adapters[AdapterCount].MacAddress, MacAddressStr);
				AdapterCount++;
				pAdapterInfo = pAdapterInfo->Next;
			}
			numAdapters = AdapterCount;
		}
		delete [] buffer;
	}
*/

        return adapters;
}
