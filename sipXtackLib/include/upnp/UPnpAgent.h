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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
// 
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
// Author: Mike Cohen
// 
#ifndef _UPnpAgent_h_
#define _UPnpAgent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "upnp/UPnpDiscovery.h"
#include "upnp/UPnpService.h"
#include "upnp/UPnpControl.h"
#include "os/OsRWMutex.h"
#include "os/OsTask.h"
#include "utl/UtlString.h"
#include "tapi/sipXtapi.h"

// DEFINES
#define UPNP_REG_PATH "Software\\sipXtapi\\uPnp"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UPnpBindingTask;

//////////////////////////////////////////////////////////////////////////////
/**
 * Interface class defining a UPnp status callback method.
 */
class IUPnpNotifier
{
public: 
    virtual void notifyUpnpStatus(bool bSuccess) = 0;
};

/**
 * The UPnpBindingTask performs a port binding via Udp, in the background.
 * A mehtod on the IUPnpNotifier object is invoked indicate success / failure of the binding.  
 */
class UPnpBindingTask : public OsTask
{
public:
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
   UPnpBindingTask(UtlString sBoundIp,
                   const int port,
                   IUPnpNotifier* const pNotifier);
    virtual ~UPnpBindingTask();
/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    virtual int run(void* pArg);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlString m_sBoundIp;
    const int m_iPort;
    IUPnpNotifier* m_pNotifier;
};


//////////////////////////////////////////////////////////////////////////////
class UPnpAgent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
    static UPnpAgent* getInstance();
    static void release();

    virtual ~UPnpAgent();

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
    int bindToAvailablePort(const char* szClientAddress,
        const int internalPort,
        const int maxRetries = 10);
    void removeBinding(const char* szClientIp,
        const int internalPort);
    void setEnabled(const bool enabled);
    bool isEnabled();
    void setAvailable(const bool available);
    bool isAvailable();
    
    void setTimeoutSeconds(const int timeoutSeconds);
    int getTimeoutSeconds();
    void setRetries(const int numRetries);
    int getRetries();
    SIPX_RESULT getLastResults(char* szInternalAddress,
                               const size_t internalAddressSize,
                               int& internalPort,
                               int& externalPort);

/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UPnpAgent();
    bool initialize();
    int loadPortSetting(const char* szClientAddress, const int internalPort) const;
    void savePortSetting(const char* szClientAddress, const int internalPort, const int externalPort) const;
    void setLastStatus(const char* szInternalAddress,
                       const int nInternalPort,
                       const int nExternalPort) const;
    const UtlString getAdapterStateDigest() const;                       
    static UPnpAgent* mpInstance;
    static OsRWMutex* spMutex;
    UPnpDiscovery* mpDiscovery;
    UPnpService* mpService;
    UtlString* mpLocation;
    UPnpControl* mpControl;
    mutable bool mLastResult;
    mutable UtlString mLastInternalAddress;
    mutable int mLastInternalPort;
    mutable int mLastExternalPort;

};

#endif
