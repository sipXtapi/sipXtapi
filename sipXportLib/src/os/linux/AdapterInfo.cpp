//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// APPLICATION INCLUDES
#include <os/HostAdapterAddress.h>
#include <os/OsSysLog.h>

/* Get the addresses associated with all of the IP interfaces.
 * The core work is done by the SIOCGIFCONF ioctl, documented in the
 * netdevice(7) manual page, with additional hints in ioctl(2), ip(7) and inet(3).
 */
bool getAllLocalHostIps(const HostAdapterAddress* localHostAddresses[],
                        int &numAddresses)
{
   numAddresses = 0;
   UtlBoolean rc;

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
#ifdef __MACH__
      // Under MacOS, SIOCFIGCONF assumes a variable length structure,
      // using OSIOCGIFCONF appears to be the popular workaround.
      int ret = ioctl(sock, OSIOCGIFCONF, (void*) &ifconf_structure);
#else
      int ret = ioctl(sock, SIOCGIFCONF, (void*) &ifconf_structure);
#endif

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
   return rc;
}

bool getContactAdapterName(char* szAdapter, const char* szIp, bool unusedHere)
{
   bool found = false;
   
   UtlString ipAddress(szIp);
   
   int numAddresses = 0;
   const HostAdapterAddress* adapterAddresses[MAX_IP_ADDRESSES];
   getAllLocalHostIps(adapterAddresses, numAddresses);

   for (int i = 0; i < numAddresses; i++)
   {
      if (ipAddress.compareTo(adapterAddresses[i]->mAddress.data()) == 0)
      {
         strcpy(szAdapter, adapterAddresses[i]->mAdapter.data());
/*
         OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                       "getContactAdapterName found name %s for ip %s",
                       szAdapter, szIp);
*/
         found = true;
      }
      delete adapterAddresses[i];
   }
   
   if (found)
   {
      return true;
   }
   else
   {
      return false;
   }
}

