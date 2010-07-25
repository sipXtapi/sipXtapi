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
#include "os/OsSocket.h"
#include "upnp/UPnpAgent.h"
#include "tapi/sipXtapiInternal.h"
#include "tapi/sipXtapiEvents.h"
#include "os/OsWriteLock.h"
#include "net/TapiMgr.h"
#include "utl/UtlCrc32.h"
#include "os/OsRegistry.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define  AVAILABILITY_KEY_SUFFIX "net_interface_available_"
// STATIC VARIABLE INITIALIZATIONS
UPnpAgent* UPnpAgent::mpInstance = NULL;
OsRWMutex* UPnpAgent::spMutex = new OsRWMutex(OsRWMutex::Q_PRIORITY);


#ifndef _WIN32
bool s_bEnabled = true ;
bool s_bAvailable = true ;
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
    OsWriteLock lock(*spMutex);
    delete mpInstance;
    mpInstance = NULL;
}

int UPnpAgent::bindToAvailablePort(const char* szClientAddress,
                                   const int internalPort,
                                   const int maxRetries)
{
    OsSysLog::add(FAC_NAT, PRI_DEBUG, "UPnpAgent::bindToAvailablePort begin - %s:%d (internal)\n", 
                  szClientAddress, internalPort);
    OsTime now;
    OsTime start;
    OsDateTime::getCurTime(start);

    int boundPort = -1;
    int previouslyUsedPort = -1;
    if (isEnabled() == false)
    {
        OsSysLog::add(FAC_NAT, PRI_INFO,
            "UPnpAgent::bindToAvailablePort - aborting because uPNP is disabled");
        return -1;
    }
    previouslyUsedPort = loadPortSetting(szClientAddress, internalPort);
    
    bool bInitialized = false;
    if (NULL == mpDiscovery)
    {
        bInitialized = initialize();
    }
    else
    {
       bInitialized = true;
    }

    if (bInitialized && mpControl && previouslyUsedPort != -1)
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
            OsSysLog::add(FAC_NAT, PRI_INFO,
                "UPnpAgent::bindToAvailablePort success - %s:%d (internal), external port = %d, total time(msecs) = %d\n", 
                        szClientAddress,
                        internalPort,
                        boundPort,
                        diff.cvtToMsecs());
            setLastStatus(szClientAddress, internalPort, boundPort);
            return boundPort;
        }
        else
        {
        
        }
    }

    int i = 0;
    while (bInitialized && mpControl && boundPort == -1 && i < maxRetries)
    {
        mpControl->addRandomPortMapping(boundPort,
            internalPort,
            szClientAddress,
            getTimeoutSeconds());
        i++;
    }
    OsDateTime::getCurTime(now);
    OsTime diff = now - start;
    if (boundPort != -1)
    {
        savePortSetting(szClientAddress, internalPort, boundPort);
        setLastStatus(szClientAddress, internalPort, boundPort);

        OsDateTime::getCurTime(now);
        OsTime diff = now - start;
        OsSysLog::add(FAC_NAT, PRI_INFO,
            "UPnpAgent::bindToAvailablePort success - %s:%d (internal), external port = %d, total time(msecs) = %d\n", 
                    szClientAddress,
                    internalPort,
                    boundPort,
                    diff.cvtToMsecs());
        return boundPort;
    }
    else
    {
        OsSysLog::add(FAC_NAT, PRI_INFO,
            "UPnpAgent::bindToAvailablePort failed - %s:%d (internal), external port = %d, total time(msecs) = %d\n", 
                    szClientAddress,
                    internalPort,
                    boundPort,
                    diff.cvtToMsecs());
        setLastStatus(szClientAddress, internalPort, boundPort);
        return boundPort;
    }
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

bool UPnpAgent::initialize()
{
    bool bSuccess = false;
    if (!mpDiscovery)
    {
        mpDiscovery = new UPnpDiscovery(getTimeoutSeconds() * 1000);
        *mpLocation = mpDiscovery->discoverWANIPConnectionLocation();
        if (mpLocation->length() > 0)
        {
            mpService = new UPnpService(Url(mpLocation->data(), true), UPnpService::WANIPConnection);
            
            if (mpService->initialize())
            {
                // create control url based on the WANIPConnection location
                Url controlUrl(mpService->getControlUrl(), true);

                // create a UPnpControl for the controlURL
                mpControl = new UPnpControl(controlUrl);
                bSuccess = true;
                OsSysLog::add(FAC_NAT, PRI_INFO,
                    "UPnpAgent::initialize - initialized uPNP service: %s\n", mpLocation->data());
            }
            else
            {
                delete mpService;
                mpService = NULL;
                delete mpDiscovery;
                mpDiscovery = NULL;
                OsSysLog::add(FAC_NAT, PRI_INFO,
                    "UPnpAgent::initialize - Failed to initialize - service could not initialize.\n");
                setAvailable(false);
            }
        }
        else
        {
            delete mpDiscovery;
            mpDiscovery = false;
            OsSysLog::add(FAC_NAT, PRI_INFO,
                "UPnpAgent::initialize - Failed to initialize - no WANIPConnection location found.\n");
            setAvailable(false);
        }
    }
    return bSuccess;
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

   OsRegistry reg;
   reg.readInteger(UPNP_REG_PATH, hostPort, externalPort);
   return externalPort;
}

void UPnpAgent::savePortSetting(const char* szClientAddress, const int internalPort, const int externalPort) const
{
   char szInternalPort[16];
   UtlString hostPort(szClientAddress);
   snprintf(szInternalPort, sizeof(szInternalPort), "%d", internalPort);
   hostPort += ":";
   hostPort += szInternalPort;
   
   OsRegistry reg;
   reg.writeInteger(UPNP_REG_PATH, hostPort, externalPort);
}

void UPnpAgent::setEnabled(const bool bEnabled)
{

   const char* strKey = "Enabled";
   
   OsRegistry reg;
   long dwValue = bEnabled ? 1 : 0;
   reg.writeInteger(UPNP_REG_PATH, strKey, dwValue);
}

bool UPnpAgent::isEnabled()
{
   bool bEnabled = false;
   
   OsRegistry reg;
   int value = 0;
   if (reg.readInteger(UPNP_REG_PATH, "Enabled", value))
   {
      bEnabled = (value > 0);
   }
   return bEnabled;
}

void UPnpAgent::setAvailable(const bool bAvailable)
{
   // the registry entry will be keyed by a digest of a string
   // representing the current adapter info (ip, apdapter name, gateway ip, dns list -
   //                                             for all of the adapters)
   UtlString keyDigest = UtlString(AVAILABILITY_KEY_SUFFIX) + getAdapterStateDigest();
      
   OsRegistry reg;
   reg.writeInteger(UPNP_REG_PATH, keyDigest, (int) bAvailable);
}

bool UPnpAgent::isAvailable()
{
   bool bAvailable = true;  // if we don't know, assume it is available -
                            // so that we will try to bind a port
   
   UtlString keyDigest =  UtlString(AVAILABILITY_KEY_SUFFIX) + getAdapterStateDigest();
   int value = 0;
   OsRegistry reg;
   if (reg.readInteger(UPNP_REG_PATH, keyDigest, value))
   {
      bAvailable = (value > 0);
   }
   else
   {
      // do nothing
      // bAvailable is true
   }
      
   return bAvailable;
}


void UPnpAgent::setTimeoutSeconds(const int timeoutSeconds)
{
   const char* strKey = "Timeout";
   
   OsRegistry reg;
   reg.writeInteger(UPNP_REG_PATH, strKey, timeoutSeconds);
}


int UPnpAgent::getTimeoutSeconds()
{
   int timeoutSeconds = 10;

   const char* strKey = "Timeout";
   OsRegistry reg;
   reg.readInteger(UPNP_REG_PATH, strKey, timeoutSeconds);
   
   return timeoutSeconds;
}

void UPnpAgent::setRetries(const int numRetries)
{
   const char *strKey = "Retries";

   OsRegistry reg;
   reg.writeInteger(UPNP_REG_PATH, strKey, numRetries);
}

int UPnpAgent::getRetries() 
{
   int retries = 2;
   const char* strKey = "Retries";
   
   OsRegistry reg;
   reg.readInteger(UPNP_REG_PATH, strKey, retries);
   
      return retries;
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
        UPnpAgent::getInstance()->setAvailable(false);
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

const UtlString UPnpAgent::getAdapterStateDigest() const
{
   char strKeyPart[4096];
   UtlString sKey;
   
   int numAdapters = 0;
   AdapterInfoRec* pRec = ::getAdaptersInfo(numAdapters, true);
   for (int index = 0; index < numAdapters; index++)
   {
       snprintf(strKeyPart,
           sizeof(strKeyPart),
           "%s,%s,%s,%s+",
           pRec[index].IpAddress,
           pRec[index].AdapterName,
           pRec[index].GatewayList,
           pRec[index].DnsList);
            
       sKey += strKeyPart;
   }
   
   UtlCrc32 crc;
   crc.calc(sKey);
   
   char szDigest[256];
   snprintf(szDigest, sizeof(szDigest), "%d", crc.getValue());
   return UtlString(szDigest);
}
//////////////////////////////////////////////////////////////////////////////

UPnpBindingTask::UPnpBindingTask(UtlString sBoundIp,
                   const int port,
                   IUPnpNotifier* const pNotifier) :
    m_sBoundIp(sBoundIp),
    m_iPort(port),
    m_pNotifier(pNotifier),
    OsTask("UPnpBinding %d")
{
}

UPnpBindingTask::~UPnpBindingTask()
{
}

int UPnpBindingTask::run(void* pArg)
{
    int ret = 0;
    OsSysLog::add(FAC_NAT, PRI_DEBUG, "UPnpBindingTask::run Attempting bind.\n");
    if (UPnpAgent::getInstance()->bindToAvailablePort(m_sBoundIp,
            m_iPort,
            UPnpAgent::getInstance()->getTimeoutSeconds()) == -1)
    {
        // mark as unavailable
        UPnpAgent::getInstance()->setAvailable(false);
        assert (m_pNotifier);
        if (m_pNotifier)
        {
            requestShutdown();
            ackShutdown();
            m_pNotifier->notifyUpnpStatus(false) ;
        }
    }
    else
    {
        UPnpAgent::getInstance()->setAvailable(true);
        assert (m_pNotifier);
        if (m_pNotifier)
        {
            requestShutdown();
            ackShutdown();
            m_pNotifier->notifyUpnpStatus(true) ;
        }
    }
    OsSysLog::add(FAC_NAT, PRI_DEBUG, "UPnpBindingTask::run Exiting.\n");
    return ret;
}


