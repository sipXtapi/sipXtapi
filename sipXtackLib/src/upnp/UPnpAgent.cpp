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
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
// Author: Mike Cohen
// 

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "upnp/UPnpAgent.h"
#include "tapi/sipXtapiInternal.h"
#include "tapi/sipXtapiEvents.h"
#include "os/OsWriteLock.h"
#include "net/TapiMgr.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UPnpAgent* UPnpAgent::mpInstance = NULL;
OsRWMutex* UPnpAgent::spMutex = new OsRWMutex(OsRWMutex::Q_PRIORITY);
#ifndef _WIN32
bool s_bEnabled = true ;
int s_iTimeoutSecs = 10 ;
#endif
// MACROS

//////////////////////////////////////////////////////////////////////////////
UPnpAgent::UPnpAgent() :
    mpDiscovery(NULL),
    mpService(NULL),
    mpLocation(new UtlString()),
    mpControl(NULL),
    mLastResult(false),
    mLastInternalAddress(""),
    mLastInternalPort(-1),
    mLastExternalPort(-1)
{
}

UPnpAgent::~UPnpAgent()
{
    delete mpDiscovery;
    delete mpService;
    delete mpLocation;
    delete mpControl;
}

UPnpAgent* UPnpAgent::getInstance()
{
    OsWriteLock lock(*spMutex);
    if (NULL == mpInstance)
    {
        mpInstance = new UPnpAgent();
    }
    return mpInstance;
}

void UPnpAgent::release()
{
    mpInstance->setEnabled(true);
    OsWriteLock lock(*spMutex);
    delete mpInstance;
    mpInstance = NULL;
}

int UPnpAgent::loadPortSetting(const char* szClientIp, 
                               const int   internalPort) const
{
   int externalPort = -1;
   char szInternalPort[16];
   snprintf(szInternalPort, sizeof(szInternalPort), "%d", internalPort);
   UtlString hostPort(szClientIp);
   hostPort += ":";
   hostPort += szInternalPort;
   // first, check static map
   // TODO - check a hashmap for stored port values before
   //        performing a costly registry read
  
#ifdef _WIN32 
   HKEY hKey;
   DWORD    cbData;
   DWORD    dataType;
   DWORD    dwValue;

   
   DWORD err = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,   // handle to open key
              WIN32_UPNP_REG_PATH,  // subkey name
              0,                    // reserved
              KEY_READ,             // security access mask
              &hKey                 // handle to open key
              );

   if (err == ERROR_SUCCESS)
   {
      cbData = sizeof(DWORD);
      dataType = REG_DWORD;
      
      err = RegQueryValueEx(
                  hKey,                      // handle to key
                  hostPort.data(),           // value name
                  0,                         // reserved
                  &dataType,                 // type buffer
                  (LPBYTE)&dwValue,          // data buffer
                  &cbData);                  // size of data buffer

      if (err == ERROR_SUCCESS)
      {
          externalPort = dwValue;
      }

      RegCloseKey(hKey);
   }
#endif
   if (-1 != externalPort)
   {
      // TODO - add it to a hashmap of stored port values
   }
   return externalPort;
}

void UPnpAgent::savePortSetting(const char* szClientAddress, const int internalPort, const int externalPort) const
{
#ifdef _WIN32
   HKEY hKey;
   char szInternalPort[16];
   UtlString hostPort(szClientAddress);
   snprintf(szInternalPort, sizeof(szInternalPort), "%d", internalPort);
   hostPort += ":";
   hostPort += szInternalPort;

   DWORD err = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,   // handle to open key
              WIN32_UPNP_REG_PATH,  // subkey name
              0,                    // reserved
              KEY_WRITE,            // security access mask
              &hKey                 // handle to open key
              );

    DWORD dwDisposition = NULL;
    if (err != ERROR_SUCCESS)
    {
        err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
            WIN32_UPNP_REG_PATH, 
            0,
            NULL, 
            REG_OPTION_NON_VOLATILE, 
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &dwDisposition);
    }
    DWORD dwValue = externalPort;
    if (err == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey,
            hostPort.data(),
            0,
            REG_DWORD,
            (const BYTE*)&dwValue,
            sizeof(&dwValue));
    }
   RegCloseKey(hKey);
#endif
}

int UPnpAgent::bindToAvailablePort(const char* szClientAddress,
                                   const int internalPort,
                                   const int maxRetries)
{
    OsSysLog::add(FAC_SIP, PRI_INFO, "UPnpAgent::bindToAvailablePort begin - %s:%d (internal)\n", 
                  szClientAddress, internalPort);
    OsTime now;
    OsTime start;
    OsDateTime::getCurTime(start);

    int boundPort = -1;
    int previouslyUsedPort = -1;
    if (isEnabled() == false)
    {
        return -1;
    }
    previouslyUsedPort = loadPortSetting(szClientAddress, internalPort);
    if (NULL == mpDiscovery)
    {
        initialize();
    }

    if (mpControl && previouslyUsedPort != -1)
    {
        mpControl->deletePortMapping(previouslyUsedPort, 
            getTimeoutSeconds());
        bool ret = mpControl->addPortMapping(previouslyUsedPort,
            internalPort,
            szClientAddress,
            getTimeoutSeconds());
        if (ret == true)
        {
            boundPort = previouslyUsedPort;

            OsDateTime::getCurTime(now);
            OsTime diff = now - start;
            OsSysLog::add(FAC_SIP, PRI_INFO,
                "UPnpAgent::bindToAvailablePort end - %s:%d (internal), external port = %d, total time(msecs) = %d\n", 
                        szClientAddress,
                        internalPort,
                        boundPort,
                        diff.cvtToMsecs());
            setLastStatus(szClientAddress, internalPort, boundPort);
            return boundPort;
        }
    }

    int i = 0;
    while (mpControl && boundPort == -1 && i < maxRetries)
    {
        mpControl->addRandomPortMapping(boundPort,
            internalPort,
            szClientAddress,
            getTimeoutSeconds());
        i++;
    }
    if (boundPort != -1)
    {
        savePortSetting(szClientAddress, internalPort, boundPort);
    }
    else
    {
    }

    setLastStatus(szClientAddress, internalPort, boundPort);

    OsDateTime::getCurTime(now);
    OsTime diff = now - start;
    OsSysLog::add(FAC_SIP, PRI_INFO,
        "UPnpAgent::bindToAvailablePort end - %s:%d (internal), external port = %d, total time(msecs) = %d\n", 
                szClientAddress,
                internalPort,
                boundPort,
                diff.cvtToMsecs());
    return boundPort;
}

void UPnpAgent::removeBinding(const char* szClientIp,
    const int internalPort)
{
    if (NULL == mpDiscovery)
    {
        initialize();
    }
    int externalPort = -1;
    externalPort = loadPortSetting(szClientIp, internalPort);
    if (mpControl && externalPort > -1)
    {
        mpControl->deletePortMapping(externalPort,
            getTimeoutSeconds());
    }
}

void UPnpAgent::initialize()
{
    if (!mpDiscovery)
    {
        mpDiscovery = new UPnpDiscovery(getTimeoutSeconds() * 1000);
        *mpLocation = mpDiscovery->discoverWANIPConnectionLocation();
        if (mpLocation->length() > 0)
        {
            mpService = new UPnpService(Url(mpLocation->data(), true), UPnpService::WANIPConnection);

            // create control url based on the WANIPConnection location
            Url controlUrl(mpService->getControlUrl(), true);

            // create a UPnpControl for the controlURL
            mpControl = new UPnpControl(controlUrl);
        }
        else
        {
            setEnabled(false);
        }
    }
}

void UPnpAgent::setEnabled(const bool enabled)
{
#ifdef _WIN32
   HKEY hKey;
   const char *strKey = "Enabled";

   DWORD err = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,   // handle to open key
              WIN32_UPNP_REG_PATH,  // subkey name
              0,                    // reserved
              KEY_WRITE,            // security access mask
              &hKey                 // handle to open key
              );

    DWORD dwDisposition = NULL;
    if (err != ERROR_SUCCESS)
    {
        err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
            WIN32_UPNP_REG_PATH, 
            0,
            NULL, 
            REG_OPTION_NON_VOLATILE, 
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &dwDisposition);
    }
    DWORD dwValue = enabled ? 1 : 0;
    if (err == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey,
            strKey,
            0,
            REG_DWORD,
            (const BYTE*)&dwValue,
            sizeof(&dwValue));
    }
   RegCloseKey(hKey);
#else
    s_bEnabled = enabled ;
#endif
}

bool UPnpAgent::isEnabled()
{
   bool enabled = true;
#ifdef _WIN32
   const char* strKey = "Enabled";
   HKEY hKey;
   DWORD    cbData;
   DWORD    dataType;
   DWORD    dwValue;
   
   DWORD err = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,   // handle to open key
              WIN32_UPNP_REG_PATH,  // subkey name
              0,                    // reserved
              KEY_READ,             // security access mask
              &hKey                 // handle to open key
              );

   if (err == ERROR_SUCCESS)
   {
      cbData = sizeof(DWORD);
      dataType = REG_DWORD;
      
      err = RegQueryValueEx(
                  hKey,                      // handle to key
                  strKey,                    // value name
                  0,                         // reserved
                  &dataType,                 // type buffer
                  (LPBYTE)&dwValue,          // data buffer
                  &cbData);                  // size of data buffer

      if (err == ERROR_SUCCESS)
      {
          if (dwValue == 1)
            enabled = true;
          else
            enabled = false;
      }

      RegCloseKey(hKey);
   }
#else
    enabled = s_bEnabled ;
#endif
   return enabled;
}


void UPnpAgent::setTimeoutSeconds(const int timeoutSeconds)
{
#ifdef WIN32
   HKEY hKey;
   const char *strKey = "Timeout";

   DWORD err = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,   // handle to open key
              WIN32_UPNP_REG_PATH,  // subkey name
              0,                    // reserved
              KEY_WRITE,            // security access mask
              &hKey                 // handle to open key
              );

    DWORD dwDisposition = NULL;
    if (err != ERROR_SUCCESS)
    {
        err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
            WIN32_UPNP_REG_PATH, 
            0,
            NULL, 
            REG_OPTION_NON_VOLATILE, 
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &dwDisposition);
    }
    DWORD dwValue = timeoutSeconds;
    if (err == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey,
            strKey,
            0,
            REG_DWORD,
            (const BYTE*)&dwValue,
            sizeof(&dwValue));
    }
   RegCloseKey(hKey);
#else
    s_iTimeoutSecs = timeoutSeconds ;
#endif
}


int UPnpAgent::getTimeoutSeconds()
{
   int timeoutSeconds = 10 ;

#ifdef _WIN32
   const char* strKey = "Timeout";
   HKEY hKey;
   DWORD    cbData;
   DWORD    dataType;
   DWORD    dwValue;

   
   DWORD err = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,   // handle to open key
              WIN32_UPNP_REG_PATH,  // subkey name
              0,                    // reserved
              KEY_READ,             // security access mask
              &hKey                 // handle to open key
              );

   if (err == ERROR_SUCCESS)
   {
      cbData = sizeof(DWORD);
      dataType = REG_DWORD;
      
      err = RegQueryValueEx(
                  hKey,                      // handle to key
                  strKey,                    // value name
                  0,                         // reserved
                  &dataType,                 // type buffer
                  (LPBYTE)&dwValue,          // data buffer
                  &cbData);                  // size of data buffer

      if (err == ERROR_SUCCESS)
      {
          timeoutSeconds = dwValue;
      }

      RegCloseKey(hKey);
   }
#else
    timeoutSeconds = s_iTimeoutSecs ;
#endif
   return timeoutSeconds;
}


void UPnpAgent::setLastStatus(const char* szInternalAddress,
                              const int nInternalPort,
                              const int nExternalPort) const
{
    char szBindingDescription[256];
    memset(szBindingDescription, 0, sizeof(szBindingDescription));
    snprintf(szBindingDescription, sizeof(szBindingDescription), "%s:%d",
        szInternalAddress,
        nInternalPort);
    if (nExternalPort == -1)
    {
        UPnpAgent::getInstance()->setEnabled(false);
        mLastResult = false;
    }
    else
    {
        char szExternalPort[16];
        snprintf(szExternalPort, sizeof(szExternalPort), ",%d",
            nExternalPort);
        strncat(szBindingDescription,  szExternalPort, sizeof(szBindingDescription));
        mLastResult = true;
    }
    mLastExternalPort = nExternalPort;
    mLastInternalPort = nInternalPort;
    mLastInternalAddress = szInternalAddress;

}


SIPX_RESULT UPnpAgent::getLastResults(char* szInternalAddress,
                                      const size_t internalAddressSize,
                                      int& internalPort,
                                      int& externalPort)
{
    SIPX_RESULT res = mLastResult ? SIPX_RESULT_SUCCESS : SIPX_RESULT_FAILURE;
    strncpy(szInternalAddress, mLastInternalAddress.data(), internalAddressSize);
    internalPort = mLastInternalPort;
    externalPort = mLastExternalPort;
    return res;
}
//////////////////////////////////////////////////////////////////////////////

