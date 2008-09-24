//  
// Copyright (C) 2006-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifdef WIN32

// SYSTEM INCLUDES
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winreg.h>
#include <stdio.h>
#include <assert.h>

// APPLICATION INCLUDES
#include <os/wnt/getWindowsDNSServers.h>
#include <os/HostAdapterAddress.h>
#include <os/OsSysLog.h>

// DEFINES
#define MAXNUM_DNS_ENTRIES 40

//used by getWindowsVersion
#define WINDOWS_VERSION_ERROR 0
#define WINDOWS_VERSION_98    1
#define WINDOWS_VERSION_NT4   2
#define WINDOWS_VERSION_2000  3

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// GLOBALS
static DWORD (WINAPI *GetNetworkParams)(PFIXED_INFO, PULONG);


static DWORD (WINAPI *GetAdaptersInfo)(
  PIP_ADAPTER_INFO pAdapterInfo,
  PULONG pOutBufLen
);

static DWORD (WINAPI *GetPerAdapterInfo)(
  ULONG IfIndex,
  PIP_PER_ADAPTER_INFO pPerAdapterInfo,
  PULONG pOutBufLen
);

static DWORD (WINAPI *GetAdapterIndex)(
  LPWSTR AdapterName,
  PULONG IfIndex
);

static DWORD (WINAPI *GetInterfaceInfo)(
  PIP_INTERFACE_INFO pIfTable,
  PULONG dwOutBufLen
);

static HMODULE hIpHelperModule = NULL;

//retrieves the current windows version and returns
//one of the WINDOWS_VERSION definitions.
static int getWindowsVersion()
{
    OSVERSIONINFO osInfo;
   int retVal = WINDOWS_VERSION_ERROR;

    //try to figure out what version of windows we are running
    osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

   if (GetVersionEx(&osInfo))
    {
    
       //check if it's the right version of windows
       if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
       {
           if (osInfo.dwMinorVersion > 0) //true if it is 98
            retVal = WINDOWS_VERSION_98;
       }
       else
       if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
       {
           if (osInfo.dwMajorVersion == 4)
           {
            retVal = WINDOWS_VERSION_NT4;
           }
           else
           if (osInfo.dwMajorVersion > 4)
           {
            retVal = WINDOWS_VERSION_2000;
           }
       }
   }

   return retVal;
}


//loads the iphlpapi.dll and sets any func pointers we may need
static HMODULE loadIPHelperAPI()
{
    char caFullDLLPath[_MAX_PATH];

    if (hIpHelperModule == NULL)
    {
        //first try loading it using the systems path
        hIpHelperModule = LoadLibrary("iphlpapi.dll");
        
		if (!hIpHelperModule)
		{
			//if that fails, (it shouldn't), try using the GetSystemPath func 
			GetSystemDirectory(caFullDLLPath,sizeof(caFullDLLPath));
			strcat(caFullDLLPath,"\\iphlpapi.dll");

			//try again
			hIpHelperModule = LoadLibrary(caFullDLLPath);
		}
        //ok, I give up...where the heck did they put the iphlpapi.dll???????
        if (!hIpHelperModule)
        {
            OsSysLog::add(FAC_KERNEL, PRI_ERR, "Cannot find iphlpapi.dll!\n");
        }
        else
        {
            //now find IPHelper functions
            int windowsVersion = getWindowsVersion();
            if (windowsVersion >= WINDOWS_VERSION_2000)
            {
                *(FARPROC*)&GetPerAdapterInfo = GetProcAddress(hIpHelperModule,"GetPerAdapterInfo");
                if (GetPerAdapterInfo == NULL)
                {
                    OsSysLog::add(FAC_KERNEL, PRI_ERR, "Could not get the proc address to GetPerAdapterInfo!\n");
                    FreeLibrary(hIpHelperModule);
                    hIpHelperModule = NULL;
                }   
            } 
            else
            {
                *(FARPROC*)&GetNetworkParams = GetProcAddress(hIpHelperModule,"GetNetworkParams");
                if (GetNetworkParams == NULL)
                {
                    OsSysLog::add(FAC_KERNEL, PRI_ERR, "Could not get the proc address to GetNetworkParams!\n");
                    FreeLibrary(hIpHelperModule);
                    hIpHelperModule = NULL;
                }   
            }

            *(FARPROC*)&GetInterfaceInfo = GetProcAddress(hIpHelperModule,"GetInterfaceInfo");
            if (GetInterfaceInfo == NULL)
            {
                OsSysLog::add(FAC_KERNEL, PRI_ERR, "Could not get the proc address to GetInterfaceInfo!\n");
                FreeLibrary(hIpHelperModule);
                hIpHelperModule = NULL;
            }   

            *(FARPROC*)&GetAdapterIndex = GetProcAddress(hIpHelperModule,"GetAdapterIndex");
            if (GetAdapterIndex == NULL)
            {
                OsSysLog::add(FAC_KERNEL, PRI_ERR, "Could not get the proc address to GetAdapterIndex!\n");
                FreeLibrary(hIpHelperModule);
                hIpHelperModule = NULL;
            }   

            *(FARPROC*)&GetAdaptersInfo = GetProcAddress(hIpHelperModule,"GetAdaptersInfo");
            if (GetAdaptersInfo == NULL)
            {
                OsSysLog::add(FAC_KERNEL, PRI_ERR, "Could not get the proc address to GetAdaptersInfo!\n");
                FreeLibrary(hIpHelperModule);
                hIpHelperModule = NULL;
            }   
        }
    }

    return hIpHelperModule;
}


static int getIPHelperDNSEntries(char DNSServers[][MAXIPLEN], int max, const char* szLocalIp)
{
    int ipHelperDNSServerCount = 0;

    int windowsVersion = getWindowsVersion();
    if (windowsVersion != WINDOWS_VERSION_98   && 
        windowsVersion < WINDOWS_VERSION_2000)
    {
        // iphlpapi.dll not supported
        return ipHelperDNSServerCount;
    }

    HMODULE hModule = loadIPHelperAPI();
    if (!hModule)
    {
        // unabled to load iphlpapi.dll
        return ipHelperDNSServerCount;
    }

    if (windowsVersion >= WINDOWS_VERSION_2000 && GetAdaptersInfo && GetPerAdapterInfo)
    {
        // Get list of adapters and find the index of the one associated with szLocalIp
        long index = -1;
        unsigned long outBufLen = 0;
        DWORD dwResult = GetAdaptersInfo(NULL, &outBufLen);
        if (outBufLen)
        {
            PIP_ADAPTER_INFO pIpAdapterInfo = (PIP_ADAPTER_INFO)malloc(outBufLen);
            dwResult = GetAdaptersInfo(pIpAdapterInfo, &outBufLen);
                            
            if (ERROR_SUCCESS == dwResult)
            {
                PIP_ADAPTER_INFO pNextInfoRecord = pIpAdapterInfo;
                while (pNextInfoRecord && index < 0)
                {
                    PIP_ADDR_STRING pIpAddrString = &(pNextInfoRecord->IpAddressList);
                    while (pIpAddrString)
                    {
                        if (strcmp (szLocalIp,  pIpAddrString->IpAddress.String) == 0)
                        {
                            index = static_cast<long>(pNextInfoRecord->Index);
                            break;
                        }
                        pIpAddrString = pIpAddrString->Next;
                    }
                    pNextInfoRecord = pNextInfoRecord->Next;
                }
            }
            free(pIpAdapterInfo);
        }

        if (index >= 0)
        {
            // now that we have the index, we
            // can call GetPerAdapterInfo
            outBufLen = 0;
            GetPerAdapterInfo(index, NULL, &outBufLen);
            if (outBufLen)
            {
                IP_PER_ADAPTER_INFO* pPerAdapterInfo = (IP_PER_ADAPTER_INFO*) malloc(outBufLen);
                dwResult = GetPerAdapterInfo(index, pPerAdapterInfo, &outBufLen);  
                if (ERROR_SUCCESS == dwResult)
                {
                    IP_ADDR_STRING* pDns = &pPerAdapterInfo->DnsServerList;
                    while (pDns && ipHelperDNSServerCount < max)
                    {
                        strcpy(DNSServers[ipHelperDNSServerCount++], pDns->IpAddress.String);
                        pDns = pDns->Next;
                    }
                }              
                free(pPerAdapterInfo);
            }
        }
    }
    else if (GetNetworkParams)
    {
        // use GetNetworkParams 

        //force size to 0 so the GetNetworkParams gets the correct size
        DWORD dwNetworkInfoSize = 0;
        DWORD retErr = GetNetworkParams( NULL, &dwNetworkInfoSize );
        if( retErr == ERROR_BUFFER_OVERFLOW )
        {
            // Allocate memory from sizing information
            PFIXED_INFO pNetworkInfo;
            if( ( pNetworkInfo = (PFIXED_INFO)GlobalAlloc( GPTR, dwNetworkInfoSize ) ) != NULL )
            {
                // Get actual network params
                if( ( retErr = GetNetworkParams( pNetworkInfo, &dwNetworkInfoSize ) ) == 0 )
                {
                    //point to the server list 
                    PIP_ADDR_STRING pAddrStr = &(pNetworkInfo->DnsServerList);

                    // first, add the 'current dns'
                    if (pNetworkInfo && pNetworkInfo->CurrentDnsServer)
                    {
                        strcpy(DNSServers[ipHelperDNSServerCount++], pNetworkInfo->CurrentDnsServer->IpAddress.String);
                    }
                    //walk the list of IP addresses
                    while( pAddrStr && ipHelperDNSServerCount < max )
                    {
                        //copy one of the ip addresses
                        strcpy(DNSServers[ipHelperDNSServerCount++],pAddrStr->IpAddress.String);
                        pAddrStr = pAddrStr->Next;
                    }
                    //free the memory
                    GlobalFree(pNetworkInfo);   // handle to global memory object
                }
                else
                {
                    OsSysLog::add(FAC_KERNEL, PRI_ERR,  "DNS ERROR: GetNetworkParams failed with error %d\n", retErr );
                    GlobalFree(pNetworkInfo);   // handle to global memory object
                }
            }
        }
        else
        {
            OsSysLog::add(FAC_KERNEL, PRI_ERR,  "DNS ERROR: Memory allocation error\n" );
        }
    }

    return ipHelperDNSServerCount;
}


static int getDNSEntriesFromRegistry(char regDNSServers[][MAXIPLEN], int max)
{
   int retRegDNSServerCount = 0;
    const char *strParametersKey    = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters";
    const char *strDhcpNameServerValue       = "DhcpNameServer";
    const char *strNameServerValue           = "NameServer";
    char *token;  //used for parsing the ip addresses
    HKEY hKey;
    BYTE    data[255];
    DWORD    cbData;
    DWORD    dataType;
    char *ptr = NULL;   //pointer to ip addresses when parsing
   DWORD err;
   
   err = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,         // handle to open key
              strParametersKey,  // subkey name
              0,   // reserved
              KEY_READ, // security access mask
              &hKey    // handle to open key
              );

   if (err == ERROR_SUCCESS)
   {
      cbData = sizeof(data);
      
      err = RegQueryValueEx(
                  hKey,                      // handle to key
                  strDhcpNameServerValue,    // value name
                  0,                         // reserved
                  &dataType,                 // type buffer
                  data,                      // data buffer
                  &cbData);                  // size of data buffer

      if (err != ERROR_SUCCESS)
       {
           //try a different value
           err = RegQueryValueEx(
                     hKey,                // handle to key
                     strNameServerValue,  // value name
                     0,                   // reserved
                     &dataType,           // type buffer
                     data,                // data buffer
                     &cbData);            // size of data buffer

       }

       if (err == ERROR_SUCCESS)
       {
          //we need to break it up on NT.  It puts all the IP's on one line.
          //it may not be a space, which I set as default, ...lets check for a ','
          if (strstr((char *)data,","))
             token = ",";
          else
             token = " ";

          //find the first token
          ptr = strtok((char *)data,token);
          while (ptr != NULL && retRegDNSServerCount < max)
          {
             strncpy(regDNSServers[retRegDNSServerCount++],ptr,MAXIPLEN);
                       
             //search for the next one
             ptr = strtok(NULL,token);
          }
      }
      else
      {
         OsSysLog::add(FAC_KERNEL, PRI_ERR, "Error reading values from registry in func: getDNSEntriesFromRegistry\n"); 
      }

      RegCloseKey(hKey);
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "Error opening registry in func: getDNSEntriesFromRegistry\n");
   }   
   
   return retRegDNSServerCount;
}

extern "C" int getWindowsDNSServers(char DNSServers[][MAXIPLEN], int max, const char* szLocalIp)
{
    int     finalDNSServerCount = 0; //number of dns entries returned to user
    int     ipHelperDNSServerCount = 0; //number of dns entries found through ipHelperAPI
    int     regDNSServerCount = 0; //num entries found in registry
   char regDNSServers[MAXNUM_DNS_ENTRIES][MAXIPLEN]; //used to store registry DNS entries 
   int i;
//   int j;  //general purpose looping variables
   int swapPos = 0; //location to move the DNS entries that match the registry

   //retrieve the DNS entries from a MS provided DLL
   //This func will also load the dll if on win98 or NT 2000
   ipHelperDNSServerCount = getIPHelperDNSEntries(DNSServers,max,szLocalIp);
   finalDNSServerCount = ipHelperDNSServerCount;
   
   //We always search the registry now... 
   //Before we only did it for NT, but because 
   //we want the most recently used entries at the top, we will consult 
   //the registry and use the list retrieved and sort it.
    regDNSServerCount = getDNSEntriesFromRegistry(regDNSServers,max);
   
    // Because we now support multiple interfaces, we cannot use the following code:
    // (registry could contain DNS servers that are not associated with our
    //  current interface)
/*
   //now walk through the entries found through the registry
   //and make sure the registry entries are at the top
   //NOTE: We do this because windows seem to be hanging on to old entries.
   //      This is causing really bad timeouts when doing a DNS search.
   //      If you use the registry, we should be getting the newest entries
   //      and moving those to the top of the DNS list
   if (ipHelperDNSServerCount && regDNSServerCount)
   {
      for (i = 0; i < ipHelperDNSServerCount;i++)
      {
         for (j = 0; j < regDNSServerCount;j++)
         {
            if (strcmp(DNSServers[i],regDNSServers[j]) == 0 && i != j)
            {
               char tmpdns[MAXIPLEN];
               //save off the original
               strcpy(tmpdns,DNSServers[swapPos]);
               //copy the zero index entry to the that location
               strcpy(DNSServers[swapPos],DNSServers[i]);
               //copy the saved to the old location
               strcpy(DNSServers[i],tmpdns);
               swapPos++;
            }
         }
      }
   }
*/
   
   //if we only found reg entries and no ipHelper entries , then we need to return those 
   //to the user (The ones from the registry)
   if (regDNSServerCount && !ipHelperDNSServerCount)
   {

      finalDNSServerCount = regDNSServerCount;
      //copy to final list
      for (i = 0; i < finalDNSServerCount; i++)
         strcpy(DNSServers[i],regDNSServers[i]);
   }

    //return the number of DNS entries found
    return finalDNSServerCount;
}


bool getContactAdapterName(UtlString &adapterName, const UtlString &ipAddress, bool trueName)
{
    bool rc = false;

    if (ipAddress == "127.0.0.1")
    {
        rc = true;
        adapterName = "loopback";
        return rc;
    }
    else if (loadIPHelperAPI())
    {
        
        unsigned long outBufLen = 0;
        DWORD dwResult = GetAdaptersInfo(NULL, &outBufLen);
        PIP_ADAPTER_INFO pIpAdapterInfo = (PIP_ADAPTER_INFO)malloc(outBufLen);
        dwResult = GetAdaptersInfo(pIpAdapterInfo, &outBufLen);
                                
        if (ERROR_SUCCESS == dwResult)
        {
            rc = true;
            PIP_ADAPTER_INFO pNextInfoRecord = pIpAdapterInfo;
            unsigned int adapterId = 0;
            char szAdapterId[MAX_ADAPTER_NAME_LENGTH + 4];
            bool bFound = false;
            while (pNextInfoRecord && !bFound)
            {
                sprintf(szAdapterId, "eth%u", adapterId);
                PIP_ADDR_STRING pNextAddress = &(pNextInfoRecord->IpAddressList);
                while (pNextAddress)
                {
                    const char *szAddr = pNextAddress->IpAddress.String;
                    // if the target matches this address or if the target is any
                    if (ipAddress == szAddr || ipAddress == "0.0.0.0")
                    {
                        if (trueName)
                        {
                            adapterName = pNextInfoRecord->AdapterName;
                        }
                        else
                        {
                            adapterName = szAdapterId;
                        }
                        bFound = true;
                        break;
                    }                                            
                    pNextAddress = pNextAddress->Next;
                }
                adapterId++;
                pNextInfoRecord = pNextInfoRecord->Next;
            }
        }
        free((void*)pIpAdapterInfo);
    }                
    
    return rc;
}


bool getAllLocalHostIps(const HostAdapterAddress* localHostAddresses[], int &numAddresses)
{
    bool rc = false;

    if (loadIPHelperAPI())
    {
        PIP_ADAPTER_INFO pIpAdapterInfo = (PIP_ADAPTER_INFO)malloc(sizeof(IP_ADAPTER_INFO) * MAX_IP_ADDRESSES);
        unsigned long outBufLen = sizeof(IP_ADAPTER_INFO) * MAX_IP_ADDRESSES;

        DWORD dwResult = GetAdaptersInfo(pIpAdapterInfo, &outBufLen);
                                
        if (ERROR_SUCCESS == dwResult)
        {
            int maxAddresses = numAddresses;
            rc = true;
            numAddresses = 0;

            PIP_ADAPTER_INFO pNextInfoRecord = pIpAdapterInfo;

            unsigned int adapterId = 0;
            char szAdapterId[MAX_IP_ADDRESSES];
            while (pNextInfoRecord && (numAddresses<maxAddresses))
            {
                sprintf(szAdapterId, "eth%u", adapterId);
                PIP_ADDR_STRING pNextAddress = &pNextInfoRecord->IpAddressList;
                while (pNextAddress && (numAddresses<maxAddresses))
                {
                    const char *szAddr = pNextAddress->IpAddress.String;
                    // ignore the loopback address
                    if (  strncmp(szAddr, "127.0.0.1", 9) != 0
                       && strncmp(szAddr, "169.154", 7) != 0
                       && strncmp(szAddr, "0.", 2) != 0
                       )
                    {
                        localHostAddresses[numAddresses] = new HostAdapterAddress(szAdapterId, szAddr);
                        numAddresses++;
                    }
                    pNextAddress = pNextAddress->Next;
                }
                adapterId++;
                pNextInfoRecord = pNextInfoRecord->Next;
            }
        }
        free((void*)pIpAdapterInfo);
    }                

    return rc;
}

#endif


